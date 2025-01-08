#include "gem/gem.h"
#include "SDL3_shadercross/SDL_shadercross.h"

struct SDLShader
{
  SDL_GPUShader* m_shader;
  SDL_ShaderCross_GraphicsShaderMetadata m_info;
};


SDLShader LoadShader(
    SDL_GPUDevice* device,
    const char* shaderSource,
    SDL_ShaderCross_ShaderStage shaderStage)
{
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

int main()
{
    std::string hlsl = gem::Utils::load_string_from_path("assets/shaders-hlsl/gbuffer.shader");
    auto stages = gem::Shader::split_composite_shader(hlsl);
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

    std::vector<SDL_GPUColorTargetDescription> formats(6);

    for(int i = 0; i < 6; i++)
    {
      formats.push_back({SDL_GetGPUSwapchainTextureFormat(device, window)});
    }

    SDL_GPUGraphicsPipelineTargetInfo targetInfo {};
    targetInfo.num_color_targets = 6;
    targetInfo.color_target_descriptions = formats.data();


    SDL_GPUVertexBufferDescription bufferDesc;
    bufferDesc.slot = 0;
    bufferDesc.input_rate  = SDL_GPU_VERTEXINPUTRATE_VERTEX;
    bufferDesc.instance_step_rate = 0;
    bufferDesc.pitch = 32;

    std::vector<SDL_GPUVertexAttribute> vertexAttributes;
    vertexAttributes.push_back({0,0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, 0});
    vertexAttributes.push_back({1,0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, sizeof(float) * 3});
    vertexAttributes.push_back({2,0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, sizeof(float) * 6});

    std::vector<SDL_GPUVertexBufferDescription> bufferDescs {bufferDesc};
    SDL_GPUVertexInputState vertexState;
    vertexState.num_vertex_buffers = 1;
    vertexState.vertex_buffer_descriptions = bufferDescs.data();
    vertexState.num_vertex_attributes = 3;
    vertexState.vertex_attributes = vertexAttributes.data();

    // TODO: Wont ship with shadercross as its huge, need to abstract the concept of shaderstage
    // outwith shader cross
    auto vert = LoadShader(device, stages[gem::Shader::stage::vertex].c_str(), SDL_SHADERCROSS_SHADERSTAGE_VERTEX);
    auto frag = LoadShader(device, stages[gem::Shader::stage::fragment].c_str(), SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT);

    SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo = {};
    pipelineCreateInfo.target_info = targetInfo;
    pipelineCreateInfo.vertex_input_state = vertexState;
    pipelineCreateInfo.vertex_shader = vert.m_shader;
    pipelineCreateInfo.fragment_shader = frag.m_shader;
    pipelineCreateInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    pipelineCreateInfo.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;

    auto fillPipeline = SDL_CreateGPUGraphicsPipeline(device, &pipelineCreateInfo);
    if(!fillPipeline)
    {
      spdlog::error("SDL GPU : Failed to create graphics pipeline for triangle shader");
      return -1;
    }
//
//    SDL_ReleaseGPUShader(device, vert.m_shader);
//    SDL_ReleaseGPUShader(device, frag.m_shader);

    static bool quit = false;

    while(!quit)
    {
        SDL_GPUCommandBuffer* cmdbuf = SDL_AcquireGPUCommandBuffer(device);
        if (cmdbuf == NULL)
        {
            SDL_Log("AcquireGPUCommandBuffer failed: %s", SDL_GetError());
            return -1;
        }

        SDL_GPUTexture* swapchainTexture;
        if (!SDL_AcquireGPUSwapchainTexture(cmdbuf, window, &swapchainTexture, NULL, NULL)) {
            SDL_Log("WaitAndAcquireGPUSwapchainTexture failed: %s", SDL_GetError());
            return -1;
        }

        if (swapchainTexture != NULL)
        {
            SDL_GPUColorTargetInfo colorTargetInfo = { 0 };
            colorTargetInfo.texture = swapchainTexture;
            colorTargetInfo.clear_color = SDL_FColor{ 0.0f, 0.0f, 0.0f, 1.0f };
            colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
            colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;

            SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(cmdbuf, &colorTargetInfo, 1, NULL);
            SDL_BindGPUGraphicsPipeline(renderPass, fillPipeline);
//            if (UseSmallViewport)
//            {
//                SDL_SetGPUViewport(renderPass, &SmallViewport);
//            }
//            if (UseScissorRect)
//            {
//                SDL_SetGPUScissor(renderPass, &ScissorRect);
//            }
            //SDL_DrawGPUPrimitives(renderPass, 3, 1, 0, 0);
            SDL_EndGPURenderPass(renderPass);
        }

        SDL_SubmitGPUCommandBuffer(cmdbuf);
        
    }

    return 0;
}