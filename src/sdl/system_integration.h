#pragma once
#include <SDL.h>
#include <functional>
#include "imgui/system_integration.h"

namespace imgui {
struct Renderer;
struct Context;
namespace sdl {
struct SystemIntegration : imgui::SystemIntegration {
    SystemIntegration(SDL_Window* window);
    ~SystemIntegration();
    void set_context(Context* context) override;
    void set_renderer(Renderer* renderer) override;
    void set_ui_call(std::function<void(Context*)>&& fun) override;
    void execute_once() override;

   private:
    void process_event(SDL_Event* const event);
    void reset_mouse_state();
    void loop();
    void update_imgui_state();
    void cleanup_imgui_state();

    std::function<void(Context*)> create_ui;
    SDL_Window* window{nullptr};
    int width{0}, height{0};
    imgui::Renderer* renderer{nullptr};
    imgui::Context* context{nullptr};
    int mouse_status[8];
};
}  // namespace sdl
}  // namespace imgui
