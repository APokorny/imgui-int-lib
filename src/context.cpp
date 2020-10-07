#include "imgui/context.h"
#include "imgui/renderer.h"
#include "imgui/style.h"
#include "imgui/system_integration.h"
#include "imgui/imgui.h"

#include <iostream>

imgui::Context::Context(std::unique_ptr<SystemIntegration> integration,
                        std::unique_ptr<Renderer> r,
                        std::unique_ptr<Style> s,
                        std::function<void(imgui::Context*)>&& fun)
    : system(std::move(integration)), renderer(std::move(r)), style(std::move(s)) {
    ImGui::CreateContext(style->get_font_atlas());

    system->set_renderer(this->renderer.get());
    system->set_context(this);
    system->set_ui_call(std::move(fun));

    system->setup_imgui();
    renderer->setup_imgui();
    renderer->acquire_cached_resources();
    style->apply_style();
}

void imgui::Context::run() {
    while (continue_to_run) {
        run_once();
        if (system->in_cooperative_environment()) return;
    }
}
void imgui::Context::run_once() { system->execute_once(); }

imgui::Context::~Context() = default;
void imgui::Context::stop() { continue_to_run = false; }
void imgui::Context::resume() { continue_to_run = true; }
std::pair<int, int> imgui::Context::window_size() { return system->window_size(); }
