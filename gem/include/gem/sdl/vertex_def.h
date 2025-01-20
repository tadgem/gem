#pragma once

#include "SDL3/SDL.h"
#include <array>
#include <vector>

namespace gem {
namespace sdl {

template<size_t NumElements>
using VertexAttributeCollection = std::array<SDL_GPUVertexAttribute, NumElements>;

namespace VertexDescriptions
{
  class PosNormalUV
  {
  public:
    inline static VertexAttributeCollection<3> VertexAttributes
    {
      SDL_GPUVertexAttribute{0,0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, 0},
      SDL_GPUVertexAttribute{1,0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, sizeof(float) * 3},
      SDL_GPUVertexAttribute{2,0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, sizeof(float) * 6},
    };

    inline static constexpr uint32_t Offset = 32;

    inline static std::vector<SDL_GPUVertexBufferDescription> BufferDescs
    {
      SDL_GPUVertexBufferDescription{
        0,
        sdl::VertexDescriptions::PosNormalUV::Offset,
        SDL_GPU_VERTEXINPUTRATE_VERTEX,
        0
      }
    };

    inline static SDL_GPUVertexInputState   VertexInputState
    {
      BufferDescs.data(), 1,
      VertexAttributes.data(),
      VertexAttributes.size()
    };

  };
}
}
}