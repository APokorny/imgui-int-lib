#include "imgui/default_style.h"

imgui::DefaultStyle::DefaultStyle() {}
void imgui::DefaultStyle::apply_style() {}
ImFontAtlas* imgui::DefaultStyle::get_font_atlas() { return &fonts; }
