#include "gem/gl/gl_framebuffer.h"
#include "gem/profile.h"
#include <iostream>

namespace gem {

GLFramebuffer::GLFramebuffer() {
  ZoneScoped;
  handle = INVALID_GL_HANDLE;
}

void GLFramebuffer::Unbind() {
  ZoneScoped;
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GLFramebuffer::Cleanup() {
  ZoneScoped;
  glDeleteFramebuffers(1, &handle);
}

void GLFramebuffer::AddColourAttachment(GLenum attachment_index,
                                           uint32_t width, uint32_t height,
                                           GLenum internal_format,
                                           GLenum format, GLenum filter,
                                           GLenum pixel_format) {
  ZoneScoped;
  gl_handle textureColorbuffer;
  glGenTextures(1, &textureColorbuffer);
  glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
  glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, internal_format,
               pixel_format, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
  glBindTexture(GL_TEXTURE_2D, 0);

  glFramebufferTexture2D(GL_FRAMEBUFFER, attachment_index, GL_TEXTURE_2D,
                         textureColorbuffer, 0);
  colour_attachments.push_back(textureColorbuffer);
  framebuffer_width = width;
  framebuffer_height = height;
}

void GLFramebuffer::AddDepthAttachment(uint32_t width, uint32_t height,
                                          GLenum format) {
  ZoneScoped;
  gl_handle rbo;
  glGenRenderbuffers(1, &rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, format, width, height);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                            GL_RENDERBUFFER, rbo);

  depth_attachment = rbo;
  framebuffer_width = width;
  framebuffer_height = height;
}

void GLFramebuffer::AddDepthAttachmentSamplerFriendly(uint32_t width,
                                                           uint32_t height,
                                                           GLenum format) {
  ZoneScoped;
  gl_handle depthMap;
  glGenTextures(1, &depthMap);
  glBindTexture(GL_TEXTURE_2D, depthMap);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0,
               GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                         depthMap, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);

  depth_attachment = depthMap;
  framebuffer_width = width;
  framebuffer_height = height;
}

void GLFramebuffer::Check() {
  ZoneScoped;
  auto zero = GL_COLOR_ATTACHMENT0;
  std::vector<unsigned int> attachments;

  for (auto &a : colour_attachments) {
    attachments.push_back(zero);
    zero++;
  }

  if (attachments.empty()) {
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
  } else {
    glDrawBuffers(attachments.size(), attachments.data());
  }

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!"
              << std::endl;
  }
}

GLFramebuffer GLFramebuffer::Create(glm::vec2 resolution,
                                      std::vector<GLAttachmentInfo> attachments,
                                      bool add_depth) {
  ZoneScoped;
  GLenum attachment = GL_COLOR_ATTACHMENT0;
  GLFramebuffer fb{};
  fb.Bind();
  for (auto &info : attachments) {
    fb.AddColourAttachment(attachment, resolution.x, resolution.y,
                             info.internal_format, info.format,
                             info.filter, info.pixel_format);
    attachment++;
  }
  if (add_depth) {
    fb.AddDepthAttachmentSamplerFriendly(resolution.x, resolution.y,
                                             GL_DEPTH24_STENCIL8);
  }
  fb.Check();
  fb.Unbind();
  return fb;
}

void GLFramebuffer::Bind() {
  ZoneScoped;
  if (handle == INVALID_GL_HANDLE) {
    glGenFramebuffers(1, &handle);
  }
  glBindFramebuffer(GL_FRAMEBUFFER, handle);
}
} // namespace gem