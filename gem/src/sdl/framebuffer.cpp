#include "gem/sdl/framebuffer.h"
#include "gem/sdl/texture.h"
#include "gem/dbg_assert.h"

static constexpr auto COLOUR_ATTACHMENT_FLAGS = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COLOR_TARGET;
static constexpr auto DEPTH_ATTACHMENT_FLAGS = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;

void gem::sdl::Framebuffer::add_colour_attachment(SDL_GPUDevice* device,
                                                  glm::ivec2 resolution,
                                                  SDL_GPUTextureFormat format) {

  // TODO: Add shared define for max colour attachments in pipeline...
  GEM_ASSERT(6 > m_colour_target_descriptions.size(),
             "SDLGPU Framebuffer : Too Many Colour Attachments");


  sdl::Texture attachment =
      sdl::Texture::CreateTexture2D(device, format, resolution, COLOUR_ATTACHMENT_FLAGS);

  SDL_GPUColorTargetDescription description {};
  description.format = format;

  m_colour_target_descriptions.push_back(description);
  m_colour_target_textures.push_back(attachment);

}
void gem::sdl::Framebuffer::add_depth_attachment(SDL_GPUDevice* device,
                                                 glm::ivec2 resolution,
                                                 SDL_GPUTextureFormat format) {
  if(m_depth_target_format.has_value() || m_depth_target_texture.has_value())
  {
    spdlog::error("SDLGPU Framebuffer : A depth attachment has already been created for this framebuffer");
    return;
  }

  m_depth_target_texture =
      sdl::Texture::CreateTexture2D(device, format, resolution, DEPTH_ATTACHMENT_FLAGS);

  m_depth_target_format = format;

}
void gem::sdl::Framebuffer::build() {

  if(m_render_pass_data.is_built())
  {
    spdlog::error("SDLGPU Framebuffer : Framebuffer has already been built");
  }

  if(m_depth_target_format.has_value())
  {
    m_render_pass_data.m_depth_target = SDL_GPUDepthStencilTargetInfo {};
    m_render_pass_data.m_depth_target.value().texture = m_depth_target_texture.value().m_texture;
  }

  m_render_pass_data.m_colour_targets.resize(m_colour_target_descriptions.size());

  for(int i = 0; i < m_colour_target_descriptions.size(); i++)
  {
    m_render_pass_data.m_colour_targets[i].texture = m_colour_target_textures[i].m_texture;
    // TODO : Make this configurable
    m_render_pass_data.m_colour_targets[i].clear_color = SDL_FColor {0.0f,0.0f,0.0f,0.0f};
    m_render_pass_data.m_colour_targets[i].load_op = SDL_GPU_LOADOP_CLEAR;
    m_render_pass_data.m_colour_targets[i].store_op = SDL_GPU_STOREOP_STORE;
  }

  m_graphics_pipeline_target_info.num_color_targets = m_colour_target_descriptions.size();
  m_graphics_pipeline_target_info.color_target_descriptions = m_colour_target_descriptions.data();
  if(m_depth_target_format.has_value())
  {
    m_graphics_pipeline_target_info.has_depth_stencil_target = true;
    m_graphics_pipeline_target_info.depth_stencil_format = m_depth_target_format.value();
  }
  else
  {
    m_graphics_pipeline_target_info.has_depth_stencil_target = false;

  }
}

SDL_GPURenderPass *
gem::sdl::Framebuffer::begin_render_pass(SDL_GPUCommandBuffer *cmd_buf) {
  const SDL_GPUDepthStencilTargetInfo * depth_info = nullptr;
  if(m_render_pass_data.m_depth_target.has_value())
  {
    depth_info = &m_render_pass_data.m_depth_target.value();
  }
  return SDL_BeginGPURenderPass(
      cmd_buf,
      m_render_pass_data.m_colour_targets.data(),
      m_render_pass_data.m_colour_targets.size(),
      depth_info);
}

void gem::sdl::Swapchain::init(SDL_GPUDevice *device, SDL_Window *window) {
  m_format = SDL_GetGPUSwapchainTextureFormat(device, window);
}

SDL_GPUTexture *
gem::sdl::Swapchain::acquire_texture(SDL_GPUCommandBuffer *cmd_buf,
                                     SDL_Window *window) {
  SDL_GPUTexture* tex = nullptr;
  uint32_t w, h;
  if (!SDL_AcquireGPUSwapchainTexture(cmd_buf, window, &tex, &w, &h)) {
    spdlog::error("WaitAndAcquireGPUSwapchainTexture failed: %s", SDL_GetError());
  }
  return tex;
}
