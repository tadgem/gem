#include "gem/sdl/texture.h"


gem::sdl::Texture gem::sdl::Texture::CreateTexture2D(
    SDL_GPUDevice *device, SDL_GPUTextureFormat format, glm::ivec2 resolution,
    SDL_GPUTextureUsageFlags usage, uint32_t layer_count, uint32_t num_levels) {

  SDL_GPUTextureCreateInfo textureCreateInfo {};
  textureCreateInfo.type = SDL_GPU_TEXTURETYPE_2D;
  textureCreateInfo.format = format;
  textureCreateInfo.width = resolution.x;
  textureCreateInfo.height = resolution.y;
  textureCreateInfo.usage = usage;
  textureCreateInfo.layer_count_or_depth = layer_count;
  textureCreateInfo.num_levels = num_levels;

  SDL_GPUTexture* sdl_tex = SDL_CreateGPUTexture(device, &textureCreateInfo);
  return Texture(Texture::Type::_2D, sdl_tex);
}

gem::sdl::Texture::Texture(const gem::sdl::Texture::Type &type,
                           SDL_GPUTexture *tex) : m_type(type), m_texture(tex){}
