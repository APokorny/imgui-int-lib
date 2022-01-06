#include <iostream>
#include <memory>
#include <imgui.h>
#include <imgui/ui.h>
#include <imgui/context.h>
#include <imgui/default_style.h>
#include <imgui/widgets.h>
#include <imgui_node_editor.h>

namespace ed = ax::NodeEditor;
std::unique_ptr<imgui::Context> ui_context;
int                             main()
{
    ed::Config config;
    config.SettingsFile = "node-ed-example.json";
    ui_context          = imgui::create_ui(
                 [clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f), m_Context = ed::CreateEditor(&config)](imgui::Context*) mutable
                 {
            ImGui::Text("FPS: 2323s)");

            ImGui::Separator();

            ed::SetCurrentEditor(m_Context);
            ed::Begin("My Editor", ImVec2(0.0, 0.0f));
            int uniqueId = 1;
            // Start drawing nodes.
            ed::BeginNode(uniqueId++);
            ImGui::Text("Node A");
            ed::BeginPin(uniqueId++, ed::PinKind::Input);
            ImGui::Text("-> In");
            ed::EndPin();
            ImGui::SameLine();
            ed::BeginPin(uniqueId++, ed::PinKind::Output);
            ImGui::Text("Out ->");
            ed::EndPin();
            ed::EndNode();
            ed::End();
            ed::SetCurrentEditor(nullptr);

            ImGui::ShowMetricsWindow();
                 },
                 std::make_unique<imgui::DefaultStyle>(), "ImGui Example", 1200, 800);
    ui_context->run();
}
