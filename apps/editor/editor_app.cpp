#include "editor_app.h"

editor_application::editor_application()
{
    engine::init(engine_init{ {1920, 1080}, true });
}

void editor_application::run()
{
    while (!engine::s_quit)
    {
        m_asset_manager.update();

        engine::process_sdl_event();
        engine::engine_pre_frame();

        main_menu_bar();

        engine::engine_post_frame();
    }
    engine::engine_shut_down();
}

void editor_application::main_menu_bar()
{
    ImGui::BeginMainMenuBar();
    if (ImGui::BeginMenu("File"))
    {
        ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
}
