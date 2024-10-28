#pragma once
#include "shader.h"
#include "lights.h"
class framebuffer;
class scene;

namespace tech
{
	class shadow
	{
	public:
		static void dispatch_shadow_pass(framebuffer& shadow_fb, shader& shadow_shader, dir_light& sun, std::vector<scene*>& scenes, glm::ivec2 window_res);
	};
}