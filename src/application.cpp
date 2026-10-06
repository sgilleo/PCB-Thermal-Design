#include <algorithm>
#include "component.h"
#include "imgui.h"
#include "imgui_stdlib.h"
#include "GLFW/glfw3.h"
#include "application.h"
#include "pcb.h"
#include "component.h"


void App::Parameters(PCB& pcb)
{
    ImGui::Begin("Simulation Settings");

    bool resize_pcb = false;
    bool reload_components = false;

    if(ImGui::CollapsingHeader("PCB Parameters", ImGuiTreeNodeFlags_DefaultOpen)){
        resize_pcb |= ImGui::InputFloat("Width (mm)", &pcb.width, 1, 5, "%.2f");
        resize_pcb |= ImGui::InputFloat("Height (mm)", &pcb.height, 1, 5, "%.2f");
        resize_pcb |= ImGui::InputFloat("Cell Size (mm)", &pcb.cell_size, 1.0f, 5.0f, "%.1f");
        ImGui::SliderFloat("Layer thicknes (um)", &pcb.layer_thickness, 18.0f, 70.0f, "%.1f");
        ImGui::SliderInt("Number of copper planes", &pcb.layers, 2.0f, 32.0f, "%d");
    }

    if(resize_pcb){
        pcb.resize();
    }

    if(ImGui::CollapsingHeader("Ambient Parameters", ImGuiTreeNodeFlags_DefaultOpen)){
        ImGui::InputFloat("Ambient Temperature (ºC)", &pcb.t_amb);
        ImGui::InputFloat("Convection Coefficient (W/m²K)", &pcb.h_conv);
    }

    if(ImGui::CollapsingHeader("Components", ImGuiTreeNodeFlags_DefaultOpen)){
        if(ImGui::Button("Add Component")){
            pcb.components.push_back(Component("U" + std::to_string(pcb.components.size() + 1), pcb.width*0.5, 
            pcb.height*0.5, 8, 8, 0.5));
            pcb.component_selected = (int)pcb.components.size() - 1;
            reload_components = true;

        }

        ImGui::SameLine();
        if(ImGui::Button("Delete Component")){
            pcb.components.erase(pcb.components.begin()+pcb.component_selected);
            pcb.component_selected = -1;
            reload_components = true;
        }

        for(int i = 0; i<(int)pcb.components.size(); i++){
            if(ImGui::Selectable((pcb.components[i].name+"##").c_str(), pcb.component_selected == i)) pcb.component_selected = i;
        }

        if(pcb.component_selected >= 0 && pcb.component_selected < (int)pcb.components.size()){
            reload_components |= ImGui::InputText("Name", &pcb.components[pcb.component_selected].name, 0, nullptr, nullptr);
            reload_components |= ImGui::InputFloat("X (mm)", &pcb.components[pcb.component_selected].x_pos, 1.0f, 2.0f, "%.1f");
            reload_components |= ImGui::InputFloat("Y (mm)", &pcb.components[pcb.component_selected].y_pos, 1.0f, 2.0f, "%.1f");
            reload_components |= ImGui::SliderFloat("Component width (mm)", &pcb.components[pcb.component_selected].width, 0.1f, 50.0f, "%.1f");
            reload_components |= ImGui::SliderFloat("Component height (mm)", &pcb.components[pcb.component_selected].height, 0.1f, 50.0f, "%.1f");
            reload_components |= ImGui::SliderFloat("Dissipated fpwer (W)", &pcb.components[pcb.component_selected].power, 0.1f, 20.0f, "%.1f");
        }   
    }

    if(reload_components) {
        pcb.reload_components();
    }



    if(ImGui::CollapsingHeader("Simulation", ImGuiTreeNodeFlags_DefaultOpen)){
        ImGui::SliderFloat("Simulation Speed", &pcb.simulation_speed, 0.1f, 5.0f, "%.1f");

        ImGui::Checkbox("Autorange", &pcb.autorange);
        if(!pcb.autorange){
            ImGui::DragFloat("Cold Temperature", &pcb.range_min, 0.2f, -10.0f, pcb.range_max-1);
            ImGui::DragFloat("Hot Temperature", &pcb.range_max, 0.2f, pcb.range_min+1, 200.0f);
        }

        ImGui::ColorEdit3("Cold Color", (float*)&pcb.cold_color);
        ImGui::ColorEdit3("Medium Color", (float*)&pcb.medium_color);
        ImGui::ColorEdit3("Hot Color", (float*)&pcb.hot_color);

        ImGui::Text("Tmin: %.1f ºC | Tmax: %.1f ºC", pcb.T_min, pcb.T_max);

        


    }

    ImGui::End();
}

void App::Viewport(PCB& pcb){

    ImGui::Begin("PCB Thermal Simulation");    

    ImVec2 avail = ImGui::GetContentRegionAvail();
    float scale = std::min(avail.x / pcb.width, avail.y / pcb.height); //Scale in px/mm
    scale = std::max(scale, 1.f);    
    ImVec2 size(pcb.width*scale, pcb.height*scale);

    ImVec2 p0 = ImGui::GetCursorScreenPos();
    ImGui::Image((ImTextureID)(intptr_t)pcb.texture, size);
    bool hovered = ImGui::IsItemHovered();
    ImDrawList* dl = ImGui::GetWindowDrawList();


    if(hovered){
        ImVec2 mouse = ImGui::GetMousePos();
        ImVec2 pcb_pos((mouse.x-p0.x)/scale, (mouse.y-p0.y)/scale);
        int ci = std::clamp((int)(pcb_pos.x / pcb.cell_size), 0, pcb.nx - 1);
        int cj = std::clamp((int)(pcb_pos.y / pcb.cell_size), 0, pcb.ny - 1);
        ImGui::SetTooltip("(%.1f, %.1f) mm\nT = %.1f ºC", pcb_pos.x, pcb_pos.y, pcb.temp_grid[cj*pcb.nx+ci]);
    }   

    //Component drawing
    for(int i = 0; i < (int)pcb.components.size(); i++){
        ImVec2 p_min(p0.x+pcb.components[i].x_pos*scale, p0.y+pcb.components[i].y_pos*scale); //Upper Left corner
        ImVec2 p_max(p_min.x+pcb.components[i].width*scale, p_min.y+pcb.components[i].height*scale); //Lower right corner
        dl->AddRect(p_min, p_max, i==pcb.component_selected? IM_COL32(255, 255, 255, 255):IM_COL32(0, 0, 0, 255), 5.0f, 0, 2.0f);
        dl->AddText(ImVec2(p_min.x + 2, p_min.y + 2), IM_COL32(0, 0, 0, 255), pcb.components[i].name.c_str());
    }


    ImGui::End();
}

void App::RenderUI(PCB& pcb){
    pcb.update();
    Viewport(pcb);
    Parameters(pcb);
}


