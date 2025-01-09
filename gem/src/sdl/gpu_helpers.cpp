#include "gem/sdl/gpu_helpers.h"

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
