#pragma once

#include <functional>
#include <memory>

namespace imgui {
struct Context;
struct Style;

std::unique_ptr<Context> create_ui(std::function<void(Context*)>&& fun,
                                   std::unique_ptr<Style> style,
                                   size_t initial_width, size_t initial_height);

}  // namespace imgui
