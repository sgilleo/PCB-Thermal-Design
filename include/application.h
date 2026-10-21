#pragma once

#include "pcb.h"

class App{
    public:
    //~App();

    PCB pcb1;

    ImFont* jetbrainsmono_bold;
    ImFont* jetbrainsmono_regular;

    ImGuiIO& io = ImGui::GetIO();

    void RenderUI();


    void SetupStyle(float main_scale);

    private:
        void Parameters(PCB& pcb);
        void Viewport(PCB& pcb);
        void DrawTexture(PCB& pcb, std::vector<float>& grid);
};
