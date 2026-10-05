#pragma once
#include <string>

class Component{

    public:
    std::string name;
    float width;
    float height;
    float x_pos;
    float y_pos;
    float power = 0.5;


    Component(std::string name, float x_pos, float y_pos, float width, float height, float power);
    private:

   //Add duty cycle option

};