#pragma once
#include "stackup.h"

// Visual stackup editor for Dear ImGui.
// Draws the layer stack (click a layer to select it) and a property panel for the selected layer.
// Call Draw() inside an ImGui window; it returns true when the stackup changed
// (then call ThermalSolver::configure() again).
class StackupEditor {
public:
    bool Draw(Stackup& stackup);

private:
    enum class Kind { TopPlate, Copper, Dielectric, BottomPlate };

    bool DrawStack(Stackup& s);
    bool DrawProperties(Stackup& s);
    void FixSelection(const Stackup& s);

    Kind sel_kind_  = Kind::Copper;
    int  sel_index_ = 0;
};
