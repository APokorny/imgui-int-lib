#pragma once

// internal header for replacing imgui rendering infrastructure

struct ImDrawData;
namespace imgui
{
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
};
}  // namespace imgui
