#pragma once
#include "shader.h"
#include "framebuffer.h"

class camera;
class scene;

namespace tech
{
	class gbuffer
	{
	public:

		static void dispatch_gbuffer(u32 frame_index, framebuffer& gbuffer, framebuffer& previous_position_buffer, shader& gbuffer_shader, camera& cam, scene& current_scene, glm::ivec2 win_res);
		static void dispatch_gbuffer_with_id(u32 frame_index, framebuffer& gbuffer, framebuffer& previous_position_buffer, shader& gbuffer_shader,  camera& cam, scene& current_scene, glm::ivec2 win_res);
	};
}