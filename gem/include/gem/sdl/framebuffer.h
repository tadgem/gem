#pragma once
#include <vector>
#include <optional>
#include "SDL3/SDL.h"
#include "glm/glm.hpp"
namespace gem {
namespace sdl {
class framebuffer {
public:

  void add_colour_attachment(SDL_GPUDevice* device, glm::ivec2 resolution,
                             SDL_GPUTextureFormat format);

  void add_depth_attachment(SDL_GPUDevice* device, glm::ivec2 resolution,
                            SDL_GPUTextureFormat format);

  std::vector<  SDL_GPUColorTargetDescription>    m_colour_target_descriptions;
  std::vector<  SDL_GPUTexture*>                  m_colour_target_textures;
  std::optional<SDL_GPUDepthStencilTargetInfo>    m_depth_target_description;
  SDL_GPUTexture*                                 m_depth_target_texture;
};
}
}