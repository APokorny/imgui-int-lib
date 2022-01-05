#include "system_integration.h"

#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/key_codes.h>
#include "imgui.h"
#include "imgui/context.h"

namespace ie                = imgui::emscripten;
constexpr uint32_t UNMAPPED = 1024;
enum HIDKeyCodes
{
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
struct KeyInfo
{
    uint32_t code;
    bool     printable{false};
};

static KeyInfo const emscripten_scancode_table[] = {
    /*  0 */ {UNMAPPED, false},
    /*  1 */ {UNMAPPED, false},
    /*  2 */ {UNMAPPED, false},
    /*  3 */ {Keyboard_Cancel, false},
    /*  4 */ {UNMAPPED, false},
    /*  5 */ {UNMAPPED, false},
    /*  6 */ {Keyboard_Menu, false},
    /*  7 */ {UNMAPPED, false},
    /*  8 */ {Key_Backspace, false},
    /*  9 */ {Key_Tab, false},
    /*  10 */ {UNMAPPED, false},
    /*  11 */ {UNMAPPED, false},
    /*  12 */ {UNMAPPED, false},
    /*  13 */ {Key_Return, false},
    /*  14 */ {UNMAPPED, false},
    /*  15 */ {UNMAPPED, false},
    /*  16 */ {Keyboard_LeftShift, false},
    /*  17 */ {Keyboard_LeftControl, false},
    /*  18 */ {Keyboard_LeftAlt, false},
    /*  19 */ {Key_Pause, false},
    /*  20 */ {Key_CapsLock, false},
    /*  21 */ {UNMAPPED, false},
    /*  22 */ {UNMAPPED, false},
    /*  23 */ {UNMAPPED, false},
    /*  24 */ {UNMAPPED, false},
    /*  25 */ {UNMAPPED, false},
    /*  26 */ {UNMAPPED, false},
    /*  27 */ {Key_ESCAPE, false},
    /*  28 */ {UNMAPPED, false},
    /*  29 */ {UNMAPPED, false},
    /*  30 */ {UNMAPPED, false},
    /*  31 */ {UNMAPPED, false},
    /*  32 */ {Key_Spacebar, true},
    /*  33 */ {Key_PageUp, false},
    /*  34 */ {Key_PageDown, false},
    /*  35 */ {Key_End, false},
    /*  36 */ {Key_Home, false},
    /*  37 */ {Key_LeftArrow, false},
    /*  38 */ {Key_UpArrow, false},
    /*  39 */ {Key_RightArrow, false},
    /*  40 */ {Key_DownArrow, false},
    /*  41 */ {UNMAPPED, false},
    /*  42 */ {UNMAPPED, false},
    /*  43 */ {UNMAPPED, false},
    /*  44 */ {UNMAPPED, false},
    /*  45 */ {Key_Insert, false},
    /*  46 */ {Key_Delete_Forward, false},
    /*  47 */ {UNMAPPED, false},
    /*  48 */ {Key_0, true},
    /*  49 */ {Key_1, true},
    /*  50 */ {Key_2, true},
    /*  51 */ {Key_3, true},
    /*  52 */ {Key_4, true},
    /*  53 */ {Key_5, true},
    /*  54 */ {Key_6, true},
    /*  55 */ {Key_7, true},
    /*  56 */ {Key_8, true},
    /*  57 */ {Key_9, true},
    /*  58 */ {UNMAPPED, false},
    /*  59 */ {Key_Semicolon, true},
    /*  60 */ {UNMAPPED, false},
    /*  61 */ {Key_Equals, true},
    /*  62 */ {UNMAPPED, false},
    /*  63 */ {UNMAPPED, false},
    /*  64 */ {UNMAPPED, false},
    /*  65 */ {Key_A, true},
    /*  66 */ {Key_B, true},
    /*  67 */ {Key_C, true},
    /*  68 */ {Key_D, true},
    /*  69 */ {Key_E, true},
    /*  70 */ {Key_F, true},
    /*  71 */ {Key_G, true},
    /*  72 */ {Key_H, true},
    /*  73 */ {Key_I, true},
    /*  74 */ {Key_J, true},
    /*  75 */ {Key_K, true},
    /*  76 */ {Key_L, true},
    /*  77 */ {Key_M, true},
    /*  78 */ {Key_N, true},
    /*  79 */ {Key_O, true},
    /*  80 */ {Key_P, true},
    /*  81 */ {Key_Q, true},
    /*  82 */ {Key_R, true},
    /*  83 */ {Key_S, true},
    /*  84 */ {Key_T, true},
    /*  85 */ {Key_U, true},
    /*  86 */ {Key_V, true},
    /*  87 */ {Key_W, true},
    /*  88 */ {Key_X, true},
    /*  89 */ {Key_Y, true},
    /*  90 */ {Key_Z, true},
    /*  91 */ {Keyboard_Left_GUI, false},
    /*  92 */ {UNMAPPED, false},
    /*  93 */ {Keyboard_Application, false},
    /*  94 */ {UNMAPPED, false},
    /*  95 */ {UNMAPPED, false},
    /*  96 */ {Keypad_0, true},
    /*  97 */ {Keypad_1, true},
    /*  98 */ {Keypad_2, true},
    /*  99 */ {Keypad_3, true},
    /* 100 */ {Keypad_4, true},
    /* 101 */ {Keypad_5, true},
    /* 102 */ {Keypad_6, true},
    /* 103 */ {Keypad_7, true},
    /* 104 */ {Keypad_8, true},
    /* 105 */ {Keypad_9, true},
    /* 106 */ {Keypad_Multiply, true},
    /* 107 */ {Keypad_Plus, true},
    /* 108 */ {UNMAPPED, false},
    /* 109 */ {Keypad_Minus, true},
    /* 110 */ {Keypad_Comma, true},
    /* 111 */ {Keypad_Divide, true},
    /* 112 */ {Key_F1, false},
    /* 113 */ {Key_F2, false},
    /* 114 */ {Key_F3, false},
    /* 115 */ {Key_F4, false},
    /* 116 */ {Key_F5, false},
    /* 117 */ {Key_F6, false},
    /* 118 */ {Key_F7, false},
    /* 119 */ {Key_F8, false},
    /* 120 */ {Key_F9, false},
    /* 121 */ {Key_F10, false},
    /* 122 */ {Key_F11, false},
    /* 123 */ {Key_F12, false},
    /* 124 */ {Key_F13, false},
    /* 125 */ {Key_F14, false},
    /* 126 */ {Key_F15, false},
    /* 127 */ {Key_F16, false},
    /* 128 */ {Key_F17, false},
    /* 129 */ {Key_F18, false},
    /* 130 */ {Key_F19, false},
    /* 131 */ {Key_F20, false},
    /* 132 */ {Key_F21, false},
    /* 133 */ {Key_F22, false},
    /* 134 */ {Key_F23, false},
    /* 135 */ {Key_F24, false},
    /* 136 */ {UNMAPPED, false},
    /* 137 */ {UNMAPPED, false},
    /* 138 */ {UNMAPPED, false},
    /* 139 */ {UNMAPPED, false},
    /* 140 */ {UNMAPPED, false},
    /* 141 */ {UNMAPPED, false},
    /* 142 */ {UNMAPPED, false},
    /* 143 */ {UNMAPPED, false},
    /* 144 */ {Keypad_NumLock, false},
    /* 145 */ {Key_Scroll_Lock, false},
    /* 146 */ {UNMAPPED, false},
    /* 147 */ {UNMAPPED, false},
    /* 148 */ {UNMAPPED, false},
    /* 149 */ {UNMAPPED, false},
    /* 150 */ {UNMAPPED, false},
    /* 151 */ {UNMAPPED, false},
    /* 152 */ {UNMAPPED, false},
    /* 153 */ {UNMAPPED, false},
    /* 154 */ {UNMAPPED, false},
    /* 155 */ {UNMAPPED, false},
    /* 156 */ {UNMAPPED, false},
    /* 157 */ {UNMAPPED, false},
    /* 158 */ {UNMAPPED, false},
    /* 159 */ {UNMAPPED, false},
    /* 160 */ {UNMAPPED, false},
    /* 161 */ {UNMAPPED, false},
    /* 162 */ {UNMAPPED, false},
    /* 163 */ {UNMAPPED, false},
    /* 164 */ {UNMAPPED, false},
    /* 165 */ {UNMAPPED, false},
    /* 166 */ {UNMAPPED, false},
    /* 167 */ {UNMAPPED, false},
    /* 168 */ {UNMAPPED, false},
    /* 169 */ {UNMAPPED, false},
    /* 170 */ {UNMAPPED, false},
    /* 171 */ {UNMAPPED, false},
    /* 172 */ {UNMAPPED, false},
    /* 173 */ {Keypad_Minus, true},          /*FX*/
    /* 174 */ {Keyboard_Volume_Down, false}, /*IE, Chrome*/
    /* 175 */ {Keyboard_Volume_Up, false},   /*IE, Chrome*/
    /* 176 */ {UNMAPPED, false},             /*IE, Chrome*/
    /* 177 */ {UNMAPPED, false},             /*IE, Chrome*/
    /* 178 */ {UNMAPPED, false},
    /* 179 */ {UNMAPPED, false}, /*IE, Chrome*/
    /* 180 */ {UNMAPPED, false},
    /* 181 */ {UNMAPPED, false},             /*FX*/
    /* 182 */ {Keyboard_Volume_Down, false}, /*FX*/
    /* 183 */ {Keyboard_Volume_Up, false},   /*FX*/
    /* 184 */ {UNMAPPED, false},
    /* 185 */ {UNMAPPED, false},
    /* 186 */ {Key_Semicolon, false}, /*IE, Chrome, D3E legacy*/
    /* 187 */ {Key_Equals, true},     /*IE, Chrome, D3E legacy*/
    /* 188 */ {Key_Comma, true},
    /* 189 */ {Key_Minus, true}, /*IE, Chrome, D3E legacy*/
    /* 190 */ {Key_Dot, true},
    /* 191 */ {Key_Slash, true},
    /* 192 */ {Key_Grave, false}, /*FX, D3E legacy (SDL_SCANCODE_APOSTROPHE in IE/Chrome)*/
    /* 193 */ {UNMAPPED, false},
    /* 194 */ {UNMAPPED, false},
    /* 195 */ {UNMAPPED, false},
    /* 196 */ {UNMAPPED, false},
    /* 197 */ {UNMAPPED, false},
    /* 198 */ {UNMAPPED, false},
    /* 199 */ {UNMAPPED, false},
    /* 200 */ {UNMAPPED, false},
    /* 201 */ {UNMAPPED, false},
    /* 202 */ {UNMAPPED, false},
    /* 203 */ {UNMAPPED, false},
    /* 204 */ {UNMAPPED, false},
    /* 205 */ {UNMAPPED, false},
    /* 206 */ {UNMAPPED, false},
    /* 207 */ {UNMAPPED, false},
    /* 208 */ {UNMAPPED, false},
    /* 209 */ {UNMAPPED, false},
    /* 210 */ {UNMAPPED, false},
    /* 211 */ {UNMAPPED, false},
    /* 212 */ {UNMAPPED, false},
    /* 213 */ {UNMAPPED, false},
    /* 214 */ {UNMAPPED, false},
    /* 215 */ {UNMAPPED, false},
    /* 216 */ {UNMAPPED, false},
    /* 217 */ {UNMAPPED, false},
    /* 218 */ {UNMAPPED, false},
    /* 219 */ {Key_LeftBrace, true},
    /* 220 */ {Key_Backslash, true},
    /* 221 */ {Key_RightBrace, true},
    /* 222 */ {Key_Apostrophe, false}, /*FX, D3E legacy*/
};

auto to_scancode(uint32_t key_code)
{
    if (key_code < sizeof(emscripten_scancode_table) / sizeof(const KeyInfo)) return emscripten_scancode_table[key_code];
    return KeyInfo{UNMAPPED, false};
}

ie::SystemIntegration::SystemIntegration() : create_ui{[](Context*) {}}
{
    char const* target = "#canvas";
    emscripten_get_screen_size(&width, &height);
    emscripten_set_canvas_element_size(target, width, height);
    void*     data        = this;
    pthread_t this_thread = EM_CALLBACK_THREAD_CONTEXT_CALLING_THREAD;
    emscripten_set_keydown_callback_on_thread(
        target, data, 0,
        [](int type, EmscriptenKeyboardEvent const* event, void*) -> int
        {
            ImGuiIO& io        = ImGui::GetIO();
            auto     scan_code = to_scancode(event->keyCode);
            if (scan_code.printable) io.AddInputCharactersUTF8(event->key);
            if (scan_code.code == UNMAPPED) return EMSCRIPTEN_RESULT_FAILED;
            io.KeysDown[scan_code.code & 511] = 1;
            io.KeyCtrl                        = event->ctrlKey;
            io.KeyShift                       = event->shiftKey;
            io.KeyAlt                         = event->altKey;
            io.KeySuper                       = event->metaKey;
            return EMSCRIPTEN_RESULT_SUCCESS;
        },
        this_thread);
    emscripten_set_keyup_callback_on_thread(
        target, data, 0,
        [](int type, EmscriptenKeyboardEvent const* event, void*) -> int
        {
            ImGuiIO& io        = ImGui::GetIO();
            auto     scan_code = to_scancode(event->keyCode);
            if (scan_code.code == UNMAPPED) return EMSCRIPTEN_RESULT_FAILED;
            io.KeysDown[scan_code.code & 511] = 0;
            io.KeyCtrl                        = event->ctrlKey;
            io.KeyShift                       = event->shiftKey;
            io.KeyAlt                         = event->altKey;
            io.KeySuper                       = event->metaKey;
            return EMSCRIPTEN_RESULT_SUCCESS;
        },
        this_thread);
    emscripten_set_click_callback_on_thread(
        target, data, true, [](int type, EmscriptenMouseEvent const* event, void*) -> int { return false; }, this_thread);
    emscripten_set_mousedown_callback_on_thread(
        target, data, true,
        [](int type, EmscriptenMouseEvent const* event, void* obj) -> int
        {
            ImGuiIO&           io             = ImGui::GetIO();
            SystemIntegration* self           = static_cast<SystemIntegration*>(obj);
            self->mouse_status[event->button] = 1;
            return false;
        },
        this_thread);

    emscripten_set_mouseup_callback_on_thread(
        target, data, true,
        [](int type, EmscriptenMouseEvent const* event, void* obj) -> int
        {
            ImGuiIO&           io             = ImGui::GetIO();
            SystemIntegration* self           = static_cast<SystemIntegration*>(obj);
            self->mouse_status[event->button] = 2;

            return false;
        },
        this_thread);
    emscripten_set_dblclick_callback_on_thread(
        target, data, true, [](int type, EmscriptenMouseEvent const* event, void*) -> int { return 0; }, this_thread);
    emscripten_set_mousemove_callback_on_thread(
        target, data, true,
        [](int type, EmscriptenMouseEvent const* event, void*) -> int
        {
            ImGuiIO& io = ImGui::GetIO();
            io.MousePos = ImVec2(static_cast<float>(event->targetX), static_cast<float>(event->targetY));
            return 0;
        },
        this_thread);
    emscripten_set_mouseenter_callback_on_thread(
        target, data, true, [](int type, EmscriptenMouseEvent const* event, void*) -> int { return 0; }, this_thread);
    emscripten_set_mouseleave_callback_on_thread(
        target, data, true,
        [](int type, EmscriptenMouseEvent const* event, void*) -> int
        {
            ImGuiIO& io = ImGui::GetIO();
            io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
            return 0;
        },
        this_thread);
    emscripten_set_mouseover_callback_on_thread(
        target, data, true, [](int type, EmscriptenMouseEvent const* event, void*) -> int { return 0; }, this_thread);
    emscripten_set_mouseout_callback_on_thread(
        target, data, true, [](int type, EmscriptenMouseEvent const* event, void*) -> int { return 0; }, this_thread);
    emscripten_set_wheel_callback_on_thread(
        target, data, true,
        [](int type, EmscriptenWheelEvent const* event, void*) -> int
        {
            ImGuiIO& io = ImGui::GetIO();
            io.MouseWheel += event->deltaY;
            io.MouseWheelH += event->deltaX;

            return 0;
        },
        this_thread);
    emscripten_set_resize_callback_on_thread(
        EMSCRIPTEN_EVENT_TARGET_WINDOW, data, true,
        [](int type, EmscriptenUiEvent const* event, void* obj) -> int
        {
            SystemIntegration* self = static_cast<SystemIntegration*>(obj);
            self->width             = event->windowInnerWidth;
            self->height            = event->windowInnerHeight;
            emscripten_get_screen_size(&self->width, &self->height);
            emscripten_set_canvas_element_size("#canvas", self->width, self->height);
            return 0;
        },
        this_thread);
    emscripten_set_scroll_callback_on_thread(
        target, data, true, [](int type, EmscriptenUiEvent const* event, void*) -> int { return 0; }, this_thread);
    emscripten_set_blur_callback_on_thread(
        target, data, true, [](int type, EmscriptenFocusEvent const* event, void*) -> int { return 0; }, this_thread);
    emscripten_set_focus_callback_on_thread(
        target, data, true, [](int type, EmscriptenFocusEvent const* event, void*) -> int { return 0; }, this_thread);
    emscripten_set_focusin_callback_on_thread(
        target, data, true, [](int type, EmscriptenFocusEvent const* event, void*) -> int { return 0; }, this_thread);
    emscripten_set_focusout_callback_on_thread(
        target, data, true, [](int type, EmscriptenFocusEvent const* event, void*) -> int { return 0; }, this_thread);
    emscripten_set_deviceorientation_callback_on_thread(
        data, true, [](int type, EmscriptenDeviceOrientationEvent const* event, void*) -> int { return 0; }, this_thread);
    emscripten_set_devicemotion_callback_on_thread(
        data, true, [](int type, EmscriptenDeviceMotionEvent const* event, void*) -> int { return 0; }, this_thread);
    emscripten_set_main_loop_arg(
        [](void* arg)
        {
            auto self = static_cast<emscripten::SystemIntegration*>(arg);
            self->loop();
        },
        this, 0, 0);
}

void ie::SystemIntegration::setup_imgui()
{
    ImGuiIO& io                    = ImGui::GetIO();
    io.KeyMap[ImGuiKey_Tab]        = Key_Tab;
    io.KeyMap[ImGuiKey_LeftArrow]  = Key_LeftArrow;
    io.KeyMap[ImGuiKey_RightArrow] = Key_RightArrow;
    io.KeyMap[ImGuiKey_UpArrow]    = Key_UpArrow;
    io.KeyMap[ImGuiKey_DownArrow]  = Key_DownArrow;
    io.KeyMap[ImGuiKey_PageUp]     = Key_PageUp;
    io.KeyMap[ImGuiKey_PageDown]   = Key_PageDown;
    io.KeyMap[ImGuiKey_Home]       = Key_Home;
    io.KeyMap[ImGuiKey_End]        = Key_End;
    io.KeyMap[ImGuiKey_Insert]     = Key_Insert;
    io.KeyMap[ImGuiKey_Delete]     = Key_Delete_Forward;
    io.KeyMap[ImGuiKey_Backspace]  = Key_Backspace;
    io.KeyMap[ImGuiKey_Space]      = Key_Spacebar;
    io.KeyMap[ImGuiKey_Enter]      = Key_Return;
    io.KeyMap[ImGuiKey_Escape]     = Key_ESCAPE;
    io.KeyMap[ImGuiKey_A]          = Key_A;
    io.KeyMap[ImGuiKey_C]          = Key_C;
    io.KeyMap[ImGuiKey_V]          = Key_V;
    io.KeyMap[ImGuiKey_X]          = Key_X;
    io.KeyMap[ImGuiKey_Y]          = Key_Y;
    io.KeyMap[ImGuiKey_Z]          = Key_Z;
}

void ie::SystemIntegration::update_imgui_state()
{
    ImGuiIO& io  = ImGui::GetIO();
    io.DeltaTime = 1.0f / 30.0f;

    emscripten_get_canvas_element_size("#canvas", &width, &height);
    if (width * height == 0) return;
    io.DisplaySize             = ImVec2(static_cast<float>(width), static_cast<float>(height));
    io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

    // put up the new mouse state and input state
    io.MouseDown[0] = (mouse_status[0] != 0);
    io.MouseDown[1] = (mouse_status[1] != 0);
    io.MouseDown[2] = (mouse_status[2] != 0);
}

void ie::SystemIntegration::cleanup_imgui_state()
{
    // put up the new mouse state and input state
    reset_mouse_state();
}

void ie::SystemIntegration::loop()
{
    update_imgui_state();
    if (renderer && context)
    {
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

void ie::SystemIntegration::reset_mouse_state()
{
    for (auto& status : mouse_status)
        if (status == 2) status = 0;
}

void ie::SystemIntegration::set_renderer(Renderer* r) { renderer = r; }

void ie::SystemIntegration::set_ui_call(std::function<void(Context*)>&& f) { create_ui = std::move(f); }

void                ie::SystemIntegration::set_context(imgui::Context* c) { context = c; }
std::pair<int, int> ie::SystemIntegration::window_size() { return {width, height}; }

ie::SystemIntegration::~SystemIntegration() = default;
