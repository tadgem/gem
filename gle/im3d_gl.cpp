#include "im3d_gl.h"
#include "utils.h"
#include "im3d.h"
#include "im3d_math.h"
#include "gtc/type_ptr.hpp"
#include "imgui.h"
#include "input.h"
#include "engine.h"

Im3d::Mat4 ToIm3D(const glm::mat4& _m) {
    Im3d::Mat4 m(1.0);
    for (int i = 0; i < 16; ++i)
    {
        m[i] = *(&(_m[0][0]) + i);
    }
    return m;
}


void DrawIm3dTextListsImGuiAsChild(const Im3d::TextDrawList _textDrawLists[], uint32_t _count, uint32_t width, uint32_t height, glm::mat4 _viewProj)
{
    ImDrawList* imDrawList = ImGui::GetWindowDrawList();
    const Im3d::Mat4 viewProj = ToIm3D(_viewProj);
    for (uint32_t i = 0; i < _count; ++i)
    {
        const Im3d::TextDrawList& textDrawList = Im3d::GetTextDrawLists()[i];

        if (textDrawList.m_layerId == Im3d::MakeId("NamedLayer"))
        {
            // The application may group primitives into layers, which can be used to change the draw state (e.g. enable depth testing, use a different shader)
        }

        for (uint32_t j = 0; j < textDrawList.m_textDataCount; ++j)
        {
            const Im3d::TextData& textData = textDrawList.m_textData[j];
            if (textData.m_positionSize.w == 0.0f || textData.m_color.getA() == 0.0f)
            {
                continue;
            }

            // Project world -> screen space.
            Im3d::Vec4 clip = viewProj * Im3d::Vec4(textData.m_positionSize.x, textData.m_positionSize.y, textData.m_positionSize.z, 1.0f);
            Im3d::Vec2 screen = Im3d::Vec2(clip.x / clip.w, -clip.y / clip.w);

            // Cull text which falls offscreen. Note that this doesn't take into account text size but works well enough in practice.
            if (clip.w < 0.0f || screen.x >= 1.0f || screen.y >= 1.0f)
            {
                continue;
            }

            // Pixel coordinates for the ImGuiWindow ImGui.
            screen = screen * Im3d::Vec2(0.5f) + Im3d::Vec2(0.5f);
            auto windowSize = ImGui::GetWindowSize();
            screen = screen * Im3d::Vec2{ windowSize.x, windowSize.y };

            // All text data is stored in a single buffer; each textData instance has an offset into this buffer.
            const char* text = textDrawList.m_textBuffer + textData.m_textBufferOffset;

            // Calculate the final text size in pixels to apply alignment flags correctly.
            ImGui::SetWindowFontScale(textData.m_positionSize.w); // NB no CalcTextSize API which takes a font/size directly...
            auto textSize = ImGui::CalcTextSize(text, text + textData.m_textLength);
            ImGui::SetWindowFontScale(1.0f);

            // Generate a pixel offset based on text flags.
            Im3d::Vec2 textOffset = Im3d::Vec2(-textSize.x * 0.5f, -textSize.y * 0.5f); // default to center
            if ((textData.m_flags & Im3d::TextFlags_AlignLeft) != 0)
            {
                textOffset.x = -textSize.x;
            }
            else if ((textData.m_flags & Im3d::TextFlags_AlignRight) != 0)
            {
                textOffset.x = 0.0f;
            }

            if ((textData.m_flags & Im3d::TextFlags_AlignTop) != 0)
            {
                textOffset.y = -textSize.y;
            }
            else if ((textData.m_flags & Im3d::TextFlags_AlignBottom) != 0)
            {
                textOffset.y = 0.0f;
            }
            ImFont* font = nullptr;
            // Add text to the window draw list.
            screen = screen + textOffset;
            ImVec2 windowPos = ImGui::GetWindowPos();
            ImVec2 imguiScreen{ windowPos.x + screen.x, windowPos.y + screen.y };
            imDrawList->AddText(
                font,
                (float)(textData.m_positionSize.w * ImGui::GetFontSize()),
                imguiScreen,
                textData.m_color.getABGR(),
                text,
                text + textData.m_textLength);
        }
    }
}


void DrawIm3dTextListsImGui(const Im3d::TextDrawList _textDrawLists[], uint32_t _count, uint32_t width, uint32_t height, glm::mat4 _viewProj)
{
    // Using ImGui here as a simple means of rendering text draw lists, however as with primitives the application is free to draw text in any conceivable  manner.

        // Invisible ImGui window which covers the screen.
    ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32_BLACK_TRANS);
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(ImVec2((float)width, (float)height));
    ImGui::Begin("Invisible", nullptr, 0
        | ImGuiWindowFlags_NoTitleBar
        | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoScrollbar
        | ImGuiWindowFlags_NoInputs
        | ImGuiWindowFlags_NoSavedSettings
        | ImGuiWindowFlags_NoFocusOnAppearing
        | ImGuiWindowFlags_NoBringToFrontOnFocus
    );

    DrawIm3dTextListsImGuiAsChild(_textDrawLists, _count, width, height, _viewProj);

    ImGui::End();
    ImGui::PopStyleColor(1);
}

im3d_state im3d_gl::load_im3d()
{
    std::string tris_vert = utils::load_string_from_path("assets/shaders/im3d/im3d.tris.vert.glsl");
    std::string tris_frag = utils::load_string_from_path("assets/shaders/im3d/im3d.tris.frag.glsl");

    std::string points_vert = utils::load_string_from_path("assets/shaders/im3d/im3d.points.vert.glsl");
    std::string points_frag = utils::load_string_from_path("assets/shaders/im3d/im3d.points.frag.glsl");

    std::string lines_vert = utils::load_string_from_path("assets/shaders/im3d/im3d.lines.vert.glsl");
    std::string lines_frag = utils::load_string_from_path("assets/shaders/im3d/im3d.lines.frag.glsl");

    std::string geo = utils::load_string_from_path("assets/shaders/im3d/im3d.geom.glsl");

    shader points_shader(points_vert, points_frag);
    shader line_shader(lines_vert, geo, lines_frag);
    shader tris_shader(tris_vert, tris_frag);

    gl_handle im3d_vertex_buffer;
    gl_handle im3d_vao;

    glGenBuffers(1, &im3d_vertex_buffer);
    glGenVertexArrays(1, &im3d_vao);
    glBindVertexArray(im3d_vao);
    glBindBuffer(GL_ARRAY_BUFFER, im3d_vertex_buffer);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Im3d::VertexData), (GLvoid*)offsetof(Im3d::VertexData, m_positionSize));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Im3d::VertexData), (GLvoid*)offsetof(Im3d::VertexData, m_color));
    glBindVertexArray(0);

    return { points_shader, line_shader, tris_shader, im3d_vertex_buffer, im3d_vao };
}


void im3d_gl::shutdown_im3d(im3d_state& state)
{
    glDeleteVertexArrays(1, &state.im3d_vao);
    glDeleteBuffers(1, &state.im3d_vertex_buffer);
    glDeleteProgram(state.points_shader.m_shader_id);
    glDeleteProgram(state.line_shader.m_shader_id);
    glDeleteProgram(state.tris_shader.m_shader_id);
}

void im3d_gl::new_frame_im3d(im3d_state& state, glm::vec2 screen_dim, camera& cam)
{
    // update app data e.g. mouse pos, viewport size keys etc.
	Im3d::AppData& ad = Im3d::GetAppData(); 
	ad.m_viewportSize = { 1920, 1080};
    ad.m_keyDown[Im3d::Key::Mouse_Left] = input::get_mouse_button(mouse_button::left);
    ad.m_keyDown[Im3d::Key::Key_L] = input::get_keyboard_key(keyboard_key::l);
    ad.m_keyDown[Im3d::Key::Key_R] = input::get_keyboard_key(keyboard_key::r);
    ad.m_keyDown[Im3d::Key::Key_S] = input::get_keyboard_key(keyboard_key::s);
    ad.m_keyDown[Im3d::Key::Key_T] = input::get_keyboard_key(keyboard_key::t);
    ad.m_deltaTime = engine::get_frame_time();

    glm::vec2 cursor_pos = input::get_mouse_position();

    glm::vec3 rayOrigin = utils::screen_to_world_pos(cursor_pos, screen_dim, glm::inverse(cam.m_view), glm::inverse(cam.m_proj));;
    glm::vec3 rayDirection = glm::normalize(rayOrigin - cam.m_pos);

    ad.m_cursorRayOrigin = Im3d::Vec3(rayOrigin.x, rayOrigin.y, rayOrigin.z);
    ad.m_cursorRayDirection = Im3d::Vec3(rayDirection.x, rayDirection.y, rayDirection.z);
    
    Im3d::NewFrame();
}

void im3d_gl::end_frame_im3d(im3d_state& state, glm::ivec2 screen_dim, camera& cam)
{
    Im3d::EndFrame();
	//glEnable(GL_BLEND);
	//glBlendEquation(GL_FUNC_ADD);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_PROGRAM_POINT_SIZE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	glViewport(0, 0, (GLsizei)screen_dim.x, (GLsizei)screen_dim.y);
	glm::mat4 viewProj = cam.m_proj * cam.m_view;

	for (uint32_t i = 0, n = Im3d::GetDrawListCount(); i < n; ++i)
	{
		const Im3d::DrawList& drawList = Im3d::GetDrawLists()[i];

		GLenum prim;
		GLuint sh;
		switch (drawList.m_primType)
		{
		case Im3d::DrawPrimitive_Points:
			prim = GL_POINTS;
			sh = state.points_shader.m_shader_id;
			glDisable(GL_CULL_FACE); // points are view-aligned
			break;
		case Im3d::DrawPrimitive_Lines:
			prim = GL_LINES;
			sh = state.line_shader.m_shader_id;
			glDisable(GL_CULL_FACE); // lines are view-aligned
			break;
		case Im3d::DrawPrimitive_Triangles:
			prim = GL_TRIANGLES;
			sh = state.tris_shader.m_shader_id;
			//glAssert(glEnable(GL_CULL_FACE)); // culling valid for triangles, but optional
			break;
		default:
			IM3D_ASSERT(false);
			return;
		};

		glBindVertexArray(state.im3d_vao);
		glBindBuffer(GL_ARRAY_BUFFER, state.im3d_vertex_buffer);
		glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)drawList.m_vertexCount * sizeof(Im3d::VertexData), (GLvoid*)drawList.m_vertexData, GL_STREAM_DRAW);

		// AppData& ad = GetAppData();
		glUseProgram(sh);
		glUniform2f(glGetUniformLocation(sh, "uViewport"), screen_dim.x, screen_dim.y);
		glUniformMatrix4fv(glGetUniformLocation(sh, "uViewProjMatrix"), 1, false, glm::value_ptr(viewProj));
		glDrawArrays(prim, 0, (GLsizei)drawList.m_vertexCount);
	}

    DrawIm3dTextListsImGui(Im3d::GetContext().getTextDrawLists(), Im3d::GetContext().getTextDrawListCount(),
        1920, 1080, viewProj);

}


