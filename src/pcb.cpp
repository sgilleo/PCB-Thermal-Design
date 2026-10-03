#include <algorithm>
#include "GLFW/glfw3.h"
#include "pcb.h"
#include "imgui.h"

PCB::PCB(): PCB(100.0f, 50.0f, 3.0f) { //Set defaults
    
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

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //The texture minifying filter
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); //The texture magnifying filter
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, nx, ny, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
}

void PCB::update(){
    calculate();
    for (int i = 0; i < nx * ny; ++i){
        color_map(temp_grid[i], &pixels[i*3]);
    }
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, nx, ny, GL_RGB,
         GL_UNSIGNED_BYTE, pixels.data());

}

void PCB::calculate(){
    Rcond = 1/(Cu_K*layers*layer_thickness);
    Rconv = 1/(h_conv*cell_size*cell_size);
    dt = ImGui::GetIO().DeltaTime;
    Cth = Cp_FR4*rho_FR4*cell_size*cell_size*pcb_thickness*1e3;
    for(int n = 0; n < max_iterations; n++){
        for(int j = 0; j < ny; j++){
            for(int i = 0; i < nx; i++){
                int index = j * nx + i;
                float Tc = temp_grid[index]; //Temperature of the current cell

                float Tu = j > 0? temp_grid[index-nx]: Tc; //Temperature of the upper cell
                float Tr = i < nx-1? temp_grid[index+1]: Tc; //Temperature of the right cell
                float Td = j < ny-1? temp_grid[index+nx]: Tc; //Temperature of the lower cell 
                float Tl = i > 0? temp_grid[index-1]: Tc; //Temperature of the left cell

                float P = (Tu+Tr+Td+Tl-4*Tc)/(Rcond)+(t_amb-Tc)/Rconv;
                temp_grid[index] += P/Cth*dt*simulation_speed;
            }
        }
    }
    
}

void PCB::color_map(float temp, unsigned char* color){
    temp = std::clamp(temp, 0.f, 1.f);
    float r, g, b;
    if      (temp < 0.25f) { float u = temp / 0.25f;         r = 0;     g = u;     b = 1; }
    else if (temp < 0.50f) { float u = (temp - 0.25f)/0.25f; r = 0;     g = 1;     b = 1 - u; }
    else if (temp < 0.75f) { float u = (temp - 0.50f)/0.25f; r = u;     g = 1;     b = 0; }
    else                { float u = (temp - 0.75f)/0.25f; r = 1;     g = 1 - u; b = 0; }
    color[0] = (unsigned char)(r * 255); //R Channel
    color[1] = (unsigned char)(g * 255); //G Channel
    color[2] = (unsigned char)(b * 255); //B Channel

}