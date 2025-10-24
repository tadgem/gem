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
namespace gl {

void tech::GBuffer::DispatchGBufferWithID(
    u32 frame_index, GLFramebuffer &gbuffer,
    GLFramebuffer &previous_position_buffer, GLShader &gbuffer_shader,
    AssetManager &am, Camera &cam, std::vector<Scene *> &scenes,
    glm::ivec2 win_res) {
  ZoneScoped;
  GEM_GPU_MARKER("GBuffer-EntityID");
  glDisable(GL_DITHER);

  gbuffer.Bind();
  glm::mat4 current_vp = cam.m_proj * cam.m_view;

  gbuffer_shader.Use();
  gbuffer_shader.SetVec2f("u_resolution", {win_res.x, win_res.y});
  gbuffer_shader.SetMat4f("u_vp", current_vp);
  gbuffer_shader.SetMat4f("u_last_vp", cam.m_last_vp);
  gbuffer_shader.SetInt("u_frame_index", frame_index);
  gbuffer_shader.SetInt("u_diffuse_map", 0);
  gbuffer_shader.SetInt("u_normal_map", 1);
  gbuffer_shader.SetInt("u_metallic_map", 2);
  gbuffer_shader.SetInt("u_roughness_map", 3);
  gbuffer_shader.SetInt("u_ao_map", 4);
  gbuffer_shader.SetInt("u_prev_position_map", 5);

  Texture::BindSamplerHandle(0, GL_TEXTURE0);
  Texture::BindSamplerHandle(0, GL_TEXTURE1);
  Texture::BindSamplerHandle(0, GL_TEXTURE2);
  Texture::BindSamplerHandle(0, GL_TEXTURE3);
  Texture::BindSamplerHandle(0, GL_TEXTURE4);
  Texture::BindSamplerHandle(
      previous_position_buffer.m_colour_attachments.front(), GL_TEXTURE5);

  for (Scene *current_scene : scenes) {
    auto renderables =
        current_scene->m_registry.view<Transform, MeshComponent, Material>();

    for (auto [e, trans, emesh, ematerial] : renderables.each()) {

      if (ematerial.m_prog.m_shader_id != gbuffer_shader.m_shader_id) {
        continue;
      }

      ematerial.BindUniforms(am);
      gbuffer_shader.SetMat4f("u_model", trans.m_model);
      gbuffer_shader.SetMat4f("u_last_model", trans.m_last_model);
      gbuffer_shader.SetMat4f("u_normal", trans.m_normal_matrix);
      int entity_index = static_cast<int>(e);
      gbuffer_shader.SetInt("u_entity_index", entity_index);
      emesh.m_mesh->m_vao.Draw();
    }
  }
  gbuffer.Unbind();
  glEnable(GL_DITHER);
}
void tech::GBuffer::DispatchGBufferTexturelessWithID(
    u32 frame_index, GLFramebuffer &gbuffer,
    GLFramebuffer &previous_position_buffer,
    GLShader &gbuffer_textureless_shader, AssetManager &am, Camera &cam,
    std::vector<Scene *> &scenes, glm::ivec2 win_res) {
  ZoneScoped;
  GEM_GPU_MARKER("GBuffer-Textureless-EntityID");
  glDisable(GL_DITHER);

  gbuffer.Bind();
  glm::mat4 current_vp = cam.m_proj * cam.m_view;

  gbuffer_textureless_shader.Use();
  gbuffer_textureless_shader.SetVec2f("u_resolution", {win_res.x, win_res.y});
  gbuffer_textureless_shader.SetMat4f("u_vp", current_vp);
  gbuffer_textureless_shader.SetMat4f("u_last_vp", cam.m_last_vp);
  gbuffer_textureless_shader.SetMat4f("u_view", cam.m_view);
  gbuffer_textureless_shader.SetInt("u_frame_index", frame_index);
  gbuffer_textureless_shader.SetInt("u_prev_position_map", 0);

  Texture::BindSamplerHandle(
      previous_position_buffer.m_colour_attachments.front(), GL_TEXTURE0);

  for (Scene *current_scene : scenes) {
    auto renderables =
        current_scene->m_registry.view<Transform, MeshComponent, Material>();

    for (auto [e, trans, emesh, ematerial] : renderables.each()) {
      if (ematerial.m_prog.m_shader_id !=
          gbuffer_textureless_shader.m_shader_id) {
        continue;
      }

      ematerial.BindUniforms(am);
      gbuffer_textureless_shader.SetMat4f("u_model", trans.m_model);
      gbuffer_textureless_shader.SetMat4f("u_last_model", trans.m_last_model);

      int entity_index = static_cast<int>(e);
      gbuffer_textureless_shader.SetInt("u_entity_index", entity_index);
      emesh.m_mesh->m_vao.Draw();
    }
  }
  gbuffer.Unbind();
  Texture::BindSamplerHandle(0, GL_TEXTURE0);
  glEnable(GL_DITHER);
}
} // namespace open_gl
} // namespace gem