#pragma once
#include "win32/windows_headers.h"
#include "imgui/system_integration.h"
#include <imgui.h>
#include "imgui/renderer.h"

#include <functional>

namespace imgui
{
struct Renderer;
struct Context;
namespace win32
{
struct SystemIntegration : imgui::SystemIntegration
{
    SystemIntegration(size_t width, size_t height, char const* name);
    ~SystemIntegration();
    void                setup_imgui() override;
    void                set_context(Context* context) override;
    void                set_renderer(Renderer* renderer) override;
    void                set_ui_call(std::function<void(Context*)>&& fun) override;
    void                execute_once() override;
    bool                in_cooperative_environment() override;
    std::pair<int, int> window_size() override;
    inline HWND         handle() { return window_handle; }
    void                update_mouse_cursor();
    void                update_mouse_pos();
    inline void         needs_gamepad_update() { want_update_has_gamepad = true; }
    inline void         resize_backbuffer(size_t w, size_t h) { renderer->resize(w, h); }

   private:
    void update_gamepad();
    void process_event();
    void reset_mouse_state();
    void loop();
    void update_imgui_state();
    void cleanup_imgui_state();

    std::function<void(Context*)> create_ui;
    int                           width{0}, height{0};
    imgui::Renderer*              renderer{nullptr};
    imgui::Context*               context{nullptr};
    int                           mouse_status[8];
    WNDCLASSEX                    wc;
    HWND                          window_handle{nullptr};
    INT64                         run_time{0};
    INT64                         ticks_per_second{0};
    ImGuiMouseCursor              last_mouse_cursor       = ImGuiMouseCursor_COUNT;
    bool                          has_gamepad             = false;
    bool                          want_update_has_gamepad = true;
};
}  // namespace win32
}  // namespace imgui
