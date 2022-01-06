#include "renderer.h"
#include <imgui.h>

#include "imgui_impl_opengl3.h"

namespace ig = imgui::gl;

ig::Renderer::Renderer() = default;

ig::Renderer::Renderer(int /*version_num*/, std::string const& version) : glsl_version{version} {  }

ig::Renderer::~Renderer() { ImGui_ImplOpenGL3_Shutdown(); }

void ig::Renderer::render_imgui_data(ImDrawData& draw_data) { ImGui_ImplOpenGL3_RenderDrawData(&draw_data); }

void ig::Renderer::acquire_cached_resources()
{
    ImGui_ImplOpenGL3_CreateDeviceObjects();
    ImGui_ImplOpenGL3_CreateFontsTexture();
}

void ig::Renderer::setup_imgui() {ImGui_ImplOpenGL3_Init(glsl_version.c_str());}
void ig::Renderer::finish_frame() {}
void ig::Renderer::pre_frame() {}
void ig::Renderer::resize(size_t, size_t)
{ /**/
}
