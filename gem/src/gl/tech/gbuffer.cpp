#define GLM_ENABLE_EXPERIMENTAL
#include "gem/gl/tech/gbuffer.h"
#include "gem/camera.h"
#include "gem/gl/gl_dbg.h"
#include "gem/material.h"
#include "gem/mesh.h"
#include "gem/profile.h"
#include "gem/scene.h"
#include "gem/transform.h"
namespace gem {
namespace open_gl {

void tech::GBuffer::dispatch_gbuffer_with_id(
    u32 frame_index, GLFramebuffer &gbuffer,
    GLFramebuffer &previous_position_buffer, GLShader &gbuffer_shader,
    AssetManager &am, Camera &cam, std::vector<Scene *> &scenes,
    glm::ivec2 win_res) {
  ZoneScoped;
  GEM_GPU_MARKER("GBuffer-EntityID");
  glDisable(GL_DITHER);

  gbuffer.bind();
  glm::mat4 current_vp = cam.m_proj * cam.m_view;

  gbuffer_shader.use();
  gbuffer_shader.set_vec2("u_resolution", {win_res.x, win_res.y});
  gbuffer_shader.set_mat4("u_vp", current_vp);
  gbuffer_shader.set_mat4("u_last_vp", cam.m_last_vp);
  gbuffer_shader.set_int("u_frame_index", frame_index);
  gbuffer_shader.set_int("u_diffuse_map", 0);
  gbuffer_shader.set_int("u_normal_map", 1);
  gbuffer_shader.set_int("u_metallic_map", 2);
  gbuffer_shader.set_int("u_roughness_map", 3);
  gbuffer_shader.set_int("u_ao_map", 4);
  gbuffer_shader.set_int("u_prev_position_map", 5);

  Texture::bind_sampler_handle(0, GL_TEXTURE0);
  Texture::bind_sampler_handle(0, GL_TEXTURE1);
  Texture::bind_sampler_handle(0, GL_TEXTURE2);
  Texture::bind_sampler_handle(0, GL_TEXTURE3);
  Texture::bind_sampler_handle(0, GL_TEXTURE4);

  Texture::bind_sampler_handle(
      previous_position_buffer.m_colour_attachments.front(), GL_TEXTURE5);

  for (Scene *current_scene : scenes) {
    auto renderables =
        current_scene->m_registry.view<Transform, MeshComponent, Material>();

    for (auto [e, trans, emesh, ematerial] : renderables.each()) {

      if (ematerial.m_prog.m_shader_id != gbuffer_shader.m_shader_id) {
        continue;
      }

      ematerial.bind_material_uniforms(am);
      gbuffer_shader.set_mat4("u_model", trans.m_model);
      gbuffer_shader.set_mat4("u_last_model", trans.m_last_model);
      gbuffer_shader.set_mat4("u_normal", trans.m_normal_matrix);
      int entity_index = static_cast<int>(e);
      gbuffer_shader.set_int("u_entity_index", entity_index);
      emesh.m_mesh->m_vao.draw();
    }
  }
  gbuffer.unbind();
  glEnable(GL_DITHER);
}
void tech::GBuffer::dispatch_gbuffer_textureless_with_id(
    u32 frame_index, GLFramebuffer &gbuffer,
    GLFramebuffer &previous_position_buffer,
    GLShader &gbuffer_textureless_shader, AssetManager &am, Camera &cam,
    std::vector<Scene *> &scenes, glm::ivec2 win_res) {
  ZoneScoped;
  GEM_GPU_MARKER("GBuffer-Textureless-EntityID");
  glDisable(GL_DITHER);

  gbuffer.bind();
  glm::mat4 current_vp = cam.m_proj * cam.m_view;

  gbuffer_textureless_shader.use();
  gbuffer_textureless_shader.set_vec2("u_resolution", {win_res.x, win_res.y});
  gbuffer_textureless_shader.set_mat4("u_vp", current_vp);
  gbuffer_textureless_shader.set_mat4("u_last_vp", cam.m_last_vp);
  gbuffer_textureless_shader.set_mat4("u_view", cam.m_view);
  gbuffer_textureless_shader.set_int("u_frame_index", frame_index);
  gbuffer_textureless_shader.set_int("u_prev_position_map", 0);

  Texture::bind_sampler_handle(
      previous_position_buffer.m_colour_attachments.front(), GL_TEXTURE0);

  for (Scene *current_scene : scenes) {
    auto renderables =
        current_scene->m_registry.view<Transform, MeshComponent, Material>();

    for (auto [e, trans, emesh, ematerial] : renderables.each()) {
      if (ematerial.m_prog.m_shader_id !=
          gbuffer_textureless_shader.m_shader_id) {
        continue;
      }

      ematerial.bind_material_uniforms(am);
      gbuffer_textureless_shader.set_mat4("u_model", trans.m_model);
      gbuffer_textureless_shader.set_mat4("u_last_model", trans.m_last_model);

      int entity_index = static_cast<int>(e);
      gbuffer_textureless_shader.set_int("u_entity_index", entity_index);
      emesh.m_mesh->m_vao.draw();
    }
  }
  gbuffer.unbind();
  Texture::bind_sampler_handle(0, GL_TEXTURE0);
  glEnable(GL_DITHER);
}
} // namespace open_gl
} // namespace gem