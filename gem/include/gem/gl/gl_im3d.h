#pragma once
#include "gem/camera.h"
#include "gl_shader.h"
#include "im3d/im3d.h"

namespace gem {
Im3d::Vec3 ToIm3D(glm::vec3 &input);
Im3d::Vec2 ToIm3D(glm::vec2 &input);

struct im3d_state {
  gl_shader points_shader;
  gl_shader line_shader;
  gl_shader tris_shader;

  gl_handle im3d_vertex_buffer;
  gl_handle im3d_vao;
};

class gl_im3d {
public:
  static im3d_state load_im3d();
  static void shutdown_im3d(im3d_state &state);
  static void new_frame_im3d(im3d_state &state, glm::vec2 screen_dim,
                             camera &cam);
  static void end_frame_im3d(im3d_state &state, glm::ivec2 screen_dim,
                             camera &cam);
};
} // namespace gem