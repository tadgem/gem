#include "gem/sdl/gpu_helpers.h"

SDL_GPUTexture *gem::sdl::GPUHelpers::CreateTexture2D(
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

  return SDL_CreateGPUTexture(device, &textureCreateInfo);
}
gem::sdl::Shader
gem::sdl::GPUHelpers::LoadShader(SDL_GPUDevice *device, const char *shaderSource,
                               SDL_ShaderCross_ShaderStage shaderStage) {
  SDL_ShaderCross_GraphicsShaderMetadata info;
  SDL_ShaderCross_HLSL_Info source_info{};
  source_info.source = shaderSource;
  source_info.entrypoint = "main";
  source_info.defines = NULL;
  source_info.include_dir = NULL;
  source_info.name = NULL;
  source_info.shader_stage = shaderStage;

  SDL_GPUShader* shader = SDL_ShaderCross_CompileGraphicsShaderFromHLSL(
      device,
      &source_info,
      &info);

  return {shader, info};
}
