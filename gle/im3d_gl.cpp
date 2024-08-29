#include "im3d_gl.h"
#include "utils.h"
#include "im3d.h"
#include "gtc/type_ptr.hpp"
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

void im3d_gl::new_frame_im3d(im3d_state& state)
{
    // update app data e.g. mouse pos, viewport size keys etc.
	Im3d::AppData& ad = Im3d::GetAppData(); 

    Im3d::NewFrame();
}

void im3d_gl::end_frame_im3d(im3d_state& state, glm::ivec2 screen_dim, camera& cam)
{
    Im3d::EndFrame();
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_PROGRAM_POINT_SIZE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	glViewport(0, 0, (GLsizei)screen_dim.x, (GLsizei)screen_dim.y);

	for (uint32_t i = 0, n = Im3d::GetDrawListCount(); i < n; ++i)
	{
		const Im3d::DrawList& drawList = Im3d::GetDrawLists()[i];

		if (drawList.m_layerId == Im3d::MakeId("NamedLayer"))
		{
			// The application may group primitives into layers, which can be used to change the draw state (e.g. enable depth testing, use a different shader)
		}

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
		glm::mat4 viewProj = cam.m_proj * cam.m_view;
		glUseProgram(sh);
		glUniform2f(glGetUniformLocation(sh, "uViewport"), screen_dim.x, screen_dim.y);
		glUniformMatrix4fv(glGetUniformLocation(sh, "uViewProjMatrix"), 1, false, glm::value_ptr(viewProj));
		glDrawArrays(prim, 0, (GLsizei)drawList.m_vertexCount);
	}

}
