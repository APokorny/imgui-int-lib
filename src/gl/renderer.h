#pragma once
#include <string>
#include "imgui/renderer.h"

struct ImDrawData;

namespace imgui
{
namespace gl
{
struct Renderer : imgui::Renderer
{
   public:
    std::string glsl_version;
    Renderer();
    Renderer(int glsl_version, std::string const& version);
    void setup_imgui() override;
    void acquire_cached_resources() override;
    void render_imgui_data(ImDrawData& draw_data) override;
    void pre_frame() override;
    void finish_frame() override;
    void resize(size_t w, size_t h) override;
    ~Renderer();
};

}  // namespace gl
}  // namespace imgui
