#include "tech/utils.h"
#include "shader.h"
#include "shape.h"
#include "texture.h"

void tech::utils::dispatch_present_image(shader& present_shader, const std::string& uniform_name, const int texture_slot, gl_handle texture)
{
    present_shader.use();
    shapes::s_screen_quad.use();
    present_shader.set_int(uniform_name.c_str(), texture_slot);
    texture::bind_sampler_handle(texture, GL_TEXTURE0 + texture_slot);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    texture::bind_sampler_handle(0, GL_TEXTURE0);
}