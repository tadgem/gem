#include <iostream>
#include <sstream>
#include "gem/engine.h"

using namespace gem;

int main()
{
    glm::ivec2 window_res{ 1280, 720};
    Engine::Init(window_res);
    AssetManager am{};
    Camera cam{};

    auto im3d_s = GLIm3d::LoadIm3D();
    while (!GPUBackend::Selected()->m_quit)
    {       
        GPUBackend::Selected()->ProcessEvents();
        GPUBackend::Selected()->PreFrame();
        glm::vec2 window_dim = GPUBackend::Selected()->GetWindowDimensions();
        GLIm3d::NewFrameIm3D(im3d_s, window_dim, cam);

        am.Update();
        am.OnImGui();

        GLIm3d::EndFrameIm3D(im3d_s, { window_res.x, window_res.y }, cam);
        GPUBackend::Selected()->PostFrame();
    }

    am.Shutdown();
    GLIm3d::ShutdownIm3D(im3d_s);
    GPUBackend::Selected()->ShutDown();

    return 0;
}