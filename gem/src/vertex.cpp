#include "gem/vertex.h"
#include "gem/profile.h"

namespace gem {

void VAO::Use() {
  ZoneScoped;
  glBindVertexArray(vao_handle);
}
void VAO::Release() {
  ZoneScoped;
  if (ibo_handle > 0) {
    glDeleteBuffers(1, &ibo_handle);
  }
  if (!vbos.empty()) {
    glDeleteBuffers(vbos.size(), vbos.data());
  }
  glDeleteVertexArrays(1, &vao_handle);
}
void VAO::Draw() {
  Use();
  if (ibo_handle != INVALID_GL_HANDLE) {
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(index_count),
                   GL_UNSIGNED_INT, 0);
  } else {
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(index_count));
  }
}

void VAOBuilder::Begin() {
  ZoneScoped;
  offset_counter_ = 0;
  ibo_handle = INVALID_GL_HANDLE;
  vbos.clear();
  glGenVertexArrays(1, &vao_handle);
  glBindVertexArray(vao_handle);
}

void VAOBuilder::AddIndexBuffer(uint32_t *data, uint32_t data_count) {
  ZoneScoped;
  gl_handle ibo;
  glGenBuffers(1, &ibo);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * data_count, data,
               GL_STATIC_DRAW);
  ibo_handle = ibo;
  index_count = data_count;
}

void VAOBuilder::AddIndexBuffer(std::vector<uint32_t> data) {
  ZoneScoped;
  AddIndexBuffer(data.data(), data.size());
}

void VAOBuilder::AddVertexAttribute(uint32_t binding,
                                       uint32_t total_vertex_size,
                                       uint32_t num_elements,
                                       uint32_t element_size,
                                       GLenum primitive_type) {
  ZoneScoped;
  glVertexAttribPointer(binding, num_elements, primitive_type, GL_FALSE,
                        total_vertex_size, (void *)offset_counter_);
  glEnableVertexAttribArray(binding);
  // glBindBuffer(GL_ARRAY_BUFFER, m_vbos.back());
  offset_counter_ += num_elements * element_size;
}

VAO VAOBuilder::BuildVAO() {
  ZoneScoped;
  return VAO{vao_handle, ibo_handle, index_count, vbos};
}
} // namespace gem