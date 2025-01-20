#pragma once
#include "SDL3/SDL.h"
#include "gem/sdl/framebuffer.h"
namespace gem
{
namespace sdl
{
  struct GPUState
  {
    SDL_Window*       m_window;
    SDL_GPUDevice*    m_device;
    sdl::Swapchain    m_swapchain;

    static GPUState Init();
  };

}
}