#pragma once
#include "gem/shader.h"
#include "gem/framebuffer.h"

class camera;
class scene;
class asset_manager; 

namespace tech
{
	class gbuffer
	{
	public:

		static void dispatch_gbuffer(u32 frame_index, framebuffer& gbuffer, framebuffer& previous_position_buffer, shader& gbuffer_shader, asset_manager& am, camera& cam, std::vector<scene*>& scenes, glm::ivec2 win_res);
		static void dispatch_gbuffer_with_id(u32 frame_index, framebuffer& gbuffer, framebuffer& previous_position_buffer, shader& gbuffer_shader, asset_manager& am, camera& cam, std::vector<scene*>& scenes, glm::ivec2 win_res);
	};
}