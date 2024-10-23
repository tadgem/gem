#include "renderer.h"
#include "asset_manager.h"
#include "lights.h"
#include "transform.h"
#include "tech/vxgi.h"
#include "tech/gbuffer.h"
#include "tech/shadow.h"
#include "tech/lighting.h"
#include "tech/tech_utils.h"
#include "tech/ssr.h"
#include "tech/taa.h"
#include "im3d_math.h"

void gl_renderer_builtin::init(asset_manager& am)
{
    m_frame_index = 0;
    m_im3d_state = im3d_gl::load_im3d();

	am.wait_all_assets();
	m_gbuffer_shader                    = am.get_asset<shader, asset_type::shader>("assets/shaders2/gbuffer.shader");
	m_lighting_shader                   = am.get_asset<shader, asset_type::shader>("assets/shaders2/lighting.shader");
	m_visualise_3d_tex_shader           = am.get_asset<shader, asset_type::shader>("assets/shaders2/visualize_3d_tex.shader");
	m_present_shader                    = am.get_asset<shader, asset_type::shader>("assets/shaders2/present.shader");
	m_dir_light_shadow_shader           = am.get_asset<shader, asset_type::shader>("assets/shaders2/dir_light_shadow.shader");
	m_voxel_cone_tracing_shader         = am.get_asset<shader, asset_type::shader>("assets/shaders2/voxel_cone_tracing.shader");
	m_ssr_shader                        = am.get_asset<shader, asset_type::shader>("assets/shaders2/ssr.shader");
	m_taa_shader                        = am.get_asset<shader, asset_type::shader>("assets/shaders2/taa.shader");
	m_denoise_shader                    = am.get_asset<shader, asset_type::shader>("assets/shaders2/denoise.shader");
	m_combine_shader                    = am.get_asset<shader, asset_type::shader>("assets/shaders2/gi_combine.shader");
	m_downsample_shader                 = am.get_asset<shader, asset_type::shader>("assets/shaders2/downsample.shader");
	m_compute_voxelize_gbuffer_shader   = am.get_asset<shader, asset_type::shader>("assets/shaders2/gbuffer_voxelization.shader");
	m_compute_voxel_mips_shader         = am.get_asset<shader, asset_type::shader>("assets/shaders2/voxel_mips.shader");

    m_window_resolution = { 1920.0, 1080.0 };
    const int shadow_resolution = 4096;
    const float gi_resolution_scale = 1.0;
    const float ssr_resolution_scale= 0.66;

    m_gbuffer = framebuffer::create(m_window_resolution, {
        {GL_RGBA, GL_RGBA16F, GL_LINEAR, GL_FLOAT},
        {GL_RGBA, GL_RGBA16F, GL_LINEAR, GL_FLOAT},
        {GL_RGBA, GL_RGBA16F, GL_LINEAR, GL_FLOAT},
        {GL_RGBA, GL_RGBA16F, GL_LINEAR, GL_FLOAT},
        {GL_RGBA, GL_RGBA16F, GL_LINEAR, GL_FLOAT},
        {GL_RGB, GL_RGB16F, GL_NEAREST, GL_FLOAT},
        }, true);

    m_gbuffer_downsample = framebuffer::create(m_window_resolution, {
        {GL_RGBA,GL_RGBA16F, GL_LINEAR, GL_FLOAT},
        }, false);

    m_dir_light_shadow_buffer = framebuffer::create({ shadow_resolution, shadow_resolution }, {}, true);

    m_lightpass_buffer = framebuffer::create(m_window_resolution, {
        {GL_RGBA,GL_RGBA16F, GL_LINEAR, GL_FLOAT},
        }, false);

    m_lightpass_buffer_resolve = framebuffer::create(m_window_resolution, {
        {GL_RGBA,GL_RGBA16F, GL_LINEAR, GL_FLOAT},
        }, false);

    m_lightpass_buffer_history = framebuffer::create(m_window_resolution, {
        {GL_RGBA,GL_RGBA16F, GL_LINEAR, GL_FLOAT},
        }, false);

    m_position_buffer_history = framebuffer::create(m_window_resolution, {
        {GL_RGBA,GL_RGBA16F, GL_LINEAR, GL_FLOAT},
        }, false);

    glm::vec2 gi_res = { m_window_resolution.x * gi_resolution_scale, m_window_resolution.y * gi_resolution_scale };
    m_conetracing_buffer = framebuffer::create(gi_res, {
        {GL_RGBA,GL_RGBA16F, GL_LINEAR, GL_FLOAT},
        }, false);

    m_conetracing_buffer_denoise = framebuffer::create(gi_res, {
        {GL_RGBA,GL_RGBA16F, GL_LINEAR, GL_FLOAT},
        }, false);

    m_conetracing_buffer_resolve = framebuffer::create(m_window_resolution, {
        {GL_RGBA,GL_RGBA16F, GL_LINEAR, GL_FLOAT},
        }, false);

    m_conetracing_buffer_history = framebuffer::create(m_window_resolution, {
        {GL_RGBA,GL_RGBA16F, GL_LINEAR, GL_FLOAT},
        }, false);

    glm::vec2 ssr_res = { m_window_resolution.x * ssr_resolution_scale, m_window_resolution.y * ssr_resolution_scale };
    m_ssr_buffer = framebuffer::create(ssr_res, {
    {GL_RGBA,GL_RGBA16F, GL_LINEAR, GL_FLOAT},
        }, false);

    m_ssr_buffer_denoise = framebuffer::create(ssr_res, {
        {GL_RGBA,GL_RGBA16F, GL_LINEAR, GL_FLOAT},
        }, false);

    m_ssr_buffer_resolve = framebuffer::create(m_window_resolution, {
        {GL_RGBA,GL_RGBA16F, GL_LINEAR, GL_FLOAT},
        }, false);

    m_ssr_buffer_history = framebuffer::create(m_window_resolution, {
    {GL_RGBA,GL_RGBA16F, GL_LINEAR, GL_FLOAT},
        }, false);

    m_final_pass = framebuffer::create(m_window_resolution, {
        {GL_RGBA, GL_RGBA8, GL_LINEAR, GL_FLOAT},
        }, false);

    m_voxel_data = voxel::create_grid(s_voxel_resolution, aabb{});
    m_voxel_visualiser = voxel::create_grid_visualiser(m_voxel_data, m_visualise_3d_tex_shader->m_data, 8);
}

void gl_renderer_builtin::pre_frame(camera& cam, scene& current_scene)
{
    im3d_gl::new_frame_im3d(m_im3d_state, m_window_resolution, cam);
}

void gl_renderer_builtin::render(camera& cam, scene& current_scene)
{
    tech::vxgi::dispatch_gbuffer_voxelization(
        m_compute_voxelize_gbuffer_shader->m_data, 
        current_scene.m_scene_bounding_volume, 
        m_voxel_data, 
        m_gbuffer, 
        m_lightpass_buffer_resolve, 
        m_window_resolution);

    tech::vxgi::dispatch_gen_voxel_mips(
        m_compute_voxel_mips_shader->m_data, 
        m_voxel_data, 
        s_voxel_resolution);

    tech::gbuffer::dispatch_gbuffer_with_id(
        m_frame_index, 
        m_gbuffer, 
        m_position_buffer_history, 
        m_gbuffer_shader->m_data, 
        cam, 
        current_scene, 
        m_window_resolution);

    m_frame_index++;

    // TODO: Need a way to get a single instance more efficiently
    dir_light dir{};
    auto dir_light_view = current_scene.m_registry.view<dir_light>();
    for (auto [e, dir_light_c] : dir_light_view.each())
    {
        dir = dir_light_c;
    }

    std::vector<point_light> point_lights{};
    tech::shadow::dispatch_shadow_pass(
        m_dir_light_shadow_buffer, 
        m_dir_light_shadow_shader->m_data, 
        dir, 
        current_scene, 
        m_window_resolution);

    tech::lighting::dispatch_light_pass(
        m_lighting_shader->m_data, 
        m_lightpass_buffer, m_gbuffer, 
        m_dir_light_shadow_buffer, 
        cam, 
        point_lights, 
        dir);

    m_gbuffer_downsample.bind();
    tech::utils::dispatch_present_image(
        m_downsample_shader->m_data, 
        "u_prev_mip", 
        0, 
        m_gbuffer.m_colour_attachments[2]);
    m_gbuffer_downsample.unbind();

    tech::taa::dispatch_taa_pass(
        m_taa_shader->m_data, 
        m_lightpass_buffer, 
        m_lightpass_buffer_resolve, 
        m_lightpass_buffer_history, 
        m_gbuffer.m_colour_attachments[4], 
        m_window_resolution);

    if (m_debug_draw_cone_tracing_pass || m_debug_draw_cone_tracing_pass_no_taa)
    {
        tech::vxgi::dispatch_cone_tracing_pass(
            m_voxel_cone_tracing_shader->m_data, 
            m_voxel_data, 
            m_conetracing_buffer, 
            m_gbuffer, 
            m_window_resolution, 
            current_scene.m_scene_bounding_volume, 
            s_voxel_resolution, 
            cam, 
            m_vxgi_cone_trace_distance, 
            m_vxgi_resolution_scale, 
            m_vxgi_diffuse_specular_mix);
    }

    if (m_debug_draw_lighting_pass)
    {
        tech::utils::dispatch_present_image(m_present_shader->m_data, 
            "u_image_sampler", 
            0, 
            m_lightpass_buffer_resolve.m_colour_attachments.front());
    }

    m_ssr_buffer_resolve.bind();
    glClear(GL_COLOR_BUFFER_BIT);
    m_ssr_buffer_resolve.unbind();

    if (m_debug_draw_ssr_pass)
    {
        glViewport(0, 0, m_window_resolution.x * m_ssr_resolution_scale, m_window_resolution.y * m_ssr_resolution_scale);
        tech::ssr::dispatch_ssr_pass(m_ssr_shader->m_data, cam, m_ssr_buffer, m_gbuffer, m_lightpass_buffer, m_window_resolution);
        glViewport(0, 0, m_window_resolution.x, m_window_resolution.y);
        tech::taa::dispatch_taa_pass(
            m_taa_shader->m_data, 
            m_conetracing_buffer, 
            m_conetracing_buffer_resolve, 
            m_conetracing_buffer_history, 
            m_gbuffer.m_colour_attachments[4], 
            m_window_resolution);
    }

    if (m_debug_draw_cone_tracing_pass)
    {
        tech::taa::dispatch_taa_pass(
            m_taa_shader->m_data, 
            m_conetracing_buffer, 
            m_conetracing_buffer_resolve, 
            m_conetracing_buffer_history, 
            m_gbuffer.m_colour_attachments[4], 
            m_window_resolution);

        glViewport(0, 0, m_window_resolution.x * m_vxgi_resolution_scale, m_window_resolution.y * m_vxgi_resolution_scale);
        
        tech::utils::dispatch_denoise_image(
            m_denoise_shader->m_data, 
            m_conetracing_buffer_resolve, 
            m_conetracing_buffer_denoise, 
            m_denoise_sigma, 
            m_denoise_threshold, 
            m_denoise_k_sigma, 
            m_window_resolution);
        texture::bind_sampler_handle(0, GL_TEXTURE0);
        glViewport(0, 0, m_window_resolution.x, m_window_resolution.y);
    }
    if (m_debug_draw_cone_tracing_pass_no_taa)
    {
        tech::utils::dispatch_present_image(
            m_present_shader->m_data, 
            "u_image_sampler", 
            0, 
            m_conetracing_buffer.m_colour_attachments.front());
    }
    if (m_debug_draw_lighting_pass_no_taa)
    {
        tech::utils::dispatch_present_image(
            m_present_shader->m_data, 
            "u_image_sampler", 
            0, 
            m_lightpass_buffer.m_colour_attachments.front());
    }

    if (m_debug_draw_ssr_pass)
    {
        tech::utils::dispatch_present_image(
            m_present_shader->m_data, 
            "u_image_sampler", 
            0, 
            m_ssr_buffer_resolve.m_colour_attachments.front());
    }

    tech::utils::blit_to_fb(
        m_lightpass_buffer_history, 
        m_present_shader->m_data, 
        "u_image_sampler", 
        0, 
        m_lightpass_buffer_resolve.m_colour_attachments[0]);

    tech::utils::blit_to_fb(
        m_position_buffer_history, 
        m_present_shader->m_data, 
        "u_image_sampler", 
        0, 
        m_gbuffer.m_colour_attachments[1]);

    tech::utils::blit_to_fb(
        m_conetracing_buffer_history, 
        m_present_shader->m_data, 
        "u_image_sampler", 
        0, 
        m_conetracing_buffer_denoise.m_colour_attachments.front());

    tech::utils::blit_to_fb(
        m_ssr_buffer_history, 
        m_present_shader->m_data, 
        "u_image_sampler", 
        0, 
        m_ssr_buffer_resolve.m_colour_attachments.front());

    glClear(GL_DEPTH_BUFFER_BIT);

    if (m_debug_draw_final_pass)
    {
        m_final_pass.bind();
        shapes::s_screen_quad.use();
        m_combine_shader->m_data.use();
        m_combine_shader->m_data.set_float("u_brightness", m_tonemapping_brightness);
        m_combine_shader->m_data.set_float("u_contrast", m_tonemapping_contrast);
        m_combine_shader->m_data.set_float("u_saturation", m_tonemapping_saturation);
        m_combine_shader->m_data.set_int("lighting_pass", 0);
        texture::bind_sampler_handle(m_lightpass_buffer_resolve.m_colour_attachments.front(), GL_TEXTURE0);
        m_combine_shader->m_data.set_int("cone_tracing_pass", 1);
        texture::bind_sampler_handle(m_conetracing_buffer_denoise.m_colour_attachments.front(), GL_TEXTURE1);
        m_combine_shader->m_data.set_int("ssr_pass", 2);
        m_combine_shader->m_data.set_int("ssr_pass", 2);
        texture::bind_sampler_handle(m_ssr_buffer.m_colour_attachments.front(), GL_TEXTURE2);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        texture::bind_sampler_handle(0, GL_TEXTURE0);
        texture::bind_sampler_handle(0, GL_TEXTURE1);
        m_final_pass.unbind();
        tech::utils::dispatch_present_image(m_present_shader->m_data, "u_image_sampler", 0, m_final_pass.m_colour_attachments.front());
    }

}

void gl_renderer_builtin::cleanup(asset_manager& am)
{
    m_gbuffer.cleanup();
    m_gbuffer_downsample.cleanup();
    m_dir_light_shadow_buffer.cleanup();
    m_lightpass_buffer.cleanup();
    m_lightpass_buffer_resolve.cleanup();
    m_lightpass_buffer_history.cleanup();
    m_position_buffer_history.cleanup();
    m_conetracing_buffer.cleanup();
    m_conetracing_buffer_denoise.cleanup();
    m_conetracing_buffer_resolve.cleanup();
    m_conetracing_buffer_history.cleanup();
    m_ssr_buffer.cleanup();
    m_ssr_buffer_denoise.cleanup();
    m_ssr_buffer_resolve.cleanup();
    m_ssr_buffer_history.cleanup();
    m_final_pass.cleanup();
    im3d_gl::shutdown_im3d(m_im3d_state);
}

entt::entity gl_renderer_builtin::get_mouse_entity(glm::vec2 mouse_position)
{
    auto pixels = m_gbuffer.read_pixels<glm::vec4, 1, 1>(
        mouse_position.x, 
        m_window_resolution.y - mouse_position.y, 
        5, 
        GL_RGBA, 
        GL_FLOAT);

    return  entt::entity(
        pixels[0][0] +
        pixels[0][1] * 256 +
        pixels[0][2] * 256 * 256
    );
}
