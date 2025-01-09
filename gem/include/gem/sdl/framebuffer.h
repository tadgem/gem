#pragma once
#include <vector>
#include <optional>
#include "gem/sdl/texture.h"
#include "SDL3/SDL.h"
#include "glm/glm.hpp"
namespace gem {
namespace sdl {

class Swapchain
{
public:

};

class Framebuffer {
public:

  struct RenderPassData
  {
    std::vector<SDL_GPUColorTargetInfo>           m_colour_targets;
    std::optional<SDL_GPUDepthStencilTargetInfo>  m_depth_target;

    bool is_built() { return !m_colour_targets.empty() || m_depth_target.has_value();}
  };

  void add_colour_attachment(SDL_GPUDevice* device, glm::ivec2 resolution,
                             SDL_GPUTextureFormat format);

  void add_depth_attachment(SDL_GPUDevice* device, glm::ivec2 resolution,
                            SDL_GPUTextureFormat format = SDL_GPU_TEXTUREFORMAT_D24_UNORM_S8_UINT);

  void build();

  SDL_GPURenderPass*  begin_render_pass(SDL_GPUCommandBuffer* cmd_buf);

  std::vector<  SDL_GPUColorTargetDescription>    m_colour_target_descriptions;
  std::vector<  sdl::Texture>                     m_colour_target_textures;
  std::optional<SDL_GPUDepthStencilTargetInfo>    m_depth_target_description;
  std::optional<sdl::Texture>                     m_depth_target_texture;

  RenderPassData                                  m_render_pass_data;
};
}
}