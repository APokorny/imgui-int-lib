#pragma once
// internal header for integrating imgui backends
//
#include <imgui_internal.h>
#include <memory>

struct ImDrawData;
namespace imgui
{
enum color_layout
{
    none
};
struct Texture
{
    std::size_t width{0};
    std::size_t height{0};
    bool        alpha{false};
    Texture() = default;
    Texture(std::size_t width, std::size_t height, bool alpha) : width(width), height(height), alpha(alpha) {}
    Texture(Texture const&) = default;
    Texture(Texture&&)      = delete;
    Texture&     operator=(Texture const&) = delete;
    Texture&     operator=(Texture&&)                                                                             = delete;
    virtual      operator ImTextureID() const                                                                     = 0;
    virtual void upload(unsigned char* buffer, std::size_t stride, std::size_t height, color_layout pixel_layout) = 0;
    virtual ~Texture()                                                                                            = default;
};

struct Renderer
{
    Renderer()          = default;
    virtual ~Renderer() = default;
    //! Call after initial setup of imgui
    virtual void setup_imgui() = 0;
    //! prepare font files and other data
    //! This call may occur multiple times if the hardware context got lost.
    virtual void acquire_cached_resources() = 0;
    //! renders the ui data of ImGui - but does not flush/swap..
    virtual void render_imgui_data(ImDrawData& data) = 0;
    //! Preparation for next imgui render
    virtual void pre_frame() = 0;
    //! Use this call to perform buffer swapping or flushing
    virtual void finish_frame() = 0;
    //! hw buffer got resized
    virtual void resize(size_t w, size_t h) = 0;

    virtual std::unique_ptr<Texture> create_texture(std::size_t width, std::size_t height, bool alpha) = 0;
};
}  // namespace imgui
