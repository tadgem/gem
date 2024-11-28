#include <iostream>
#include <sstream>
#include "im3d.h"
#include "imgui.h"
#include "gtc/matrix_transform.hpp"
#include "gtc/quaternion.hpp"
#include "im3d_math.h"
#include "ImFileDialog.h"

#include "gem/asset.h"
#include "gem/asset_manager.h"
#include "gem/backend.h"
#include "gem/camera.h"
#include "gem/events.h"
#include "gem/gl/gl_framebuffer.h"
#include "gem/gl/im3d_gl.h"
#include "gem/gl/tech/gbuffer.h"
#include "gem/gl/tech/lighting.h"
#include "gem/gl/tech/shadow.h"
#include "gem/gl/tech/ssr.h"
#include "gem/gl/tech/taa.h"
#include "gem/gl/tech/tech_utils.h"
#include "gem/gl/tech/vxgi.h"
#include "gem/input.h"
#include "gem/json.hpp"
#include "gem/lights.h"
#include "gem/material.h"
#include "gem/model.h"
#include "gem/scene.h"
#include "gem/shader.h"
#include "gem/shape.h"
#include "gem/texture.h"
#include "gem/transform.h"
#include "gem/utils.h"
#include "gem/vertex.h"
#include "gem/voxelisation.h"

using namespace gem;

int main()
{
    glm::ivec2 window_res{ 1280, 720};
    gl_backend::init(backend_init{ window_res, true });
    asset_manager am{};

    auto im3d_s =  im3d_gl::load_im3d();
    while (!gl_backend::s_quit)
    {       
        gl_backend::process_sdl_event();
        gl_backend::engine_pre_frame();        
        glm::vec2 window_dim = gl_backend::get_window_dim();
        im3d_gl::new_frame_im3d(im3d_s, window_dim, camera{});

        am.update();

        if (ImGui::Begin("Assets Debug"))
        {
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / gl_backend::s_imgui_io->Framerate, gl_backend::s_imgui_io->Framerate);
            if (ImGui::CollapsingHeader("CPU Memory")) {
                ImGui::Text("Untracked : %.4f KB", (float)debug_memory_tracker::s_UntrackedSize / 1024.0f);
                for (auto& [k, v] : debug_memory_tracker::s_instance->s_allocation_info) {
                    ImGui::Text("%s : %zu KB", k.c_str(), (v.size * v.count) / 1024);
                }
            }
            ImGui::Separator();
            ImGui::Text("Any Assets Loading? : %s", am.any_assets_loading() ? "true" : "false");
            ImGui::Text("Any Pending Async Tasks : %d", am.p_pending_load_tasks.size());
            ImGui::Text("Any Pending Synchronous Callbacks : %d", am.p_pending_load_callbacks.size());
            ImGui::Text("Any Pending Unload Tasks: %d", am.p_pending_unload_callbacks.size());

            if (ImGui::CollapsingHeader("Loaded Assets"))
            {
                for (const auto& [handle, u_asset] : am.p_loaded_assets)
                {
                    if (!u_asset) { continue; }
                    ImGui::PushID(handle.m_path_hash);
                    ImGui::Text("%s : %s", u_asset->m_path.c_str(), get_asset_type_name(handle.m_type).c_str());
                    ImGui::SameLine();
                    if (ImGui::Button("Unload"))
                    {
                        am.unload_asset(handle);
                    }
                    ImGui::PopID();
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
                    ImGui::Text("%s : %s", info.m_loaded_asset_intermediate->m_asset_data->m_path.c_str(), get_asset_type_name(handle.m_type));
                }
            }
            ImGui::Separator();
            if (ImGui::Button("Unload All Assets"))
            {
                am.unload_all_assets();
            }

            if (ImGui::Button("Load Model"))
            {
                ifd::FileDialog::Instance().Open("ModelOpenDialog", "Import a model", "Model file (*.dae;*.obj;*.fbx;*.gltf;){.dae,.obj,.fbx,.gltf},.*");
            }
            ImGui::SameLine();
            if (ImGui::Button("Load Shader"))
            {
                ifd::FileDialog::Instance().Open("ShaderOpenDialog", "Import a shader", "Shader file (*.shader){.shader,.*}");
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

        if (ifd::FileDialog::Instance().IsDone("ShaderOpenDialog")) {
            if (ifd::FileDialog::Instance().HasResult()) {
                std::filesystem::path p = ifd::FileDialog::Instance().GetResult();
                std::string res = p.u8string();
                printf("OPEN[%s]\n", res.c_str());
                am.load_asset(res, asset_type::shader);
            }
            ifd::FileDialog::Instance().Close();
        }
        ImGui::End();
        
        im3d_gl::end_frame_im3d(im3d_s, { window_res.x, window_res.y }, camera{});
        gl_backend::engine_post_frame();
    }

    am.shutdown();
    im3d_gl::shutdown_im3d(im3d_s);
    gl_backend::engine_shut_down();

    return 0;
}