#include "component.h"

Component::Component(std::string name, float x_pos, float y_pos, float width, float height, float power){
    this->name = name;
    this->width = width;
    this->height = height;
    this->x_pos = x_pos;
    this->y_pos = y_pos;
    this->power = power;
}