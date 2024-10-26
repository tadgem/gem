#include "editor_app.h"
#include "ImFileDialog.h"
#include "engine.h"

editor_application::editor_application()
{
    gl_backend::init(backend_init{ {1920, 1080}, true });
    m_editor_fsm.set_starting_state(editor_mode::no_open_project);
    m_editor_fsm.add_state(editor_mode::no_open_project, [this]() {
        this->on_open_project();
        return fsm::NO_TRIGGER; 
    });
    m_editor_fsm.add_state(editor_mode::edit, [this]() {
        this->on_edit();
        return fsm::NO_TRIGGER; 
    });
    m_editor_fsm.add_state(editor_mode::play, [this]() {
        this->on_play();
        return fsm::NO_TRIGGER; 
    });

    m_editor_fsm.add_state_entry(editor_mode::play, []() {});
    m_editor_fsm.add_state_exit(editor_mode::play, []() {});

    m_editor_fsm.add_trigger(editor_trigger::project_loaded, editor_mode::no_open_project, editor_mode::edit);
    m_editor_fsm.add_trigger(editor_trigger::begin_play_mode, editor_mode::edit, editor_mode::play);
    m_editor_fsm.add_trigger(editor_trigger::exit_play_mode, editor_mode::play, editor_mode::edit);
}

void editor_application::run()
{
    while (!gl_backend::s_quit)
    {
        engine::assets.update();

        gl_backend::process_sdl_event();
        gl_backend::engine_pre_frame();

        main_menu_bar();

        m_editor_fsm.update();

        gl_backend::engine_post_frame();
    }
    gl_backend::engine_shut_down();
}

void editor_application::on_open_project()
{
    ImGui::Begin("Open Project");
    if (ImGui::Button("Load Project"))
    {
        m_editor_fsm.trigger(editor_trigger::project_loaded);
    }
    ImGui::End();
}

void editor_application::on_edit()
{
    ImGui::Begin("Edit");
    if (ImGui::Button("Play"))
    {
        m_editor_fsm.trigger(editor_trigger::begin_play_mode);
    }
    ImGui::End();
}

void editor_application::on_play()
{
    ImGui::Begin("Play");
    if (ImGui::Button("Edit"))
    {
        m_editor_fsm.trigger(editor_trigger::exit_play_mode);
    }
    ImGui::End();
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
