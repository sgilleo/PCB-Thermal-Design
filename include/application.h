#pragma once

#include "pcb.h"

class App{
    public:
    //App();
    //~App();

    PCB pcb1;

    void Parameters(PCB& pcb);
    void Viewport(PCB& pcb);
    void RenderUI();

    private:

};
