#pragma once
#include <string>

struct Component{

    std::string name;
    float width;
    float height;
    float x_pos;
    float y_pos;
    float power = 0.5; //Power dissipated by the component
    float Rjb = 0.1f; //Thermal resistance from junction to board (K/W)
    float Rjc = 5.0f; //Thermal resistance from junction to case (K/W)

    float duty_cycle = 1.0f; //On off duty cycle
    float freq_on_off = 1.0f; //Working frequency
};