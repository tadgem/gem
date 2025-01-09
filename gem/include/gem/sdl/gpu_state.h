#pragma once
#include "SDL3/SDL.h"

namespace gem
{
namespace sdl
{
  struct GPUState
  {
    SDL_Window*       m_window;
    SDL_GPUDevice*    m_device;

    static GPUState Init();
  };

}
}