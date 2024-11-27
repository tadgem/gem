#include "gem/shape.h"
#include "gem/profile.h"
#include "gem/asset_manager.h"
#include "gem/model.h"

namespace gem {

VAO shapes::gen_cube_instanced_vao(std::vector<glm::mat4> &matrices,
                                   std::vector<glm::vec3> &uvs) {
  ZoneScoped;
  //  Set up vertex attribute data and attribute pointers
  std::vector<float> cube_data = {
      -0.5f, -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, 0.5f,  0.5f,  -0.5f,
      -0.5f, 0.5f,  -0.5f, -0.5f, -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,
      0.5f,  0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,  -0.5f,
      -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, 0.5f,  -0.5f, 0.5f,  0.5f,
      0.5f,  -0.5f, -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,  0.5f,
      0.5f,  -0.5f, 0.5f,  -0.5f, -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f,
      0.5f,  -0.5f, 0.5f,  -0.5f, -0.5f, 0.5f,  0.5f,  0.5f,  -0.5f,
      -0.5f, 0.5f,  -0.5f, -0.5f, 0.5f,  0.5f,  0.5f,  0.5f,  0.5f,
  };
  // index data
  std::vector<unsigned int> cube_indices = {
      // front and back
      0, 3, 2, 2, 1, 0, 4, 5, 6, 6, 7, 4,
      // left and right
      11, 8, 9, 9, 10, 11, 12, 13, 14, 14, 15, 12,
      // bottom and top
      16, 17, 18, 18, 19, 16, 20, 21, 22, 22, 23, 20};

  vao_builder cube_builder;
  cube_builder.begin();
  cube_builder.add_vertex_buffer(cube_data);
  cube_builder.add_vertex_attribute(0, 3 * sizeof(float), 3);
  cube_builder.add_vertex_buffer(matrices);
  auto matrices_vbo = cube_builder.m_vbos.back();
  cube_builder.add_vertex_buffer(uvs);
  auto uvs_vbo = cube_builder.m_vbos.back();
  cube_builder.add_index_buffer(cube_indices);

  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, uvs_vbo);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);

  std::size_t vec4Size = sizeof(glm::vec4);
  glEnableVertexAttribArray(2);
  glBindBuffer(GL_ARRAY_BUFFER, matrices_vbo);
  glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void *)0);
  glEnableVertexAttribArray(3);
  glBindBuffer(GL_ARRAY_BUFFER, matrices_vbo);
  glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size,
                        (void *)(1 * vec4Size));
  glEnableVertexAttribArray(4);
  glBindBuffer(GL_ARRAY_BUFFER, matrices_vbo);
  glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size,
                        (void *)(2 * vec4Size));
  glEnableVertexAttribArray(5);
  glBindBuffer(GL_ARRAY_BUFFER, matrices_vbo);
  glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size,
                        (void *)(3 * vec4Size));

  glVertexAttribDivisor(1, 1);
  glVertexAttribDivisor(2, 1);
  glVertexAttribDivisor(3, 1);
  glVertexAttribDivisor(4, 1);
  glVertexAttribDivisor(5, 1);
  return cube_builder.build();
}

void shapes::init_built_in_assets(asset_manager& am) {
  ZoneScoped;
  {
    std::vector<float> screen_quad_verts{
        // positions        texture coords
        1.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top right
        1.0f,  -1.0f, 0.0f, 1.0f, 0.0f, // bottom right
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // bottom left
        -1.0f, 1.0f,  0.0f, 0.0f, 1.0f  // top left
    };

    std::vector<unsigned int> screen_quad_indices = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    vao_builder screen_quad_builder;
    screen_quad_builder.begin();
    screen_quad_builder.add_vertex_buffer(screen_quad_verts);
    screen_quad_builder.add_vertex_attribute(0, 5 * sizeof(float), 3);
    screen_quad_builder.add_vertex_attribute(1, 5 * sizeof(float), 2);
    screen_quad_builder.add_index_buffer(screen_quad_indices);
    s_screen_quad = screen_quad_builder.build();
  }

  {
    //  Set up vertex attribute data and attribute pointers
    std::vector<float> cube_data = {
        -0.5f, -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, 0.5f,  0.5f,  -0.5f,
        -0.5f, 0.5f,  -0.5f, -0.5f, -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,
        0.5f,  0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,  -0.5f,
        -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, 0.5f,  -0.5f, 0.5f,  0.5f,
        0.5f,  -0.5f, -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,  0.5f,
        0.5f,  -0.5f, 0.5f,  -0.5f, -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f,
        0.5f,  -0.5f, 0.5f,  -0.5f, -0.5f, 0.5f,  0.5f,  0.5f,  -0.5f,
        -0.5f, 0.5f,  -0.5f, -0.5f, 0.5f,  0.5f,  0.5f,  0.5f,  0.5f,
    };
    // index data
    std::vector<unsigned int> cube_indices = {
        // front and back
        0, 3, 2, 2, 1, 0, 4, 5, 6, 6, 7, 4,
        // left and right
        11, 8, 9, 9, 10, 11, 12, 13, 14, 14, 15, 12,
        // bottom and top
        16, 17, 18, 18, 19, 16, 20, 21, 22, 22, 23, 20};

    vao_builder cube_builder;
    cube_builder.begin();
    cube_builder.add_vertex_buffer(cube_data);
    cube_builder.add_index_buffer(cube_indices);
    cube_builder.add_vertex_attribute(0, 3 * sizeof(float), 3);

    s_cube_pos_only = cube_builder.build();
  }

  glm::vec3 vertices[] = {
      /* 0, 1, 2, 3 */
      { -0.5f, -0.5f, 0.5f },
      {  0.5f, -0.5f, 0.5f },
      { -0.5f,  0.5f, 0.5f },
      {  0.5f,  0.5f, 0.5f },

      /* 4, 5, 6, 7 */
      { -0.5f, -0.5f, -0.5f },
      {  0.5f, -0.5f, -0.5f },
      { -0.5f,  0.5f, -0.5f },
      {  0.5f,  0.5f, -0.5f },
  };

  uint32_t indices[] = {
      0, 1, 3, /* front */
      0, 3, 2,

      1, 5, 7, /* right */
      1, 7, 3,

      5, 4, 6, /* back */
      5, 6, 7,

      4, 0, 2, /* left */
      4, 2, 6,

      4, 5, 1, /* bottom */
      4, 1, 0,

      2, 3, 7, /* top */
      2, 7, 6,
  };

  glm::vec3 vertAttribs[sizeof(indices)*2];

  for (int i = 0; i < sizeof(indices); i += 3) {
    GLubyte vertA = indices[i];
    GLubyte vertB = indices[i+1];
    GLubyte vertC = indices[i+2];

    glm::vec3 edgeAB;
    glm::vec3 edgeAC;

    /* AB = B - A
     * AC = C - A */
    edgeAB = vertices[vertB] - vertices[vertA];
    edgeAC = vertices[vertC] - vertices[vertA];

    /* normal = norm(cross(AB, AC)) */
    glm::vec3 normal {};
    glm::normalize(glm::cross(edgeAB, edgeAC));

    /* copy vertex postions into first half array */
    vertAttribs[i]   = vertices[vertA];
    vertAttribs[i+1] = vertices[vertB];
    vertAttribs[i+2] = vertices[vertC];

    vertAttribs[i+36]   = normal;
    vertAttribs[i+1+36]   = normal;
    vertAttribs[i+2+36]   = normal;
  }

  vao_builder cube_with_normals_builder;
  cube_with_normals_builder.begin();
  cube_with_normals_builder.add_vertex_buffer(&vertAttribs[0], sizeof(indices) * 2);
  cube_with_normals_builder.add_index_buffer(&indices[0], sizeof(indices));
  /* postions */
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);
  /* normals */
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)(36 * sizeof(glm::vec3)));
  glEnableVertexAttribArray(1);

  s_cube_vao = cube_with_normals_builder.build();
  aabb cube_bb{{0,0,0}, {1,1,1}};
  s_cube_mesh = mesh {s_cube_vao, 36, cube_bb, cube_bb, 0};


  model cube_model {};
  cube_model.m_meshes.push_back(s_cube_mesh);
  cube_model.m_aabb = s_cube_mesh.m_original_aabb;

  am.provide_asset<model, asset_type::model>("cube", cube_model);

}
} // namespace gem