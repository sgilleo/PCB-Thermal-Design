#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>
#include "GLFW/glfw3.h"
#include "pcb.h"
#include "component.h"
#include "imgui.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

PCB::PCB(): PCB(100.0f, 50.0f, 4.0f) { //Set defaults
    components.push_back(Component{"CPU", width/2-10, height/2-10, 20, 20, 0.5});
    components.push_back(Component{"LDO", width/2-40, height/2-10, 10, 5, 0.1});
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
    PCB_temp.assign(nx * ny, t_amb);

    reload_components();

    Gcond = Cu_K*layers*layer_thickness*1e-6;
    G_si = Si_K*0.3e-3; //300um silicon thickness


    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //The texture minifying filter
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); //The texture magnifying filter
    //Avoid border interpolation
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, nx, ny, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
}

void PCB::update(std::vector<float>& Temp_grid){

    T_min = *std::min_element(Temp_grid.begin(), Temp_grid.end());
    T_max = *std::max_element(Temp_grid.begin(), Temp_grid.end());
    T_avg = (float)(std::accumulate(Temp_grid.begin(), Temp_grid.end(), 0.0) / Temp_grid.size());

    float lo = autorange ? T_min : range_min;
    float hi = autorange ? std::max(T_max, T_min+1): range_max;

    for (int i = 0; i < nx * ny; ++i){
        color_map(Temp_grid[i], lo, hi, &pixels[i*3]);
    }
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, nx, ny, GL_RGB,
         GL_UNSIGNED_BYTE, pixels.data());

}

void PCB::calculate(){
    
    //dt = ImGui::GetIO().DeltaTime;
    //Cth = Cp_FR4*rho_FR4*cell_size*cell_size*pcb_thickness*1e-3;
    Gconv = ((convection_2_faces)? 2:1) * h_conv*cell_size*cell_size*1e-6;

    for(int n = 0; n < max_iterations; n++){

        //Loop for PCB temperatures
        for(int j = 0; j < ny; j++){
            for(int i = 0; i < nx; i++){
                int index = j * nx + i;

                float T_sum = 0.0f; //Sum of neighbouring cell temperatures
                float n_cells = 0;

                if(j>0){
                    T_sum += PCB_temp[index-nx]; //Temperature of the upper cell
                    n_cells++;
                }
                if(i < nx-1){
                    T_sum += PCB_temp[index+1]; //Temperature of the right cell
                    n_cells++;

                }
                if(j < ny-1){
                    T_sum += PCB_temp[index+nx]; //Temperature of the lower cell 
                    n_cells++;

                }
                if(i > 0){
                    T_sum += PCB_temp[index-1]; //Temperature of the left cell
                    n_cells++;
                }

                //Estado transitorio
                //float P = (Tu+Tr+Td+Tl-4*Tc)/(Rcond)+(t_amb-Tc)/Rconv + Component_grid[index];
                //PCB_temp[index] += P/Cth*dt*simulation_speed;

                //Estado estacionario
                float Gconv_mask = (Component_grid[index]>0) ? 0 : Gconv; //Convection conductance only in cells exposed to air

                PCB_temp[index] = (T_sum*Gcond+t_amb*Gconv_mask+Junction_temp[index]*Gjb_grid[index])/(n_cells*Gcond+Gconv_mask+Gjb_grid[index]);
            }
        }

        //Look for junction temperatures
        for(const Component& component : components){
            int i0 = std::max(0,  (int)std::floor(component.x_pos / cell_size));
            int i1 = std::min(nx, (int)std::ceil((component.x_pos + component.width) / cell_size));
            int j0 = std::max(0,  (int)std::floor(component.y_pos / cell_size));
            int j1 = std::min(ny, (int)std::ceil((component.y_pos + component.height) / cell_size));

            for(int j = j0; j < j1; j++){
                for(int i = i0; i < i1; i++){
                    int index = j * nx + i;

                    float T_sum = 0.0f; //Sum of neighbouring cell temperatures
                    float n_cells = 0;

                    if(j>j0){
                        T_sum += Junction_temp[index-nx]; //Temperature of the upper cell
                        n_cells++;
                    }
                    if(i < i1-1){
                        T_sum += Junction_temp[index+1]; //Temperature of the right cell
                        n_cells++;

                    }
                    if(j < j1-1){
                        T_sum += Junction_temp[index+nx]; //Temperature of the lower cell 
                        n_cells++;

                    }
                    if(i > i0){
                        T_sum += Junction_temp[index-1]; //Temperature of the left cell
                        n_cells++;
                    }
                    Junction_temp[index] = (T_sum*G_si+PCB_temp[index]*Gjb_grid[index]+Component_grid[index])/(n_cells*G_si+Gjb_grid[index]);

                }
            }
        }
    }
}

void PCB::reload_components(){
    Component_grid.assign(nx * ny, 0);
    Gjb_grid.assign(nx * ny, 0);
    Junction_temp.assign(nx * ny, t_amb);
    for(const Component& component : components){
        int i0 = std::max(0,  (int)std::floor(component.x_pos / cell_size));
        int i1 = std::min(nx, (int)std::ceil((component.x_pos + component.width) / cell_size));
        int j0 = std::max(0,  (int)std::floor(component.y_pos / cell_size));
        int j1 = std::min(ny, (int)std::ceil((component.y_pos + component.height) / cell_size));
        int n = std::max(0, i1 - i0) * std::max(0, j1 - j0);   // celdas que de verdad ocupa
        if(n==0) continue;

        float cell_gjb = 1/(component.Rjb*n);
        float cell_q = component.power/(n);

        for(int j = j0; j < j1; j++){
            for(int i = i0; i < i1; i++){
                Component_grid[j*nx+i] += cell_q;
                Gjb_grid[j*nx+i] += cell_gjb;
            }
        }
    }

    
}

void PCB::color_map(float temp, float t_min, float t_max, unsigned char* color){
    temp = (temp-t_min)/(t_max-t_min); //Map temperature to 0-1 range
    float r = 0;
    float g = 0;
    float b = 0;

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

void PCB::load_image(){

    int width, height, nrChannels;
    
    unsigned char* image_data = stbi_load("pcb.png", &width, &height, &nrChannels, 3);
    glGenTextures(1, &pcb_image);
    glBindTexture(GL_TEXTURE_2D, pcb_image);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image_data);

    stbi_image_free(image_data);

}
