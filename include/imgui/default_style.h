#pragma once
#include "imgui/style.h"
#include <imgui.h>

namespace imgui {
struct DefaultStyle : imgui::Style {
    DefaultStyle();
    void apply_style();
    ImFontAtlas* get_font_atlas();

   private:
    ImFontAtlas fonts;
};
}  // namespace imgui
