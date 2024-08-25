#pragma once
#include <SDL.h>
#undef main
#include "GL/glew.h"
#include "imgui.h"

class engine {
public:
	static void init_gl_sdl();
	static void process_sdl_event();
	static void engine_pre_frame();
	static void engine_post_frame();
	static void engine_shut_down();
	static float get_frame_time();

	inline static SDL_Window*	s_window = nullptr;
	inline static bool			s_quit = false;
	inline static ImGuiIO*		s_imgui_io = nullptr;

private:
	inline static float			s_frametime = 0.0f;
};