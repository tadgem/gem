#define GLM_ENABLE_EXPERIMENTAL
#include "GL/glew.h"
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "gem/backend.h"
#include "gem/gl/gl_dbg.h"
#include "gem/profile.h"
#include "gem/stb_image.h"
#include "gem/texture.h"
#include "gem/utils.h"
#include "cpptrace/cpptrace.hpp"
#include "gem/sdl/texture.h"

namespace gem {

Texture::Texture() { ZoneScoped; }

Texture::Texture(const std::string &path) {
  ZoneScoped;
  std::string compressed_format_type = "";
  int block_size = -1;

  std::vector<unsigned char> data = Utils::load_binary_from_path(path);

  if (path.find("dds") != std::string::npos) {
    load_texture_gli(data);
  } else {
    load_texture_stbi(data);
  }
}

Texture::Texture(const std::string &path, std::vector<unsigned char> data) {
  ZoneScoped;
}

Texture::~Texture() { ZoneScoped; }

void Texture::bind_sampler(GLenum texture_slot, GLenum texture_target) {
  ZoneScoped;
  bind_sampler_handle(m_handle, texture_slot, texture_target);
}

void Texture::bind_sampler_handle(gl_handle handle, GLenum texture_slot,
                                  GLenum texture_target) {
  ZoneScoped;
  glActiveTexture(texture_slot);
  if(glGetError() != GL_NO_ERROR)
  {
    spdlog::error("Texture::bind_sampler_handle : ERROR");
    cpptrace::generate_trace().print();
  }
  glAssert(glBindTexture(texture_target, handle));
}

void Texture::bind_image_handle(gl_handle handle, uint32_t binding,
                                uint32_t mip_level, GLenum format) {
  ZoneScoped;
  glAssert(glBindImageTexture(binding, handle, mip_level, GL_TRUE, 0,
                              GL_READ_WRITE, format));
}

void Texture::unbind_image(uint32_t binding) {
  ZoneScoped;
  glAssert(
      glBindImageTexture(binding, 0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8));
}

Texture Texture::from_data(unsigned int *data, unsigned int count, int width,
                           int height, int depth, int nr_channels) {
  ZoneScoped;
  Texture t{};
  glGenTextures(1, &t.m_handle);
  glBindTexture(GL_TEXTURE_2D, t.m_handle);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                  GL_LINEAR_MIPMAP_NEAREST);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
               GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);

  return t;
}

Texture Texture::create_3d_texture(glm::ivec3 dim, GLenum format,
                                   GLenum pixel_format, GLenum data_type,
                                   void *data, GLenum filter,
                                   GLenum wrap_mode) {
  ZoneScoped;
  Texture t{};
  glAssert(glGenTextures(1, &t.m_handle));
  glAssert(glBindTexture(GL_TEXTURE_3D, t.m_handle));

  glAssert(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, wrap_mode));
  glAssert(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, wrap_mode));
  glAssert(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, wrap_mode));
  glAssert(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 0));
  glAssert(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 5));

  glAssert(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER,
                           GL_LINEAR_MIPMAP_NEAREST));
  glAssert(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER,
                           GL_LINEAR_MIPMAP_NEAREST));
  glAssert(glTexImage3D(GL_TEXTURE_3D, 0, pixel_format, dim.x, dim.y, dim.z, 0,
                        format, data_type, data));
  glGenerateTextureMipmap(t.m_handle);
  return t;
}

Texture Texture::create_3d_texture_empty(glm::ivec3 dim, GLenum format,
                                         GLenum pixel_format, GLenum data_type,
                                         GLenum filter, GLenum wrap_mode) {
  ZoneScoped;
  Texture t{};
  glAssert(glGenTextures(1, &t.m_handle));
  glAssert(glBindTexture(GL_TEXTURE_3D, t.m_handle));

  glAssert(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, wrap_mode));
  glAssert(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, wrap_mode));
  glAssert(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, wrap_mode));
  glAssert(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 0));
  glAssert(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 5));

  glAssert(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER,
                           GL_LINEAR_MIPMAP_LINEAR));
  glAssert(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
  GLfloat clear[4] = {0.0f, 0.0f, 0.0f, 0.0f};
  glAssert(glTexImage3D(GL_TEXTURE_3D, 0, pixel_format, dim.x, dim.y, dim.z, 0,
                        format, data_type, NULL));
  glAssert(
      glClearTexImage(GL_TEXTURE_3D, 0, pixel_format, GL_FLOAT, &clear[0]));
  glGenerateTextureMipmap(t.m_handle);
  return t;
}

void Texture::load_texture_stbi(std::vector<unsigned char> &data) {
  ZoneScoped;
  // TODO: Split up STBI and GLI CPU loading and GL submission
  //  STBI CPU processing taking 22ms in release, only 2ms to submit to GPU
  m_mode = Mode::stb;
  unsigned char *stbi_data = nullptr;

  stbi_set_flip_vertically_on_load(1);
  stbi_data = stbi_load_from_memory(data.data(), data.size(), &m_width,
                                    &m_height, &m_num_channels, 0);
  if (!stbi_data) {
    return;
  }
  m_cpu_data.stb_data = stbi_data;
}

void Texture::load_texture_gli(std::vector<unsigned char> &data) {
  ZoneScoped;
  m_mode = Mode::gli;
  gli::texture dds_tex_raw =
      gli::load_dds((const char *)data.data(), data.size());
  gli::texture *dds_tex = new gli::texture(gli::flip(dds_tex_raw));

  m_width = dds_tex->extent().x;
  m_height = dds_tex->extent().y;
  m_num_channels = dds_tex->extent().z;
  m_cpu_data.gli_data = dds_tex;
}
void Texture::release() {
  ZoneScoped;
  glDeleteTextures(1, &m_handle);
}

void Texture::submit_to_gpu() {
  if (m_mode == Mode::stb) {
    ZoneScopedN("STBI Submit to GPU");
    glGenTextures(1, &m_handle);
    glBindTexture(GL_TEXTURE_2D, m_handle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLenum format = m_num_channels == 4 ? GL_RGBA : GL_RGB;

    glTexImage2D(GL_TEXTURE_2D, 0, format, m_width, m_height, 0, format,
                 GL_UNSIGNED_BYTE, m_cpu_data.stb_data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(m_cpu_data.stb_data);
  } else if (m_mode == Mode::gli) {
    ZoneScopedN("GLI Submit to GPU");
    gli::gl GL(gli::gl::PROFILE_GL33);
    gli::gl::format const format = GL.translate(
        m_cpu_data.gli_data->format(), m_cpu_data.gli_data->swizzles());
    GLenum target = GL.translate(m_cpu_data.gli_data->target());

    glGenTextures(1, &m_handle);
    glBindTexture(target, m_handle);
    glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(target, GL_TEXTURE_MAX_LEVEL,
                    static_cast<GLint>(m_cpu_data.gli_data->levels() - 1));
    glTexParameteriv(target, GL_TEXTURE_SWIZZLE_RGBA, &format.Swizzles[0]);
    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexStorage2D(target, static_cast<GLint>(m_cpu_data.gli_data->levels()),
                   format.Internal, m_cpu_data.gli_data->extent().x,
                   m_cpu_data.gli_data->extent().y);

    for (std::size_t Level = 0; Level < m_cpu_data.gli_data->levels();
         ++Level) {
      glm::tvec3<GLsizei> Extent(m_cpu_data.gli_data->extent(Level));
      glCompressedTexSubImage2D(
          target, static_cast<GLint>(Level), 0, 0, Extent.x, Extent.y,
          format.Internal,
          static_cast<GLsizei>(m_cpu_data.gli_data->size(Level)),
          m_cpu_data.gli_data->data(0, 0, Level));
    }
    delete m_cpu_data.gli_data;
  } else {
    spdlog::error("texture::submit_to_gpu : memory mode not supported");
  }
}

} // namespace gem