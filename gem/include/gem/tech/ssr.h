#pragma once
#include "gem/shader.h"

class camera;
class framebuffer;
namespace tech
{
	class ssr
	{
	public:

		static void dispatch_ssr_pass(shader& ssr, camera& cam, framebuffer& ssr_buffer, framebuffer& gbuffer, framebuffer& lighting_buffer, glm::vec2 screen_dim);
	};
}