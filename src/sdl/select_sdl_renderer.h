#pragma once
#include <memory>

namespace imgui {
struct Renderer;
struct SystemIntegration;
namespace sdl {
std::pair<std::unique_ptr<SystemIntegration>, std::unique_ptr<Renderer>>
select_sdl_setup(size_t initial_width, size_t initial_height, std::string const& wn);
}  // namespace sdl
}  // namespace imgui
