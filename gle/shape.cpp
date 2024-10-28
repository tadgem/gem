#include "shape.h"
#include "tracy/Tracy.hpp"

VAO shapes::gen_cube_instanced_vao(std::vector<glm::mat4>& matrices, std::vector<glm::vec3>& uvs)
{
    ZoneScoped;
    //  Set up vertex attribute data and attribute pointers
    std::vector<float>  cube_data = {
        -0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
        0.5f, -0.5f,  0.5f,
        0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f,  0.5f, -0.5f,
        0.5f,  0.5f,  0.5f,
        0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
        0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f,  0.5f,
        0.5f,  0.5f,  0.5f,
    };
    // index data
    std::vector<unsigned int> cube_indices = {
        // front and back
        0, 3, 2,
        2, 1, 0,
        4, 5, 6,
        6, 7 ,4,
        // left and right
        11, 8, 9,
        9, 10, 11,
        12, 13, 14,
        14, 15, 12,
        // bottom and top
        16, 17, 18,
        18, 19, 16,
        20, 21, 22,
        22, 23, 20
    };


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
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

    std::size_t vec4Size = sizeof(glm::vec4);
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, matrices_vbo);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);
    glEnableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, matrices_vbo);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(1 * vec4Size));
    glEnableVertexAttribArray(4);
    glBindBuffer(GL_ARRAY_BUFFER, matrices_vbo);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(2 * vec4Size));
    glEnableVertexAttribArray(5);
    glBindBuffer(GL_ARRAY_BUFFER, matrices_vbo);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(3 * vec4Size));

    glVertexAttribDivisor(1, 1);
    glVertexAttribDivisor(2, 1);
    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);
    glVertexAttribDivisor(5, 1);
    return cube_builder.build();


}