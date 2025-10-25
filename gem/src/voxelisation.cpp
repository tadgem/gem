#define GLM_ENABLE_EXPERIMENTAL
#include "gem/voxelisation.h"
#include "gem/backend.h"
#include "gem/gl/gl_dbg.h"
#include "gem/profile.h"
#include "gem/utils.h"
#include "glm/vec3.hpp"
namespace gem {

void Voxel::Grid::UpdateVoxelUnit() {
  ZoneScoped;
  glm::vec3 dim = current_bounding_box.max - current_bounding_box.min;
  voxel_unit =
      glm::vec3((dim.x / resolution.x), (dim.y / resolution.y),
                (dim.z / resolution.z));
}

Voxel::Grid Voxel::CreateGrid(glm::ivec3 resolution, AABB bb) {
  ZoneScoped;
  Grid grid{};
  grid.resolution = resolution;
  grid.current_bounding_box = bb;
  grid.UpdateVoxelUnit();
  grid.voxel_texture = Texture::Create3DTextureEmpty(resolution, GL_RGBA,
                                                        GL_RGBA16F, GL_FLOAT);
  glAssert(glBindImageTexture(0, grid.voxel_texture.handle, 0, GL_TRUE, 0,
                              GL_READ_WRITE, GL_RGBA16F));
  glAssert(glBindImageTexture(1, grid.voxel_texture.handle, 1, GL_TRUE, 0,
                              GL_READ_WRITE, GL_RGBA16F));
  glAssert(glBindImageTexture(2, grid.voxel_texture.handle, 2, GL_TRUE, 0,
                              GL_READ_WRITE, GL_RGBA16F));
  glAssert(glBindImageTexture(3, grid.voxel_texture.handle, 3, GL_TRUE, 0,
                              GL_READ_WRITE, GL_RGBA16F));
  glAssert(glBindImageTexture(4, grid.voxel_texture.handle, 4, GL_TRUE, 0,
                              GL_READ_WRITE, GL_RGBA16F));
  glAssert(glBindImageTexture(5, grid.voxel_texture.handle, 5, GL_TRUE, 0,
                              GL_READ_WRITE, GL_RGBA16F));

  return grid;
}
Voxel::GridVisualizer
Voxel::CreateGridVisualizer(Voxel::Grid &vg, GLShader &visualisation_shader,
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
        instance_matrices.push_back(Utils::GetModelMatrix(
            glm::vec3{x, y, z}, glm::vec3(0.0), scaled_unit));
      }
    }
  }

  VAOBuilder builder;
  builder.Begin();
  builder.AddVertexBuffer(vertex_data);
  builder.AddVertexAttribute(0, 3 * sizeof(float), 3);
  builder.AddIndexBuffer(index_data);
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

  vgv.instance_matrices_ssbo = matrices_ssbo;
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

  vgv.voxel_unit_shape = builder.BuildVAO();
  vgv.required_draw_calls = instance_matrices.size();
  vgv.index_count = index_data.size();
  return vgv;
}

void Voxel::GridVisualizer::Draw(Voxel::Grid &vg, Camera &cam) {
  // compute instance matrices
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, instance_matrices_ssbo);
  compute_instance_matrices_shader.Use();
  // set uniforms
  compute_instance_matrices_shader.SetVec3f("u_resolution", vg.resolution);
  compute_instance_matrices_shader.SetVec3f("u_voxel_unit", vg.voxel_unit);
  compute_instance_matrices_shader.SetVec3f("u_instance_resolution",
                                      glm::vec3(texel_resolution_cubed));
  compute_instance_matrices_shader.SetVec3f("u_current_aabb.min",
                                      vg.current_bounding_box.min);
  compute_instance_matrices_shader.SetVec3f("u_current_aabb.max",
                                      vg.current_bounding_box.max);

  // dispatch
  glm::ivec3 dispatch_dims = vg.resolution / texel_resolution_cubed;
  glAssert(glDispatchCompute(required_draw_calls, 1, 1));

  // draw
  voxel_unit_shape.Use();
  auto &vs = visualization_shader;
  vs.Use();
  vs.SetVec3i("u_texture_resolution", vg.resolution);
  vs.SetVec3i("u_voxel_group_resolution", glm::ivec3(texel_resolution_cubed));
  vs.SetMat4f("u_view_projection", cam.proj_matrix * cam.view_matrix);
  vs.SetMat4f("u_model",
              Utils::GetModelMatrix(
                  vg.current_bounding_box.min + debug_position_offset,
                  glm::vec3(0.0f), vg.voxel_unit * debug_scale));
  vs.SetVec3f("u_aabb.min", vg.current_bounding_box.min);
  vs.SetVec3f("u_aabb.max", vg.current_bounding_box.max);
  vs.SetInt("u_volume", 0);
  Texture::BindSamplerHandle(vg.voxel_texture.handle, GL_TEXTURE0,
                               GL_TEXTURE_3D);
  glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(index_count),
                          GL_UNSIGNED_INT, GL_ZERO, required_draw_calls);
  Texture::BindSamplerHandle(0, GL_TEXTURE0);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
}
} // namespace gem