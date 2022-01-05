#pragma once
#include <functional>
#include "imgui/renderer.h"
#include "imgui/system_integration.h"

namespace imgui {
namespace emscripten {
struct SystemIntegration : imgui::SystemIntegration {
    Renderer* renderer;
    int width, height;
    int mouse_status[8];
    std::function<void(Context*)> create_ui;
    SystemIntegration();
    ~SystemIntegration();
    void loop();
    void setup_imgui() override;
    void set_renderer(Renderer* renderer) override;
    void set_context(Context* context) override;
    void set_ui_call(std::function<void(Context*)>&& fun) override;
    void execute_once() override;
    bool in_cooperative_environment() override;
    std::pair<int, int> window_size() override;

   private:
    Context* context{nullptr};
    void update_imgui_state();
    void cleanup_imgui_state();
    void reset_mouse_state();
};

}  // namespace emscripten
}  // namespace imgui
