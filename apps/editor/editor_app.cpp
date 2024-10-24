#include "editor_app.h"
#include "ImFileDialog.h"


editor_application::editor_application()
{
    backend::init(backend_init{ {1920, 1080}, true });
}

void editor_application::run()
{
    while (!backend::s_quit)
    {
        m_asset_manager.update();

        backend::process_sdl_event();
        backend::engine_pre_frame();

        main_menu_bar();

        backend::engine_post_frame();
    }
    backend::engine_shut_down();
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
