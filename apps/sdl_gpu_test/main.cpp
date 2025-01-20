#include "gem/gem.h"
#include "gem/sdl/gpu_helpers.h"
#include "gem/sdl/gpu_state.h"
#include "gem/sdl/vertex_def.h"
#include "gem/sdl/framebuffer.h"

int main()
{
    using namespace gem;
    sdl::GPUState state = sdl::GPUState::Init();

    std::string gbuffer_hlsl = Utils::load_string_from_path("assets/shaders-hlsl/gbuffer.shader");
    auto gbuffer_stages = Shader::split_composite_shader(gbuffer_hlsl);

    std::string blit_hlsl = Utils::load_string_from_path("assets/shaders-hlsl/blit.shader");
    auto blit_stages = Shader::split_composite_shader(blit_hlsl);

    sdl::Framebuffer fb {};
    fb.add_colour_attachment(state.m_device, {1600, 900}, SDL_GPU_TEXTUREFORMAT_R32G32B32A32_FLOAT);
    fb.add_colour_attachment(state.m_device, {1600, 900}, SDL_GPU_TEXTUREFORMAT_R32G32B32A32_FLOAT);
    fb.add_colour_attachment(state.m_device, {1600, 900}, SDL_GPU_TEXTUREFORMAT_R32G32B32A32_FLOAT);
    fb.add_colour_attachment(state.m_device, {1600, 900}, SDL_GPU_TEXTUREFORMAT_R32G32B32A32_FLOAT);
    fb.add_colour_attachment(state.m_device, {1600, 900}, SDL_GPU_TEXTUREFORMAT_R32G32_FLOAT);
    fb.add_colour_attachment(state.m_device, {1600, 900}, SDL_GPU_TEXTUREFORMAT_R32G32B32A32_FLOAT);
    fb.add_depth_attachment(state.m_device, {1600, 900}, SDL_GPU_TEXTUREFORMAT_D32_FLOAT);
    fb.build();



    // TODO: Wont ship with shadercross as its huge, need to abstract the concept of shaderstage
    // outwith shader cross
    auto gbuffer_vert = sdl::GPUHelpers::LoadShaderStage(
        state.m_device, gbuffer_stages[Shader::stage::vertex].c_str(),
        SDL_SHADERCROSS_SHADERSTAGE_VERTEX);

    auto gbuffer_frag = sdl::GPUHelpers::LoadShaderStage(
        state.m_device, gbuffer_stages[Shader::stage::fragment].c_str(),
        SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT);


    auto blit_vert = sdl::GPUHelpers::LoadShaderStage(
        state.m_device, blit_stages[Shader::stage::vertex].c_str(),
        SDL_SHADERCROSS_SHADERSTAGE_VERTEX);
    auto blit_frag = sdl::GPUHelpers::LoadShaderStage(
        state.m_device, blit_stages[Shader::stage::fragment].c_str(),
        SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT);

    SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo = {};
    pipelineCreateInfo.target_info = fb.m_graphics_pipeline_target_info;
    pipelineCreateInfo.vertex_input_state = sdl::VertexDescriptions::PosNormalUV::VertexInputState;
    pipelineCreateInfo.vertex_shader = gbuffer_vert.m_shader;
    pipelineCreateInfo.fragment_shader = gbuffer_frag.m_shader;
    pipelineCreateInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    pipelineCreateInfo.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;

    auto fillPipeline = SDL_CreateGPUGraphicsPipeline(state.m_device, &pipelineCreateInfo);
    if(!fillPipeline)
    {
      spdlog::error("SDL GPU : Failed to create graphics pipeline for triangle shader");
      return -1;
    }

    SDL_ReleaseGPUShader(state.m_device, gbuffer_vert.m_shader);
    SDL_ReleaseGPUShader(state.m_device, gbuffer_frag.m_shader);

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

        SDL_GPUTexture* swapchainTexture = state.m_swapchain.acquire_texture(cmdbuf, state.m_window);

        if (swapchainTexture != NULL)
        {
            SDL_GPUColorTargetInfo colorTargetInfo = { 0 };
            colorTargetInfo.texture = swapchainTexture;
            colorTargetInfo.clear_color = SDL_FColor{ 0.0f, 0.0f, 0.0f, 1.0f };
            colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
            colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;

            SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(cmdbuf, &colorTargetInfo, 1, NULL);
            SDL_BindGPUGraphicsPipeline(renderPass, fillPipeline);
//          if (UseSmallViewport)
//          {
//              SDL_SetGPUViewport(renderPass, &SmallViewport);
//          }
//          if (UseScissorRect)
//          {
//              SDL_SetGPUScissor(renderPass, &ScissorRect);
//          }
//          SDL_DrawGPUPrimitives(renderPass, 3, 1, 0, 0);
            SDL_EndGPURenderPass(renderPass);
        }

        SDL_SubmitGPUCommandBuffer(cmdbuf);
        
    }

    return 0;
}