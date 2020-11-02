#pragma once
#include <memory>
#include <string>

namespace imgui
{
struct Renderer;
struct SystemIntegration;
namespace win32
{
std::pair<std::unique_ptr<SystemIntegration>, std::unique_ptr<Renderer>> select_win32_setup(size_t initial_width, size_t initial_height, std::string const& window_name);
}  // namespace win32
}  // namespace imgui
