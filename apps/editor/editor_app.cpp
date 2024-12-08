#include "gem/engine.h"
#include "editor_app.h"
#include "ImFileDialog.h"

using namespace gem;

editor_application::editor_application()
{
    glm::ivec2 resolution = {1920, 1080};
    engine::init();
    gl_renderer renderer{};
    renderer.init(engine::assets, resolution);

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
    while (!gpu_backend::selected()->m_quit)
    {
        engine::assets.update();

        gpu_backend::selected()->process_sdl_event();
        gpu_backend::selected()->engine_pre_frame();

        main_menu_bar();

        m_editor_fsm.update();

        gpu_backend::selected()->engine_post_frame();
    }
    gpu_backend::selected()->engine_shut_down();
    engine::shutdown();
}

static char s_create_project_name_buffer[256]{ 0 };

void editor_application::on_open_project()
{
    ImGui::SetNextWindowSize(ImGui::GetWindowSize(), ImGuiCond_Always);
    auto y = ImGui::GetCursorPosY();
    ImGui::SetNextWindowPos(ImVec2{ 0.0f, y });
    ImGui::Begin("GE Main Menu");
    ImGui::InputText("Project Name", &s_create_project_name_buffer[0], 256);

    if (ImGui::Button("Create Project"))
    {
        std::string proj_name = std::string(s_create_project_name_buffer);
        if (!proj_name.empty())
        {
            ifd::FileDialog::Instance().Save("ProjectSaveDialog", "Choose a destination for your project", "Project file (*.proj;){.proj},.*");
        }
        else
        {
            ImGui::OpenPopup("NoProjName");
            
        }
    }
    if (ImGui::BeginPopup("NoProjName"))
    {
        if (ImGui::Selectable("Please specify a project name in the input box"))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    if (ImGui::Button("Load Project"))
    {
        ifd::FileDialog::Instance().Open("ProjectOpenDialog", "Choose project file", "Project file (*.proj;){.proj},.*");
    }
    ImGui::End();


    if (ifd::FileDialog::Instance().IsDone("ProjectSaveDialog")) {
        if (ifd::FileDialog::Instance().HasResult()) {
            std::filesystem::path p = ifd::FileDialog::Instance().GetResult();
            std::string res = p.u8string();
            std::filesystem::path directory = p.parent_path();
            engine::active_project = create_project(std::string(s_create_project_name_buffer), directory.string());
            engine::save_project_to_disk(p.filename().string(), directory.string());
            m_editor_fsm.trigger(editor_trigger::project_loaded);
        }
        ifd::FileDialog::Instance().Close();
    }

    if (ifd::FileDialog::Instance().IsDone("ProjectOpenDialog")) {
        if (ifd::FileDialog::Instance().HasResult()) {
            std::filesystem::path p = ifd::FileDialog::Instance().GetResult();
            std::string res = p.u8string();
            engine::load_project_from_disk(res);
            m_editor_fsm.trigger(editor_trigger::project_loaded);
        }
        ifd::FileDialog::Instance().Close();
    }
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

project editor_application::create_project(const std::string& name, const std::string& path)
{
    project proj{};
    proj.m_name = name;
    std::filesystem::current_path(std::filesystem::path(path));
    return proj;
}
