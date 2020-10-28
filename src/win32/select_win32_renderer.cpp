#include "windows_headers.h"
#include "select_win32_renderer.h"
#include "dx9/renderer.h"
#include "win32/system_integration.h"
#include <iostream>

namespace id = imgui::dx9;
namespace iw = imgui::win32;
auto WndProc = nullptr;

std::pair<std::unique_ptr<imgui::SystemIntegration>, std::unique_ptr<imgui::Renderer>> iw::select_win32_setup(size_t initial_width,
                                                                                                              size_t initial_height, std::string const& window_name)
{
    auto is_d3d_9_available = Direct3DCreate9(D3D_SDK_VERSION);
    // further tests for other versions could be added..
    // then the preferred dx version selected
    if (is_d3d_9_available)
        is_d3d_9_available->Release();
    else
        return {nullptr, nullptr};

    auto system   = std::make_unique<iw::SystemIntegration>(initial_width, initial_height, window_name.c_str());
    auto renderer = std::make_unique<id::Renderer>(system->handle());
    return {std::move(system), std::move(renderer)};
}

