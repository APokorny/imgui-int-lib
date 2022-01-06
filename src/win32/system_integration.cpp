#include "win32/windows_headers.h"
#include <tchar.h>
#include <dbt.h>

#include "system_integration.h"
#include "imgui/context.h"
#include "imgui/renderer.h"

#include <unordered_map>
#include <iostream>
#include <iomanip>

namespace iw = imgui::win32;

namespace
{
static std::unordered_map<HWND, iw::SystemIntegration*> active_instances;
LRESULT                                                 WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    iw::SystemIntegration* ptr = nullptr;
    if (hwnd) ptr = active_instances[hwnd];
    switch (msg)
    {
        case WM_LBUTTONDOWN:
        case WM_LBUTTONDBLCLK:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONDBLCLK:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONDBLCLK:
        case WM_XBUTTONDOWN:
        case WM_XBUTTONDBLCLK:
        {
            ImGuiIO& io     = ImGui::GetIO();
            int      button = 0;
            if (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONDBLCLK) { button = 0; }
            if (msg == WM_RBUTTONDOWN || msg == WM_RBUTTONDBLCLK) { button = 1; }
            if (msg == WM_MBUTTONDOWN || msg == WM_MBUTTONDBLCLK) { button = 2; }
            if (msg == WM_XBUTTONDOWN || msg == WM_XBUTTONDBLCLK) { button = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? 3 : 4; }
            if (!ImGui::IsAnyMouseDown() && ::GetCapture() == NULL) ::SetCapture(hwnd);
            io.MouseDown[button] = true;
            return 0;
        }
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
        case WM_XBUTTONUP:
        {
            ImGuiIO& io     = ImGui::GetIO();
            int      button = 0;
            if (msg == WM_LBUTTONUP) { button = 0; }
            if (msg == WM_RBUTTONUP) { button = 1; }
            if (msg == WM_MBUTTONUP) { button = 2; }
            if (msg == WM_XBUTTONUP) { button = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? 3 : 4; }
            io.MouseDown[button] = false;
            if (!ImGui::IsAnyMouseDown() && ::GetCapture() == hwnd) ::ReleaseCapture();
            return 0;
        }
        case WM_MOUSEWHEEL:
        {
            ImGuiIO& io = ImGui::GetIO();
            io.MouseWheel += (float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA;
            return 0;
        }
        case WM_MOUSEHWHEEL:
        {
            ImGuiIO& io = ImGui::GetIO();
            io.MouseWheelH += (float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA;
            return 0;
        }

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        {
            ImGuiIO& io = ImGui::GetIO();
            if (wParam < 256) io.KeysDown[wParam] = 1;
            return 0;
        }
        case WM_KEYUP:
        case WM_SYSKEYUP:
        {
            ImGuiIO& io = ImGui::GetIO();
            if (wParam < 256) io.KeysDown[wParam] = 0;
            return 0;
        }
        case WM_CHAR:
        {
            ImGuiIO& io = ImGui::GetIO();
            // You can also use ToAscii()+GetKeyboardState() to retrieve characters.
            if (wParam > 0 && wParam < 0x10000) io.AddInputCharacterUTF16((unsigned short)wParam);
            return 0;
        }
        case WM_SETCURSOR:
            if (LOWORD(lParam) == HTCLIENT && ptr)
            {
                ptr->update_mouse_cursor();
                return 1;
            }
            return 0;
        case WM_DEVICECHANGE:
            if ((UINT)wParam == DBT_DEVNODES_CHANGED) ptr->needs_gamepad_update();
            return 0;
        case WM_SIZE:
            if (wParam != SIZE_MINIMIZED) { ptr->resize_backbuffer(LOWORD(lParam), HIWORD(lParam)); }
            return 0;
        case WM_SYSCOMMAND:
            if ((wParam & 0xfff0) == SC_KEYMENU)  // Disable ALT application menu
                return 0;
            break;
        case WM_DESTROY: ::PostQuitMessage(0); return 0;
    }
    return ::DefWindowProc(hwnd, msg, wParam, lParam);
}
}  // namespace

iw::SystemIntegration::SystemIntegration(size_t initial_width, size_t initial_height, char const* name)
    :

      wc{sizeof(WNDCLASSEX), CS_CLASSDC, WndProcHandler, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, name, NULL}
{
    ::RegisterClassEx(&wc);
    window_handle = ::CreateWindow(wc.lpszClassName, name, WS_OVERLAPPEDWINDOW, 100, 100, initial_width, initial_height, NULL, NULL,
                                   wc.hInstance, NULL);
    if (!window_handle) std::cerr << "Failed to create window\n";
    active_instances[window_handle] = this;
}

void iw::SystemIntegration::setup_imgui()
{
    ::ShowWindow(window_handle, SW_SHOWDEFAULT);
    ::UpdateWindow(window_handle);
    if (!::QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&ticks_per_second))) return;
    if (!::QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&run_time))) return;

    // Setup back-end capabilities flags
    ImGuiIO& io = ImGui::GetIO();
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;  // We can honor GetMouseCursor() values (optional)
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;   // We can honor io.WantSetMousePos requests (optional, rarely used)
    io.BackendPlatformName = "imgui_impl_win32";
    io.ImeWindowHandle = window_handle;

    // Keyboard mapping. ImGui will use those indices to peek into the io.KeysDown[] array that we will update during the application
    // lifetime.
    io.KeyMap[ImGuiKey_Tab]         = VK_TAB;
    io.KeyMap[ImGuiKey_LeftArrow]   = VK_LEFT;
    io.KeyMap[ImGuiKey_RightArrow]  = VK_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow]     = VK_UP;
    io.KeyMap[ImGuiKey_DownArrow]   = VK_DOWN;
    io.KeyMap[ImGuiKey_PageUp]      = VK_PRIOR;
    io.KeyMap[ImGuiKey_PageDown]    = VK_NEXT;
    io.KeyMap[ImGuiKey_Home]        = VK_HOME;
    io.KeyMap[ImGuiKey_End]         = VK_END;
    io.KeyMap[ImGuiKey_Insert]      = VK_INSERT;
    io.KeyMap[ImGuiKey_Delete]      = VK_DELETE;
    io.KeyMap[ImGuiKey_Backspace]   = VK_BACK;
    io.KeyMap[ImGuiKey_Space]       = VK_SPACE;
    io.KeyMap[ImGuiKey_Enter]       = VK_RETURN;
    io.KeyMap[ImGuiKey_Escape]      = VK_ESCAPE;
    io.KeyMap[ImGuiKey_KeyPadEnter] = VK_RETURN;
    io.KeyMap[ImGuiKey_A]           = 'A';
    io.KeyMap[ImGuiKey_C]           = 'C';
    io.KeyMap[ImGuiKey_V]           = 'V';
    io.KeyMap[ImGuiKey_X]           = 'X';
    io.KeyMap[ImGuiKey_Y]           = 'Y';
    io.KeyMap[ImGuiKey_Z]           = 'Z';
}

void iw::SystemIntegration::process_event() {}

void iw::SystemIntegration::loop()
{
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
    {
        ::TranslateMessage(&msg);
        if (msg.message == WM_QUIT) context->stop();
        ::DispatchMessage(&msg);
    }
    update_imgui_state();

    if (renderer && context)
    {
        ImGui::NewFrame();

        create_ui(context);
        ImGui::EndFrame();
        renderer->pre_frame();
        ImGui::Render();
        renderer->render_imgui_data(*ImGui::GetDrawData());
        renderer->finish_frame();

        reset_mouse_state();
    }
    cleanup_imgui_state();
}

void iw::SystemIntegration::cleanup_imgui_state()
{
    // put up the new mouse state and input state
    reset_mouse_state();
}

void iw::SystemIntegration::reset_mouse_state() {}

void iw::SystemIntegration::update_imgui_state()
{
    auto& io                   = ImGui::GetIO();
    io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

    // Setup display size (every frame to accommodate for window resizing)
    RECT rect;
    ::GetClientRect(window_handle, &rect);
    width          = rect.right - rect.left;
    height         = rect.bottom - rect.top;
    io.DisplaySize = ImVec2(static_cast<float>(width), static_cast<float>(height));

    // Setup time step
    INT64 current_time;
    ::QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&current_time));
    io.DeltaTime = static_cast<float>(current_time - run_time) / ticks_per_second;
    run_time     = current_time;

    // Read keyboard modifiers inputs
    io.KeyCtrl  = (::GetKeyState(VK_CONTROL) & 0x8000) != 0;
    io.KeyShift = (::GetKeyState(VK_SHIFT) & 0x8000) != 0;
    io.KeyAlt   = (::GetKeyState(VK_MENU) & 0x8000) != 0;
    io.KeySuper = false;
    // io.KeysDown[], io.MousePos, io.MouseDown[], io.MouseWheel: filled by the WndProc handler below.

    // Update OS mouse position
    update_mouse_pos();

    // Update OS mouse cursor with the cursor requested by imgui
    ImGuiMouseCursor mouse_cursor = io.MouseDrawCursor ? ImGuiMouseCursor_None : ImGui::GetMouseCursor();
    if (last_mouse_cursor != mouse_cursor)
    {
        last_mouse_cursor = mouse_cursor;
        update_mouse_cursor();
    }

    update_gamepad();
}

bool iw::SystemIntegration::in_cooperative_environment() { return false; }
void iw::SystemIntegration::execute_once() { loop(); }

void iw::SystemIntegration::set_context(Context* c) { context = c; }
void iw::SystemIntegration::set_renderer(Renderer* r) { renderer = r; }

void                iw::SystemIntegration::set_ui_call(std::function<void(Context*)>&& f) { create_ui = std::move(f); }
std::pair<int, int> iw::SystemIntegration::window_size() { return {width, height}; }

iw::SystemIntegration::~SystemIntegration()
{
    active_instances.erase(window_handle);
    ::DestroyWindow(window_handle);
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);
    window_handle = nullptr;
}

void iw::SystemIntegration::update_mouse_cursor()
{
    auto& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) return;

    ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
    if (imgui_cursor == ImGuiMouseCursor_None || io.MouseDrawCursor)
    {
        // Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
        ::SetCursor(NULL);
    }
    else
    {
        // Show OS mouse cursor
        LPTSTR win32_cursor = IDC_ARROW;
        switch (imgui_cursor)
        {
            case ImGuiMouseCursor_Arrow: win32_cursor = IDC_ARROW; break;
            case ImGuiMouseCursor_TextInput: win32_cursor = IDC_IBEAM; break;
            case ImGuiMouseCursor_ResizeAll: win32_cursor = IDC_SIZEALL; break;
            case ImGuiMouseCursor_ResizeEW: win32_cursor = IDC_SIZEWE; break;
            case ImGuiMouseCursor_ResizeNS: win32_cursor = IDC_SIZENS; break;
            case ImGuiMouseCursor_ResizeNESW: win32_cursor = IDC_SIZENESW; break;
            case ImGuiMouseCursor_ResizeNWSE: win32_cursor = IDC_SIZENWSE; break;
            case ImGuiMouseCursor_Hand: win32_cursor = IDC_HAND; break;
            case ImGuiMouseCursor_NotAllowed: win32_cursor = IDC_NO; break;
        }
        ::SetCursor(::LoadCursor(NULL, win32_cursor));
    }
    return;
}

void iw::SystemIntegration::update_gamepad()
{
#ifndef IMGUI_IMPL_WIN32_DISABLE_GAMEPAD
    auto& io = ImGui::GetIO();
    memset(io.NavInputs, 0, sizeof(io.NavInputs));
    if ((io.ConfigFlags & ImGuiConfigFlags_NavEnableGamepad) == 0) return;

#if 0
    // Calling XInputGetState() every frame on disconnected gamepads is unfortunately too slow.
    // Instead we refresh gamepad availability by calling XInputGetCapabilities() _only_ after receiving WM_DEVICECHANGE.
    if (want_update_has_gamepad)
    {
        XINPUT_CAPABILITIES caps;
        has_gamepad             = (XInputGetCapabilities(0, XINPUT_FLAG_GAMEPAD, &caps) == ERROR_SUCCESS);
        want_update_has_gamepad = false;
    }

    XINPUT_STATE xinput_state;
    io.BackendFlags &= ~ImGuiBackendFlags_HasGamepad;
    if (has_gamepad && XInputGetState(0, &xinput_state) == ERROR_SUCCESS)
    {
        const XINPUT_GAMEPAD& gamepad = xinput_state.Gamepad;
        io.BackendFlags |= ImGuiBackendFlags_HasGamepad;

#define MAP_BUTTON(NAV_NO, BUTTON_ENUM)                                        \
    {                                                                          \
        io.NavInputs[NAV_NO] = (gamepad.wButtons & BUTTON_ENUM) ? 1.0f : 0.0f; \
    }
#define MAP_ANALOG(NAV_NO, VALUE, V0, V1)                                      \
    {                                                                          \
        float vn = (float)(VALUE - V0) / (float)(V1 - V0);                     \
        if (vn > 1.0f) vn = 1.0f;                                              \
        if (vn > 0.0f && io.NavInputs[NAV_NO] < vn) io.NavInputs[NAV_NO] = vn; \
    }
        MAP_BUTTON(ImGuiNavInput_Activate, XINPUT_GAMEPAD_A);                // Cross / A
        MAP_BUTTON(ImGuiNavInput_Cancel, XINPUT_GAMEPAD_B);                  // Circle / B
        MAP_BUTTON(ImGuiNavInput_Menu, XINPUT_GAMEPAD_X);                    // Square / X
        MAP_BUTTON(ImGuiNavInput_Input, XINPUT_GAMEPAD_Y);                   // Triangle / Y
        MAP_BUTTON(ImGuiNavInput_DpadLeft, XINPUT_GAMEPAD_DPAD_LEFT);        // D-Pad Left
        MAP_BUTTON(ImGuiNavInput_DpadRight, XINPUT_GAMEPAD_DPAD_RIGHT);      // D-Pad Right
        MAP_BUTTON(ImGuiNavInput_DpadUp, XINPUT_GAMEPAD_DPAD_UP);            // D-Pad Up
        MAP_BUTTON(ImGuiNavInput_DpadDown, XINPUT_GAMEPAD_DPAD_DOWN);        // D-Pad Down
        MAP_BUTTON(ImGuiNavInput_FocusPrev, XINPUT_GAMEPAD_LEFT_SHOULDER);   // L1 / LB
        MAP_BUTTON(ImGuiNavInput_FocusNext, XINPUT_GAMEPAD_RIGHT_SHOULDER);  // R1 / RB
        MAP_BUTTON(ImGuiNavInput_TweakSlow, XINPUT_GAMEPAD_LEFT_SHOULDER);   // L1 / LB
        MAP_BUTTON(ImGuiNavInput_TweakFast, XINPUT_GAMEPAD_RIGHT_SHOULDER);  // R1 / RB
        MAP_ANALOG(ImGuiNavInput_LStickLeft, gamepad.sThumbLX, -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, -32768);
        MAP_ANALOG(ImGuiNavInput_LStickRight, gamepad.sThumbLX, +XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, +32767);
        MAP_ANALOG(ImGuiNavInput_LStickUp, gamepad.sThumbLY, +XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, +32767);
        MAP_ANALOG(ImGuiNavInput_LStickDown, gamepad.sThumbLY, -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, -32767);
#undef MAP_BUTTON
#undef MAP_ANALOG
    }
#endif
#endif  // #ifndef IMGUI_IMPL_WIN32_DISABLE_GAMEPAD
}

void iw::SystemIntegration::update_mouse_pos()
{
    auto& io = ImGui::GetIO();

    // Set OS mouse position if requested (rarely used, only when ImGuiConfigFlags_NavEnableSetMousePos is enabled by user)
    if (io.WantSetMousePos)
    {
        POINT pos{static_cast<int>(io.MousePos.x), static_cast<int>(io.MousePos.y)};
        ::ClientToScreen(window_handle, &pos);
        ::SetCursorPos(pos.x, pos.y);
    }

    // Set mouse position
    io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
    POINT pos;
    if (HWND active_window = ::GetForegroundWindow())
        if (active_window == window_handle || ::IsChild(active_window, window_handle))
            if (::GetCursorPos(&pos) && ::ScreenToClient(window_handle, &pos))
                io.MousePos = ImVec2(static_cast<float>(pos.x), static_cast<float>(pos.y));
}
