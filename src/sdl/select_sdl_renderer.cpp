#include "select_sdl_renderer.h"
#include "SDL2/SDL.h"
#include "gl/renderer.h"
#include "sdl/system_integration.h"
// TODO only include GL and glew when needed..
#include <GL/glew.h>    // Initialize with glewInit()
#include <iostream>
//#include "vulkan/renderer.h"

namespace ig = imgui::gl;
// namespace iv = imgui::vulkan;
namespace is = imgui::sdl;
namespace {
std::pair<int,std::string> run_gl_init() {
    int version = 100;
    std::string ver_str;
#if __APPLE__
    // GL 3.2 Core + GLSL 150
    version = 150;
    ver_str = "#version 150\n";
    SDL_GL_SetAttribute(
        SDL_GL_CONTEXT_FLAGS,
        SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);  // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    // GL 3.0 + GLSL 130
    version = 130;
    ver_str = "#version 130\n";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif
    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_DisplayMode current;
    SDL_GetCurrentDisplayMode(0, &current);
    return {version, ver_str};
}
}  // namespace

std::pair<std::unique_ptr<imgui::SystemIntegration>, std::unique_ptr<imgui::Renderer>>
is::select_sdl_setup(size_t initial_width, size_t initial_height, std::string const& wn) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
        throw std::runtime_error("error");
    auto version = run_gl_init();
    auto try_window = SDL_CreateWindow(
        wn.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, initial_width,
        initial_height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    if (try_window) {
        auto context = SDL_GL_CreateContext(try_window);
        SDL_GL_MakeCurrent(try_window, context);
        SDL_GL_SetSwapInterval(1); // Enable vsync
        bool err = glewInit() != GLEW_OK;
        if (err) {
            std::cerr << "failed to initialized glew" << std::endl;
        }

        auto integration = std::make_unique<is::SystemIntegration>(try_window);
        auto renderer = std::make_unique<ig::Renderer>(version.first, version.second);
        //[context]() { SDL_GL_DeleteContext(context); });

        return {std::move(integration), std::move(renderer)};
    }

#if 0
    if (!try_window) {
        try_window = SDL_CreateWindow("window", SDL_WINDOWPOS_CENTERED,
                                      SDL_WINDOWPOS_CENTERED, initial_width,
                                      initial_height,
                                      SDL_WINDOW_VULKAN| SDL_WINDOW_RESIZABLE);
    }
    if (try_window) {
        auto context = SDL_VK_CreateContext(try_window);
        auto integration = std::make_unique<is::SystemIntegration>(try_window);
        auto renderer = std::make_unique<cuv::Renderer>(
        [context]() { SDL_VK_DeleteContext(context); });
        return {std::move(integration), std::move(renderer)};
    }
#endif
    return {nullptr, nullptr};
}
