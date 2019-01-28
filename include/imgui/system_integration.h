#pragma once
#include <functional>

namespace imgui {
struct Renderer;
struct Context;

struct SystemIntegration {
    SystemIntegration() = default;
    SystemIntegration(SystemIntegration const&) = delete;
    SystemIntegration& operator=(SystemIntegration const&) = delete;
    virtual ~SystemIntegration() = default;

    virtual void setup_imgui() = 0;
    virtual void set_context(Context* context) = 0;
    virtual void set_renderer(Renderer* rend) = 0;
    virtual void set_ui_call(std::function<void(Context*)>&& f) = 0;
    virtual void execute_once() = 0;
    virtual bool in_cooperative_environment() = 0;
    virtual std::pair<int, int> window_size() = 0;
};

}  // namespace imgui
