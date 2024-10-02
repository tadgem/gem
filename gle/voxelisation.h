#pragma once
#include "texture.h"
#include "shape.h"
#include "shader.h"



class voxel
{
public:
	struct grid
	{
		texture		voxel_texture;		// 3D Texture (Voxel Data)
		glm::ivec3	resolution;
		glm::vec3	voxel_unit;		// scale of each texel
		aabb		bounding_box;
	};

	struct grid_visualiser
	{
		shader			visual_shader;
		VAO				texel_shape;
		int				texel_resolution;
		int				total_invocations;
		unsigned int	index_count;
	};

	static grid				create_grid(glm::ivec3 resolution, aabb bb);
	static grid_visualiser	create_grid_visualiser(grid& vg, shader& vvisualisation_shader, int texel_resolution = 8);
};