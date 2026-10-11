#pragma once

#include "pcb.h"

class App{
    public:
    //App();
    //~App();

    PCB pcb1;

    void RenderUI();


    void SetupStyle(ImGuiStyle& style);

    private:
        void Parameters(PCB& pcb);
        void Viewport(PCB& pcb);
};
