#pragma once
#include <cstddef>
#include <string>

enum Heatsink_Material{
    Copper,
    Aluminum,
    Ceramic
};

struct Component{

    std::string name;
    float width;
    float height;
    float x_pos;
    float y_pos;
    float power = 1; //Power dissipated by the component
    float Rjb = 1.0f; //Thermal resistance from junction to board (K/W)
    float Rjc = 5.0f; //Thermal resistance from junction to case (K/W)

    bool hasHeatsink = false; //Heatsink on top of component

    Heatsink_Material material = Aluminum;

    float heatsink_thickness = 1; //Heatsink thickness in (mm)

    float Rca = 5; //Thermal resistance from case to air (K/W)

    //float duty_cycle = 1.0f; //On off duty cycle
    //float freq_on_off = 1.0f; //Working frequency

};