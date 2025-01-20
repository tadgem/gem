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
  SDL_GPUTextureFormat m_format;

  void init(SDL_GPUDevice* device, SDL_Window* window);

  SDL_GPUTexture* acquire_texture(SDL_GPUCommandBuffer* cmd_buf, SDL_Window* window);
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
  std::optional<SDL_GPUTextureFormat>             m_depth_target_format;
  std::optional<sdl::Texture>                     m_depth_target_texture;
  SDL_GPUGraphicsPipelineTargetInfo               m_graphics_pipeline_target_info;
  RenderPassData                                  m_render_pass_data;
};
}
}