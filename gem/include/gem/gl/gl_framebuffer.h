#pragma once
#include "GL/glew.h"
#include "gem/alias.h"
#include "glm.hpp"
#include <array>
#include <cstdint>
#include <vector>

namespace gem {
class GLFramebuffer {
public:
  GLFramebuffer();

  void Bind();
  void Unbind();
  void Cleanup();

  void AddColourAttachment(GLenum attachment_index, uint32_t width,
                             uint32_t height, GLenum internal_format,
                             GLenum format, GLenum filter = GL_LINEAR,
                             GLenum pixel_format = GL_UNSIGNED_BYTE);

  void AddDepthAttachment(uint32_t width, uint32_t height,
                            GLenum format = GL_DEPTH24_STENCIL8);

  void
  AddDepthAttachmentSamplerFriendly(uint32_t width, uint32_t height,
                                        GLenum format = GL_DEPTH24_STENCIL8);
  void Check();

  template <typename _Ty, size_t w, size_t h>
  std::array<_Ty, w * h> ReadPixels(u32 x, u32 y, u32 attachment_index,
                                     GLenum pixel_format, GLenum pixel_type) {
    std::array<_Ty, w * h> out_data{};
    Bind();
    glReadBuffer(GL_COLOR_ATTACHMENT0 + attachment_index);
    glReadPixels(x, y, w, h, pixel_format, pixel_type, &out_data[0]);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    Unbind();
    return out_data;
  }

  struct GLAttachmentInfo {
    GLenum internal_format;
    GLenum format;
    GLenum filter;
    GLenum pixel_format;
  };

  static GLFramebuffer Create(glm::vec2 resolution,
                               std::vector<GLAttachmentInfo> colour_attachments,
                               bool add_depth = true);

  gl_handle handle;
  std::vector<gl_handle> colour_attachments;
  gl_handle depth_attachment;
  uint32_t framebuffer_width, framebuffer_height;
};
} // namespace gem