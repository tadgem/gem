#include <iostream>
#include "imgui.h"

#include "gl.h"
#include "texture.h" 
#include "shader.h"
#include "material.h"
#include "vertex.h"
#include "utils.h"
#include "model.h"
#include "camera.h"
#include "framebuffer.h"
#include "shape.h"
#include "voxelisation.h"

#include <sstream>
#include "im3d.h"
#include "im3d_gl.h"
#include "gtc/matrix_transform.hpp"
#include "gtc/quaternion.hpp"
#include "input.h"
#include "json.hpp"
#include "scene.h"
#include "asset.h"
#include "asset_manager.h"
#include "events.h"
#include "lights.h"
#include "transform.h"
#include "tech/vxgi.h"
#include "tech/gbuffer.h"
#include "tech/shadow.h"
#include "tech/lighting.h"
#include "tech/tech_utils.h"
#include "tech/ssr.h"
#include "tech/taa.h"
#include "im3d_math.h"
#include "ImFileDialog.h"


int main()
{
    glm::ivec2 window_res{ 1280, 720};
    engine::init(window_res);
    asset_manager am{};



    auto im3d_s =  im3d_gl::load_im3d();
    while (!engine::s_quit)
    {       
        engine::process_sdl_event();
        engine::engine_pre_frame();        
        glm::vec2 window_dim = engine::get_window_dim();
        im3d_gl::new_frame_im3d(im3d_s, window_dim, camera{});

        am.update();

        if (ImGui::Begin("Assets Debug"))
        {
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / engine::s_imgui_io->Framerate, engine::s_imgui_io->Framerate);
            ImGui::Text("Any Assets Loading? : %s", am.any_assets_loading() ? "true" : "false");

            if (ImGui::CollapsingHeader("Loaded Assets"))
            {
                for (const auto& [handle, u_asset] : am.p_loaded_assets)
                {
                    if (!u_asset) { continue; }
                    ImGui::Text("%s : %s", u_asset->m_path.c_str(), get_asset_type_name(handle.m_type).c_str());
                }
            }

            if (ImGui::CollapsingHeader("Enqueued Loads"))
            {
                for (const auto& info : am.p_queued_loads)
                {
                    ImGui::Text("%s : %s", info.m_path.c_str(), get_asset_type_name(info.m_type));
                }
            }

            if (ImGui::CollapsingHeader("In Progress Loads"))
            {
                for (const auto& [handle , info ]: am.p_pending_load_callbacks)
                {
                    ImGui::Text("%s : %s", info.m_loaded_asset->m_path.c_str(), get_asset_type_name(handle.m_type));
                }
            }

            if (ImGui::Button("Load Model"))
            {
                ifd::FileDialog::Instance().Open("ModelOpenDialog", "Import a model", "Image file (*.dae;*.obj;*.fbx;*.gltf;){.dae,.obj,.fbx,.gltf},.*");
            }

        }

        if (ifd::FileDialog::Instance().IsDone("ModelOpenDialog")) {
            if (ifd::FileDialog::Instance().HasResult()) {
                std::filesystem::path p = ifd::FileDialog::Instance().GetResult();
                std::string res = p.u8string();
                printf("OPEN[%s]\n", res.c_str());
                am.load_asset(res, asset_type::model);
            }
            ifd::FileDialog::Instance().Close();
        }
        ImGui::End();
        
        im3d_gl::end_frame_im3d(im3d_s, { window_res.x, window_res.y }, camera{});
        engine::engine_post_frame();
    }

    am.shutdown();
    im3d_gl::shutdown_im3d(im3d_s);
    engine::engine_shut_down();

    return 0;
}