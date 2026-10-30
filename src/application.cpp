#include <algorithm>
#include <vector>
#include "component.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_stdlib.h"
#include "GLFW/glfw3.h"
#include "application.h"
#include "pcb.h"
#include "component.h"


void App::Parameters(PCB& pcb)
{
    ImGui::Begin("Simulation Settings");

    ImVec2 avail = ImGui::GetContentRegionAvail();

    ImGui::PushItemWidth(std::max(200.0f, avail.x*0.3f)); // Widget Bar width

    bool resize_pcb = false;
    bool reload_components = false;

    ImGui::PushFont(jetbrainsmono_bold, 0.0f); // change font, keep current size
    ImGui::SeparatorText("PCB Parameters");

    if(ImGui::CollapsingHeader("PCB Parameters", ImGuiTreeNodeFlags_DefaultOpen)){
        ImGui::PushFont(jetbrainsmono_regular, 0.0f);
        resize_pcb |= ImGui::InputFloat("Width", &pcb.width, 1, 5, "%.2f", ImGuiInputTextFlags_EnterReturnsTrue);
        pcb.width = std::max(2*pcb.cell_size, pcb.width);
        ImGui::SameLine(); HelpMarker("PCB Width in mm.");
        resize_pcb |= ImGui::InputFloat("Height", &pcb.height, 1, 5, "%.2f", ImGuiInputTextFlags_EnterReturnsTrue);
        pcb.height = std::max(2*pcb.cell_size, pcb.height);
        ImGui::SameLine(); HelpMarker("PCB Height in mm.");
        resize_pcb |= ImGui::InputFloat("Cell Size", &pcb.cell_size, 1.0f, 5.0f, "%.1f", ImGuiInputTextFlags_EnterReturnsTrue);
        pcb.cell_size = std::max(pcb.cell_size, 0.5f);
        ImGui::SameLine(); HelpMarker("PCB mesh cell size in mm.");
        resize_pcb |= ImGui::SliderFloat("Layer thicknes", &pcb.layer_thickness, 18.0f, 70.0f, "%.1f");
        ImGui::SameLine(); HelpMarker("Copper layer thickness in um.\n1/2oz = 18um\n1oz = 35um\n2oz = 70um");
        resize_pcb |= ImGui::SliderInt("Number of copper planes", &pcb.layers, 2.0f, 32.0f, "%d");
        ImGui::SameLine(); HelpMarker("Number of approximately solid copper planes.");
        ImGui::PopFont();
    }


    ImGui::SeparatorText("Ambient Parameters");
    if(ImGui::CollapsingHeader("Ambient Parameters", ImGuiTreeNodeFlags_DefaultOpen)){
        ImGui::PushFont(jetbrainsmono_regular, 0.0f);
        resize_pcb |= ImGui::InputFloat("Ambient Temperature", &pcb.t_amb, 1, 5, "%.1f");
        ImGui::SameLine(); HelpMarker("Temperature of surrounding fluid in ºC.");
        resize_pcb |= ImGui::InputFloat("Convection Coefficient", &pcb.h_conv, 1, 5, "%.1f");
        ImGui::SameLine(); HelpMarker("Natural Convection Coefficient for heat dissipation to air in W/m²K.");
        resize_pcb |= ImGui::Checkbox("Convection from both faces of PCB", &pcb.convection_2_faces);
        ImGui::SameLine(); HelpMarker("Activates heat convection from the bottom side of the PCB.");
        ImGui::PopFont();
    }

    if(resize_pcb){
        pcb.resize();
    }

    ImGui::SeparatorText("Components");
    if(ImGui::CollapsingHeader("Components", ImGuiTreeNodeFlags_DefaultOpen)){
        ImGui::PushFont(jetbrainsmono_regular, 0.0f);
        if(ImGui::Button("Add Component")){
            pcb.components.push_back(Component{"U" + std::to_string(pcb.components.size() + 1), pcb.width*0.2f, 
            pcb.height*0.2f, pcb.width*0.5f, pcb.height*0.5f});
            pcb.component_selected = (int)pcb.components.size() - 1;
            reload_components = true;

        }

        ImGui::SameLine();
        if(ImGui::Button("Delete Component") && pcb.component_selected >= 0){
            pcb.components.erase(pcb.components.begin()+pcb.component_selected);
            pcb.component_selected = -1;
            reload_components = true;
        }

        ImGui::SameLine(); HelpMarker("Adds a component or deletes selected component.");

        for(int i = 0; i<(int)pcb.components.size(); i++){
            if(ImGui::Selectable((pcb.components[i].name+"##").c_str(), pcb.component_selected == i)) pcb.component_selected = i;
        }

        ImGui::Separator();

        if(pcb.component_selected >= 0 && pcb.component_selected < (int)pcb.components.size()){
            reload_components |= ImGui::InputText("Name", &pcb.components[pcb.component_selected].name, 0, nullptr, nullptr);
            ImGui::SameLine(); HelpMarker("Identifier of the component.");
            reload_components |= ImGui::InputFloat("X", &pcb.components[pcb.component_selected].x_pos, 1.0f, 2.0f, "%.1f");
            ImGui::SameLine(); HelpMarker("Horizontal position of the component in mm.");
            reload_components |= ImGui::InputFloat("Y", &pcb.components[pcb.component_selected].y_pos, 1.0f, 2.0f, "%.1f");
            ImGui::SameLine(); HelpMarker("Vertical position of the component in mm.");
            reload_components |= ImGui::DragFloat("Component width", &pcb.components[pcb.component_selected].width, 1.0f, 0.1f, 50.0f, "%.1f");
            ImGui::SameLine(); HelpMarker("Horizontal size of the component in mm.");
            reload_components |= ImGui::DragFloat("Component height", &pcb.components[pcb.component_selected].height, 1.0f, 0.1f, 50.0f, "%.1f");
            ImGui::SameLine(); HelpMarker("Vertizal size of the component in mm.");
            reload_components |= ImGui::DragFloat("Dissipated power", &pcb.components[pcb.component_selected].power, 0.5f, 0.1f, 20.0f, "%.2f");
            ImGui::SameLine(); HelpMarker("Heat power dissipated by the component in Watts.");
            reload_components |= ImGui::DragFloat("R_jb", &pcb.components[pcb.component_selected].Rjb, 0.05f, 1.0f, 50.0f, "%.1f");
            ImGui::SameLine(); HelpMarker("Thermal resistance from junction to base in K/W.");
            reload_components |= ImGui::DragFloat("R_jc", &pcb.components[pcb.component_selected].Rjc, 0.05f, 5.0f, 100.0f, "%.1f");
            ImGui::SameLine(); HelpMarker("Thermal resistance from junction to case in K/W.");
            if (ImGui::Checkbox("Has Heatsink", &pcb.components[pcb.component_selected].hasHeatsink)){
                reload_components = 1;
            }
            ImGui::SameLine(); HelpMarker("Puts a heatsink on top of the component.");
            if(!pcb.components[pcb.component_selected].hasHeatsink){
                ImGui::BeginDisabled();
            }

            const char* material_names[3] = { "Copper", "Aluminum", "Ceramic"};
            const char* material_name = (pcb.components[pcb.component_selected].material >= 0 && pcb.components[pcb.component_selected].material < 3) ? material_names[pcb.components[pcb.component_selected].material] : "Unknown";
            reload_components |= ImGui::SliderInt("Heatsink Material",(int *) &pcb.components[pcb.component_selected].material, 0, 2, material_name);
            ImGui::SameLine(); HelpMarker("Heatsink material selector used to estimate thermal conductivity for lateral heat diffusion.\nCopper: 350 W/mK\nAluminum: 237 W/mK\nCeramic (Non conductive): 30 W/mK");
            reload_components |= ImGui::DragFloat("R_ca", &pcb.components[pcb.component_selected].Rca, 0.05f, 1.0f, 100.0f, "%.1f");
            ImGui::SameLine(); HelpMarker("Thermal resistance from component case to ambient in K/W.\nUsually given in heatsinks datasheet.");
            reload_components |= ImGui::DragFloat("Heatsink Thickness", &pcb.components[pcb.component_selected].heatsink_thickness, 0.05f, 0.5f, 10, "%.1f");
            ImGui::SameLine(); HelpMarker("Heatsink thickness in mm used to estimate the cross-section for lateral heat diffusion.");


            if(!pcb.components[pcb.component_selected].hasHeatsink){
                ImGui::EndDisabled();
            }

        }  

        ImGui::PopFont(); 
    }

    if(reload_components) {
        pcb.reload_components();
    }

    ImGui::SeparatorText("Simulation");

    if(ImGui::CollapsingHeader("Simulation", ImGuiTreeNodeFlags_DefaultOpen)){
        ImGui::PushFont(jetbrainsmono_regular, 0.0f);
        ImGui::Checkbox("Autorange", &pcb.autorange);
        ImGui::SameLine(); HelpMarker("Activates the autorange feature for the colormap.");
        if(!pcb.autorange){
            ImGui::DragFloat("Cold Temperature", &pcb.range_min, 0.2f, -10.0f, pcb.range_max-1);
            ImGui::SameLine(); HelpMarker("Minimum temperature of the heatmap in ºC.");
            ImGui::DragFloat("Hot Temperature", &pcb.range_max, 0.2f, pcb.range_min+1, 200.0f);
            ImGui::SameLine(); HelpMarker("Maximum temperature of the heatmap in ºC.");
        }

        ImGui::ColorEdit3("Color 1", (float*)&pcb.color_1, ImGuiColorEditFlags_NoInputs);
        ImGui::SameLine(); HelpMarker("Color at 0\% of the temperature range.");
        ImGui::ColorEdit3("Color 2", (float*)&pcb.color_2, ImGuiColorEditFlags_NoInputs);
        ImGui::SameLine(); HelpMarker("Color at 33\% of the temperature range.");
        ImGui::ColorEdit3("Color 3", (float*)&pcb.color_3, ImGuiColorEditFlags_NoInputs);
        ImGui::SameLine(); HelpMarker("Color at 66\% of the temperature range.");
        ImGui::ColorEdit3("Color 4", (float*)&pcb.color_4, ImGuiColorEditFlags_NoInputs);
        ImGui::SameLine(); HelpMarker("Color at 100\% of the temperature range.");

        ImGui::PopFont();
    }

    ImGui::SeparatorText("Statistics");

    ImGui::Text("Tmin:");
    ImGui::SameLine();
    ImGui::Text(" %.1f ºC | ", pcb.T_min);
    ImGui::SameLine();
    ImGui::Text("Tavg:");
    ImGui::SameLine();
    ImGui::Text(" %.1f ºC | ", pcb.T_avg);
    ImGui::SameLine();
    ImGui::Text("Tmax:");
    ImGui::SameLine();
    ImGui::Text(" %.1f ºC", pcb.T_max);
    ImGui::SameLine(); HelpMarker("Minimum, maximum and average temperatures of the current heatmap.");

    ImGui::PopFont();

    ImGui::PopItemWidth();

    ImGui::End();
}

void App::Viewport(PCB& pcb){

    ImGui::Begin("PCB Thermal Simulation", nullptr, ImGuiWindowFlags_MenuBar);    

    if(ImGui::BeginMenuBar()){

        if (ImGui::BeginMenu("File"))
        {
            if(ImGui::MenuItem("Load", "Ctrl+S")){
                pcb.load_image();
            }
            ImGui::EndMenu();
        }


        ImGui::EndMenuBar();

    }

    if(ImGui::BeginTabBar("Tab bar")){

        ImGui::PushFont(jetbrainsmono_bold, 0.0f);
        if (ImGui::BeginTabItem("PCB", NULL)){

            DrawTexture(pcb, pcb.PCB_temp);
            
            ImGui::EndTabItem();

        }

        if(ImGui::BeginTabItem("Junction")){

            DrawTexture(pcb, pcb.Junction_temp);

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Heatsink", NULL)){

            DrawTexture(pcb, pcb.Heatsink_temp);
            
            ImGui::EndTabItem();

        }



        ImGui::PopFont();


        ImGui::EndTabBar();
    }


    

    


    ImGui::End();
}

void App::RenderUI(){

#ifdef DEBUG_MODE
        //Show Demo Window
        ImGui::ShowDemoWindow();
#endif



    ImGuiID dockspace_id = ImGui::GetID("My Dockspace");
    ImGuiViewport* viewport = ImGui::GetMainViewport();

    if (ImGui::DockBuilderGetNode(dockspace_id) == nullptr)
    {
        ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);
        ImGuiID dock_id_left = 0;
        ImGuiID dock_id_main = dockspace_id;
        ImGui::DockBuilderSplitNode(dock_id_main, ImGuiDir_Left, 0.40f, &dock_id_left, &dock_id_main);
        ImGui::DockBuilderDockWindow("PCB Thermal Simulation", dock_id_main);
        ImGui::DockBuilderDockWindow("Simulation Settings", dock_id_left);
        ImGui::DockBuilderFinish(dockspace_id);
    }

    ImGui::DockSpaceOverViewport(dockspace_id, viewport, ImGuiDockNodeFlags_PassthruCentralNode);

    pcb1.calculate();

    Viewport(pcb1);
    Parameters(pcb1);
}

void App::SetupStyle(float main_scale){
    // Configure App Style


    //io = ImGui::GetIO();

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable window docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-viewports feature
    io.ConfigDpiScaleFonts=true; 

    //First loaded font is default font
    jetbrainsmono_regular = io.Fonts->AddFontFromFileTTF("fonts/JetBrainsMono-Regular.ttf");
    jetbrainsmono_bold = io.Fonts->AddFontFromFileTTF("fonts/JetBrainsMono-Bold.ttf");

    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

        // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale;        // Set initial font scale. (in docking branch: using io.ConfigDpiScaleFonts=true automatically overrides this for every window depending on the current monitor)

    style.FrameRounding = 5.0f;
    style.WindowPadding = ImVec2(10.0f,8.0f);
    style.FramePadding = ImVec2(4.0f, 1.0f);
    style.ItemSpacing = ImVec2(6.0f, 4.0f);
    style.WindowRounding = 8.0f;
    style.FrameRounding = 4.0f;
    style.GrabRounding = 12.0f;
    style.SeparatorTextAlign = ImVec2(0.5f, 0.5f);
    style.ImageRounding = 10.0f;
    style.DockingSeparatorSize = 3.0f;
    style.TabMinWidthBase = 100.0f;

}


void App::DrawTexture(PCB& pcb, std::vector<float>& grid){
    ImVec2 avail = ImGui::GetContentRegionAvail();
    float scale = std::min(avail.x / pcb.width, avail.y / pcb.height); //Scale in px/mm
    scale = std::max(scale, 1.f);    
    ImVec2 size(pcb.width*scale, pcb.height*scale);

    ImVec2 p0 = ImGui::GetCursorScreenPos();

    pcb.update(grid);

    ImGui::Image((ImTextureID)(intptr_t)pcb.pcb_image, size);
    ImGui::SetCursorScreenPos(p0);
    ImGui::Image((ImTextureID)(intptr_t)pcb.texture, size);
    bool hovered = ImGui::IsItemHovered();
    ImDrawList* dl = ImGui::GetWindowDrawList();

    //Temperature probe
    if(hovered){
        ImVec2 mouse = ImGui::GetMousePos();
        ImVec2 pcb_pos((mouse.x-p0.x)/scale, (mouse.y-p0.y)/scale);
        int ci = std::clamp((int)(pcb_pos.x / pcb.cell_size), 0, pcb.nx - 1);
        int cj = std::clamp((int)(pcb_pos.y / pcb.cell_size), 0, pcb.ny - 1);
        ImGui::SetTooltip("(%.1f, %.1f) mm\nT = %.1f ºC", pcb_pos.x, pcb_pos.y, grid[cj*pcb.nx+ci]);

        //Component dragging
        if(pcb.component_selected >= 0 && ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.0f)){
            ImVec2 delta = ImGui::GetIO().MouseDelta;
            pcb.components[pcb.component_selected].x_pos += delta.x / scale;
            pcb.components[pcb.component_selected].y_pos += delta.y / scale;
            pcb.reload_components();
        }

    }   

    //Component drawing
    for(int i = 0; i < (int)pcb.components.size(); i++){
        ImVec2 p_min(p0.x+pcb.components[i].x_pos*scale, p0.y+pcb.components[i].y_pos*scale); //Upper Left corner
        ImVec2 p_max(p_min.x+pcb.components[i].width*scale, p_min.y+pcb.components[i].height*scale); //Lower right corner
        dl->AddRect(p_min, p_max, i==pcb.component_selected? IM_COL32(255, 255, 255, 255):IM_COL32(0, 0, 0, 255), 5.0f, 0, 2.0f);
        dl->AddText(ImVec2(p_min.x + 2, p_min.y + 2), IM_COL32(0, 0, 0, 255), pcb.components[i].name.c_str());
    }
}


void App::HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}