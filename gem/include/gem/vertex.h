#pragma once
#include "GL/glew.h"
#include "backend.h"
#include "gem/alias.h"
#include <vector>

namespace gem {
// TODO: turn this into a union based on the backend
// vulkan version will have buffer & alloc for vertex buffer and index buffer
struct VAO {
  gl_handle m_vao_id;
  gl_handle m_ibo = INVALID_GL_HANDLE;
  uint32_t m_index_count;
  std::vector<gl_handle> m_vbos;
  void Use();
  void Draw();
  void Release();
};

class VAOBuilder {
public:
  VAOBuilder() = default;

  void Begin();

  template <typename _Ty>
  void AddVertexBuffer(_Ty *data, uint32_t count,
                         GLenum usage_flags = GL_STATIC_DRAW) {
    if (GPUBackend::GetBackendAPI() == BackendAPI::open_gl) {
      gl_handle vbo;
      glGenBuffers(1, &vbo);

      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      auto data_size = sizeof(_Ty) * count;
      glBufferData(GL_ARRAY_BUFFER, data_size, data, usage_flags);
      m_vbos.push_back(vbo);
    }
  }

  template <typename _Ty>
  void AddVertexBuffer(std::vector<_Ty> data,
                         GLenum usage_flags = GL_STATIC_DRAW) {
    AddVertexBuffer<_Ty>(&data[0], data.size(), usage_flags);
  }

  void AddIndexBuffer(uint32_t *data, uint32_t data_count);
  void AddIndexBuffer(std::vector<uint32_t> data);

  void AddVertexAttribute(uint32_t binding, uint32_t total_vertex_size,
                            uint32_t num_elements, uint32_t element_size = 4,
                            GLenum primitive_type = GL_FLOAT);

  VAO BuildVAO();

  gl_handle m_vao;
  gl_handle m_ibo = INVALID_GL_HANDLE;
  uint32_t m_index_count = 0;
  std::vector<gl_handle> m_vbos;
  uint32_t m_offset_counter;
};
} // namespace gem