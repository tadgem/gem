#include "gem/gem.h"
#include "SDL3_shadercross/SDL_shadercross.h"

struct Shader
{
  SDL_GPUShader* m_shader;
  SDL_ShaderCross_GraphicsShaderInfo m_info;
};


Shader LoadShader(
    SDL_GPUDevice* device,
    const char* shaderSource,
    SDL_GPUShaderStage shaderStage)
{
  SDL_ShaderCross_GraphicsShaderInfo info;
  SDL_GPUShader* shader = SDL_ShaderCross_CompileGraphicsShaderFromHLSL(
      device,
      shaderSource,
      "main",
      NULL,
      NULL,
      0,
      shaderStage,
      &info);

  return {shader, info};
}

const char vert_source[] = R"hlsl(
struct Input
{
    uint VertexIndex : SV_VertexID;
};

struct Output
{
    float4 Color : TEXCOORD0;
    float4 Position : SV_Position;
};

Output main(Input input)
{
    Output output;
    float2 pos;
    if (input.VertexIndex == 0)
    {
        pos = (-1.0f).xx;
        output.Color = float4(1.0f, 0.0f, 0.0f, 1.0f);
    }
    else
    {
        if (input.VertexIndex == 1)
        {
            pos = float2(1.0f, -1.0f);
            output.Color = float4(0.0f, 1.0f, 0.0f, 1.0f);
        }
        else
        {
            if (input.VertexIndex == 2)
            {
                pos = float2(0.0f, 1.0f);
                output.Color = float4(0.0f, 0.0f, 1.0f, 1.0f);
            }
        }
    }
    output.Position = float4(pos, 0.0f, 1.0f);
    return output;
})hlsl";

const char frag_source[] = R"hlsl(
float4 main(float4 Color : TEXCOORD0) : SV_Target0
{
    return Color;
}
)hlsl";


int main()
{
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO);
  SDL_ShaderCross_Init();

  SDL_GPUDevice* device = SDL_CreateGPUDevice(
      SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL,
      true,
      NULL
      );

  if(!device)
  {
    spdlog::error("SDL GPU : Failed to create a gpu device : {}", SDL_GetError());
    return -1;
  }

  SDL_Window* window = SDL_CreateWindow("SDL GPU", 1600, 900, 0);

  if(!window)
  {
    spdlog::error("SDL GPU : Failed to create a window");
    return -1;
  }

  if (!SDL_ClaimWindowForGPUDevice(device, window))
  {
    spdlog::error("SDL GPU : Failed to claim window for gpu device");
    return -1;
  }

  SDL_GPUColorTargetDescription targetDescriptions[] = {
      SDL_GetGPUSwapchainTextureFormat(device, window)
  };

  SDL_GPUGraphicsPipelineTargetInfo targetInfo {};
  targetInfo.num_color_targets = 1;
  targetInfo.color_target_descriptions = targetDescriptions;

  auto vert = LoadShader(device, &vert_source[0], SDL_GPU_SHADERSTAGE_VERTEX);
  auto frag = LoadShader(device, &frag_source[0], SDL_GPU_SHADERSTAGE_FRAGMENT);

  SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo = {};
  pipelineCreateInfo.target_info = targetInfo;
  pipelineCreateInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
  pipelineCreateInfo.vertex_shader = vert.m_shader;
  pipelineCreateInfo.fragment_shader = frag.m_shader;


  return 0;
}