#pragma once

// internal header for replacing imgui rendering infrastructure

struct ImDrawData;
namespace imgui {
class Renderer {
   public:
    Renderer() = default;
    virtual ~Renderer() = default;
    //! prepare font files and other data
    //! This call may occur multiple times if the hardware context got lost.
    virtual void acquire_cached_resources() = 0;
    //! renders the ui data of ImGui - but does not flush/swap..
    virtual void render_imgui_data(ImDrawData& data) = 0;
    //! Use this call to perform buffer swapping or flushing
    virtual void finish_frame() = 0;
};
}  // namespace imgui
