#include "vxgi.h"
#include "gl.h"
void tech::vxgi::dispatch_gbuffer_voxelization(shader& voxelization, aabb& volume_bounding_box, voxel::grid& voxel_data, framebuffer& gbuffer, framebuffer& lightpass_buffer, glm::ivec2 window_res)
{
    voxelization.use();
    voxelization.set_vec2("u_input_resolution", { window_res.x, window_res.y });
    voxelization.set_vec3("u_aabb.min", volume_bounding_box.min);
    voxelization.set_vec3("u_aabb.max", volume_bounding_box.max);
    texture::bind_image_handle(voxel_data.voxel_texture.m_handle, 0, 0, GL_RGBA32F);
    texture::bind_sampler_handle(gbuffer.m_colour_attachments[1], GL_TEXTURE0);
    texture::bind_sampler_handle(lightpass_buffer.m_colour_attachments[0], GL_TEXTURE1);
    glAssert(glDispatchCompute(window_res.x / 10, window_res.y / 10, 1));
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void tech::vxgi::dispatch_gen_voxel_mips(shader& voxelization_mips, voxel::grid& voxel_data, glm::vec3 _3d_tex_res_vec)
{
    constexpr int MAX_MIPS = 5;
    voxelization_mips.use();
    // for each mip in remaining_mipps
    glm::vec3 last_mip_resolution = _3d_tex_res_vec;
    glm::vec3 current_mip_resolution = _3d_tex_res_vec / 2.0f;
    for (int i = 1; i < MAX_MIPS; i++)
    {
        glBindTexture(GL_TEXTURE_3D, voxel_data.voxel_texture.m_handle);
        texture::bind_image_handle(voxel_data.voxel_texture.m_handle, 0, i, GL_RGBA32F);
        texture::bind_image_handle(voxel_data.voxel_texture.m_handle, 1, i - 1, GL_RGBA32F);
        voxelization_mips.set_vec3("u_current_resolution", current_mip_resolution);
        glm::ivec3 dispatch_dims = current_mip_resolution;
        glAssert(glDispatchCompute(dispatch_dims.x / 8, dispatch_dims.y / 8, dispatch_dims.z / 8));
        current_mip_resolution /= 2.0f;
        last_mip_resolution /= 2.0f;
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    }
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}
