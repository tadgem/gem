#include "voxelisation.h"
#include "gl.h"
#include "utils.h"

voxel::grid voxel::create_grid(glm::ivec3 resolution, aabb bb)
{
	grid grid{};
	glm::vec3 aabb_dim = bb.max - bb.min;
	grid.voxel_unit = glm::vec3((aabb_dim.x / resolution.x), (aabb_dim.y / resolution.y), (aabb_dim.z / resolution.z));
	grid.resolution = resolution;
	grid.bounding_box = bb;	
	grid.texture = texture::create_3d_texture_empty(resolution, GL_RGBA, GL_RGBA16F, GL_FLOAT);
	glAssert(glBindImageTexture(0, grid.texture.m_handle, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16F));
	return grid;
}

voxel::grid_visualiser voxel::create_grid_visualiser(voxel::grid& vg, shader& visualisation_shader, int texel_resolution)
{
	grid_visualiser vgv{ visualisation_shader, {}, texel_resolution, 0 };
	// create the voxel texel shape
	std::vector<float>			vertex_data;
	std::vector<unsigned int>   index_data;
	unsigned int current_cube = 0;

	for (int z = 0; z < texel_resolution; z++)
	{
		for (int y = 0; y < texel_resolution; y++)
		{
			for (int x = 0; x < texel_resolution; x++)
			{
				glm::vec3 current_pos = glm::vec3(x, y, z);

				std::vector<float>  cube_data = {
					current_pos.x + -0.5f,	current_pos.y + -0.5f,	current_pos.z + -0.5f,
					current_pos.x + 0.5f,	current_pos.y + -0.5f,	current_pos.z + -0.5f,
					current_pos.x + 0.5f,	current_pos.y + 0.5f,	current_pos.z + -0.5f,
					current_pos.x + -0.5f,  current_pos.y + 0.5f,	current_pos.z + -0.5f,
					current_pos.x + -0.5f,	current_pos.y + -0.5f,  current_pos.z + 0.5f,
					current_pos.x + 0.5f,	current_pos.y + -0.5f,  current_pos.z + 0.5f,
					current_pos.x + 0.5f,	current_pos.y + 0.5f,	current_pos.z + 0.5f,
					current_pos.x + -0.5f,  current_pos.y + 0.5f,	current_pos.z + 0.5f,
					current_pos.x + -0.5f,  current_pos.y + 0.5f,	current_pos.z + -0.5f,
					current_pos.x + -0.5f,	current_pos.y + -0.5f,	current_pos.z + -0.5f,
					current_pos.x + -0.5f,	current_pos.y + -0.5f,  current_pos.z + 0.5f,
					current_pos.x + -0.5f,  current_pos.y + 0.5f,	current_pos.z + 0.5f,
					current_pos.x + 0.5f,	current_pos.y + -0.5f,	current_pos.z + -0.5f,
					current_pos.x + 0.5f,	current_pos.y + 0.5f,	current_pos.z + -0.5f,
					current_pos.x + 0.5f,	current_pos.y + 0.5f,	current_pos.z + 0.5f,
					current_pos.x + 0.5f,	current_pos.y + -0.5f,  current_pos.z + 0.5f,
					current_pos.x + -0.5f,	current_pos.y + -0.5f,	current_pos.z + -0.5f,
					current_pos.x + 0.5f,	current_pos.y + -0.5f,	current_pos.z + -0.5f,
					current_pos.x + 0.5f,	current_pos.y + -0.5f,  current_pos.z + 0.5f,
					current_pos.x + -0.5f,	current_pos.y + -0.5f,  current_pos.z + 0.5f,
					current_pos.x + 0.5f,	current_pos.y + 0.5f,	current_pos.z + -0.5f,
					current_pos.x + -0.5f,  current_pos.y + 0.5f,	current_pos.z + -0.5f,
					current_pos.x + -0.5f,  current_pos.y + 0.5f,	current_pos.z + 0.5f,
					current_pos.x + 0.5f,	current_pos.y + 0.5f,	current_pos.z + 0.5f,
					};

				std::vector<unsigned int> cube_indices = {
					// front and back
					current_cube + 0, current_cube + 3,current_cube +  2,
					current_cube + 2, current_cube + 1,current_cube +  0,
					current_cube + 4, current_cube + 5,current_cube +  6,
					current_cube + 6, current_cube + 7 ,current_cube + 4,
					// left and right
					current_cube + 11, current_cube + 8, current_cube + 9,
					current_cube + 9,  current_cube + 10, current_cube + 11,
					current_cube + 12, current_cube + 13, current_cube + 14,
					current_cube + 14, current_cube + 15, current_cube + 12,
					// bottom and top
					current_cube + 16, current_cube + 17, current_cube + 18,
					current_cube + 18, current_cube + 19, current_cube + 16,
					current_cube + 20, current_cube + 21, current_cube + 22,
					current_cube + 22, current_cube + 23, current_cube + 20
				};

				index_data.insert(index_data.end(), cube_indices.begin(), cube_indices.end());
				vertex_data.insert(vertex_data.end(), cube_data.begin(), cube_data.end());
				// number of vertices in a cube
				current_cube += 24;
			}
		}
	}

	const unsigned int total_voxels = vg.resolution.x * vg.resolution.y * vg.resolution.z;
	const unsigned int voxels_per_shape = texel_resolution * texel_resolution * texel_resolution;
	const unsigned int total_instances = total_voxels / voxels_per_shape;

	glm::ivec3 scaled_resolution = vg.resolution / glm::ivec3(texel_resolution);

	std::vector<glm::mat4> instance_matrices;
	auto scaled_unit = glm::vec3{ vg.voxel_unit.x, vg.voxel_unit.y, vg.voxel_unit.z };
	for (auto i = 0; i < total_instances; i++)
	{
		// instance vbo is per-instance transform
		float z = vg.bounding_box.min.z;
		float y = vg.bounding_box.min.y;
		float x = vg.bounding_box.min.x;

		float z_offset = i / (scaled_resolution.x * scaled_resolution.y);
		float y_offset = (i / scaled_resolution.x) % scaled_resolution.y;
		float x_offset = i % scaled_resolution.x;

		float z_offset2 = z_offset * (vg.voxel_unit.z * texel_resolution);
		float y_offset2 = y_offset * (vg.voxel_unit.y * texel_resolution);
		float x_offset2 = x_offset * (vg.voxel_unit.x * texel_resolution);

		z += z_offset2;
		y += y_offset2;
		x += x_offset2;


		instance_matrices.push_back(utils::get_model_matrix({ x,y,z }, { 0,0,0 }, scaled_unit));
	}

	vao_builder builder;
	builder.begin();
	builder.add_vertex_buffer(vertex_data);
	builder.add_vertex_attribute(0, 3 * sizeof(float), 3);
	builder.add_vertex_buffer(instance_matrices);
	auto matrices_vbo = builder.m_vbos.back();
	builder.add_index_buffer(index_data);

	constexpr std::size_t vec4Size = sizeof(glm::vec4);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, matrices_vbo);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, matrices_vbo);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(1 * vec4Size));
	glEnableVertexAttribArray(3);
	glBindBuffer(GL_ARRAY_BUFFER, matrices_vbo);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(2 * vec4Size));
	glEnableVertexAttribArray(4);
	glBindBuffer(GL_ARRAY_BUFFER, matrices_vbo);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(3 * vec4Size));

	glVertexAttribDivisor(1, 1);
	glVertexAttribDivisor(2, 1);
	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);

	vgv.texel_shape = builder.build();
	vgv.total_invocations = instance_matrices.size();
	vgv.index_count = index_data.size();
	return vgv;
}
