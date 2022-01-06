#include "system_integration.h"
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_vulkan.h>
#include "imgui/context.h"
#include <imgui.h>
#include "imgui/renderer.h"
#include "imgui_impl_sdl.h"

namespace is = imgui::sdl;

is::SystemIntegration::SystemIntegration(SDL_Window* win) : window(win) {}

void is::SystemIntegration::setup_imgui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    ImGui_ImplSDL2_InitForMetal(window);
}

is::SystemIntegration::~SystemIntegration()
{
    SDL_DestroyWindow(window);
    SDL_Quit();  // single window for now..
}
void is::SystemIntegration::loop()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_QUIT) context->stop();
        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
            event.window.windowID == SDL_GetWindowID(window))
            context->stop();
    }
    update_imgui_state();
    ImGui_ImplSDL2_NewFrame();

    if (renderer && context)
    {
        ImGui::NewFrame();

        create_ui(context);
        ImGui::EndFrame();
        ImGui::Render();

        auto       draw_data    = ImGui::GetDrawData();
        const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
        if (!is_minimized)
        {
            renderer->pre_frame();
            renderer->render_imgui_data(*draw_data);
            renderer->finish_frame();

            SDL_GL_SwapWindow(window);
        }

        reset_mouse_state();
    }
    cleanup_imgui_state();
}

void is::SystemIntegration::cleanup_imgui_state()
{
    // put up the new mouse state and input state
    reset_mouse_state();
}

void is::SystemIntegration::reset_mouse_state() {}

void is::SystemIntegration::update_imgui_state()
{
    ImGuiIO& io  = ImGui::GetIO();
    io.DeltaTime = 1.0f / 30.0f;
}

bool is::SystemIntegration::in_cooperative_environment() { return false; }
void is::SystemIntegration::execute_once() { loop(); }

void is::SystemIntegration::set_context(Context* c) { context = c; }
void is::SystemIntegration::set_renderer(Renderer* r) { renderer = r; }

void                is::SystemIntegration::set_ui_call(std::function<void(Context*)>&& f) { create_ui = std::move(f); }
std::pair<int, int> is::SystemIntegration::window_size() { return {width, height}; }
