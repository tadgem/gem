#pragma once
#include "gem/camera.h"
#include "gl_shader.h"
#include "im3d/im3d.h"

namespace gem {
Im3d::Vec3 ToIm3D(glm::vec3 &input);
Im3d::Vec2 ToIm3D(glm::vec2 &input);

struct Im3dState {
  GLShader points_shader;
  GLShader line_shader;
  GLShader tris_shader;

  gl_handle im3d_vertex_buffer;
  gl_handle im3d_vao;
};

class GLIm3d {
public:
  static Im3dState load_im3d();
  static void shutdown_im3d(Im3dState &state);
  static void new_frame_im3d(Im3dState &state, glm::vec2 screen_dim,
                             Camera &cam);
  static void end_frame_im3d(Im3dState &state, glm::ivec2 screen_dim,
                             Camera &cam);
};
} // namespace gem