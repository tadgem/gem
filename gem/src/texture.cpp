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

namespace gem {

Texture::Texture() { ZoneScoped; }

Texture::Texture(const std::string &path) {
  ZoneScoped;
  std::string compressed_format_type = "";
  int block_size = -1;

  std::vector<unsigned char> data = Utils::LoadBinaryFromPath(path);

  if (path.find("dds") != std::string::npos) {
    LoadTextureGLI(data);
  } else {
    LoadTextureSTB(data);
  }
}

Texture::Texture(const std::string &path, std::vector<unsigned char> data) {
  ZoneScoped;
}

Texture::~Texture() { ZoneScoped; }

void Texture::BindSampler(GLenum texture_slot, GLenum texture_target) {
  ZoneScoped;
  BindSamplerHandle(handle, texture_slot, texture_target);
}

void Texture::BindSamplerHandle(gl_handle handle, GLenum texture_slot,
                                  GLenum texture_target) {
  ZoneScoped;
  glAssert(glActiveTexture(texture_slot));
  glAssert(glBindTexture(texture_target, handle));
}

void Texture::BindImageHandle(gl_handle handle, uint32_t binding,
                                uint32_t mip_level, GLenum format) {
  ZoneScoped;
  glAssert(glBindImageTexture(binding, handle, mip_level, GL_TRUE, 0,
                              GL_READ_WRITE, format));
}

void Texture::UnbindImage(uint32_t binding) {
  ZoneScoped;
  glAssert(
      glBindImageTexture(binding, 0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8));
}

Texture Texture::FromData(unsigned int *data, unsigned int count, int width,
                           int height, int depth, int nr_channels) {
  ZoneScoped;
  Texture t{};
  glGenTextures(1, &t.handle);
  glBindTexture(GL_TEXTURE_2D, t.handle);

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

Texture Texture::Create3DTexture(glm::ivec3 dim, GLenum format,
                                   GLenum pixel_format, GLenum data_type,
                                   void *data, GLenum filter,
                                   GLenum wrap_mode) {
  ZoneScoped;
  Texture t{};
  glAssert(glGenTextures(1, &t.handle));
  glAssert(glBindTexture(GL_TEXTURE_3D, t.handle));

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
  glGenerateTextureMipmap(t.handle);
  return t;
}

Texture Texture::Create3DTextureEmpty(glm::ivec3 dim, GLenum format,
                                         GLenum pixel_format, GLenum data_type,
                                         GLenum filter, GLenum wrap_mode) {
  ZoneScoped;
  Texture t{};
  glAssert(glGenTextures(1, &t.handle));
  glAssert(glBindTexture(GL_TEXTURE_3D, t.handle));

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
  glGenerateTextureMipmap(t.handle);
  return t;
}

void Texture::LoadTextureSTB(std::vector<unsigned char> &data) {
  ZoneScoped;
  // TODO: Split up STBI and GLI CPU loading and GL submission
  //  STBI CPU processing taking 22ms in release, only 2ms to submit to GPU
  mode = Mode::kSTB;
  unsigned char *stbi_data = nullptr;

  stbi_set_flip_vertically_on_load(1);
  stbi_data = stbi_load_from_memory(data.data(), data.size(), &texture_width,
                                    &texture_height, &num_channels, 0);
  if (!stbi_data) {
    return;
  }
  cpu_data.stb_data = stbi_data;
}

void Texture::LoadTextureGLI(std::vector<unsigned char> &data) {
  ZoneScoped;
  mode = Mode::kGLI;
  gli::texture dds_tex_raw =
      gli::load_dds((const char *)data.data(), data.size());
  gli::texture *dds_tex = new gli::texture(gli::flip(dds_tex_raw));

  texture_width = dds_tex->extent().x;
  texture_height = dds_tex->extent().y;
  num_channels = dds_tex->extent().z;
  cpu_data.gli_data = dds_tex;
}
void Texture::ReleaseGPU() {
  ZoneScoped;
  glDeleteTextures(1, &handle);
}

void Texture::SubmitToGPU() {
  if (mode == Mode::kSTB) {
    ZoneScopedN("STBI Submit to GPU");
    glGenTextures(1, &handle);
    glBindTexture(GL_TEXTURE_2D, handle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLenum format = num_channels == 4 ? GL_RGBA : GL_RGB;

    glTexImage2D(GL_TEXTURE_2D, 0, format, texture_width, texture_height, 0, format,
                 GL_UNSIGNED_BYTE, cpu_data.stb_data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(cpu_data.stb_data);
  } else if (mode == Mode::kGLI) {
    ZoneScopedN("GLI Submit to GPU");
    gli::gl GL(gli::gl::PROFILE_GL33);
    gli::gl::format const format = GL.translate(
        cpu_data.gli_data->format(), cpu_data.gli_data->swizzles());
    GLenum target = GL.translate(cpu_data.gli_data->target());

    glGenTextures(1, &handle);
    glBindTexture(target, handle);
    glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(target, GL_TEXTURE_MAX_LEVEL,
                    static_cast<GLint>(cpu_data.gli_data->levels() - 1));
    glTexParameteriv(target, GL_TEXTURE_SWIZZLE_RGBA, &format.Swizzles[0]);
    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexStorage2D(target, static_cast<GLint>(cpu_data.gli_data->levels()),
                   format.Internal, cpu_data.gli_data->extent().x,
                   cpu_data.gli_data->extent().y);

    for (std::size_t Level = 0; Level < cpu_data.gli_data->levels();
         ++Level) {
      glm::tvec3<GLsizei> Extent(cpu_data.gli_data->extent(Level));
      glCompressedTexSubImage2D(
          target, static_cast<GLint>(Level), 0, 0, Extent.x, Extent.y,
          format.Internal,
          static_cast<GLsizei>(cpu_data.gli_data->size(Level)),
          cpu_data.gli_data->data(0, 0, Level));
    }
    delete cpu_data.gli_data;
  } else {
    spdlog::error("texture::submit_to_gpu : memory mode not supported");
  }
}

} // namespace gem