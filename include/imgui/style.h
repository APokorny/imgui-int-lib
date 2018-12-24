#pragma once

struct ImFontAtlas;
namespace imgui {
struct Style {
    Style() = default;
    Style(Style const&) = delete;
    Style& operator=(Style const&) = delete;
    ~Style() = default;
    virtual void apply_style() = 0;
    virtual ImFontAtlas* get_font_atlas() = 0;
};
}  // namespace ImGui
