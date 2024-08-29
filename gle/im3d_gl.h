#pragma once
#include "shader.h"
#include "camera.h"

struct im3d_state
{
    shader points_shader;
    shader line_shader;
    shader tris_shader;

    gl_handle im3d_vertex_buffer;
    gl_handle im3d_vao;
};
class im3d_gl
{
public:
    static im3d_state  load_im3d();
    static void        shutdown_im3d(im3d_state& state);
    static void        new_frame_im3d(im3d_state& state);
    static void        end_frame_im3d(im3d_state& state, glm::ivec2 screen_dim, camera& cam);
};