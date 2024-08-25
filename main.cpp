#include <iostream>
#include "imgui.h"

#include "gl.h"
#include "texture.h" 
#include "shader.h"
#include "vertex.h"
#include "utils.h"
#include "model.h"

int main()
{
    engine::init_gl_sdl();

    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    texture tex("assets/textures/crate.jpg");

    std::string vertexShaderSource = "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
        "}\0";
    std::string fragmentShaderSource = "#version 330 core\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
        "}\n\0";

    shader basic(vertexShaderSource, fragmentShaderSource);

    std::string texture_vert = utils::load_string_from_path("assets/shaders/texture.vert.glsl");
    std::string texture_frag = utils::load_string_from_path("assets/shaders/texture.frag.glsl");

    shader texture(texture_vert, texture_frag);

    model sponza = model::load_model_from_path("assets/models/sponza/Sponza.gltf");


    float vertices[] = {
         0.5f, -0.5f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f,  // bottom left
         0.0f,  0.5f, 0.0f   // top 
    };

    vao_builder builder;

    builder.begin();
    builder.add_vertex_buffer(&vertices[0], 9);
    builder.add_vertex_attribute(0, 3 * sizeof(float), 3);

    VAO new_vao = builder.build();

    while (!engine::s_quit)
    {
        engine::process_sdl_event();
        engine::engine_pre_frame();

        basic.use();
        new_vao.use();

        // update shader uniform
        double  timeValue = engine::get_frame_time();
        float greenValue = static_cast<float>(sin(timeValue) / 2.0 + 0.5);

        basic.setVec4("outColour", { 0.0f, greenValue, 0.0f, 1.0f });
        glDrawArrays(GL_TRIANGLES, 0, 3);

        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                          

            ImGui::Text("This is some useful text.");               
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            
            ImGui::ColorEdit3("clear color", (float*)&clear_color); 

            if (ImGui::Button("Button"))                            
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / engine::s_imgui_io->Framerate, engine::s_imgui_io->Framerate);
            ImGui::End();
        }

        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

        engine::engine_post_frame();
    }
    engine::engine_shut_down();

    return 0;
}