#pragma once

#include "SDL3/SDL.h"
#include "glm/glm.hpp"

namespace gem {
namespace sdl {
class Texture {
public:
  enum class Type
  {
    _2D,
    _3D,
    _CUBEMAP
  };
  SDL_GPUTexture* m_texture;
  Type m_type;

  explicit Texture(const Type& type, SDL_GPUTexture* tex = nullptr);
  // Copy
  Texture(const Texture& other);
  Texture& operator=(const Texture& other);

  // Move
  Texture(Texture&& other) noexcept ;
  Texture& operator=(Texture&& other) noexcept;

  ~Texture();
  static Texture CreateTexture2D(SDL_GPUDevice* device, SDL_GPUTextureFormat format,
                          glm::ivec2 resolution, SDL_GPUTextureUsageFlags usage,
                          uint32_t layer_count = 1, uint32_t num_levels = 1);
};
}
}