# PCB Thermal Simulation

A lightweight interactive 2D thermal simulator for printed circuit boards (PCBs) built with **C++**, **OpenGL 3.3**, and **Dear ImGui**.

This tool visualizes heat distribution and dissipation across a PCB in real time by numerically solving the **2D heat equation (Finite Difference Method / FDM)**, rendering a dynamic heatmap directly over the board geometry.

<!--![License](https://img.shields.io/badge/License-MIT-blue.svg)-->
![C++](https://img.shields.io/badge/Language-C%2B%2B17-00599C.svg)
![OpenGL](https://img.shields.io/badge/Graphics-OpenGL%203.3-5586A4.svg)
![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20macOS%20%7C%20Linux-lightgrey.svg)

---

## Key Features

* **Real-Time Simulation:** Iterative thermal conduction calculations using a 2D numerical grid.
* **Dynamic Heatmap:** Maps scalar temperature values to color palettes rendered directly on the GPU via OpenGL textures.
* **Intuitive User Interface:** Interactive controls using Dear ImGui to modify PCB dimensions, grid resolution, and ambient thermal conditions.
* **Spot Inspection:** Query exact temperature values at any point on the board by hovering with the mouse (*hover tooltip*).
* **Self-Contained & Cross-Platform:** Configured with CMake to automatically download and build dependencies on Linux, Windows, and macOS.

---

## Build and Run Instructions

Follow these simple steps for your operating system. No need to manually install GLFW or ImGui, as the project handles its own dependencies.

### Prerequisites
Before getting started, ensure you have installed:

1. A C++ compiler:
    - **Linux:** `g++` or `clang`
    - **Windows:** Visual Studio or MinGW
    - **macOS:** Xcode Command Line Tools   
2. CMake (Version 3.12 or higher)
3. Git

### Building the project

Clone the repository in a folder of your choice and run the following:

```
cmake -B build && cmake --build build
./build/PCB_Thermal_Design
```
---

## Usage

The application consists on two separate panels that work together: the **settings panel** and the **visualizing panel**.



---

## Project Structure

The project follows a clean structure separating the UI, physics engine, and third-party libraries:
<!-- 
```text
PCB-Thermal-Design/
├── CMakeLists.txt          # CMake build system configuration
├── README.md               # Project documentation
├── include/                # Header files (.h)
│   ├── application.h       # Window management and ImGui interface
│   └── pcb.h               # PCB logic, thermal physics, and OpenGL texture
├── src/                    # Source files (.cpp)
│   ├── main.cpp            # Application entry point and main loop
│   ├── application.cpp     # UI implementation (Viewport and Panels)
│   └── pcb.cpp             # Thermal physics and color map implementation
└── external/               # Third-party libraries (Dear ImGui, GLFW, etc.)

``` -->
---
## Acknowledgments

The idea of this project came with a cool subject of the Electronic Systems Master's Degree of the Politechnic University of Valencia: *Thermal Design and EMC in Electronic Products*