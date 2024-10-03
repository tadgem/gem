#include "tech/shadow.h"
#include "framebuffer.h"
#include "scene.h"
#include "gtc/matrix_transform.hpp"
#include "gtc/quaternion.hpp"
#include "transform.h"
#include "mesh.h"
#include "material.h"

void tech::shadow::dispatch_shadow_pass(framebuffer& shadow_fb, shader& shadow_shader, dir_light& sun, scene& current_scene, glm::ivec2 window_res)
{
    float near_plane = 0.01f, far_plane = 1000.0f;
    glm::mat4 lightProjection = glm::ortho(-200.0f, 200.0f, -200.0f, 200.0f, near_plane, far_plane);

    glm::vec3 dir = glm::quat(glm::radians(sun.direction)) * glm::vec3(0.0f, 0.0f, 1.0f);

    glm::vec3 lightPos = glm::vec3(0.0) - (dir * 100.0f);
    glm::mat4 lightView = glm::lookAt(lightPos,
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 lightSpaceMatrix = lightProjection * lightView;

    sun.light_space_matrix = lightSpaceMatrix;

    shadow_fb.bind();
    glClear(GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, shadow_fb.m_width, shadow_fb.m_height);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    shadow_shader.use();
    shadow_shader.set_mat4("lightSpaceMatrix", lightSpaceMatrix);

    auto renderables = current_scene.m_registry.view<transform, mesh, material>();

    for (auto& [e, trans, emesh, ematerial] : renderables.each())
    {
        shadow_shader.set_mat4("model", trans.m_model);
        emesh.draw();
    }


    shadow_fb.unbind();
    glDisable(GL_CULL_FACE);
    glViewport(0, 0, window_res.x, window_res.y);

}

