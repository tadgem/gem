#pragma once
#include <string>
#include "alias.h"

class shader;
class framebuffer;

namespace tech
{
	class utils
	{
	public:
		static void dispatch_present_image(shader& present_shader, const std::string& uniform_name, const int texture_slot, gl_handle texture);
		static void blit_to_fb(framebuffer& fb, shader& present_shader, const std::string& uniform_name, const int texture_slot, gl_handle texture);
	};
}