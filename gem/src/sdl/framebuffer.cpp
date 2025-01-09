#include "gem/sdl/framebuffer.h"
#include "gem/sdl/texture.h"
#include "gem/dbg_assert.h"

void gem::sdl::framebuffer::add_colour_attachment(SDL_GPUDevice* device,
                                                  glm::ivec2 resolution,
                                                  SDL_GPUTextureFormat format) {

  // TODO: Add shared define for max colour attachments in pipeline...
  GEM_ASSERT(m_colour_target_descriptions.size() < 6,
             "SDLGPU Framebuffer : Too Many Colour Attachments");


}
void gem::sdl::framebuffer::add_depth_attachment(SDL_GPUDevice* device,
                                                 glm::ivec2 resolution,
                                                 SDL_GPUTextureFormat format) {

}
