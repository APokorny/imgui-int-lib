#include "system_integration.h"

#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/key_codes.h>
#include <iostream>
#include "imgui.h"
#include "imgui/context.h"

namespace ie = imgui::emscripten;

ie::SystemIntegration::SystemIntegration(size_t initial_width,
                                         size_t initial_height)
    : create_ui{[](Context*) {}} {
    emscripten_set_canvas_element_size("", initial_width, initial_height);
    char const* target = nullptr;
    void* data = this;
    pthread_t this_thread = EM_CALLBACK_THREAD_CONTEXT_CALLING_THREAD;
    emscripten_set_keydown_callback_on_thread(
        "#window", data, false,
        [](int type, EmscriptenKeyboardEvent const* event, void*) -> int {
            ImGuiIO& io = ImGui::GetIO();
            std::cout << "Key Down " << event->keyCode << std::endl;
            io.KeysDown[event->keyCode & 511] = 1;
            io.KeyCtrl = event->ctrlKey;
            io.KeyShift = event->shiftKey;
            io.KeyAlt = event->altKey;
            io.KeySuper = event->metaKey;
            return true;
        },
        this_thread);
    emscripten_set_keyup_callback_on_thread(
        "#window", data, false,
        [](int type, EmscriptenKeyboardEvent const* event, void*) -> int {
            ImGuiIO& io = ImGui::GetIO();
            std::cout << "Key Up " << event->keyCode << std::endl;
            io.KeysDown[event->keyCode & 511] = 0;
            io.KeyCtrl = event->ctrlKey;
            io.KeyShift = event->shiftKey;
            io.KeyAlt = event->altKey;
            io.KeySuper = event->metaKey;
            return true;
        },
        this_thread);
    emscripten_set_click_callback_on_thread(
        target, data, true,
        [](int type, EmscriptenMouseEvent const* event, void*) -> int {
            return false;
        },
        this_thread);
    emscripten_set_mousedown_callback_on_thread(
        target, data, true,
        [](int type, EmscriptenMouseEvent const* event, void* obj) -> int {
            ImGuiIO& io = ImGui::GetIO();
            SystemIntegration* self = static_cast<SystemIntegration*>(obj);
            self->mouse_status[event->button] = 1;
            return false;
        },
        this_thread);

    emscripten_set_mouseup_callback_on_thread(
        target, data, true,
        [](int type, EmscriptenMouseEvent const* event, void* obj) -> int {
            ImGuiIO& io = ImGui::GetIO();
            SystemIntegration* self = static_cast<SystemIntegration*>(obj);
            self->mouse_status[event->button] = 2;

            return false;
        },
        this_thread);
    emscripten_set_dblclick_callback_on_thread(
        target, data, true,
        [](int type, EmscriptenMouseEvent const* event, void*) -> int {
            return 0;
        },
        this_thread);
    emscripten_set_mousemove_callback_on_thread(
        target, data, true,
        [](int type, EmscriptenMouseEvent const* event, void*) -> int {
            ImGuiIO& io = ImGui::GetIO();
            io.MousePos = ImVec2(static_cast<float>(event->canvasX),
                                 static_cast<float>(event->canvasY));
            return 0;
        },
        this_thread);
    emscripten_set_mouseenter_callback_on_thread(
        target, data, true,
        [](int type, EmscriptenMouseEvent const* event, void*) -> int {
            return 0;
        },
        this_thread);
    emscripten_set_mouseleave_callback_on_thread(
        target, data, true,
        [](int type, EmscriptenMouseEvent const* event, void*) -> int {
            ImGuiIO& io = ImGui::GetIO();
            io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
            return 0;
        },
        this_thread);
    emscripten_set_mouseover_callback_on_thread(
        target, data, true,
        [](int type, EmscriptenMouseEvent const* event, void*) -> int {
            return 0;
        },
        this_thread);
    emscripten_set_mouseout_callback_on_thread(
        target, data, true,
        [](int type, EmscriptenMouseEvent const* event, void*) -> int {
            return 0;
        },
        this_thread);
    emscripten_set_wheel_callback_on_thread(
        target, data, true,
        [](int type, EmscriptenWheelEvent const* event, void*) -> int {
            ImGuiIO& io = ImGui::GetIO();
            io.MouseWheel += event->deltaY;
            io.MouseWheelH += event->deltaX;

            return 0;
        },
        this_thread);
    emscripten_set_resize_callback_on_thread(
        target, data, true,
        [](int type, EmscriptenUiEvent const* event, void* obj) -> int {
            SystemIntegration* self = static_cast<SystemIntegration*>(obj);
            self->width = event->windowInnerWidth;
            self->height = event->windowInnerHeight;
            return 0;
        },
        this_thread);
    emscripten_set_scroll_callback_on_thread(
        target, data, true,
        [](int type, EmscriptenUiEvent const* event, void*) -> int {
            return 0;
        },
        this_thread);
    emscripten_set_blur_callback_on_thread(
        target, data, true,
        [](int type, EmscriptenFocusEvent const* event, void*) -> int {
            std::cout << "blur callback" << std::endl;
            return 0;
        },
        this_thread);
    emscripten_set_focus_callback_on_thread(
        target, data, true,
        [](int type, EmscriptenFocusEvent const* event, void*) -> int {
            std::cout << "focus callback" << std::endl;
            return 0;
        },
        this_thread);
    emscripten_set_focusin_callback_on_thread(
        target, data, true,
        [](int type, EmscriptenFocusEvent const* event, void*) -> int {
            std::cout << "focus in callback" << std::endl;
            return 0;
        },
        this_thread);
    emscripten_set_focusout_callback_on_thread(
        target, data, true,
        [](int type, EmscriptenFocusEvent const* event, void*) -> int {
            std::cout << "focus out in callback" << std::endl;
            return 0;
        },
        this_thread);
    emscripten_set_deviceorientation_callback_on_thread(
        data, true,
        [](int type, EmscriptenDeviceOrientationEvent const* event,
           void*) -> int { return 0; },
        this_thread);
    emscripten_set_devicemotion_callback_on_thread(
        data, true,
        [](int type, EmscriptenDeviceMotionEvent const* event, void*) -> int {
            return 0;
        },
        this_thread);
    emscripten_set_main_loop_arg(
        [](void* arg) {
            auto self = static_cast<emscripten::SystemIntegration*>(arg);
            self->loop();
        },
        this, 0, 0);
}

void ie::SystemIntegration::setup_imgui() {
    ImGuiIO& io = ImGui::GetIO();
    io.KeyMap[ImGuiKey_Tab] = DOM_VK_TAB;
    io.KeyMap[ImGuiKey_LeftArrow] = DOM_VK_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = DOM_VK_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = DOM_VK_UP;
    io.KeyMap[ImGuiKey_DownArrow] = DOM_VK_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = DOM_VK_PAGE_UP;
    io.KeyMap[ImGuiKey_PageDown] = DOM_VK_PAGE_DOWN;
    io.KeyMap[ImGuiKey_Home] = DOM_VK_HOME;
    io.KeyMap[ImGuiKey_End] = DOM_VK_END;
    io.KeyMap[ImGuiKey_Insert] = DOM_VK_INSERT;
    io.KeyMap[ImGuiKey_Delete] = DOM_VK_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = DOM_VK_BACK_SPACE;
    io.KeyMap[ImGuiKey_Space] = DOM_VK_SPACE;
    io.KeyMap[ImGuiKey_Enter] = DOM_VK_ENTER;
    io.KeyMap[ImGuiKey_Escape] = DOM_VK_ESCAPE;
    io.KeyMap[ImGuiKey_A] = DOM_VK_A;
    io.KeyMap[ImGuiKey_C] = DOM_VK_C;
    io.KeyMap[ImGuiKey_V] = DOM_VK_V;
    io.KeyMap[ImGuiKey_X] = DOM_VK_X;
    io.KeyMap[ImGuiKey_Y] = DOM_VK_Y;
    io.KeyMap[ImGuiKey_Z] = DOM_VK_Z;
}

void ie::SystemIntegration::update_imgui_state() {
    ImGuiIO& io = ImGui::GetIO();
    io.DeltaTime = 1.0f / 30.0f;

    emscripten_get_canvas_element_size("", &width, &height);
    if (width * height == 0) {
        std::cout << "no frame size" << std::endl;
        return;
    }
    io.DisplaySize =
        ImVec2(static_cast<float>(width), static_cast<float>(height));
    io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

    // put up the new mouse state and input state
    io.MouseDown[0] = (mouse_status[0] != 0);
    io.MouseDown[1] = (mouse_status[1] != 0);
    io.MouseDown[2] = (mouse_status[2] != 0);
}

void ie::SystemIntegration::cleanup_imgui_state() {
    // put up the new mouse state and input state
    reset_mouse_state();
}

void ie::SystemIntegration::loop() {
    update_imgui_state();
    if (renderer && context) {
        ImGui::NewFrame();
        create_ui(context);
        ImGui::EndFrame();
        ImGui::Render();
        renderer->render_imgui_data(*ImGui::GetDrawData());
        renderer->finish_frame();
        emscripten_webgl_commit_frame();

        reset_mouse_state();
    }
    cleanup_imgui_state();
}

bool ie::SystemIntegration::in_cooperative_environment() { return true; }
void ie::SystemIntegration::execute_once() { loop(); }

void ie::SystemIntegration::reset_mouse_state() {
    for (auto& status : mouse_status)
        if (status == 2) status = 0;
}

void ie::SystemIntegration::set_renderer(Renderer* r) { renderer = r; }

void ie::SystemIntegration::set_ui_call(std::function<void(Context*)>&& f) {
    create_ui = std::move(f);
}

void ie::SystemIntegration::set_context(imgui::Context* c) { context = c; }
std::pair<int, int> ie::SystemIntegration::window_size() {
    return {width, height};
}

ie::SystemIntegration::~SystemIntegration() = default;
