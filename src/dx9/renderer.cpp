#include "renderer.h"
#include "imgui/imgui.h"
#include <iostream>

namespace idx9 = imgui::dx9;

namespace
{
struct CUSTOMVERTEX
{
    float    pos[3];
    D3DCOLOR col;
    float    uv[2];
};
constexpr auto D3DFVF_CUSTOMVERTEX = (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);

void clear_d3d9(LPDIRECT3D9& d9, LPDIRECT3DDEVICE9& dev)
{
    if(dev)
        dev->Release();
    if(d9)
        d9->Release();
    d9 = nullptr;
    dev = nullptr;
}

void create_dx9_device(LPDIRECT3D9& d9, D3DPRESENT_PARAMETERS& params, HWND window, LPDIRECT3DDEVICE9& dev)
{
    if ((d9 = Direct3DCreate9(D3D_SDK_VERSION)) == nullptr) return;

    auto clean_on_error = [](auto thing) -> void {
        thing->Release();
        thing = nullptr;
    };
    // Create the D3DDevice
    ZeroMemory(&params, sizeof(params));
    params.Windowed               = TRUE;
    params.SwapEffect             = D3DSWAPEFFECT_DISCARD;
    params.BackBufferFormat       = D3DFMT_UNKNOWN;
    params.EnableAutoDepthStencil = TRUE;
    params.AutoDepthStencilFormat = D3DFMT_D16;
    params.Flags                  = D3DPRESENTFLAG_DEVICECLIP;
    params.PresentationInterval   = D3DPRESENT_INTERVAL_ONE;  // Present with vsync
    // params.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
    if (d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, window, D3DCREATE_HARDWARE_VERTEXPROCESSING, &params, &dev) < 0)
    {
        std::cerr << "Failed to create DX9 device\n";
        clean_on_error(d9);
        return;
    }

    dev->AddRef();
}
}  // namespace

idx9::Renderer::Renderer(HWND window) : window(window) { create_dx9_device(direct3d9, d3d_parameters, window, d3d_device); }

void idx9::Renderer::resize(size_t width, size_t height)
{
    d3d_parameters.BackBufferWidth = width;
    d3d_parameters.BackBufferHeight =height;
    reset_device();
}

void idx9::Renderer::setup_renderer_state(ImDrawData& draw_data)
{
    // Setup viewport
    D3DVIEWPORT9 vp;
    vp.X = vp.Y = 0;
    vp.Width    = static_cast<DWORD>(draw_data.DisplaySize.x);
    vp.Height   = static_cast<DWORD>(draw_data.DisplaySize.y);
    vp.MinZ     = 0.0f;
    vp.MaxZ     = 1.0f;
    d3d_device->SetViewport(&vp);

    // Setup render state: fixed-pipeline, alpha-blending, no face culling, no depth testing, shade mode (for gradient)
    d3d_device->SetPixelShader(NULL);
    d3d_device->SetVertexShader(NULL);
    d3d_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    d3d_device->SetRenderState(D3DRS_LIGHTING, FALSE);
    d3d_device->SetRenderState(D3DRS_ZENABLE, FALSE);
    d3d_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    d3d_device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    d3d_device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
    d3d_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    d3d_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    d3d_device->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
    d3d_device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
    d3d_device->SetRenderState(D3DRS_FOGENABLE, FALSE);
    d3d_device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    d3d_device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    d3d_device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    d3d_device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    d3d_device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    d3d_device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    d3d_device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    d3d_device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

    // Setup orthographic projection matrix
    // Our visible imgui space lies from draw_data.DisplayPos (top left) to draw_data.DisplayPos+draw_data.DisplaySize (bottom right).
    // DisplayPos is (0,0) for single viewport apps. Being agnostic of whether <d3dx9.h> or <DirectXMath.h> can be used, we aren't relying
    // on D3DXMatrixIdentity()/D3DXMatrixOrthoOffCenterLH() or DirectX::XMMatrixIdentity()/DirectX::XMMatrixOrthographicOffCenterLH()
    {
        float     L              = draw_data.DisplayPos.x + 0.5f;
        float     R              = draw_data.DisplayPos.x + draw_data.DisplaySize.x + 0.5f;
        float     T              = draw_data.DisplayPos.y + 0.5f;
        float     B              = draw_data.DisplayPos.y + draw_data.DisplaySize.y + 0.5f;
        D3DMATRIX mat_identity   = {{{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f}}};
        D3DMATRIX mat_projection = {{{2.0f / (R - L), 0.0f, 0.0f, 0.0f, 0.0f, 2.0f / (T - B), 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f,
                                      (L + R) / (L - R), (T + B) / (B - T), 0.5f, 1.0f}}};
        d3d_device->SetTransform(D3DTS_WORLD, &mat_identity);
        d3d_device->SetTransform(D3DTS_VIEW, &mat_identity);
        d3d_device->SetTransform(D3DTS_PROJECTION, &mat_projection);
    }
}

void idx9::Renderer::pre_frame()
{
    // Clear:
    auto ret = d3d_device->SetRenderState(D3DRS_ZENABLE, FALSE);
    ret = d3d_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    ret = d3d_device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
    D3DCOLOR clear_col_dx = D3DCOLOR_RGBA(static_cast<int>(clear_color.x * 255.0f), static_cast<int>(clear_color.y * 255.0f),
                                          static_cast<int>(clear_color.z * 255.0f), static_cast<int>(clear_color.w * 255.0f));
    d3d_device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
    if (d3d_device->BeginScene() < 0)
    {
	invalidate_device_objects();
        clear_d3d9(direct3d9, d3d_device);
        create_dx9_device(direct3d9, d3d_parameters, window, d3d_device);
	create_device_objects();
        return;
    }

}

void idx9::Renderer::render_imgui_data(ImDrawData& draw_data)
{
    // Avoid rendering when minimized
    if (draw_data.DisplaySize.x <= 0.0f || draw_data.DisplaySize.y <= 0.0f) return;

    // Create and grow buffers if needed
    if (!vertex_buffer || vertex_buffer_size < draw_data.TotalVtxCount)
    {
        if (vertex_buffer)
        {
            vertex_buffer->Release();
            vertex_buffer = NULL;
        }
        vertex_buffer_size = draw_data.TotalVtxCount + 5000;
        if (d3d_device->CreateVertexBuffer(vertex_buffer_size * sizeof(CUSTOMVERTEX), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
                                           D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &vertex_buffer, NULL) < 0)
            return;
    }
    if (!index_buffer || index_buffer_size < draw_data.TotalIdxCount)
    {
        if (index_buffer)
        {
            index_buffer->Release();
            index_buffer = NULL;
        }
        index_buffer_size = draw_data.TotalIdxCount + 10000;
        if (d3d_device->CreateIndexBuffer(index_buffer_size * sizeof(ImDrawIdx), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
                                          sizeof(ImDrawIdx) == 2 ? D3DFMT_INDEX16 : D3DFMT_INDEX32, D3DPOOL_DEFAULT, &index_buffer,
                                          NULL) < 0)
            return;
    }

    // Backup the DX9 state
    IDirect3DStateBlock9* d3d9_state_block = NULL;
    if (d3d_device->CreateStateBlock(D3DSBT_ALL, &d3d9_state_block) < 0) return;

    // Backup the DX9 transform (DX9 documentation suggests that it is included in the StateBlock but it doesn't appear to)
    D3DMATRIX last_world, last_view, last_projection;
    d3d_device->GetTransform(D3DTS_WORLD, &last_world);
    d3d_device->GetTransform(D3DTS_VIEW, &last_view);
    d3d_device->GetTransform(D3DTS_PROJECTION, &last_projection);

    // Copy and convert all vertices into a single contiguous buffer, convert colors to DX9 default format.
    // FIXME-OPT: This is a waste of resource, the ideal is to use imconfig.h and
    //  1) to avoid repacking colors:   #define IMGUI_USE_BGRA_PACKED_COLOR
    //  2) to avoid repacking vertices: #define IMGUI_OVERRIDE_DRAWVERT_STRUCT_LAYOUT struct ImDrawVert { ImVec2 pos; float z; ImU32 col;
    //  ImVec2 uv; }
    CUSTOMVERTEX* vtx_dst;
    ImDrawIdx*    idx_dst;
    if (vertex_buffer->Lock(0, (UINT)(draw_data.TotalVtxCount * sizeof(CUSTOMVERTEX)), (void**)&vtx_dst, D3DLOCK_DISCARD) < 0) return;
    if (index_buffer->Lock(0, (UINT)(draw_data.TotalIdxCount * sizeof(ImDrawIdx)), (void**)&idx_dst, D3DLOCK_DISCARD) < 0) return;
    for (int n = 0; n < draw_data.CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data.CmdLists[n];
        const ImDrawVert* vtx_src  = cmd_list->VtxBuffer.Data;
        for (int i = 0; i < cmd_list->VtxBuffer.Size; i++)
        {
            vtx_dst->pos[0] = vtx_src->pos.x;
            vtx_dst->pos[1] = vtx_src->pos.y;
            vtx_dst->pos[2] = 0.0f;
            vtx_dst->col    = (vtx_src->col & 0xFF00FF00) | ((vtx_src->col & 0xFF0000) >> 16) |
                           ((vtx_src->col & 0xFF) << 16);  // RGBA --> ARGB for DirectX9
            vtx_dst->uv[0] = vtx_src->uv.x;
            vtx_dst->uv[1] = vtx_src->uv.y;
            vtx_dst++;
            vtx_src++;
        }
        memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
        idx_dst += cmd_list->IdxBuffer.Size;
    }
    vertex_buffer->Unlock();
    index_buffer->Unlock();
    d3d_device->SetStreamSource(0, vertex_buffer, 0, sizeof(CUSTOMVERTEX));
    d3d_device->SetIndices(index_buffer);
    d3d_device->SetFVF(D3DFVF_CUSTOMVERTEX);

    // Setup desired DX state
    setup_renderer_state(draw_data);

    // Render command lists
    // (Because we merged all buffers into a single one, we maintain our own offset into them)
    int    global_vtx_offset = 0;
    int    global_idx_offset = 0;
    ImVec2 clip_off          = draw_data.DisplayPos;
    for (int n = 0; n < draw_data.CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data.CmdLists[n];
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
		const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback != NULL)
            {
                // User callback, registered via ImDrawList::AddCallback()
                // (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render
                // state.)
                if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
                    setup_renderer_state(draw_data);
                else
                    pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                const RECT               r       = {(LONG)(pcmd->ClipRect.x - clip_off.x), (LONG)(pcmd->ClipRect.y - clip_off.y),
                                (LONG)(pcmd->ClipRect.z - clip_off.x), (LONG)(pcmd->ClipRect.w - clip_off.y)};
                const LPDIRECT3DTEXTURE9 texture = (LPDIRECT3DTEXTURE9)pcmd->TextureId;
                d3d_device->SetTexture(0, texture);
                d3d_device->SetScissorRect(&r);
                d3d_device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, pcmd->VtxOffset + global_vtx_offset, 0, (UINT)cmd_list->VtxBuffer.Size,
                                                 pcmd->IdxOffset + global_idx_offset, pcmd->ElemCount / 3);
            }
        }
        global_idx_offset += cmd_list->IdxBuffer.Size;
        global_vtx_offset += cmd_list->VtxBuffer.Size;
    }

    // Restore the DX9 transform
    d3d_device->SetTransform(D3DTS_WORLD, &last_world);
    d3d_device->SetTransform(D3DTS_VIEW, &last_view);
    d3d_device->SetTransform(D3DTS_PROJECTION, &last_projection);

    // Restore the DX9 state
    d3d9_state_block->Apply();
    d3d9_state_block->Release();
}

void idx9::Renderer::create_device_objects()
{
    // Build texture atlas
    ImGuiIO&       io = ImGui::GetIO();
    unsigned char* pixels;
    int            width, height, bytes_per_pixel;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height, &bytes_per_pixel);

    // Upload texture to graphics system
    font_texture = nullptr;
    if (d3d_device->CreateTexture(width, height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &font_texture, NULL) < 0) return;
    D3DLOCKED_RECT tex_locked_rect;
    if (font_texture->LockRect(0, &tex_locked_rect, NULL, 0) != D3D_OK) return;
    for (int y = 0; y < height; y++)
        memcpy((unsigned char*)tex_locked_rect.pBits + tex_locked_rect.Pitch * y, pixels + (width * bytes_per_pixel) * y,
               (width * bytes_per_pixel));
    font_texture->UnlockRect(0);

    // Store our identifier
    io.Fonts->TexID = (ImTextureID)font_texture;
}

void idx9::Renderer::invalidate_device_objects()
{
    if (!d3d_device) return;
    if (vertex_buffer)
    {
        vertex_buffer->Release();
        vertex_buffer = nullptr;
    }
    if (index_buffer)
    {
        index_buffer->Release();
        index_buffer = nullptr;
    }
    if (font_texture)
    {
        font_texture->Release();
        font_texture                = nullptr;
        ImGui::GetIO().Fonts->TexID = nullptr;
    }
}

idx9::Renderer::~Renderer()
{
    invalidate_device_objects();
    if (d3d_device)
    {
        d3d_device->Release();
        d3d_device = nullptr;
    }
}

void idx9::Renderer::finish_frame()
{
    d3d_device->EndScene();
    HRESULT result = d3d_device->Present(NULL, NULL, NULL, NULL);

    // Handle loss of D3D9 device
    if (result == D3DERR_DEVICELOST && d3d_device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET) reset_device();
}

void idx9::Renderer::setup_imgui()
{
    ImGuiIO& io            = ImGui::GetIO();
    io.BackendRendererName = "imgui_impl_dx9";
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;  // We can honor the ImDrawCmd::VtxOffset field, allowing for large meshes.
}

void idx9::Renderer::acquire_cached_resources()
{
    create_device_objects();
}

void idx9::Renderer::reset_device()
{
    invalidate_device_objects();
    HRESULT hr = d3d_device->Reset(&d3d_parameters);
    if (hr == D3DERR_INVALIDCALL) {
        clear_d3d9(direct3d9, d3d_device);
        create_dx9_device(direct3d9, d3d_parameters, window, d3d_device);
    }
    create_device_objects();
}
