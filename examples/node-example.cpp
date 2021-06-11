#include <iostream>
#include <memory>
#include "imgui/imgui.h"
#include "imgui/ui.h"
#include "imgui/context.h"
#include "imgui/default_style.h"
#include "imgui/widgets.h"

std::unique_ptr<imgui::Context> ui_context;
int                             main()
{
    ui_context = imgui::create_ui(
        [clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f)](imgui::Context*) mutable {
            // 1. Show the big demo window (Most of the sample code is in
            // ImGui::ShowDemoWindow()! You can browse its code to learn more
            // about Dear ImGui!).

        },
        std::make_unique<imgui::DefaultStyle>(), "ImGui Example", 1200, 800);
    ui_context->run();
}
