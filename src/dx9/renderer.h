#pragma once
#include "win32/windows_headers.h"
#include <d3d9.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <string>
#include "imgui/renderer.h"
#include "imgui/imgui.h"

struct ImDrawData;
struct IDirect3DDevice9;
namespace imgui
{
namespace dx9
{
struct Renderer : imgui::Renderer
{
   public:
    Renderer(HWND hwnd);
    void setup_imgui() override;
    void acquire_cached_resources() override;
    void render_imgui_data(ImDrawData& draw_data) override;
    void pre_frame() override;
    void finish_frame() override;
    void resize(size_t width, size_t height) override;
    ~Renderer();

   private:
    void                    create_device_objects();
    void                    reset_device();
    void                    invalidate_device_objects();
    void                    setup_renderer_state(ImDrawData& draw_data);
    HWND                    window;
    LPDIRECT3D9             direct3d9{nullptr};
    D3DPRESENT_PARAMETERS   d3d_parameters;
    LPDIRECT3DDEVICE9       d3d_device{nullptr};
    LPDIRECT3DVERTEXBUFFER9 vertex_buffer{nullptr};
    LPDIRECT3DINDEXBUFFER9  index_buffer{nullptr};
    LPDIRECT3DTEXTURE9      font_texture{nullptr};
    int                     vertex_buffer_size{5000};
    int                     index_buffer_size = 10000;
    ImVec4                  clear_color{0.0f, 0.0f, 0.0f, 1.0f};
};

}  // namespace dx9
}  // namespace imgui
