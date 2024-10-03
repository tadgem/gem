#pragma once
#include "voxelisation.h"
#include "framebuffer.h"
namespace tech {
	class vxgi
	{
	public:

		static void dispatch_gbuffer_voxelization(shader& voxelization, aabb& volume_bounding_box, voxel::grid& voxel_data, framebuffer& gbuffer, framebuffer& lightpass_buffer, glm::ivec2 window_res);
		static void dispatch_gen_voxel_mips(shader& voxelization_mips, voxel::grid& voxel_data, glm::vec3 _3d_tex_res_vec);
        
	};
}