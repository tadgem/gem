#include "gem/sdl/framebuffer.h"
#include "gem/sdl/texture.h"
#include "gem/dbg_assert.h"

static constexpr auto FB_FLAGS = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COLOR_TARGET;

void gem::sdl::Framebuffer::add_colour_attachment(SDL_GPUDevice* device,
                                                  glm::ivec2 resolution,
                                                  SDL_GPUTextureFormat format) {

  // TODO: Add shared define for max colour attachments in pipeline...
  GEM_ASSERT(6 > m_colour_target_descriptions.size(),
             "SDLGPU Framebuffer : Too Many Colour Attachments");


  sdl::Texture attachment =
      sdl::Texture::CreateTexture2D(device, format, resolution, FB_FLAGS);

  SDL_GPUColorTargetDescription description {};
  description.format = format;

  m_colour_target_descriptions.push_back(description);
  m_colour_target_textures.push_back(attachment);

}
void gem::sdl::Framebuffer::add_depth_attachment(SDL_GPUDevice* device,
                                                 glm::ivec2 resolution,
                                                 SDL_GPUTextureFormat format) {
  if(m_depth_target_description.has_value() || m_depth_target_texture.has_value())
  {
    spdlog::error("SDLGPU Framebuffer : A depth attachment has already been created for this framebuffer");
    return;
  }

  m_depth_target_texture =
      sdl::Texture::CreateTexture2D(device, format, resolution, FB_FLAGS);

  SDL_GPUColorTargetDescription description {};
  description.format = format;
  m_depth_target_description.value().texture = m_depth_target_texture->m_texture;

}
void gem::sdl::Framebuffer::build() {

  if(m_render_pass_data.is_built())
  {
    spdlog::error("SDLGPU Framebuffer : Framebuffer has already been built");
  }

  if(m_depth_target_description.has_value())
  {
    m_render_pass_data.m_depth_target = m_depth_target_description;
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
