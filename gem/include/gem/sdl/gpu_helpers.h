#pragma once

#include "SDL3/SDL.h"
#include "SDL3_shadercross/SDL_shadercross.h"

#include "glm.hpp"
namespace gem {
namespace sdl {
struct Shader {
  SDL_GPUShader *m_shader;
  SDL_ShaderCross_GraphicsShaderMetadata m_info;
};

class GPUHelpers {
public:
  static SDL_GPUTexture *
  CreateTexture2D(SDL_GPUDevice *device, SDL_GPUTextureFormat format,
                  glm::ivec2 resolution, SDL_GPUTextureUsageFlags usage,
                  uint32_t layer_count = 1, uint32_t num_levels = 1);

  static Shader LoadShader(SDL_GPUDevice *device, const char *shaderSource,
                           SDL_ShaderCross_ShaderStage shaderStage);
};
}
}