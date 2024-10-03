#pragma once
#include "voxelisation.h"
#include "framebuffer.h"
#include "shape.h"

class camera;

namespace tech {
	class vxgi
	{
	public:

		static void dispatch_gbuffer_voxelization(shader& voxelization, aabb& volume_bounding_box, voxel::grid& voxel_data, framebuffer& gbuffer, framebuffer& lightpass_buffer, glm::ivec2 window_res);
		static void dispatch_gen_voxel_mips(shader& voxelization_mips, voxel::grid& voxel_data, glm::vec3 _3d_tex_res_vec);
		static void dispatch_cone_tracing_pass(
			shader& voxel_cone_tracing, voxel::grid& voxel_data, framebuffer& buffer_conetracing, 
			framebuffer& gbuffer, glm::ivec2 window_res, aabb& bounding_volume, glm::vec3 _3d_tex_res, 
			camera& cam, float max_trace_distance, float resolution_scale);
	};
}