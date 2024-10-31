#pragma once
#include "camera.h"
#include "im3d/im3d.h"
#include "shader.h"

namespace gem {
Im3d::Vec3 ToIm3D(glm::vec3 &input);
Im3d::Vec2 ToIm3D(glm::vec2 &input);

struct im3d_state {
  shader points_shader;
  shader line_shader;
  shader tris_shader;

  gl_handle im3d_vertex_buffer;
  gl_handle im3d_vao;
};

class im3d_gl {
public:
  static im3d_state load_im3d();
  static void shutdown_im3d(im3d_state &state);
  static void new_frame_im3d(im3d_state &state, glm::vec2 screen_dim,
                             camera &cam);
  static void end_frame_im3d(im3d_state &state, glm::ivec2 screen_dim,
                             camera &cam);
};
} // namespace gem