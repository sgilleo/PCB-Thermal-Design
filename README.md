# PCB Thermal Simulation

![](demo.gif)

An **Open Source** lightweight interactive 2D thermal simulator for printed circuit boards (PCBs) built with **C++**, **OpenGL 3.3**, and **Dear ImGui**.

This tool visualizes heat distribution and dissipation across a PCB in real time rendering a dynamic heatmap directly over the board geometry.

<!--![License](https://img.shields.io/badge/License-MIT-blue.svg)-->
![C++](https://img.shields.io/badge/Language-C%2B%2B17-00599C.svg)
![OpenGL](https://img.shields.io/badge/Graphics-OpenGL%203.3-5586A4.svg)
![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20macOS%20%7C%20Linux-lightgrey.svg)

---

## Table of contents
* [Features](#features)
* [Build and Run Instructions](#build-and-run-instructions)
* [Usage Manual](#usage)
  * [Settings Panel](#the-settings-panel)
  * [Visualizing Panel](#the-visualizing-panel)
* [Project Structure](#project-structure)
* [Disclaimer and Acknowledgments](#disclaimer-and-acknowledgments)

---

## Features

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

Clone the repository in a folder of your choice and use CMake to build the source files.

There are two CMake presets already preconfigured in the `CMakePresets.json` file:
- **Release:** Compiles only the necesary files and enables compiler optimization. Recommended for most people.
```
cmake --preset "release" && cmake --build build/release
./build/release/PCB_Thermal_Design
```

- **Debug:** Includes ImGui's default demo window and enables compiler warning. Recommended for those who want to modify the source code or implement new features.
```
cmake --preset "debug" && cmake --build build/debug
./build/debug/PCB_Thermal_Design
```

You can also use **CMake GUI** to configure and generate the files to build the project.

---

## Usage

The application consists on two separate panels that work together: the **settings panel** and the **visualizing panel**.

### The settings panel:
This is where you configure all the parameters for the simulation.
#### PCB Parameters
This tab holds all the phisical parameters related to the structure of the circuit board.
- **Width:** width of the PCB in mm
- **Height:** height of the PCB in mm
- **Cell Size:** size of the discrete cells used to compute the temperatures in mm
- **Layer thickness:** thickness of the copper layers in um. Used to determine the section area for lateral diffusion. The most common value is 35 um.
- **Number of copper planes:** number of layers that are aproximately continuous (i.e. power planes, GND planes, ...).

#### Ambient Parameters
- **Ambient Temperature:** Temperature of the surrounding air in ºC.
- **Convection Coefficient:** Convection coefficient in W/m²K. Typical values are around 10 W/m²K for natural convection (taking into account heat radiation).

#### Components
This section provides a menu for adding and editing components and potential heat sources.

- **Name:** Component name.
- **X:** Component X coordinate in mm.
- **Y:** Component Y coordinate in mm.
- **Component width:** component width in mm.
- **Component height:** component height in mm.
- **Disspiated power:** total power turned into heat in W.


#### Simulation
In this tab you can configure all the simulation related parameters
- **Simulation Speed:** simulation simestep multiplier. Setting this parameter too high may lead to loss of stability.
- **Autorange:** automatic color adjustment based on the maximum and minimum temperature on the pcb. Disabling this option lets the user manually select the lower and higher temperature on the PCB.

- **Color selection**: Color picker for equally spaced ranges of temperature between minimum and maximum.

- **Tmin** and **Tmax:** shows the minimum and maximum temperature in ºC in real time.


### The visualizing panel

This panel displays a real time representation of the PCB temperatures overlayed with all the heat sources. The color map and temperature ranges can be configured in the settings panel.

Hovering the mouse around the PCB shows a marker with the temperature of the specific cell along with its coordinates in relation to the origin (top left corner).

If there is a component selected in the settings panel, it can be dragged around the board to place in its desired position.

---

## Project Structure

This project was built in a Linux machine from the ground up without the use of any propietary IDE. This means that it is compatible and ready to modify with the code editor of your choice (Vim, VSCode, Atom, Sublime Text, Emacs, Notepad, ...). 

The build process generates a `compile_commands.json` file in the project root folder used for code autocompletion. 

The folders are structured to be organized and easy to understand in the following way: 

```text
PCB-Thermal-Design/
├── CMakeLists.txt          # CMake build system configuration
├── CMakePresets.json       # CMake presets for configuration
├── README.md               # Project documentation
├── fonts/                  # Fonts used for the project
├── include/                # Header files (.h)
│   ├── application.h       # Window management and ImGui interface
│   └── pcb.h               # PCB logic and thermal physics
├── src/                    # Source files (.cpp)
│   ├── main.cpp            # Application entry point and main loop
│   ├── application.cpp     # UI implementation (Viewport and Panels)
│   └── pcb.cpp             # Thermal physics and color map implementation
├── imgui/                  # Imgui and OpenGL backend source files
└── build/                  # Directory for project building 
    ├── debug/              # Debug configuration
    └── release/            # Release configuration
``` 
---
## Disclaimer and Acknowledgments

This tool does not aim to accurately predict the exact temperatures on a final product. There are already a ton of high precision CFD simulators out there with much higher precision. This tool's purpose is to provide the enigneer with an intuition on how to take the thermal design into account with a rough yet simple visualization.


The idea of this project came up with the desire of optimizing an already made spreadsheet tool to visualize the temperature gradient in a PCB for a good subject of the Electronic Systems Master's Degree of the Politechnic University of Valencia: *Thermal Design and EMC in Electronic Products*. Rewriting the program in a custom-made C++ program gives this utility a higher performance and longer lifespan that I hope will prove useful for other designers.

