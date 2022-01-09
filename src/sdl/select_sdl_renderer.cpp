#include "select_sdl_renderer.h"

#include <imgui_impl_sdl.h>
#include "sdl/system_integration.h"
#ifdef GL
#include "gl/renderer.h"
#include <imgui_impl_opengl3.h>
#elif defined(VULKAN)
#include "vulkan/renderer.h"
#include <SDL2/SDL_vulkan.h>
#endif

#include <SDL2/SDL.h>
#include <iostream>

#ifdef GL
namespace ig = imgui::gl;
#elif defined(VULKAN)
namespace iv = imgui::vulkan;
#endif

namespace is = imgui::sdl;
namespace
{
std::pair<int, std::string> run_gl_init()
{
    int         version = 100;
    std::string ver_str;
#if __APPLE__
    // GL 3.2 Core + GLSL 150
    version = 150;
    ver_str = "#version 150\n";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,
                        SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);  // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    // GL 3.0 + GLSL 130
    version = 130;
    ver_str = "#version 130\n";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
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

std::pair<std::unique_ptr<imgui::SystemIntegration>, std::unique_ptr<imgui::Renderer>> is::select_sdl_setup(size_t initial_width,
                                                                                                            size_t initial_height,
                                                                                                            std::string const& wn)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) throw std::runtime_error("error");
#ifdef GL
    auto version    = run_gl_init();
    auto try_window = SDL_CreateWindow(wn.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, initial_width, initial_height,
                                       SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    if (try_window)
    {
        auto context = SDL_GL_CreateContext(try_window);
        SDL_GL_MakeCurrent(try_window, context);
        SDL_GL_SetSwapInterval(1);  // Enable vsync

        auto integration = std::make_unique<is::SystemIntegration>(try_window);
        auto renderer    = std::make_unique<ig::Renderer>(version.first, version.second);
        //[context]() { SDL_GL_DeleteContext(context); });

        return {std::move(integration), std::move(renderer)};
    }

#elif defined(VULKAN)
    auto try_window = SDL_CreateWindow(wn.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, initial_width, initial_height,
                                       SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    if (try_window)
    {
        int w, h;
        SDL_GetWindowSize(try_window, &w, &h);

        auto renderer = std::make_unique<iv::Renderer>(
            [try_window](VkInstance instance)
            {
                VkSurfaceKHR ret;
                SDL_Vulkan_CreateSurface(try_window, instance, &ret);
                return ret;
            },
            w, h);
        auto integration = std::make_unique<is::SystemIntegration>(try_window);
        //ImGui_ImplSDL2_InitForVulkan(try_window);
        return {std::move(integration), std::move(renderer)};
    }
#endif
    return {nullptr, nullptr};
}
