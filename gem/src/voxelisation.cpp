#define GLM_ENABLE_EXPERIMENTAL
#include "gem/voxelisation.h"
#include "gem/backend.h"
#include "gem/gl/gl_dbg.h"
#include "gem/profile.h"
#include "gem/utils.h"
#include "glm/vec3.hpp"
namespace gem {

void Voxel::Grid::update_voxel_unit() {
  ZoneScoped;
  glm::vec3 dim = current_bounding_box.m_max - current_bounding_box.m_min;
  voxel_unit =
      glm::vec3((dim.x / resolution.x), (dim.y / resolution.y),
                (dim.z / resolution.z));
}

Voxel::Grid Voxel::create_grid(glm::ivec3 resolution, AABB bb) {
  ZoneScoped;
  Grid grid{};
  grid.resolution = resolution;
  grid.current_bounding_box = bb;
  grid.update_voxel_unit();
  grid.voxel_texture = Texture::create_3d_texture_empty(resolution, GL_RGBA,
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
Voxel::GridVisualizer
Voxel::create_grid_visualiser(Voxel::Grid &vg, GLShader &visualisation_shader,
                              GLShader &compute_matrices_shader,
                              int texel_resolution) {
  ZoneScoped;
  GridVisualizer vgv{visualisation_shader,
                      compute_matrices_shader,
                      {},
                      0,
                      texel_resolution,
                      0};
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

  std::vector<glm::mat4> instance_matrices;
  auto scaled_unit =
      glm::vec3{vg.voxel_unit.x, vg.voxel_unit.y, vg.voxel_unit.z};

  for (int z = 0; z < vg.resolution.z; z += texel_resolution) {
    for (int y = 0; y < vg.resolution.y; y += texel_resolution) {
      for (int x = 0; x < vg.resolution.x; x += texel_resolution) {
        instance_matrices.push_back(Utils::get_model_matrix(
            glm::vec3{x, y, z}, glm::vec3(0.0), scaled_unit));
      }
    }
  }

  VAOBuilder builder;
  builder.begin();
  builder.add_vertex_buffer(vertex_data);
  builder.add_vertex_attribute(0, 3 * sizeof(float), 3);
  builder.add_index_buffer(index_data);
  // builder.add_vertex_buffer(instance_matrices, GL_STATIC_DRAW);
  // TODO: create an ssbo helper class
  GLuint matrices_ssbo;
  glGenBuffers(1, &matrices_ssbo);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, matrices_ssbo);
  glBufferData(GL_SHADER_STORAGE_BUFFER,
               sizeof(glm::mat4) * instance_matrices.size(),
               instance_matrices.data(), GL_DYNAMIC_COPY);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, matrices_ssbo);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

  vgv.m_instance_matrices_ssbo = matrices_ssbo;
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
  vgv.m_total_invocations = instance_matrices.size();
  vgv.m_index_count = index_data.size();
  return vgv;
}

void Voxel::GridVisualizer::dispatch_draw(Voxel::Grid &vg, Camera &cam) {
  // compute instance matrices
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_instance_matrices_ssbo);
  m_compute_instances_shader.use();
  // set uniforms
  m_compute_instances_shader.set_vec3("u_resolution", vg.resolution);
  m_compute_instances_shader.set_vec3("u_voxel_unit", vg.voxel_unit);
  m_compute_instances_shader.set_vec3("u_instance_resolution",
                                      glm::vec3(m_texel_resolution));
  m_compute_instances_shader.set_vec3("u_current_aabb.min",
                                      vg.current_bounding_box.m_min);
  m_compute_instances_shader.set_vec3("u_current_aabb.max",
                                      vg.current_bounding_box.m_max);

  // dispatch
  glm::ivec3 dispatch_dims = vg.resolution / m_texel_resolution;
  glAssert(glDispatchCompute(m_total_invocations, 1, 1));

  // draw
  m_texel_shape.use();
  auto &vs = m_visual_shader;
  vs.use();
  vs.set_ivec3("u_texture_resolution", vg.resolution);
  vs.set_ivec3("u_voxel_group_resolution", glm::ivec3(m_texel_resolution));
  vs.set_mat4("u_view_projection", cam.m_proj * cam.m_view);
  vs.set_mat4("u_model",
              Utils::get_model_matrix(
                  vg.current_bounding_box.m_min + m_debug_position_offset,
                  glm::vec3(0.0f), vg.voxel_unit * m_debug_scale));
  vs.set_vec3("u_aabb.min", vg.current_bounding_box.m_min);
  vs.set_vec3("u_aabb.max", vg.current_bounding_box.m_max);
  vs.set_int("u_volume", 0);
  Texture::bind_sampler_handle(vg.voxel_texture.m_handle, GL_TEXTURE0,
                               GL_TEXTURE_3D);
  glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(m_index_count),
                          GL_UNSIGNED_INT, GL_ZERO, m_total_invocations);
  Texture::bind_sampler_handle(0, GL_TEXTURE0);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
}
} // namespace gem