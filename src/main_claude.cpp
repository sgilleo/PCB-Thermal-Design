// Simulador térmico 2D de una PCB - GLFW + OpenGL3 + Dear ImGui
//
// Modelo (placa delgada, temperatura uniforme en el espesor):
//   Ca * dT/dt = kt * lap(T) + q(x,y) - 2*h*(T - Tamb)
//
//   Ca = rho*c * espesor            [J/(m²·K)]  capacidad térmica por área
//   kt = k_fr4*e + n_cu*k_cu*cob*e_cu [W/K]     conductancia lateral (k·espesor)
//   q  = potencia / área del componente [W/m²]
//   h  = coef. de convección (se aplica en las 2 caras => 2*h)
//
// Bordes: adiabáticos (sin flujo). Se puede resolver en transitorio (explícito)
// o en estado estacionario (Gauss-Seidel + SOR).

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// ----------------------------------------------------------------------------
// Datos del modelo
// ----------------------------------------------------------------------------
struct Heater {              // componente que disipa calor (rectángulo)
    std::string name;
    float x, y, w, h;        // mm, origen arriba-izquierda
    float power;             // W
};

struct Params {
    float width_mm   = 100.f;
    float height_mm  = 80.f;
    float cell_mm    = 1.0f;     // tamaño de celda
    float board_mm   = 1.6f;     // espesor total
    float k_fr4      = 0.3f;     // W/mK (en el plano ~0.8-1, aquí conservador)
    float k_cu       = 385.f;    // W/mK
    float cu_um      = 35.f;     // espesor de cobre por capa (µm)
    int   cu_layers  = 2;
    float cu_cover   = 0.5f;     // fracción de cobre (0..1)
    float rho_c      = 2.0e6f;   // J/m³K (FR4 aprox.)
    float h_conv     = 10.f;     // W/m²K
    float T_amb      = 25.f;     // °C

    float kt() const {           // W/K
        return k_fr4 * board_mm * 1e-3f +
               cu_layers * k_cu * cu_cover * cu_um * 1e-6f;
    }
    float Ca() const { return rho_c * board_mm * 1e-3f; }   // J/m²K
};

// ----------------------------------------------------------------------------
// Simulación
// ----------------------------------------------------------------------------
class ThermalSim {
public:
    int nx = 0, ny = 0;
    std::vector<float> T, Tn, q;     // temperatura, buffer, fuente (W/m²)
    double simTime = 0.0;

    void resize(const Params& p) {
        nx = std::max(4, (int)std::round(p.width_mm  / p.cell_mm));
        ny = std::max(4, (int)std::round(p.height_mm / p.cell_mm));
        T.assign(nx * ny, p.T_amb);
        Tn = T;
        q.assign(nx * ny, 0.f);
        simTime = 0.0;
    }
    void resetTemp(const Params& p) {
        std::fill(T.begin(), T.end(), p.T_amb);
        simTime = 0.0;
    }

    // Convierte la lista de componentes en el campo q(x,y)
    void rasterize(const Params& p, const std::vector<Heater>& hs) {
        std::fill(q.begin(), q.end(), 0.f);
        float dx = p.cell_mm;
        for (const Heater& h : hs) {
            int i0 = std::max(0,  (int)std::floor(h.x / dx));
            int i1 = std::min(nx, (int)std::ceil((h.x + h.w) / dx));
            int j0 = std::max(0,  (int)std::floor(h.y / dx));
            int j1 = std::min(ny, (int)std::ceil((h.y + h.h) / dx));
            int n = std::max(0, i1 - i0) * std::max(0, j1 - j0);
            if (n == 0) continue;
            float cellArea = (dx * 1e-3f) * (dx * 1e-3f);
            float qq = h.power / (n * cellArea);              // W/m²
            for (int j = j0; j < j1; ++j)
                for (int i = i0; i < i1; ++i) q[j * nx + i] += qq;
        }
    }

    // dt máximo estable para el esquema explícito
    float stableDt(const Params& p) const {
        float dx = p.cell_mm * 1e-3f;
        float a  = p.kt() / (dx * dx);
        return 0.4f * p.Ca() / (4.f * a + 2.f * p.h_conv);
    }

    void stepTransient(const Params& p, int n) {
        float dx = p.cell_mm * 1e-3f;
        float a  = p.kt() / (dx * dx);
        float hh = 2.f * p.h_conv;
        float Ca = p.Ca();
        float dt = stableDt(p);
        for (int s = 0; s < n; ++s) {
            for (int j = 0; j < ny; ++j) {
                for (int i = 0; i < nx; ++i) {
                    int id = j * nx + i;
                    float Tc = T[id];
                    float Tl = i > 0      ? T[id - 1]  : Tc;   // borde adiabático
                    float Tr = i < nx - 1 ? T[id + 1]  : Tc;
                    float Tu = j > 0      ? T[id - nx] : Tc;
                    float Td = j < ny - 1 ? T[id + nx] : Tc;
                    float dTdt = (a * (Tl + Tr + Tu + Td - 4.f * Tc) + q[id]
                                  - hh * (Tc - p.T_amb)) / Ca;
                    Tn[id] = Tc + dt * dTdt;
                }
            }
            T.swap(Tn);
            simTime += dt;
        }
    }

    // Estado estacionario: Gauss-Seidel con sobrerrelajación
    void stepSteady(const Params& p, int iters, float omega = 1.8f) {
        float dx = p.cell_mm * 1e-3f;
        float a  = p.kt() / (dx * dx);
        float hh = 2.f * p.h_conv;
        for (int it = 0; it < iters; ++it) {
            for (int j = 0; j < ny; ++j) {
                for (int i = 0; i < nx; ++i) {
                    int id = j * nx + i;
                    float sum = 0.f; int cnt = 0;
                    if (i > 0)      { sum += T[id - 1];  ++cnt; }
                    if (i < nx - 1) { sum += T[id + 1];  ++cnt; }
                    if (j > 0)      { sum += T[id - nx]; ++cnt; }
                    if (j < ny - 1) { sum += T[id + nx]; ++cnt; }
                    float Tnew = (a * sum + q[id] + hh * p.T_amb) / (a * cnt + hh);
                    T[id] += omega * (Tnew - T[id]);
                }
            }
        }
    }
};

// ----------------------------------------------------------------------------
// Utilidades de visualización
// ----------------------------------------------------------------------------
static void colormap(float t, unsigned char* out) {      // azul->cian->verde->amarillo->rojo
    t = std::clamp(t, 0.f, 1.f);
    float r, g, b;
    if      (t < 0.25f) { float u = t / 0.25f;         r = 0;     g = u;     b = 1; }
    else if (t < 0.50f) { float u = (t - 0.25f)/0.25f; r = 0;     g = 1;     b = 1 - u; }
    else if (t < 0.75f) { float u = (t - 0.50f)/0.25f; r = u;     g = 1;     b = 0; }
    else                { float u = (t - 0.75f)/0.25f; r = 1;     g = 1 - u; b = 0; }
    out[0] = (unsigned char)(r * 255);
    out[1] = (unsigned char)(g * 255);
    out[2] = (unsigned char)(b * 255);
    out[3] = 255;
}

// ----------------------------------------------------------------------------
// Aplicación: estado + UI
// ----------------------------------------------------------------------------
class App {
public:
    Params params;
    ThermalSim sim;
    std::vector<Heater> heaters;
    int selected = -1;

    bool running = true;
    int  mode = 0;                 // 0 = transitorio, 1 = estacionario
    int  substeps = 200;           // pasos explícitos por frame
    int  sorIters = 30;            // iteraciones SOR por frame
    bool autoRange = true;
    float rangeMin = 20.f, rangeMax = 100.f;
    float Tmin = 0.f, Tmax = 0.f;

    GLuint tex = 0;
    std::vector<unsigned char> pixels;

    void init() {
        glGenTextures(1, &tex);
        heaters.push_back({"CPU",   30.f, 25.f, 12.f, 12.f, 3.0f});
        heaters.push_back({"LDO",   70.f, 50.f,  6.f,  6.f, 1.0f});
        rebuild();
    }

    void rebuild() {                       // cambia tamaño de malla
        sim.resize(params);
        sim.rasterize(params, heaters);
        pixels.assign(sim.nx * sim.ny * 4, 255);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sim.nx, sim.ny, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    }
    void sourcesChanged() { sim.rasterize(params, heaters); }

    // --- avance de la simulación + subida de textura (1 vez por frame)
    void update() {
        if (running) {
            if (mode == 0) sim.stepTransient(params, substeps);
            else           sim.stepSteady(params, sorIters);
        }
        Tmin = *std::min_element(sim.T.begin(), sim.T.end());
        Tmax = *std::max_element(sim.T.begin(), sim.T.end());
        float lo = autoRange ? Tmin : rangeMin;
        float hi = autoRange ? std::max(Tmax, Tmin + 0.1f) : rangeMax;
        for (int i = 0; i < sim.nx * sim.ny; ++i)
            colormap((sim.T[i] - lo) / (hi - lo), &pixels[i * 4]);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, sim.nx, sim.ny,
                        GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    }

    // --- panel de parámetros
    void drawControls() {
        ImGui::Begin("Parametros");
        bool geomChanged = false, srcChanged = false;

        if (ImGui::CollapsingHeader("Placa", ImGuiTreeNodeFlags_DefaultOpen)) {
            geomChanged |= ImGui::DragFloat("Ancho (mm)", &params.width_mm, 1, 10, 400);
            geomChanged |= ImGui::DragFloat("Alto (mm)",  &params.height_mm, 1, 10, 400);
            geomChanged |= ImGui::SliderFloat("Celda (mm)", &params.cell_mm, 0.25f, 4.f);
            ImGui::DragFloat("Espesor placa (mm)", &params.board_mm, 0.05f, 0.2f, 5.f);
            ImGui::DragFloat("k FR4 (W/mK)", &params.k_fr4, 0.01f, 0.1f, 2.f);
            ImGui::SliderInt("Capas de cobre", &params.cu_layers, 0, 8);
            ImGui::DragFloat("Cobre/capa (um)", &params.cu_um, 1, 5, 140);
            ImGui::SliderFloat("Cobertura cobre", &params.cu_cover, 0.f, 1.f);
        }
        if (ImGui::CollapsingHeader("Ambiente", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::DragFloat("T ambiente (C)", &params.T_amb, 0.5f, -40, 125);
            ImGui::DragFloat("h conveccion (W/m2K)", &params.h_conv, 0.1f, 0.f, 200.f);
        }
        if (ImGui::CollapsingHeader("Componentes", ImGuiTreeNodeFlags_DefaultOpen)) {
            if (ImGui::Button("+ Anadir")) {
                heaters.push_back({"U" + std::to_string(heaters.size() + 1),
                                   params.width_mm * 0.5f, params.height_mm * 0.5f,
                                   8.f, 8.f, 1.f});
                selected = (int)heaters.size() - 1; srcChanged = true;
            }
            ImGui::SameLine();
            if (ImGui::Button("- Borrar") && selected >= 0) {
                heaters.erase(heaters.begin() + selected);
                selected = -1; srcChanged = true;
            }
            for (int i = 0; i < (int)heaters.size(); ++i)
                if (ImGui::Selectable((heaters[i].name + "##" + std::to_string(i)).c_str(),
                                      selected == i)) selected = i;
            if (selected >= 0 && selected < (int)heaters.size()) {
                Heater& h = heaters[selected];
                char buf[32]; snprintf(buf, sizeof buf, "%s", h.name.c_str());
                if (ImGui::InputText("Nombre", buf, sizeof buf)) h.name = buf;
                srcChanged |= ImGui::DragFloat("x (mm)", &h.x, 0.5f);
                srcChanged |= ImGui::DragFloat("y (mm)", &h.y, 0.5f);
                srcChanged |= ImGui::DragFloat("ancho (mm)", &h.w, 0.5f, 0.5f, 200);
                srcChanged |= ImGui::DragFloat("alto (mm)",  &h.h, 0.5f, 0.5f, 200);
                srcChanged |= ImGui::DragFloat("Potencia (W)", &h.power, 0.05f, 0.f, 100.f);
            }
        }
        if (ImGui::CollapsingHeader("Simulacion", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::RadioButton("Transitorio", &mode, 0); ImGui::SameLine();
            ImGui::RadioButton("Estacionario", &mode, 1);
            if (ImGui::Button(running ? "Pausa" : "Continuar")) running = !running;
            ImGui::SameLine();
            if (ImGui::Button("Reiniciar T")) sim.resetTemp(params);
            if (mode == 0) {
                ImGui::SliderInt("Pasos/frame", &substeps, 1, 2000);
                ImGui::Text("t = %.1f s  (dt = %.3g s)", sim.simTime, sim.stableDt(params));
            } else {
                ImGui::SliderInt("Iter SOR/frame", &sorIters, 1, 300);
            }
            ImGui::Checkbox("Rango automatico", &autoRange);
            if (!autoRange) {
                ImGui::DragFloat("T min", &rangeMin, 0.5f);
                ImGui::DragFloat("T max", &rangeMax, 0.5f);
            }
            ImGui::Text("Tmin %.1f C | Tmax %.1f C", Tmin, Tmax);
            ImGui::Text("kt = %.4f W/K | Ca = %.0f J/m2K", params.kt(), params.Ca());
        }
        ImGui::End();

        if (geomChanged) rebuild();
        if (geomChanged || srcChanged) sourcesChanged();
    }

    // --- vista de la PCB (textura + overlay + interacción con ratón)
    void drawView() {
        ImGui::Begin("PCB");
        ImVec2 avail = ImGui::GetContentRegionAvail();
        float scale = std::min(avail.x / params.width_mm, avail.y / params.height_mm);
        scale = std::max(scale, 1.f);                       // px por mm
        ImVec2 size(params.width_mm * scale, params.height_mm * scale);

        ImVec2 p0 = ImGui::GetCursorScreenPos();
        ImGui::Image((ImTextureID)(intptr_t)tex, size);
        bool hovered = ImGui::IsItemHovered();
        ImDrawList* dl = ImGui::GetWindowDrawList();

        // contornos de componentes
        for (int i = 0; i < (int)heaters.size(); ++i) {
            const Heater& h = heaters[i];
            ImVec2 a(p0.x + h.x * scale, p0.y + h.y * scale);
            ImVec2 b(a.x + h.w * scale,  a.y + h.h * scale);
            dl->AddRect(a, b, i == selected ? IM_COL32(255,255,255,255)
                                            : IM_COL32(0,0,0,255), 0, 0, 2.f);
            dl->AddText(ImVec2(a.x + 2, a.y + 2), IM_COL32(0,0,0,255), h.name.c_str());
        }

        if (hovered) {
            ImVec2 m = ImGui::GetMousePos();
            float mx = (m.x - p0.x) / scale, my = (m.y - p0.y) / scale;   // mm
            int ci = std::clamp((int)(mx / params.cell_mm), 0, sim.nx - 1);
            int cj = std::clamp((int)(my / params.cell_mm), 0, sim.ny - 1);
            ImGui::SetTooltip("(%.1f, %.1f) mm\nT = %.2f C", mx, my, sim.T[cj * sim.nx + ci]);

            // clic: seleccionar componente bajo el cursor
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                selected = -1;
                for (int i = (int)heaters.size() - 1; i >= 0; --i) {
                    const Heater& h = heaters[i];
                    if (mx >= h.x && mx <= h.x + h.w && my >= h.y && my <= h.y + h.h) {
                        selected = i; break;
                    }
                }
            }
            // arrastrar: mover el seleccionado
            if (selected >= 0 && ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.f)) {
                ImVec2 d = ImGui::GetIO().MouseDelta;
                heaters[selected].x += d.x / scale;
                heaters[selected].y += d.y / scale;
                sourcesChanged();
            }
        }
        ImGui::End();
    }
};

// ----------------------------------------------------------------------------
// main: inicialización + bucle principal
// ----------------------------------------------------------------------------
static void glfwError(int code, const char* msg) { fprintf(stderr, "GLFW %d: %s\n", code, msg); }

int main() {
    glfwSetErrorCallback(glfwError);
    if (!glfwInit()) return 1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    GLFWwindow* win = glfwCreateWindow(1280, 800, "PCB Thermal", nullptr, nullptr);
    if (!win) { glfwTerminate(); return 1; }
    glfwMakeContextCurrent(win);
    glfwSwapInterval(1);                                  // vsync

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(win, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    App app;
    app.init();                                           // requiere contexto GL activo

    while (!glfwWindowShouldClose(win)) {
        glfwPollEvents();                                 // 1. eventos

        ImGui_ImplOpenGL3_NewFrame();                     // 2. nuevo frame de UI
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        app.drawControls();                               // 3. UI: edita el modelo
        app.update();                                     // 4. simulación + textura
        app.drawView();                                   // 5. UI: dibuja la PCB

        ImGui::Render();                                  // 6. render
        int w, h; glfwGetFramebufferSize(win, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(0.1f, 0.1f, 0.12f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(win);
    }

    glDeleteTextures(1, &app.tex);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(win);
    glfwTerminate();
    return 0;
}
