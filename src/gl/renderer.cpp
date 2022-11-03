#include "renderer.h"
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <GL/gl.h>

namespace ig = imgui::gl;

ig::Renderer::Renderer() = default;

ig::Renderer::Renderer(int /*version_num*/, std::string const& version) : glsl_version{version} {}

ig::Renderer::~Renderer() { ImGui_ImplOpenGL3_Shutdown(); }

void ig::Renderer::render_imgui_data(ImDrawData& draw_data) { ImGui_ImplOpenGL3_RenderDrawData(&draw_data); }

void ig::Renderer::acquire_cached_resources()
{
    ImGui_ImplOpenGL3_CreateDeviceObjects();
    ImGui_ImplOpenGL3_CreateFontsTexture();
}

void ig::Renderer::setup_imgui() { ImGui_ImplOpenGL3_Init(glsl_version.c_str()); }
void ig::Renderer::finish_frame() {}
void ig::Renderer::pre_frame() {glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);}
void ig::Renderer::resize(size_t, size_t)
{ /**/
}

std::unique_ptr<imgui::Texture> ig::Renderer::create_texture(std::size_t width, std::size_t height, bool alpha)
{
    GLuint tex;
    glGenTextures(1, &tex);
    return std::make_unique<ig::Texture>(width, height, alpha, tex);
}

ig::Texture::operator ImTextureID() const { return reinterpret_cast<ImTextureID>(static_cast<intptr_t>(tex)); }

void ig::Texture::upload(unsigned char* buffer, std::size_t stride, std::size_t height, color_layout pixel_layout)
{
    // TODO no color formats yet...
    GLuint last_texture;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, reinterpret_cast<GLint*>(&last_texture));
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#ifdef GL_UNPACK_ROW_LENGTH  // Not on WebGL/ES
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, alpha ? GL_RGBA : GL_RGB, width, height, 0, alpha ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, buffer);
    glBindTexture(GL_TEXTURE_2D, last_texture);
}
