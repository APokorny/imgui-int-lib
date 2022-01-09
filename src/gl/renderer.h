#pragma once
#include <string>
#include "imgui/renderer.h"

struct ImDrawData;

namespace imgui
{
namespace gl
{
struct Texture : imgui::Texture
{
    unsigned int tex{0};

    Texture(std::size_t width, std::size_t height, bool alpha, unsigned int tex) : imgui::Texture(width, height, alpha), tex(tex) {}
    operator ImTextureID() const override;
    void     upload(unsigned char* buffer, std::size_t stride, std::size_t height, color_layout pixel_layout) override;
};
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

    std::unique_ptr<imgui::Texture> create_texture(std::size_t w, std::size_t h, bool alpha) override;
    ~Renderer();
};

}  // namespace gl
}  // namespace imgui
