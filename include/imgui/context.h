#pragma once
#include <functional>
#include <memory>

namespace imgui {
struct SystemIntegration;
struct Renderer;
struct Style;

struct Context {
    Context(std::unique_ptr<SystemIntegration> integration,
            std::unique_ptr<Renderer> renderer,
            std::unique_ptr<Style> style,
            std::function<void(Context*)>&& fun);

    ~Context();

    // take over the current thread for UI execution
    void run();
    // run ui handling once - execute one step
    void run_once();

    /**
     * Loop control
     * \{
     */
    void stop();
    void resume();
    /**
     * \}
     */
    std::pair<int, int> window_size();

   private:
    std::unique_ptr<SystemIntegration> system;
    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<Style> style;
    bool continue_to_run{true};
};
}  // namespace imgui
