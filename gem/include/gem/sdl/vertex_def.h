#pragma once

#include "SDL3/SDL.h"
#include <vector>

namespace gem {
namespace sdl {
using VertexAttributeCollection = std::vector<SDL_GPUVertexAttribute>;
namespace VertexDescriptions
{
  class PosNormalUV
  {
  public:
    inline static VertexAttributeCollection Get()
    {
      VertexAttributeCollection vertexAttributes;
      vertexAttributes.push_back({0,0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, 0});
      vertexAttributes.push_back({1,0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, sizeof(float) * 3});
      vertexAttributes.push_back({2,0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, sizeof(float) * 6});
      return vertexAttributes;
    }

    static constexpr uint32_t Offset() {return 32;}
  };
}
}
}