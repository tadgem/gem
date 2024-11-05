#define GLM_ENABLE_EXPERIMENTAL
#include "gem/voxelisation.h"
#include "gem/backend.h"
#include "gem/profile.h"
#include "gem/utils.h"
#include "glm/vec3.hpp"
namespace gem {

void voxel::grid::update_voxel_unit() {
  ZoneScoped;
  glm::vec3 aabb_dim = current_bounding_box.max - current_bounding_box.min;
  voxel_unit =
      glm::vec3((aabb_dim.x / resolution.x), (aabb_dim.y / resolution.y),
                (aabb_dim.z / resolution.z));
}

void voxel::grid::update_grid_position(gem::camera& cam) {
  ZoneScoped;
  current_bounding_box.max = cam.m_pos + (aabb_dim * 0.5f);
  current_bounding_box.min = cam.m_pos - (aabb_dim * 0.5f);
}

voxel::grid voxel::create_grid(glm::ivec3 resolution, aabb bb) {
  ZoneScoped;
  grid grid{};
  grid.resolution = resolution;
  grid.update_voxel_unit();
  grid.voxel_texture = texture::create_3d_texture_empty(resolution, GL_RGBA,
                                                        GL_RGBA16F, GL_FLOAT);
  glAssert(glBindImageTexture(0, grid.voxel_texture.m_handle, 0, GL_TRUE, 0,
                              GL_READ_WRITE, GL_RGBA16F));
  glAssert(glBindImageTexture(1, grid.voxel_texture.m_handle, 1, GL_TRUE, 0,
                              GL_READ_WRITE, GL_RGBA16F));
  glAssert(glBindImageTexture(2, grid.voxel_texture.m_handle, 2, GL_TRUE, 0,
                              GL_READ_WRITE, GL_RGBA16F));
  glAssert(glBindImageTexture(3, grid.voxel_texture.m_handle, 3, GL_TRUE, 0,
                              GL_READ_WRITE, GL_RGBA16F));
  glAssert(glBindImageTexture(4, grid.voxel_texture.m_handle, 4, GL_TRUE, 0,
                              GL_READ_WRITE, GL_RGBA16F));
  glAssert(glBindImageTexture(5, grid.voxel_texture.m_handle, 5, GL_TRUE, 0,
                              GL_READ_WRITE, GL_RGBA16F));

  return grid;
}

voxel::grid_visualiser
voxel::create_grid_visualiser(voxel::grid &vg, shader &visualisation_shader,
                              int texel_resolution) {
  ZoneScoped;
  grid_visualiser vgv{visualisation_shader, {}, texel_resolution, 0};
  // create the voxel texel shape
  std::vector<float> vertex_data;
  std::vector<unsigned int> index_data;
  unsigned int current_cube = 0;

  for (int z = 0; z < texel_resolution; z++) {
    for (int y = 0; y < texel_resolution; y++) {
      for (int x = 0; x < texel_resolution; x++) {
        glm::vec3 current_pos = glm::vec3(x, y, z);

        std::vector<float> cube_data = {
            current_pos.x + -0.5f, current_pos.y + -0.5f, current_pos.z + -0.5f,
            current_pos.x + 0.5f,  current_pos.y + -0.5f, current_pos.z + -0.5f,
            current_pos.x + 0.5f,  current_pos.y + 0.5f,  current_pos.z + -0.5f,
            current_pos.x + -0.5f, current_pos.y + 0.5f,  current_pos.z + -0.5f,
            current_pos.x + -0.5f, current_pos.y + -0.5f, current_pos.z + 0.5f,
            current_pos.x + 0.5f,  current_pos.y + -0.5f, current_pos.z + 0.5f,
            current_pos.x + 0.5f,  current_pos.y + 0.5f,  current_pos.z + 0.5f,
            current_pos.x + -0.5f, current_pos.y + 0.5f,  current_pos.z + 0.5f,
            current_pos.x + -0.5f, current_pos.y + 0.5f,  current_pos.z + -0.5f,
            current_pos.x + -0.5f, current_pos.y + -0.5f, current_pos.z + -0.5f,
            current_pos.x + -0.5f, current_pos.y + -0.5f, current_pos.z + 0.5f,
            current_pos.x + -0.5f, current_pos.y + 0.5f,  current_pos.z + 0.5f,
            current_pos.x + 0.5f,  current_pos.y + -0.5f, current_pos.z + -0.5f,
            current_pos.x + 0.5f,  current_pos.y + 0.5f,  current_pos.z + -0.5f,
            current_pos.x + 0.5f,  current_pos.y + 0.5f,  current_pos.z + 0.5f,
            current_pos.x + 0.5f,  current_pos.y + -0.5f, current_pos.z + 0.5f,
            current_pos.x + -0.5f, current_pos.y + -0.5f, current_pos.z + -0.5f,
            current_pos.x + 0.5f,  current_pos.y + -0.5f, current_pos.z + -0.5f,
            current_pos.x + 0.5f,  current_pos.y + -0.5f, current_pos.z + 0.5f,
            current_pos.x + -0.5f, current_pos.y + -0.5f, current_pos.z + 0.5f,
            current_pos.x + 0.5f,  current_pos.y + 0.5f,  current_pos.z + -0.5f,
            current_pos.x + -0.5f, current_pos.y + 0.5f,  current_pos.z + -0.5f,
            current_pos.x + -0.5f, current_pos.y + 0.5f,  current_pos.z + 0.5f,
            current_pos.x + 0.5f,  current_pos.y + 0.5f,  current_pos.z + 0.5f,
        };

        std::vector<unsigned int> cube_indices = {
            // front and back
            current_cube + 0, current_cube + 3, current_cube + 2,
            current_cube + 2, current_cube + 1, current_cube + 0,
            current_cube + 4, current_cube + 5, current_cube + 6,
            current_cube + 6, current_cube + 7, current_cube + 4,
            // left and right
            current_cube + 11, current_cube + 8, current_cube + 9,
            current_cube + 9, current_cube + 10, current_cube + 11,
            current_cube + 12, current_cube + 13, current_cube + 14,
            current_cube + 14, current_cube + 15, current_cube + 12,
            // bottom and top
            current_cube + 16, current_cube + 17, current_cube + 18,
            current_cube + 18, current_cube + 19, current_cube + 16,
            current_cube + 20, current_cube + 21, current_cube + 22,
            current_cube + 22, current_cube + 23, current_cube + 20};

        index_data.insert(index_data.end(), cube_indices.begin(),
                          cube_indices.end());
        vertex_data.insert(vertex_data.end(), cube_data.begin(),
                           cube_data.end());
        // number of vertices in a cube
        current_cube += 24;
      }
    }
  }

  const unsigned int total_voxels =
      vg.resolution.x * vg.resolution.y * vg.resolution.z;
  const unsigned int voxels_per_shape =
      texel_resolution * texel_resolution * texel_resolution;
  const unsigned int total_instances = total_voxels / voxels_per_shape;

  glm::ivec3 scaled_resolution = vg.resolution / glm::ivec3(texel_resolution);

  std::vector<glm::vec3> instance_positions;
  auto scaled_unit =
      glm::vec3{vg.voxel_unit.x, vg.voxel_unit.y, vg.voxel_unit.z};

  for(int z = 0; z < vg.resolution.z; z += texel_resolution)
  {
    for(int y = 0; y < vg.resolution.y; y += texel_resolution)
    {
      for(int x = 0; x < vg.resolution.x; x += texel_resolution)
      {
        // glm::mat4 model = utils::get_model_matrix({x, y, z}, {0, 0, 0}, glm::vec3(1.0));
        glm::vec3 pos {x, y, z};
        instance_positions.push_back(pos);
      }
    }
  }

  vao_builder builder;
  builder.begin();
  builder.add_vertex_buffer(vertex_data);
  builder.add_vertex_attribute(0, 3 * sizeof(float), 3);
  builder.add_index_buffer(index_data);
  builder.add_vertex_buffer(instance_positions);
  auto matrices_vbo = builder.m_vbos.back();

  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, matrices_vbo);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);
  glVertexAttribDivisor(1, 1);

//  constexpr std::size_t vec4Size = sizeof(glm::vec4);
//  glEnableVertexAttribArray(1);
//  glBindBuffer(GL_ARRAY_BUFFER, matrices_vbo);
//  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void *)0);
//  glEnableVertexAttribArray(2);
//  glBindBuffer(GL_ARRAY_BUFFER, matrices_vbo);
//  glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size,
//                        (void *)(1 * vec4Size));
//  glEnableVertexAttribArray(3);
//  glBindBuffer(GL_ARRAY_BUFFER, matrices_vbo);
//  glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size,
//                        (void *)(2 * vec4Size));
//  glEnableVertexAttribArray(4);
//  glBindBuffer(GL_ARRAY_BUFFER, matrices_vbo);
//  glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size,
//                        (void *)(3 * vec4Size));
//
//  glVertexAttribDivisor(1, 1);
//  glVertexAttribDivisor(2, 1);
//  glVertexAttribDivisor(3, 1);
//  glVertexAttribDivisor(4, 1);

  vgv.m_texel_shape = builder.build();
  vgv.m_total_invocations = instance_positions.size();
  vgv.m_index_count = index_data.size();
  return vgv;
}

void voxel::grid_visualiser::dispatch_draw(voxel::grid& vg, camera& cam)
{
  m_texel_shape.use();
  auto& vs = m_visual_shader;
  vs.use();
  vs.set_ivec3("u_texture_resolution", vg.resolution);
  vs.set_ivec3("u_voxel_group_resolution", glm::ivec3(m_texel_resolution));
  vs.set_mat4("u_view_projection", cam.m_proj * cam.m_view);
  vs.set_mat4("u_model", utils::get_model_matrix(vg.current_bounding_box.min + m_debug_position_offset, glm::vec3(0.0f), vg.voxel_unit * m_debug_scale));
  vs.set_vec3("u_aabb.min", vg.current_bounding_box.min);
  vs.set_vec3("u_aabb.max", vg.current_bounding_box.max);
  vs.set_int("u_volume", 0);
  texture::bind_sampler_handle(vg.voxel_texture.m_handle, GL_TEXTURE0, GL_TEXTURE_3D);
  glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(m_index_count), GL_UNSIGNED_INT, GL_ZERO,
                          m_total_invocations);
  texture::bind_sampler_handle(0, GL_TEXTURE0);
}
} // namespace gem