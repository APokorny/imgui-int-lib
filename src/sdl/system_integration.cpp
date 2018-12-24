#include "system_integration.h"
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_vulkan.h>
#include <iostream>
#include "imgui/renderer.h"
#include "imgui/context.h"
#include "imgui/imgui.h"

namespace is = imgui::sdl;

is::SystemIntegration::SystemIntegration(SDL_Window* win) : window(win) {}

is::SystemIntegration::~SystemIntegration() {
    SDL_DestroyWindow(window);
    SDL_Quit();  // single window for now..
}
void is::SystemIntegration::process_event(SDL_Event* const event) {
    ImGuiIO& io = ImGui::GetIO();
    switch (event->type) {
        case SDL_MOUSEWHEEL:
            if (event->wheel.x > 0) io.MouseWheelH += 1;
            if (event->wheel.x < 0) io.MouseWheelH -= 1;
            if (event->wheel.y > 0) io.MouseWheel += 1;
            if (event->wheel.y < 0) io.MouseWheel -= 1;
            break;
        case SDL_MOUSEBUTTONDOWN:
            if (event->button.button == SDL_BUTTON_LEFT) mouse_status[0] = true;
            if (event->button.button == SDL_BUTTON_RIGHT)
                mouse_status[1] = true;
            if (event->button.button == SDL_BUTTON_MIDDLE)
                mouse_status[2] = true;
            break;
        case SDL_TEXTINPUT:
            io.AddInputCharactersUTF8(event->text.text);
            break;
        case SDL_KEYDOWN:  // fallthrough
        case SDL_KEYUP:
            int key = event->key.keysym.scancode;
            IM_ASSERT(key >= 0 && key < IM_ARRAYSIZE(io.KeysDown));
            io.KeysDown[key] = (event->type == SDL_KEYDOWN);
            io.KeyShift = ((SDL_GetModState() & KMOD_SHIFT) != 0);
            io.KeyCtrl = ((SDL_GetModState() & KMOD_CTRL) != 0);
            io.KeyAlt = ((SDL_GetModState() & KMOD_ALT) != 0);
            io.KeySuper = ((SDL_GetModState() & KMOD_GUI) != 0);
            break;
    }
}

void is::SystemIntegration::loop() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        process_event(&event);
        if (event.type == SDL_QUIT) context->stop();
        if (event.type == SDL_WINDOWEVENT &&
            event.window.event == SDL_WINDOWEVENT_CLOSE &&
            event.window.windowID == SDL_GetWindowID(window))
            context->stop();
    }
    update_imgui_state();

    if (renderer && context) {
        ImGui::NewFrame();

        create_ui(context);
        ImGui::EndFrame();
        ImGui::Render();
        renderer->render_imgui_data(*ImGui::GetDrawData());
        renderer->finish_frame();

        SDL_GL_SwapWindow(window);

        reset_mouse_state();
    }
    cleanup_imgui_state();
}

void is::SystemIntegration::cleanup_imgui_state() {
    // put up the new mouse state and input state
    reset_mouse_state();
}

void is::SystemIntegration::reset_mouse_state() {
}

void is::SystemIntegration::update_imgui_state() {
    ImGuiIO& io = ImGui::GetIO();
    io.DeltaTime = 1.0f / 30.0f;

    // get width and height
    width = 0;
    height = 0;
    auto check_drawable_size = [&](auto fun) {
        int w = 0, h = 0;
        fun(window, &w, &h);
        width = std::max(w, width);
        height = std::max(h, height);
    };
    check_drawable_size(SDL_GetWindowSize);
    check_drawable_size(SDL_Vulkan_GetDrawableSize);
    check_drawable_size(SDL_GL_GetDrawableSize);

    if (width * height == 0) {
        return;
    }
    io.DisplaySize =
        ImVec2(static_cast<float>(width), static_cast<float>(height));
    io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

    int mx, my;
    uint32_t mouse_buttons = SDL_GetMouseState(&mx, &my);
    // put up the new mouse state and input state
    io.MouseDown[0] =
        (mouse_status[0] != 0) || mouse_buttons & SDL_BUTTON(SDL_BUTTON_LEFT);
    io.MouseDown[1] =
        (mouse_status[1] != 0) || mouse_buttons & SDL_BUTTON(SDL_BUTTON_RIGHT);
    io.MouseDown[2] =
        (mouse_status[2] != 0) || mouse_buttons & SDL_BUTTON(SDL_BUTTON_MIDDLE);
    for (auto& status : mouse_status) status = 0;
    if (SDL_GetWindowFlags(window) & SDL_WINDOW_INPUT_FOCUS)
        io.MousePos = ImVec2(static_cast<float>(mx), static_cast<float>(my));
}

void is::SystemIntegration::execute_once() { loop(); }

void is::SystemIntegration::set_context(Context* c) { context = c; }
void is::SystemIntegration::set_renderer(Renderer* r) { renderer = r; }

void is::SystemIntegration::set_ui_call(
    std::function<void(Context*)>&& f) {
    create_ui = std::move(f);
}
