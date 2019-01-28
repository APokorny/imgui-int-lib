#include "system_integration.h"

#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/key_codes.h>
#include <iostream>
#include "imgui.h"
#include "imgui/context.h"

namespace ie = imgui::emscripten;
constexpr uint32_t UNMAPPED = 1024;
enum HIDKeyCodes {
    Reserved,                      // 0x00
    Key_ErrorRollOver,             // 0x01
    Key_POSTFail,                  // 0x02
    Key_ErrorUndefined,            // 0x03
    Key_A,                         // 0x04
    Key_B,                         // 0x05
    Key_C,                         // 0x06
    Key_D,                         // 0x07
    Key_E,                         // 0x08
    Key_F,                         // 0x09
    Key_G,                         // 0x0A
    Key_H,                         // 0x0B
    Key_I,                         // 0x0C
    Key_J,                         // 0x0D
    Key_K,                         // 0x0E
    Key_L,                         // 0x0F
    Key_M,                         // 0x10
    Key_N,                         // 0x11
    Key_O,                         // 0x12
    Key_P,                         // 0x13
    Key_Q,                         // 0x14
    Key_R,                         // 0x15
    Key_S,                         // 0x16
    Key_T,                         // 0x17
    Key_U,                         // 0x18
    Key_V,                         // 0x19
    Key_W,                         // 0x1A
    Key_X,                         // 0x1B
    Key_Y,                         // 0x1C
    Key_Z,                         // 0x1D
    Key_1,                         // 0x1E
    Key_2,                         // 0x1F
    Key_3,                         // 0x20
    Key_4,                         // 0x21
    Key_5,                         // 0x22
    Key_6,                         // 0x23
    Key_7,                         // 0x24
    Key_8,                         // 0x25
    Key_9,                         // 0x26
    Key_0,                         // 0x27
    Key_Return,                    // 0x28
    Key_ESCAPE,                    // 0x29
    Key_Backspace,                 // 0x2A
    Key_Tab,                       // 0x2B
    Key_Spacebar,                  // 0x2C
    Key_Minus,                     // 0x2D
    Key_Equals,                    // 0x2E
    Key_LeftBrace,                 // 0x2F
    Key_RightBrace,                // 0x30
    Key_Backslash,                 // 0x31
    Key_Tilde,                     // 0x32
    Key_Semicolon,                 // 0x33
    Key_Apostrophe,                // 0x34
    Key_Grave,                     // 0x35
    Key_Comma,                     // 0x36
    Key_Dot,                       // 0x37
    Key_Slash,                     // 0x38
    Key_CapsLock,                  // 0x39
    Key_F1,                        // 0x3A
    Key_F2,                        // 0x3B
    Key_F3,                        // 0x3C
    Key_F4,                        // 0x3D
    Key_F5,                        // 0x3E
    Key_F6,                        // 0x3F
    Key_F7,                        // 0x40
    Key_F8,                        // 0x41
    Key_F9,                        // 0x42
    Key_F10,                       // 0x43
    Key_F11,                       // 0x44
    Key_F12,                       // 0x45
    Key_PrintScreen,               // 0x46
    Key_Scroll_Lock,               // 0x47
    Key_Pause,                     // 0x48
    Key_Insert,                    // 0x49
    Key_Home,                      // 0x4A
    Key_PageUp,                    // 0x4B
    Key_Delete_Forward,            // 0x4C
    Key_End,                       // 0x4D
    Key_PageDown,                  // 0x4E
    Key_RightArrow,                // 0x4F
    Key_LeftArrow,                 // 0x50
    Key_DownArrow,                 // 0x51
    Key_UpArrow,                   // 0x52
    Keypad_NumLock,                // 0x53
    Keypad_Divide,                 // 0x54
    Keypad_Multiply,               // 0x55
    Keypad_Minus,                  // 0x56
    Keypad_Plus,                   // 0x57
    Keypad_Enter,                  // 0x58
    Keypad_1,                      // 0x59
    Keypad_2,                      // 0x5A
    Keypad_3,                      // 0x5B
    Keypad_4,                      // 0x5C
    Keypad_5,                      // 0x5D
    Keypad_6,                      // 0x5E
    Keypad_7,                      // 0x5F
    Keypad_8,                      // 0x60
    Keypad_9,                      // 0x61
    Keypad_0,                      // 0x62
    Keypad_Dot,                    // 0x63
    Keyboard_Pipe,                 // 0x64
    Keyboard_Application,          // 0x65
    Keyboard_Power,                // 0x66
    Keypad_Equals,                 // 0x67
    Key_F13,                       // 0x68
    Key_F14,                       // 0x69
    Key_F15,                       // 0x6A
    Key_F16,                       // 0x6B
    Key_F17,                       // 0x6C
    Key_F18,                       // 0x6D
    Key_F19,                       // 0x6E
    Key_F20,                       // 0x6F
    Key_F21,                       // 0x70
    Key_F22,                       // 0x71
    Key_F23,                       // 0x72
    Key_F24,                       // 0x73
    Keyboard_Execute,              // 0x74
    Keyboard_Help,                 // 0x75
    Keyboard_Menu,                 // 0x76
    Keyboard_Select,               // 0x77
    Keyboard_Stop,                 // 0x78
    Keyboard_Again,                // 0x79
    Keyboard_Undo,                 // 0x7A
    Keyboard_Cut,                  // 0x7B
    Keyboard_Copy,                 // 0x7C
    Keyboard_Paste,                // 0x7D
    Keyboard_Find,                 // 0x7E
    Keyboard_Mute,                 // 0x7F
    Keyboard_Volume_Up,            // 0x80
    Keyboard_Volume_Down,          // 0x81
    Keyboard_Locking_Caps_Lock,    // 0x82
    Keyboard_Locking_Num_Lock,     // 0x83
    Keyboard_Locking_Scroll_Lock,  // 0x84
    Keypad_Comma,                  // 0x85
    Keypad_Equal_Sign,             // 0x86
    Keyboard_International1,       // 0x87
    Keyboard_International2,       // 0x88
    Keyboard_International3,       // 0x89
    Keyboard_International4,       // 0x8A
    Keyboard_International5,       // 0x8B
    Keyboard_International6,       // 0x8C
    Keyboard_International7,       // 0x8D
    Keyboard_International8,       // 0x8E
    Keyboard_International9,       // 0x8F
    Keyboard_LANG1,                // 0x90
    Keyboard_LANG2,                // 0x91
    Keyboard_LANG3,                // 0x92
    Keyboard_LANG4,                // 0x93
    Keyboard_LANG5,                // 0x94
    Keyboard_LANG6,                // 0x95
    Keyboard_LANG7,                // 0x96
    Keyboard_LANG8,                // 0x97
    Keyboard_LANG9,                // 0x98
    Keyboard_Alternate_Erase,      // 0x99
    Keyboard_SysReq,               // 0x9A
    Keyboard_Cancel,               // 0x9B
    Keyboard_Clear,                // 0x9C
    Keyboard_Prior,                // 0x9D
    Keyboard_Return,               // 0x9E
    Keyboard_Separator,            // 0x9F
    Keyboard_Out,                  // 0xA0
    Keyboard_Oper,                 // 0xA1
    Keyboard_ClearAgain,           // 0xA2
    Keyboard_CrSel,                // 0xA3
    Keyboard_ExSel,                // 0xA4
    Keyboard_LeftControl,          // 0xE0
    Keyboard_LeftShift,            // 0xE1
    Keyboard_LeftAlt,              // 0xE2
    Keyboard_Left_GUI,             // 0xE3
    Keyboard_RightControl,         // 0xE4
    Keyboard_RightShift,           // 0xE5
    Keyboard_RightAlt,             // 0xE6
    Keyboard_Right_GUI             // 0xE7
};

static const uint32_t emscripten_scancode_table[] = {
    /*  0 */ UNMAPPED,
    /*  1 */ UNMAPPED,
    /*  2 */ UNMAPPED,
    /*  3 */ Keyboard_Cancel,
    /*  4 */ UNMAPPED,
    /*  5 */ UNMAPPED,
    /*  6 */ Keyboard_Menu,
    /*  7 */ UNMAPPED,
    /*  8 */ Key_Backspace,
    /*  9 */ Key_Tab,
    /*  10 */ UNMAPPED,
    /*  11 */ UNMAPPED,
    /*  12 */ UNMAPPED,
    /*  13 */ Key_Return,
    /*  14 */ UNMAPPED,
    /*  15 */ UNMAPPED,
    /*  16 */ Keyboard_LeftShift,
    /*  17 */ Keyboard_LeftControl,
    /*  18 */ Keyboard_LeftAlt,
    /*  19 */ Key_Pause,
    /*  20 */ Key_CapsLock,
    /*  21 */ UNMAPPED,
    /*  22 */ UNMAPPED,
    /*  23 */ UNMAPPED,
    /*  24 */ UNMAPPED,
    /*  25 */ UNMAPPED,
    /*  26 */ UNMAPPED,
    /*  27 */ Key_ESCAPE,
    /*  28 */ UNMAPPED,
    /*  29 */ UNMAPPED,
    /*  30 */ UNMAPPED,
    /*  31 */ UNMAPPED,
    /*  32 */ Key_Spacebar,
    /*  33 */ Key_PageUp,
    /*  34 */ Key_PageDown,
    /*  35 */ Key_End,
    /*  36 */ Key_Home,
    /*  37 */ Key_LeftArrow,
    /*  38 */ Key_UpArrow,
    /*  39 */ Key_RightArrow,
    /*  40 */ Key_DownArrow,
    /*  41 */ UNMAPPED,
    /*  42 */ UNMAPPED,
    /*  43 */ UNMAPPED,
    /*  44 */ UNMAPPED,
    /*  45 */ Key_Insert,
    /*  46 */ Key_Delete_Forward,
    /*  47 */ UNMAPPED,
    /*  48 */ Key_0,
    /*  49 */ Key_1,
    /*  50 */ Key_2,
    /*  51 */ Key_3,
    /*  52 */ Key_4,
    /*  53 */ Key_5,
    /*  54 */ Key_6,
    /*  55 */ Key_7,
    /*  56 */ Key_8,
    /*  57 */ Key_9,
    /*  58 */ UNMAPPED,
    /*  59 */ Key_Semicolon,
    /*  60 */ UNMAPPED,
    /*  61 */ Key_Equals,
    /*  62 */ UNMAPPED,
    /*  63 */ UNMAPPED,
    /*  64 */ UNMAPPED,
    /*  65 */ Key_A,
    /*  66 */ Key_B,
    /*  67 */ Key_C,
    /*  68 */ Key_D,
    /*  69 */ Key_E,
    /*  70 */ Key_F,
    /*  71 */ Key_G,
    /*  72 */ Key_H,
    /*  73 */ Key_I,
    /*  74 */ Key_J,
    /*  75 */ Key_K,
    /*  76 */ Key_L,
    /*  77 */ Key_M,
    /*  78 */ Key_N,
    /*  79 */ Key_O,
    /*  80 */ Key_P,
    /*  81 */ Key_Q,
    /*  82 */ Key_R,
    /*  83 */ Key_S,
    /*  84 */ Key_T,
    /*  85 */ Key_U,
    /*  86 */ Key_V,
    /*  87 */ Key_W,
    /*  88 */ Key_X,
    /*  89 */ Key_Y,
    /*  90 */ Key_Z,
    /*  91 */ Keyboard_Left_GUI,
    /*  92 */ UNMAPPED,
    /*  93 */ Keyboard_Application,
    /*  94 */ UNMAPPED,
    /*  95 */ UNMAPPED,
    /*  96 */ Keypad_0,
    /*  97 */ Keypad_1,
    /*  98 */ Keypad_2,
    /*  99 */ Keypad_3,
    /* 100 */ Keypad_4,
    /* 101 */ Keypad_5,
    /* 102 */ Keypad_6,
    /* 103 */ Keypad_7,
    /* 104 */ Keypad_8,
    /* 105 */ Keypad_9,
    /* 106 */ Keypad_Multiply,
    /* 107 */ Keypad_Plus,
    /* 108 */ UNMAPPED,
    /* 109 */ Keypad_Minus,
    /* 110 */ Keypad_Comma,
    /* 111 */ Keypad_Divide,
    /* 112 */ Key_F1,
    /* 113 */ Key_F2,
    /* 114 */ Key_F3,
    /* 115 */ Key_F4,
    /* 116 */ Key_F5,
    /* 117 */ Key_F6,
    /* 118 */ Key_F7,
    /* 119 */ Key_F8,
    /* 120 */ Key_F9,
    /* 121 */ Key_F10,
    /* 122 */ Key_F11,
    /* 123 */ Key_F12,
    /* 124 */ Key_F13,
    /* 125 */ Key_F14,
    /* 126 */ Key_F15,
    /* 127 */ Key_F16,
    /* 128 */ Key_F17,
    /* 129 */ Key_F18,
    /* 130 */ Key_F19,
    /* 131 */ Key_F20,
    /* 132 */ Key_F21,
    /* 133 */ Key_F22,
    /* 134 */ Key_F23,
    /* 135 */ Key_F24,
    /* 136 */ UNMAPPED,
    /* 137 */ UNMAPPED,
    /* 138 */ UNMAPPED,
    /* 139 */ UNMAPPED,
    /* 140 */ UNMAPPED,
    /* 141 */ UNMAPPED,
    /* 142 */ UNMAPPED,
    /* 143 */ UNMAPPED,
    /* 144 */ Keypad_NumLock,
    /* 145 */ Key_Scroll_Lock,
    /* 146 */ UNMAPPED,
    /* 147 */ UNMAPPED,
    /* 148 */ UNMAPPED,
    /* 149 */ UNMAPPED,
    /* 150 */ UNMAPPED,
    /* 151 */ UNMAPPED,
    /* 152 */ UNMAPPED,
    /* 153 */ UNMAPPED,
    /* 154 */ UNMAPPED,
    /* 155 */ UNMAPPED,
    /* 156 */ UNMAPPED,
    /* 157 */ UNMAPPED,
    /* 158 */ UNMAPPED,
    /* 159 */ UNMAPPED,
    /* 160 */ UNMAPPED,
    /* 161 */ UNMAPPED,
    /* 162 */ UNMAPPED,
    /* 163 */ UNMAPPED,
    /* 164 */ UNMAPPED,
    /* 165 */ UNMAPPED,
    /* 166 */ UNMAPPED,
    /* 167 */ UNMAPPED,
    /* 168 */ UNMAPPED,
    /* 169 */ UNMAPPED,
    /* 170 */ UNMAPPED,
    /* 171 */ UNMAPPED,
    /* 172 */ UNMAPPED,
    /* 173 */ Keypad_Minus,         /*FX*/
    /* 174 */ Keyboard_Volume_Down, /*IE, Chrome*/
    /* 175 */ Keyboard_Volume_Up,   /*IE, Chrome*/
    /* 176 */ UNMAPPED,             /*IE, Chrome*/
    /* 177 */ UNMAPPED,             /*IE, Chrome*/
    /* 178 */ UNMAPPED,
    /* 179 */ UNMAPPED, /*IE, Chrome*/
    /* 180 */ UNMAPPED,
    /* 181 */ UNMAPPED,             /*FX*/
    /* 182 */ Keyboard_Volume_Down, /*FX*/
    /* 183 */ Keyboard_Volume_Up,   /*FX*/
    /* 184 */ UNMAPPED,
    /* 185 */ UNMAPPED,
    /* 186 */ Key_Semicolon, /*IE, Chrome, D3E legacy*/
    /* 187 */ Key_Equals,    /*IE, Chrome, D3E legacy*/
    /* 188 */ Key_Comma,
    /* 189 */ Key_Minus, /*IE, Chrome, D3E legacy*/
    /* 190 */ Key_Dot,
    /* 191 */ Key_Slash,
    /* 192 */ Key_Grave, /*FX, D3E legacy (SDL_SCANCODE_APOSTROPHE in IE/Chrome)*/
    /* 193 */ UNMAPPED,
    /* 194 */ UNMAPPED,
    /* 195 */ UNMAPPED,
    /* 196 */ UNMAPPED,
    /* 197 */ UNMAPPED,
    /* 198 */ UNMAPPED,
    /* 199 */ UNMAPPED,
    /* 200 */ UNMAPPED,
    /* 201 */ UNMAPPED,
    /* 202 */ UNMAPPED,
    /* 203 */ UNMAPPED,
    /* 204 */ UNMAPPED,
    /* 205 */ UNMAPPED,
    /* 206 */ UNMAPPED,
    /* 207 */ UNMAPPED,
    /* 208 */ UNMAPPED,
    /* 209 */ UNMAPPED,
    /* 210 */ UNMAPPED,
    /* 211 */ UNMAPPED,
    /* 212 */ UNMAPPED,
    /* 213 */ UNMAPPED,
    /* 214 */ UNMAPPED,
    /* 215 */ UNMAPPED,
    /* 216 */ UNMAPPED,
    /* 217 */ UNMAPPED,
    /* 218 */ UNMAPPED,
    /* 219 */ Key_LeftBrace,
    /* 220 */ Key_Backslash,
    /* 221 */ Key_RightBrace,
    /* 222 */ Key_Apostrophe, /*FX, D3E legacy*/
};

uint32_t to_scancode(uint32_t key_code) {
    if (key_code >= sizeof(emscripten_scancode_table) / sizeof(uint32_t)) return emscripten_scancode_table[key_code];
    return UNMAPPED;
}

ie::SystemIntegration::SystemIntegration(size_t initial_width, size_t initial_height) : create_ui{[](Context*) {}} {
    emscripten_set_canvas_element_size("", initial_width, initial_height);
    char const* target = nullptr;
    void* data = this;
    pthread_t this_thread = EM_CALLBACK_THREAD_CONTEXT_CALLING_THREAD;
    emscripten_set_keydown_callback_on_thread("#window", data, false,
                                              [](int type, EmscriptenKeyboardEvent const* event, void*) -> int {
                                                  ImGuiIO& io = ImGui::GetIO();
                                                  auto scan_code = to_scancode(event->keyCode);
                                                  std::cout << "Key Down " << event->keyCode << " scancode:" << scan_code << " location "
                                                            << event->location << event->locale << "  cV:" << event->charValue
                                                            << "  cC:" << event->charCode << "  which:" << event->which << std::endl;
                                                  io.KeysDown[scan_code & 511] = 1;
                                                  io.KeyCtrl = event->ctrlKey;
                                                  io.KeyShift = event->shiftKey;
                                                  io.KeyAlt = event->altKey;
                                                  io.KeySuper = event->metaKey;
                                                  return true;
                                              },
                                              this_thread);
    emscripten_set_keyup_callback_on_thread("#window", data, false,
                                            [](int type, EmscriptenKeyboardEvent const* event, void*) -> int {
                                                ImGuiIO& io = ImGui::GetIO();
                                                auto scan_code = to_scancode(event->keyCode);
                                                std::cout << "Key Up " << event->keyCode << std::endl;
                                                io.KeysDown[scan_code & 511] = 0;
                                                io.KeyCtrl = event->ctrlKey;
                                                io.KeyShift = event->shiftKey;
                                                io.KeyAlt = event->altKey;
                                                io.KeySuper = event->metaKey;
                                                return true;
                                            },
                                            this_thread);
    emscripten_set_click_callback_on_thread(target, data, true,
                                            [](int type, EmscriptenMouseEvent const* event, void*) -> int { return false; }, this_thread);
    emscripten_set_mousedown_callback_on_thread(target, data, true,
                                                [](int type, EmscriptenMouseEvent const* event, void* obj) -> int {
                                                    ImGuiIO& io = ImGui::GetIO();
                                                    SystemIntegration* self = static_cast<SystemIntegration*>(obj);
                                                    self->mouse_status[event->button] = 1;
                                                    return false;
                                                },
                                                this_thread);

    emscripten_set_mouseup_callback_on_thread(target, data, true,
                                              [](int type, EmscriptenMouseEvent const* event, void* obj) -> int {
                                                  ImGuiIO& io = ImGui::GetIO();
                                                  SystemIntegration* self = static_cast<SystemIntegration*>(obj);
                                                  self->mouse_status[event->button] = 2;

                                                  return false;
                                              },
                                              this_thread);
    emscripten_set_dblclick_callback_on_thread(target, data, true,
                                               [](int type, EmscriptenMouseEvent const* event, void*) -> int { return 0; }, this_thread);
    emscripten_set_mousemove_callback_on_thread(target, data, true,
                                                [](int type, EmscriptenMouseEvent const* event, void*) -> int {
                                                    ImGuiIO& io = ImGui::GetIO();
                                                    io.MousePos =
                                                        ImVec2(static_cast<float>(event->canvasX), static_cast<float>(event->canvasY));
                                                    return 0;
                                                },
                                                this_thread);
    emscripten_set_mouseenter_callback_on_thread(target, data, true,
                                                 [](int type, EmscriptenMouseEvent const* event, void*) -> int { return 0; }, this_thread);
    emscripten_set_mouseleave_callback_on_thread(target, data, true,
                                                 [](int type, EmscriptenMouseEvent const* event, void*) -> int {
                                                     ImGuiIO& io = ImGui::GetIO();
                                                     io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
                                                     return 0;
                                                 },
                                                 this_thread);
    emscripten_set_mouseover_callback_on_thread(target, data, true,
                                                [](int type, EmscriptenMouseEvent const* event, void*) -> int { return 0; }, this_thread);
    emscripten_set_mouseout_callback_on_thread(target, data, true,
                                               [](int type, EmscriptenMouseEvent const* event, void*) -> int { return 0; }, this_thread);
    emscripten_set_wheel_callback_on_thread(target, data, true,
                                            [](int type, EmscriptenWheelEvent const* event, void*) -> int {
                                                ImGuiIO& io = ImGui::GetIO();
                                                io.MouseWheel += event->deltaY;
                                                io.MouseWheelH += event->deltaX;

                                                return 0;
                                            },
                                            this_thread);
    emscripten_set_resize_callback_on_thread(target, data, true,
                                             [](int type, EmscriptenUiEvent const* event, void* obj) -> int {
                                                 SystemIntegration* self = static_cast<SystemIntegration*>(obj);
                                                 self->width = event->windowInnerWidth;
                                                 self->height = event->windowInnerHeight;
                                                 return 0;
                                             },
                                             this_thread);
    emscripten_set_scroll_callback_on_thread(target, data, true, [](int type, EmscriptenUiEvent const* event, void*) -> int { return 0; },
                                             this_thread);
    emscripten_set_blur_callback_on_thread(target, data, true,
                                           [](int type, EmscriptenFocusEvent const* event, void*) -> int {
                                               std::cout << "blur callback" << std::endl;
                                               return 0;
                                           },
                                           this_thread);
    emscripten_set_focus_callback_on_thread(target, data, true,
                                            [](int type, EmscriptenFocusEvent const* event, void*) -> int {
                                                std::cout << "focus callback" << std::endl;
                                                return 0;
                                            },
                                            this_thread);
    emscripten_set_focusin_callback_on_thread(target, data, true,
                                              [](int type, EmscriptenFocusEvent const* event, void*) -> int {
                                                  std::cout << "focus in callback" << std::endl;
                                                  return 0;
                                              },
                                              this_thread);
    emscripten_set_focusout_callback_on_thread(target, data, true,
                                               [](int type, EmscriptenFocusEvent const* event, void*) -> int {
                                                   std::cout << "focus out in callback" << std::endl;
                                                   return 0;
                                               },
                                               this_thread);
    emscripten_set_deviceorientation_callback_on_thread(
        data, true, [](int type, EmscriptenDeviceOrientationEvent const* event, void*) -> int { return 0; }, this_thread);
    emscripten_set_devicemotion_callback_on_thread(
        data, true, [](int type, EmscriptenDeviceMotionEvent const* event, void*) -> int { return 0; }, this_thread);
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
    io.DisplaySize = ImVec2(static_cast<float>(width), static_cast<float>(height));
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

void ie::SystemIntegration::set_ui_call(std::function<void(Context*)>&& f) { create_ui = std::move(f); }

void ie::SystemIntegration::set_context(imgui::Context* c) { context = c; }
std::pair<int, int> ie::SystemIntegration::window_size() { return {width, height}; }

ie::SystemIntegration::~SystemIntegration() = default;
