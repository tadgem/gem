#include "gem/sdl/gpu_helpers.h"
#include "spirv_reflect.h"
gem::sdl::Shader
gem::sdl::GPUHelpers::LoadShaderStage(SDL_GPUDevice *device, const char *shaderSource,
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

  size_t spirv_size = 0;
  void* spirv = SDL_ShaderCross_CompileSPIRVFromHLSL(&source_info, &spirv_size);


  SpvReflectShaderModule shader_reflect_module {};
  spvReflectCreateShaderModule(spirv_size, spirv, &shader_reflect_module);

  std::vector<SpvReflectInterfaceVariable> stage_inputs;
  stage_inputs.resize(shader_reflect_module.input_variable_count);
  for(int i = 0; i < shader_reflect_module.input_variable_count; i++)
  {
    stage_inputs[i] = *shader_reflect_module.input_variables[i];
  }

  std::vector<SpvReflectInterfaceVariable> stage_outputs;
  stage_outputs.resize(shader_reflect_module.output_variable_count);
  for(int i = 0; i < shader_reflect_module.output_variable_count; i++)
  {
    stage_outputs[i] = *shader_reflect_module.output_variables[i];
  }

  return {shader, info};
}
