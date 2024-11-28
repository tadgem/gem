#pragma once
#include "GL/glew.h"
#include "gem/alias.h"
#include "backend.h"
#include <vector>

namespace gem {
// TODO: turn this into a union based on the backend
// vulkan version will have buffer & alloc for vertex buffer and index buffer
struct VAO {
  gl_handle m_vao_id;
  gl_handle m_ibo = 0;
  std::vector<gl_handle> m_vbos;
  void use();
  void release();

};

class vao_builder {
public:
  vao_builder() = default;

  void begin();

  template <typename _Ty> void add_vertex_buffer(_Ty *data, uint32_t count, GLenum usage_flags = GL_STATIC_DRAW) {
    if(gpu_backend::get_backend_api() == backend_api::open_gl) {
      gl_handle vbo;
      glGenBuffers(1, &vbo);

      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      auto data_size = sizeof(_Ty) * count;
      glBufferData(GL_ARRAY_BUFFER, data_size, data, usage_flags);
      m_vbos.push_back(vbo);
    }
  }

  template <typename _Ty> void add_vertex_buffer(std::vector<_Ty> data, GLenum usage_flags = GL_STATIC_DRAW) {
    add_vertex_buffer<_Ty>(&data[0], data.size(), usage_flags);
  }

  void add_index_buffer(uint32_t *data, uint32_t data_count);
  void add_index_buffer(std::vector<uint32_t> data);

  void add_vertex_attribute(uint32_t binding, uint32_t total_vertex_size,
                            uint32_t num_elements, uint32_t element_size = 4,
                            GLenum primitive_type = GL_FLOAT);

  VAO build();

  gl_handle m_vao;
  gl_handle m_ibo;
  std::vector<gl_handle> m_vbos;
  uint32_t m_offset_counter;
};
} // namespace gem