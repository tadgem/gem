#include <iostream>
#include <sstream>
#include "gem/engine.h"

using namespace gem;

int main()
{
    glm::ivec2 window_res{ 1280, 720};
    Engine::init(window_res);
    AssetManager am{};
    Camera cam{};

    auto im3d_s = GLIm3d::load_im3d();
    while (!GPUBackend::selected()->m_quit)
    {       
        GPUBackend::selected()->process_sdl_event();
        GPUBackend::selected()->engine_pre_frame();
        glm::vec2 window_dim = GPUBackend::selected()->get_window_dim();
        GLIm3d::new_frame_im3d(im3d_s, window_dim, cam);

        am.update();
        am.on_imgui();

        GLIm3d::end_frame_im3d(im3d_s, { window_res.x, window_res.y }, cam);
        GPUBackend::selected()->engine_post_frame();
    }

    am.shutdown();
    GLIm3d::shutdown_im3d(im3d_s);
    GPUBackend::selected()->engine_shut_down();

    return 0;
}