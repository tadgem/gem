#pragma once
#include <string>
#define GLM_ENABLE_EXPERIMENTAL
#include "gem/asset.h"
#include "gem/dbg_memory.h"
#include "gem/vertex.h"
#include "gli.hpp"
#include "glm.hpp"
namespace gem {

enum class TextureMapType {
  diffuse,
  normal,
  specular,
  metallicness,
  roughness,
  ao

};

class Texture {
public:
  enum class Mode { stb, gli, memory };

  Texture();
  Texture(const std::string &path);
  Texture(const std::string &path, std::vector<unsigned char> data);
  ~Texture();

  void bind_sampler(GLenum texture_slot, GLenum texture_target = GL_TEXTURE_2D);

  static void bind_sampler_handle(gl_handle handle, GLenum texture_slot,
                                  GLenum texture_target = GL_TEXTURE_2D);

  static void bind_image_handle(gl_handle handle, uint32_t binding,
                                uint32_t mip_level, GLenum format);

  static void unbind_image(uint32_t binding);

  int m_width, m_height, m_depth, m_num_channels;

  // TODO: Support Vulkan texture handles
  gl_handle m_handle = INVALID_GL_HANDLE;
  Mode m_mode = Mode::memory;

  union {
    unsigned char *stb_data;
    gli::texture *gli_data;
  } m_cpu_data;

  static Texture from_data(unsigned int *data, unsigned int count, int width,
                           int height, int depth, int nr_channels);

  static Texture create_3d_texture(glm::ivec3 dim, GLenum format,
                                   GLenum pixel_format, GLenum data_type,
                                   void *data, GLenum filter = GL_LINEAR,
                                   GLenum wrap_mode = GL_REPEAT);

  static Texture create_3d_texture_empty(glm::ivec3 dim, GLenum format,
                                         GLenum pixel_format, GLenum data_type,
                                         GLenum filter = GL_LINEAR,
                                         GLenum wrap_mode = GL_REPEAT);

  void load_texture_stbi(std::vector<unsigned char> &data);
  void load_texture_gli(std::vector<unsigned char> &data);

  void submit_to_gpu();

  void release();

  inline static Texture *white;
  inline static Texture *black;

  GEM_IMPL_ALLOC(Texture);
};

struct TextureEntry {
  TextureMapType m_map_type;
  AssetHandle m_handle;
  std::string m_path;
  Texture *m_texture = nullptr;

  TextureEntry() { m_texture = nullptr; };
  TextureEntry(TextureMapType tmt, AssetHandle ah, const std::string &path,
                Texture *data);
  GEM_IMPL_ALLOC(TextureEntry);

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(TextureEntry, m_map_type, m_handle, m_path)
};

struct SamplerInfo {
  GLenum sampler_slot;
  GLenum texture_target;
  TextureEntry tex_entry;

  GEM_IMPL_ALLOC(SamplerInfo)
};
} // namespace gem