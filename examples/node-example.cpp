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

struct LinkInfo
{
    ed::LinkId Id;
    ed::PinId  InputId;
    ed::PinId  OutputId;
};

int main()
{
    ed::Config config;

    config.SettingsFile = "node-ed-example.json";
    ui_context          = imgui::create_ui(
        [links        = ImVector<LinkInfo>(),  // list of linksnext_link_id
         next_link_id = 100,                   //  utility to create link id - could be a unique user data ptr instead
         clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f), m_Context = ed::CreateEditor(&config)](imgui::Context*) mutable
        {
            static bool firstframe = true;  // Used to position the nodes on startup
            auto& io = ImGui::GetIO();
            ImGui::Text("FPS: 2323s)");

            ImGui::Separator();

            ed::SetCurrentEditor(m_Context);
            ed::Begin("My Editor", ImVec2(0.0, 0.0f));
            int uniqueId = 1;

            // Basic Widgets Demo  ==============================================================================================
            auto basic_id = uniqueId++;
            ed::BeginNode(basic_id);
            ImGui::Text("Basic Widget Demo");
            ed::BeginPin(uniqueId++, ed::PinKind::Input);
            ImGui::Text("-> In");
            ed::EndPin();
            ImGui::SameLine();
            ImGui::Dummy(ImVec2(250, 0));  // Hacky magic number to space out the output pin.
            ImGui::SameLine();
            ed::BeginPin(uniqueId++, ed::PinKind::Output);
            ImGui::Text("Out ->");
            ed::EndPin();

            // Widget Demo from imgui_demo.cpp...
            // Normal Button
            static int clicked = 0;
            if (ImGui::Button("Button")) clicked++;
            if (clicked & 1)
            {
                ImGui::SameLine();
                ImGui::Text("Thanks for clicking me!");
            }

            // Checkbox
            static bool check = true;
            ImGui::Checkbox("checkbox", &check);

            // Radio buttons
            static int e = 0;
            ImGui::RadioButton("radio a", &e, 0);
            ImGui::SameLine();
            ImGui::RadioButton("radio b", &e, 1);
            ImGui::SameLine();
            ImGui::RadioButton("radio c", &e, 2);

            // Color buttons, demonstrate using PushID() to add unique identifier in the ID stack, and changing style.
            for (int i = 0; i < 7; i++)
            {
                if (i > 0) ImGui::SameLine();
                ImGui::PushID(i);
                ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(i / 7.0f, 0.6f, 0.6f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(i / 7.0f, 0.7f, 0.7f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(i / 7.0f, 0.8f, 0.8f));
                ImGui::Button("Click");
                ImGui::PopStyleColor(3);
                ImGui::PopID();
            }

            // Use AlignTextToFramePadding() to align text baseline to the baseline of framed elements (otherwise a Text+SameLine+Button
            // sequence will have the text a little too high by default)
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Hold to repeat:");
            ImGui::SameLine();

            // Arrow buttons with Repeater
            static int counter = 0;
            float      spacing = ImGui::GetStyle().ItemInnerSpacing.x;
            ImGui::PushButtonRepeat(true);
            if (ImGui::ArrowButton("##left", ImGuiDir_Left)) { counter--; }
            ImGui::SameLine(0.0f, spacing);
            if (ImGui::ArrowButton("##right", ImGuiDir_Right)) { counter++; }
            ImGui::PopButtonRepeat();
            ImGui::SameLine();
            ImGui::Text("%d", counter);

            // The input widgets also require you to manually disable the editor shortcuts so the view doesn't fly around.
            // (note that this is a per-frame setting, so it disables it for all text boxes.  I left it here so you could find it!)
            ed::EnableShortcuts(!io.WantTextInput);
            // The input widgets require some guidance on their widths, or else they're very large. (note matching pop at the end).
            ImGui::PushItemWidth(200);
            static char str1[128] = "";
            ImGui::InputTextWithHint("input text (w/ hint)", "enter text here", str1, IM_ARRAYSIZE(str1));

            static float f0 = 0.001f;
            ImGui::InputFloat("input float", &f0, 0.01f, 1.0f, "%.3f");

            static float f1 = 1.00f, f2 = 0.0067f;
            ImGui::DragFloat("drag float", &f1, 0.005f);
            ImGui::DragFloat("drag small float", &f2, 0.0001f, 0.0f, 0.0f, "%.06f ns");
            ImGui::PopItemWidth();

            ed::EndNode();
            if (firstframe) { ed::SetNodePosition(basic_id, ImVec2(20, 20)); }

            // Tool Tip & Pop-up Demo =====================================================================================
            // Tooltips, combo-boxes, drop-down menus need to use a work-around to place the "overlay window" in the canvas.
            // To do this, we must defer the popup calls until after we're done drawing the node material.
            //
            // Relevent bugs:  https://github.com/thedmd/imgui-node-editor/issues/48
            auto popup_id = uniqueId++;
            ed::BeginNode(popup_id);
            ImGui::Text("Tool Tip & Pop-up Demo");
            ed::BeginPin(uniqueId++, ed::PinKind::Input);
            ImGui::Text("-> In");
            ed::EndPin();
            ImGui::SameLine();
            ImGui::Dummy(ImVec2(85, 0));  // Hacky magic number to space out the output pin.
            ImGui::SameLine();
            ed::BeginPin(uniqueId++, ed::PinKind::Output);
            ImGui::Text("Out ->");
            ed::EndPin();

            // Tooltip example
            ImGui::Text("Hover over me");
            static bool do_tooltip = false;
            do_tooltip             = ImGui::IsItemHovered() ? true : false;
            ImGui::SameLine();
            ImGui::Text("- or me");
            static bool do_adv_tooltip = false;
            do_adv_tooltip             = ImGui::IsItemHovered() ? true : false;

            // Use AlignTextToFramePadding() to align text baseline to the baseline of framed elements
            // (otherwise a Text+SameLine+Button sequence will have the text a little too high by default)
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Option:");
            ImGui::SameLine();
            static char popup_text[128] = "Pick one!";
            static bool do_popup        = false;
            if (ImGui::Button(popup_text))
            {
                do_popup =
                    true;  // Instead of saying OpenPopup() here, we set this bool, which is used later in the Deferred Pop-up Section
            }
            ed::EndNode();
            if (firstframe) { ed::SetNodePosition(popup_id, ImVec2(610, 20)); }

            // --------------------------------------------------------------------------------------------------
            // Deferred Pop-up Section

            // This entire section needs to be bounded by Suspend/Resume!  These calls pop us out of "node canvas coordinates"
            // and draw the popups in a reasonable screen location.
            ed::Suspend();
            // There is some stately stuff happening here.  You call "open popup" exactly once, and this
            // causes it to stick open for many frames until the user makes a selection in the popup, or clicks off to dismiss.
            // More importantly, this is done inside Suspend(), so it loads the popup with the correct screen coordinates!
            if (do_popup)
            {
                ImGui::OpenPopup("popup_button");  // Cause openpopup to stick open.
                do_popup = false;  // disable bool so that if we click off the popup, it doesn't open the next frame.
            }

            // This is the actual popup Gui drawing section.
            if (ImGui::BeginPopup("popup_button"))
            {
                // Note: if it weren't for the child window, we would have to PushItemWidth() here to avoid a crash!
                ImGui::TextDisabled("Pick One:");
                ImGui::BeginChild("popup_scroller", ImVec2(100, 100), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
                if (ImGui::Button("Option 1"))
                {
                    strcpy(popup_text, "Option 1");
                    ImGui::CloseCurrentPopup();  // These calls revoke the popup open state, which was set by OpenPopup above.
                }
                if (ImGui::Button("Option 2"))
                {
                    strcpy(popup_text, "Option 2");
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::Button("Option 3"))
                {
                    strcpy(popup_text, "Option 3");
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::Button("Option 4"))
                {
                    strcpy(popup_text, "Option 4");
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndChild();
                ImGui::EndPopup();  // Note this does not do anything to the popup open/close state. It just terminates the content
                                    // declaration.
            }

            // Handle the simple tooltip
            if (do_tooltip) ImGui::SetTooltip("I am a tooltip");

            // Handle the advanced tooltip
            if (do_adv_tooltip)
            {
                ImGui::BeginTooltip();
                ImGui::Text("I am a fancy tooltip");
                static float arr[] = {0.6f, 0.1f, 1.0f, 0.5f, 0.92f, 0.1f, 0.2f};
                ImGui::PlotLines("Curve", arr, IM_ARRAYSIZE(arr));
                ImGui::EndTooltip();
            }

            ed::Resume();
            // End of "Deferred Pop-up section"

            // Plot Widgets =========================================================================================
            // Note: most of these plots can't be used in nodes missing, because they spawn tooltips automatically,
            // so we can't trap them in our deferred pop-up mechanism.  This causes them to fly into a random screen
            // location.
            auto plot_id = uniqueId++;
            ed::BeginNode(plot_id);
            ImGui::Text("Plot Demo");
            ed::BeginPin(uniqueId++, ed::PinKind::Input);
            ImGui::Text("-> In");
            ed::EndPin();
            ImGui::SameLine();
            ImGui::Dummy(ImVec2(250, 0));  // Hacky magic number to space out the output pin.
            ImGui::SameLine();
            ed::BeginPin(uniqueId++, ed::PinKind::Output);
            ImGui::Text("Out ->");
            ed::EndPin();

            ImGui::PushItemWidth(300);

            // Animate a simple progress bar
            static float progress = 0.0f, progress_dir = 1.0f;
            progress += progress_dir * 0.4f * ImGui::GetIO().DeltaTime;
            if (progress >= +1.1f)
            {
                progress = +1.1f;
                progress_dir *= -1.0f;
            }
            if (progress <= -0.1f)
            {
                progress = -0.1f;
                progress_dir *= -1.0f;
            }

            // Typically we would use ImVec2(-1.0f,0.0f) or ImVec2(-FLT_MIN,0.0f) to use all available width,
            // or ImVec2(width,0.0f) for a specified width. ImVec2(0.0f,0.0f) uses ItemWidth.
            ImGui::ProgressBar(progress, ImVec2(0.0f, 0.0f));
            ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
            ImGui::Text("Progress Bar");

            float progress_saturated = (progress < 0.0f) ? 0.0f : (progress > 1.0f) ? 1.0f : progress;
            char  buf[32];
            sprintf(buf, "%d/%d", (int)(progress_saturated * 1753), 1753);
            ImGui::ProgressBar(progress, ImVec2(0.f, 0.f), buf);

            ImGui::PopItemWidth();
            ed::EndNode();
            if (firstframe) { ed::SetNodePosition(plot_id, ImVec2(850, 20)); }
            // ==================================================================================================
            // Link Drawing Section

            for (auto& linkInfo : links) ed::Link(linkInfo.Id, linkInfo.InputId, linkInfo.OutputId);

            // ==================================================================================================
            // Interaction Handling Section
            // This was coppied from BasicInteration.cpp. See that file for commented code.

            // Handle creation action ---------------------------------------------------------------------------
            if (ed::BeginCreate())
            {
                ed::PinId inputPinId, outputPinId;
                if (ed::QueryNewLink(&inputPinId, &outputPinId))
                {
                    if (inputPinId && outputPinId)
                    {
                        if (ed::AcceptNewItem())
                        {
                            links.push_back({ed::LinkId(next_link_id++), inputPinId, outputPinId});
                            ed::Link(links.back().Id, links.back().InputId, links.back().OutputId);
                        }
                    }
                }
            }
            ed::EndCreate();

            // Handle deletion action ---------------------------------------------------------------------------
            if (ed::BeginDelete())
            {
                ed::LinkId deletedLinkId;
                while (ed::QueryDeletedLink(&deletedLinkId))
                {
                    if (ed::AcceptDeletedItem())
                    {
                        for (auto& link : links)
                        {
                            if (link.Id == deletedLinkId)
                            {
                                links.erase(&link);
                                break;
                            }
                        }
                    }
                }
            }
            ed::EndDelete();

            ed::End();
            ed::SetCurrentEditor(nullptr);
            firstframe = false;

            ImGui::ShowMetricsWindow();
        },
        std::make_unique<imgui::DefaultStyle>(), "ImGui Example", 1200, 800);
    ui_context->run();
}
