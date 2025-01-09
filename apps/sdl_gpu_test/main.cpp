#include "gem/gem.h"
#include "gem/sdl/gpu_helpers.h"
#include "gem/sdl/gpu_state.h"
#include "gem/sdl/vertex_def.h"

int main()
{
    gem::sdl::GPUState state = gem::sdl::GPUState::Init();

    std::string hlsl = gem::Utils::load_string_from_path("assets/shaders-hlsl/gbuffer.shader");
    auto stages = gem::Shader::split_composite_shader(hlsl);


    std::vector<SDL_GPUColorTargetDescription> formats(6);
    auto swapchain_format = SDL_GetGPUSwapchainTextureFormat(state.m_device, state.m_window);
    for(int i = 0; i < 6; i++)
    {
      formats[i] = {swapchain_format};
    }

    SDL_GPUGraphicsPipelineTargetInfo targetInfo {};
    targetInfo.num_color_targets = 6;
    targetInfo.color_target_descriptions = formats.data();
    targetInfo.has_depth_stencil_target = false;

    SDL_GPUVertexBufferDescription bufferDesc;
    bufferDesc.slot = 0;
    bufferDesc.input_rate  = SDL_GPU_VERTEXINPUTRATE_VERTEX;
    bufferDesc.instance_step_rate = 0;
    bufferDesc.pitch = gem::sdl::VertexDescriptions::PosNormalUV::Offset();

    auto vertexAttributes = gem::sdl::VertexDescriptions::PosNormalUV::Get();

    std::vector<SDL_GPUVertexBufferDescription> bufferDescs {bufferDesc};
    SDL_GPUVertexInputState vertexState;
    vertexState.num_vertex_buffers = 1;
    vertexState.vertex_buffer_descriptions = bufferDescs.data();
    vertexState.num_vertex_attributes = vertexAttributes.size();
    vertexState.vertex_attributes = vertexAttributes.data();

    // TODO: Wont ship with shadercross as its huge, need to abstract the concept of shaderstage
    // outwith shader cross
    auto vert = gem::sdl::GPUHelpers::LoadShader(state.m_device,
    stages[gem::Shader::stage::vertex].c_str(), SDL_SHADERCROSS_SHADERSTAGE_VERTEX);

    auto frag = gem::sdl::GPUHelpers::LoadShader(state.m_device,
    stages[gem::Shader::stage::fragment].c_str(), SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT);

    SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo = {};
    pipelineCreateInfo.target_info = targetInfo;
    pipelineCreateInfo.vertex_input_state = vertexState;
    pipelineCreateInfo.vertex_shader = vert.m_shader;
    pipelineCreateInfo.fragment_shader = frag.m_shader;
    pipelineCreateInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    pipelineCreateInfo.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;

    auto fillPipeline = SDL_CreateGPUGraphicsPipeline(state.m_device, &pipelineCreateInfo);
    if(!fillPipeline)
    {
      spdlog::error("SDL GPU : Failed to create graphics pipeline for triangle shader");
      return -1;
    }

    SDL_ReleaseGPUShader(state.m_device, vert.m_shader);
    SDL_ReleaseGPUShader(state.m_device, frag.m_shader);

    static bool quit = false;

    while(!quit)
    {
        SDL_Event e;
        while(SDL_PollEvent(&e))
        {
          continue;
        }
        SDL_GPUCommandBuffer* cmdbuf = SDL_AcquireGPUCommandBuffer(state.m_device);
        if (cmdbuf == NULL)
        {
            SDL_Log("AcquireGPUCommandBuffer failed: %s", SDL_GetError());
            return -1;
        }

        SDL_GPUTexture* swapchainTexture;
        if (!SDL_AcquireGPUSwapchainTexture(cmdbuf, state.m_window, &swapchainTexture, NULL, NULL)) {
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