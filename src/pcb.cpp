#include <algorithm>
#include <cmath>
#include "GLFW/glfw3.h"
#include "pcb.h"
#include "component.h"
#include "imgui.h"

PCB::PCB(): PCB(100.0f, 50.0f, 4.0f) { //Set defaults
    components.push_back(Component("CPU", width/2-10, height/2-10, 20, 20, 0.5));
    components.push_back(Component("LDO", width/2-40, height/2-10, 10, 5, 0.1));
    resize();

}


PCB::PCB(float width, float height, float cell_size){

    this->width = width;
    this->height = height;
    this->cell_size = cell_size;

    glGenTextures(1, &texture);
    resize();
}


void PCB::resize(){

    nx = (int) width / cell_size;
    ny = (int) height / cell_size;

    pixels.assign(nx * ny * 3, 255);
    temp_grid.assign(nx * ny, 0);

    reload_components();

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //The texture minifying filter
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); //The texture magnifying filter
    //Avoid border interpolation
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, nx, ny, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
}

void PCB::update(){
    calculate();
    T_min = *std::min_element(temp_grid.begin(), temp_grid.end());
    T_max = *std::max_element(temp_grid.begin(), temp_grid.end());

    float lo = autorange ? T_min : range_min;
    float hi = autorange ? std::max(T_max, T_min+1): range_max;

    for (int i = 0; i < nx * ny; ++i){
        color_map(temp_grid[i], lo, hi, &pixels[i*3]);
    }
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, nx, ny, GL_RGB,
         GL_UNSIGNED_BYTE, pixels.data());

}

void PCB::calculate(){
    Rcond = 1/(Cu_K*layers*layer_thickness*1e-6);
    Rconv = 1/(h_conv*cell_size*cell_size*1e-6);
    dt = ImGui::GetIO().DeltaTime;
    Cth = Cp_FR4*rho_FR4*cell_size*cell_size*pcb_thickness*1e-3;
    for(int n = 0; n < max_iterations; n++){
        for(int j = 0; j < ny; j++){
            for(int i = 0; i < nx; i++){
                int index = j * nx + i;
                float Tc = temp_grid[index]; //Temperature of the current cell

                float Tu = j > 0? temp_grid[index-nx]: Tc; //Temperature of the upper cell
                float Tr = i < nx-1? temp_grid[index+1]: Tc; //Temperature of the right cell
                float Td = j < ny-1? temp_grid[index+nx]: Tc; //Temperature of the lower cell 
                float Tl = i > 0? temp_grid[index-1]: Tc; //Temperature of the left cell

                float P = (Tu+Tr+Td+Tl-4*Tc)/(Rcond)+(t_amb-Tc)/Rconv + heat_grid[index];
                temp_grid[index] += P/Cth*dt*simulation_speed;
            }
        }
    }
    
}

void PCB::reload_components(){
    heat_grid.assign(nx * ny, 0);
    for(const Component& component : components){
        int i0 = std::max(0,  (int)std::floor(component.x_pos / cell_size));
        int i1 = std::min(nx, (int)std::ceil((component.x_pos + component.width) / cell_size));
        int j0 = std::max(0,  (int)std::floor(component.y_pos / cell_size));
        int j1 = std::min(ny, (int)std::ceil((component.y_pos + component.height) / cell_size));
        int n = std::max(0, i1 - i0) * std::max(0, j1 - j0); //Number of cells per component
        if (n == 0) continue; //If component is out of the pcb, skip

        float cell_q = component.power/(n);

        for(int j = j0; j < j1; j++){
            for(int i = i0; i < i1; i++){
                heat_grid[j*nx+i] += cell_q;
            }
        }
    }

    
}

void PCB::color_map(float temp, float t_min, float t_max, unsigned char* color){
    temp = (temp-t_min)/(t_max-t_min); //Map temperature to 0-1 range
    float r, g, b;

    if(temp < 0.33f){
        float u = temp / 0.33f;
        u = std::clamp(u, 0.0f, 1.0f);
        r = color_1.x * (1-u) + u * color_2.x;
        g = color_1.y * (1-u) + u * color_2.y;
        b = color_1.z * (1-u) + u * color_2.z;
    }


    if(temp >= 0.33f && temp < 0.66f){
        float u = (temp-0.33f)/0.33f;
        u = std::clamp(u, 0.0f, 1.0f);
        r = color_2.x * (1-u) + u * color_3.x;
        g = color_2.y * (1-u) + u * color_3.y;
        b = color_2.z * (1-u) + u * color_3.z;
    }


    if(temp >= 0.66f){
        float u = (temp-0.66f)/0.33f;
        u = std::clamp(u, 0.0f, 1.0f);
        r = color_3.x * (1-u) + u * color_4.x;
        g = color_3.y * (1-u) + u * color_4.y;
        b = color_3.z * (1-u) + u * color_4.z;
    }   

    color[0] = (unsigned char)(r * 255); //R Channel
    color[1] = (unsigned char)(g * 255); //G Channel
    color[2] = (unsigned char)(b * 255); //B Channel

}