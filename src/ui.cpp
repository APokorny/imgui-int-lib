#include "imgui/ui.h"
#include "imgui/context.h"
#include "imgui/renderer.h"
#include "imgui/style.h"
#include "imgui/system_integration.h"

#if defined(EMSCRIPTEN)
#include <emscripten.h>
#include <emscripten/html5.h>
#include "emscripten/system_integration.h"
#include "gl/renderer.h"
#elif defined(SDL)
#include "sdl/select_sdl_renderer.h"
#include "sdl/system_integration.h"
#endif

#ifdef EMSCRIPTEN
namespace {
auto construct_gl_renderer() {
    EmscriptenWebGLContextAttributes attrs;
    emscripten_webgl_init_context_attributes(&attrs);
    attrs.enableExtensionsByDefault = 1;
    attrs.majorVersion = 2;
    attrs.minorVersion = 0;
    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context =
        emscripten_webgl_create_context(0, &attrs);
    if (!context) {
        printf("Skipped: WebGL 2 is not supported.\n");
        return std::unique_ptr<imgui::gl::Renderer>(nullptr);
    }
    emscripten_webgl_make_context_current(context);

    return std::make_unique<imgui::gl::Renderer>();
}
}  // namespace
#endif

std::unique_ptr<imgui::Context> imgui::create_ui(
    std::function<void(imgui::Context*)>&& fun, std::unique_ptr<Style> style,
    size_t initial_width, size_t initial_height) {
#ifdef EMSCRIPTEN
    return std::make_unique<imgui::Context>(
        std::make_unique<emscripten::SystemIntegration>(initial_width,
                                                        initial_height),
        construct_gl_renderer(), std::move(style), std::move(fun));
#elif defined(SDL)
    {
        auto sys = sdl::select_sdl_setup(initial_width, initial_height);
        return std::make_unique<Context>(std::move(sys.first),
                                         std::move(sys.second),
                                         std::move(style), std::move(fun));
    }
#endif
}
