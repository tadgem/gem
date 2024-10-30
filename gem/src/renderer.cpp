#include "imgui.h"
#include "im3d/im3d_math.h"
#include "gem/renderer.h"
#include "gem/asset_manager.h"
#include "gem/lights.h"
#include "gem/transform.h"
#include "gem/tech/vxgi.h"
#include "gem/tech/gbuffer.h"
#include "gem/tech/shadow.h"
#include "gem/tech/lighting.h"
#include "gem/tech/tech_utils.h"
#include "gem/tech/ssr.h"
#include "gem/tech/taa.h"
#include "gem/backend.h"
#include "gem/input.h"
#include "gem/profile.h"

void gl_renderer::init(asset_manager& am)
{
    ZoneScoped;
    TracyGpuContext;
    m_frame_index = 0;
    m_im3d_state = im3d_gl::load_im3d();

    am.load_asset("assets/shaders2/gbuffer.shader", asset_type::shader);
    am.load_asset("assets/shaders2/lighting.shader", asset_type::shader);
    am.load_asset("assets/shaders2/visualize_3d_tex.shader", asset_type::shader);
    am.load_asset("assets/shaders2/present.shader", asset_type::shader);
    am.load_asset("assets/shaders2/dir_light_shadow.shader", asset_type::shader);
    am.load_asset("assets/shaders2/voxel_cone_tracing.shader", asset_type::shader);
    am.load_asset("assets/shaders2/ssr.shader", asset_type::shader);
    am.load_asset("assets/shaders2/taa.shader", asset_type::shader);
    am.load_asset("assets/shaders2/denoise.shader", asset_type::shader);
    am.load_asset("assets/shaders2/gi_combine.shader", asset_type::shader);
    am.load_asset("assets/shaders2/downsample.shader", asset_type::shader);
    am.load_asset("assets/shaders2/gbuffer_voxelization.shader", asset_type::shader);
    am.load_asset("assets/shaders2/voxel_mips.shader", asset_type::shader);

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

    glm::vec2 gi_res = { m_window_resolution.x * m_vxgi_resolution_scale, m_window_resolution.y * m_vxgi_resolution_scale };
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

    glm::vec2 ssr_res = { m_window_resolution.x * m_ssr_resolution_scale, m_window_resolution.y * m_ssr_resolution_scale };
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

void gl_renderer::pre_frame(camera& cam)
{
    ZoneScoped;

    im3d_gl::new_frame_im3d(m_im3d_state, m_window_resolution, cam);
}

void gl_renderer::render(asset_manager& am, camera& cam, std::vector<scene*>& scenes)
{
    ZoneScoped;
    FrameMark;

    // Todo: create a total bounding volume from all active scenes;
    aabb total_bounding_volume = scenes.front()->m_scene_bounding_volume;

    {
        TracyGpuZone("GBuffer Voxelization");
        tech::vxgi::dispatch_gbuffer_voxelization(
            m_compute_voxelize_gbuffer_shader->m_data,
            total_bounding_volume,
            m_voxel_data,
            m_gbuffer,
            m_lightpass_buffer_resolve,
            m_window_resolution);
    }
    {
        TracyGpuZone("GBuffer Voxelization MIPS");
        tech::vxgi::dispatch_gen_voxel_mips(
            m_compute_voxel_mips_shader->m_data,
            m_voxel_data,
            s_voxel_resolution);
    }

    {
        TracyGpuZone("GBuffer");
        tech::gbuffer::dispatch_gbuffer_with_id(
            m_frame_index,
            m_gbuffer,
            m_position_buffer_history,
            m_gbuffer_shader->m_data,
            am,
            cam,
            scenes,
            m_window_resolution);

        m_frame_index++;
    }

    // TODO: Need a way to get a single instance more efficiently
    dir_light dir{};

    if (!scenes.empty()) {
        auto dir_light_view = scenes.front()->m_registry.view<dir_light>();
        for (auto [e, dir_light_c] : dir_light_view.each())
        {
            dir = dir_light_c;
            break;
        }
    }

    std::vector<point_light> point_lights{};
    {
        TracyGpuZone("Dir Light Shadow Pass");
        tech::shadow::dispatch_shadow_pass(
            m_dir_light_shadow_buffer,
            m_dir_light_shadow_shader->m_data,
            dir,
            scenes,
            m_window_resolution);
    }
    {
        TracyGpuZone("Direct Lighting Pass");
        tech::lighting::dispatch_light_pass(
            m_lighting_shader->m_data,
            m_lightpass_buffer, m_gbuffer,
            m_dir_light_shadow_buffer,
            cam,
            point_lights,
            dir);
    }

    {
        TracyGpuZone("GBuffer Downsample");
        m_gbuffer_downsample.bind();
        tech::utils::dispatch_present_image(
            m_downsample_shader->m_data,
            "u_prev_mip",
            0,
            m_gbuffer.m_colour_attachments[2]);
        m_gbuffer_downsample.unbind();
    }

    {
        TracyGpuZone("Light Pass TAA");

        tech::taa::dispatch_taa_pass(
            m_taa_shader->m_data,
            m_lightpass_buffer,
            m_lightpass_buffer_resolve,
            m_lightpass_buffer_history,
            m_gbuffer.m_colour_attachments[4],
            m_window_resolution);
    }
    if (m_debug_draw_cone_tracing_pass || m_debug_draw_cone_tracing_pass_no_taa)
    {
        TracyGpuZone("Voxel Cone Tracing Pass");
        tech::vxgi::dispatch_cone_tracing_pass(
            m_voxel_cone_tracing_shader->m_data, 
            m_voxel_data, 
            m_conetracing_buffer, 
            m_gbuffer, 
            m_window_resolution, 
            total_bounding_volume, 
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
        TracyGpuZone("SSR Pass");
        glViewport(0, 0, m_window_resolution.x * m_ssr_resolution_scale, m_window_resolution.y * m_ssr_resolution_scale);
        tech::ssr::dispatch_ssr_pass(m_ssr_shader->m_data, cam, m_ssr_buffer, m_gbuffer, m_lightpass_buffer, m_window_resolution * m_ssr_resolution_scale);
        glViewport(0, 0, m_window_resolution.x, m_window_resolution.y);
        tech::taa::dispatch_taa_pass(
            m_taa_shader->m_data, 
            m_ssr_buffer, 
            m_ssr_buffer_resolve, 
            m_ssr_buffer_history, 
            m_gbuffer.m_colour_attachments[4], 
            m_window_resolution);
    }

    if (m_debug_draw_cone_tracing_pass)
    {
        {
            TracyGpuZone("Voxel Cone Tracing TAA");
            tech::taa::dispatch_taa_pass(
                m_taa_shader->m_data,
                m_conetracing_buffer,
                m_conetracing_buffer_resolve,
                m_conetracing_buffer_history,
                m_gbuffer.m_colour_attachments[4],
                m_window_resolution);

            glViewport(0, 0, m_window_resolution.x * m_vxgi_resolution_scale, m_window_resolution.y * m_vxgi_resolution_scale);
        }
        {
            TracyGpuZone("Voxel Cone Tracing Denoise");
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
    {
        TracyGpuZone("Blit lightpass to history");
        tech::utils::blit_to_fb(
            m_lightpass_buffer_history,
            m_present_shader->m_data,
            "u_image_sampler",
            0,
            m_lightpass_buffer_resolve.m_colour_attachments[0]);
    }
    {
        TracyGpuZone("Blit Gbuffer position to history");
        tech::utils::blit_to_fb(
            m_position_buffer_history,
            m_present_shader->m_data,
            "u_image_sampler",
            0,
            m_gbuffer.m_colour_attachments[1]);
    }
    {
        TracyGpuZone("Blit voxel cone tracing to history");
        tech::utils::blit_to_fb(
            m_conetracing_buffer_history,
            m_present_shader->m_data,
            "u_image_sampler",
            0,
            m_conetracing_buffer_denoise.m_colour_attachments.front());
    }
    {
        TracyGpuZone("Blit ssr pass to history");
        tech::utils::blit_to_fb(
            m_ssr_buffer_history,
            m_present_shader->m_data,
            "u_image_sampler",
            0,
            m_ssr_buffer_resolve.m_colour_attachments.front());
    }
    glClear(GL_DEPTH_BUFFER_BIT);

    if (m_debug_draw_final_pass)
    {
        TracyGpuZone("Composite Final Pass");
        m_final_pass.bind();
        shapes::s_screen_quad.use();
        m_combine_shader->m_data.use();
        m_combine_shader->m_data.set_float("u_brightness", m_tonemapping_brightness);
        m_combine_shader->m_data.set_float("u_contrast", m_tonemapping_contrast);
        m_combine_shader->m_data.set_float("u_saturation", m_tonemapping_saturation);
        m_combine_shader->m_data.set_int("lighting_pass", 0);
        texture::bind_sampler_handle(m_lightpass_buffer_resolve.m_colour_attachments.front(), GL_TEXTURE0);
        m_combine_shader->m_data.set_int("cone_tracing_pass", 1);
        texture::bind_sampler_handle(m_conetracing_buffer_resolve.m_colour_attachments.front(), GL_TEXTURE1);
        m_combine_shader->m_data.set_int("ssr_pass", 2);
        m_combine_shader->m_data.set_int("ssr_pass", 2);
        texture::bind_sampler_handle(m_ssr_buffer_resolve.m_colour_attachments.front(), GL_TEXTURE2);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        texture::bind_sampler_handle(0, GL_TEXTURE0);
        texture::bind_sampler_handle(0, GL_TEXTURE1);
        m_final_pass.unbind();
        tech::utils::dispatch_present_image(m_present_shader->m_data, "u_image_sampler", 0, m_final_pass.m_colour_attachments.front());
    }

    {
        TracyGpuZone("Im3D Pass");
        im3d_gl::end_frame_im3d(m_im3d_state, m_window_resolution, cam);
    }
    TracyGpuCollect;
}

void gl_renderer::cleanup(asset_manager& am)
{
    ZoneScoped;
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

entt::entity gl_renderer::get_mouse_entity(glm::vec2 mouse_position)
{
    ZoneScoped;
    auto pixels = m_gbuffer.read_pixels<glm::vec4, 1, 1>(
        mouse_position.x, 
        m_window_resolution.y - mouse_position.y, 
        5, 
        GL_RGBA, 
        GL_FLOAT);
    m_last_selected_entity = entt::entity(
        pixels[0][0] +
        pixels[0][1] * 256 +
        pixels[0][2] * 256 * 256
    );

    return m_last_selected_entity;
}

void gl_renderer::on_imgui(asset_manager& am)
{
    ZoneScoped;
    glm::vec2 mouse_pos = input::get_mouse_position();
    ImGui::Begin("Renderer Settings");
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / gl_backend::s_imgui_io->Framerate, gl_backend::s_imgui_io->Framerate);
    ImGui::Text("Mouse Pos : %.3f, %.3f", mouse_pos.x, mouse_pos.y);
    ImGui::Text("Selected Entity ID : %d", m_last_selected_entity);
    ImGui::Separator();
    ImGui::Checkbox("Render 3D Voxel Grid", &m_debug_draw_3d_texture);
    ImGui::Checkbox("Render Final Pass", &m_debug_draw_final_pass);
    ImGui::Checkbox("Render Direct Lighting Pass", &m_debug_draw_lighting_pass);
    ImGui::Checkbox("Render Direct Lighting Pass NO TAA", &m_debug_draw_lighting_pass_no_taa);
    ImGui::Checkbox("Render Cone Tracing Pass", &m_debug_draw_cone_tracing_pass);
    ImGui::Checkbox("Render SSR", &m_debug_draw_ssr_pass);
    ImGui::Checkbox("Render Cone Tracing Pass NO TAA", &m_debug_draw_cone_tracing_pass_no_taa);
    ImGui::Separator();
    ImGui::Text("Brightness / Contrast / Saturation");
    ImGui::DragFloat("Brightness", &m_tonemapping_brightness);
    ImGui::DragFloat("Contrast", &m_tonemapping_contrast);
    ImGui::DragFloat("Saturation", &m_tonemapping_saturation);
    ImGui::Separator();
    ImGui::Text("VXGI Settings");
    ImGui::DragFloat("Trace Distance", &m_vxgi_cone_trace_distance);
    ImGui::DragFloat("Diffuse / Spec Mix", &m_vxgi_diffuse_specular_mix, 1.0f, 0.0f, 1.0f);
    ImGui::Separator();
    ImGui::Text("Denoise Settings");
    ImGui::DragFloat("Sigma", &m_denoise_sigma);
    ImGui::DragFloat("Threshold", &m_denoise_threshold);
    ImGui::DragFloat("KSigma", &m_denoise_k_sigma);
    ImGui::Separator();
    ImGui::End();
}
