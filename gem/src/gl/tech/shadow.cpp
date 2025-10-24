#define GLM_ENABLE_EXPERIMENTAL
#include "gem/gl/tech/shadow.h"
#include "gem/gl/gl_dbg.h"
#include "gem/gl/gl_framebuffer.h"
#include "gem/material.h"
#include "gem/mesh.h"
#include "gem/profile.h"
#include "gem/scene.h"
#include "gem/transform.h"
#include "gtc/matrix_transform.hpp"
#include "gtc/quaternion.hpp"

namespace gem {
namespace gl {

void tech::Shadow::DispatchShadowPass(GLFramebuffer &shadow_fb,
                                        GLShader &shadow_shader,
                                        DirectionalLight &sun,
                                        std::vector<Scene *> &scenes,
                                        glm::ivec2 window_res) {
  ZoneScoped;
  GEM_GPU_MARKER("Shadow Map Pass");
  float near_plane = 0.01f, far_plane = 1000.0f;
  glm::mat4 lightProjection =
      glm::ortho(-150.0f, 150.0f, -150.0f, 150.0f, near_plane, far_plane);

  glm::vec3 dir =
      glm::quat(glm::radians(sun.direction)) * glm::vec3(0.0f, 0.0f, 1.0f);

  glm::vec3 lightPos = glm::vec3(0.0) - (dir * 100.0f);
  glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f, 0.0f, 0.0f),
                                    glm::vec3(0.0f, 1.0f, 0.0f));
  glm::mat4 lightSpaceMatrix = lightProjection * lightView;

  sun.light_space_matrix = lightSpaceMatrix;

  shadow_fb.Bind();
  glClear(GL_DEPTH_BUFFER_BIT);
  glViewport(0, 0, shadow_fb.m_width, shadow_fb.m_height);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_FRONT);
  shadow_shader.Use();
  shadow_shader.SetMat4f("lightSpaceMatrix", lightSpaceMatrix);

  for (Scene *current_scene : scenes) {
    auto renderables =
        current_scene->m_registry.view<Transform, MeshComponent, Material>();

    for (auto [e, trans, emesh, ematerial] : renderables.each()) {
      shadow_shader.SetMat4f("model", trans.m_model);
      emesh.m_mesh->m_vao.draw();
    }
  }

  shadow_fb.Unbind();
  glDisable(GL_CULL_FACE);
  glViewport(0, 0, window_res.x, window_res.y);
}
} // namespace open_gl
} // namespace gem