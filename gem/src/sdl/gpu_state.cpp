#include "gem/sdl/gpu_state.h"
#include "SDL3_shadercross/SDL_shadercross.h"
#include "spdlog/spdlog.h"

gem::sdl::GPUState gem::sdl::GPUState::Init() {
  GPUState state {};

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
    return state;
  }

  state.m_device = device;

  SDL_Window* window = SDL_CreateWindow("SDL GPU", 1600, 900, 0);

  if(!window)
  {
    spdlog::error("SDL GPU : Failed to create a window");
    return state;
  }

  state.m_window = window;

  if (!SDL_ClaimWindowForGPUDevice(device, window))
  {
    spdlog::error("SDL GPU : Failed to claim window for gpu device");
    return state;
  }


  return state;
}
