#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"

#include <iostream>
#include <cmath>

#include "Common.h"
#include "voxel/VoxelGrid.h"
#include "gcode/GCodeParser.h"
#include "force/ForceModel.h"
#include "simulator/Simulator.h"
#include "renderer/VoxelRenderer.h"
#include "ui/UIManager.h"

static MathSim::Camera g_cam;
static double g_lastMouseX = 0.0;
static double g_lastMouseY = 0.0;

void framebuffer_size_callback(GLFWwindow*, int width, int height) {
    glViewport(0, 0, width, height);
}

void scroll_callback(GLFWwindow*, double, double yoffset) {
    g_cam.distance -= static_cast<float>(yoffset) * 8.0f;
    if (g_cam.distance <   10.0f) g_cam.distance =   10.0f;
    if (g_cam.distance > 1000.0f) g_cam.distance = 1000.0f;
}
void applyCAMTheme() {
    ImGuiStyle& s = ImGui::GetStyle();

    s.WindowRounding    = 8.0f;
    s.FrameRounding     = 5.0f;
    s.GrabRounding      = 5.0f;
    s.TabRounding       = 5.0f;
    s.ScrollbarRounding = 5.0f;
    s.PopupRounding     = 5.0f;
    s.WindowBorderSize  = 1.0f;
    s.FrameBorderSize   = 1.0f;
    s.WindowPadding     = ImVec2(14, 12);
    s.FramePadding      = ImVec2(10, 6);
    s.ItemSpacing       = ImVec2(8, 8);
    s.ItemInnerSpacing  = ImVec2(6, 4);
    s.ScrollbarSize     = 10.0f;
    s.GrabMinSize       = 8.0f;
    s.IndentSpacing     = 16.0f;

    ImVec4* c = s.Colors;

    c[ImGuiCol_WindowBg]             = ImVec4(0.118f, 0.129f, 0.157f, 1.0f); // #1e2128
    c[ImGuiCol_ChildBg]              = ImVec4(0.094f, 0.102f, 0.125f, 1.0f); // #181b20
    c[ImGuiCol_PopupBg]              = ImVec4(0.118f, 0.129f, 0.157f, 1.0f);

    c[ImGuiCol_Border]               = ImVec4(0.180f, 0.200f, 0.251f, 1.0f); // #2e3340
    c[ImGuiCol_BorderShadow]         = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

    c[ImGuiCol_FrameBg]              = ImVec4(0.075f, 0.082f, 0.110f, 1.0f); // #13151c
    c[ImGuiCol_FrameBgHovered]       = ImVec4(0.100f, 0.110f, 0.140f, 1.0f);
    c[ImGuiCol_FrameBgActive]        = ImVec4(0.000f, 0.737f, 0.831f, 0.20f); // teal tint

    c[ImGuiCol_TitleBg]              = ImVec4(0.094f, 0.106f, 0.133f, 1.0f); // #181b22
    c[ImGuiCol_TitleBgActive]        = ImVec4(0.094f, 0.106f, 0.133f, 1.0f);
    c[ImGuiCol_TitleBgCollapsed]     = ImVec4(0.094f, 0.106f, 0.133f, 1.0f);

    c[ImGuiCol_MenuBarBg]            = ImVec4(0.094f, 0.106f, 0.133f, 1.0f);

    c[ImGuiCol_ScrollbarBg]          = ImVec4(0.075f, 0.082f, 0.110f, 1.0f);
    c[ImGuiCol_ScrollbarGrab]        = ImVec4(0.180f, 0.200f, 0.251f, 1.0f);
    c[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.000f, 0.737f, 0.831f, 0.60f);
    c[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.000f, 0.737f, 0.831f, 1.0f);

    c[ImGuiCol_CheckMark]            = ImVec4(0.000f, 0.737f, 0.831f, 1.0f); // teal
    c[ImGuiCol_SliderGrab]           = ImVec4(0.000f, 0.737f, 0.831f, 1.0f);
    c[ImGuiCol_SliderGrabActive]     = ImVec4(0.100f, 0.850f, 0.950f, 1.0f);

    c[ImGuiCol_Button]               = ImVec4(0.075f, 0.082f, 0.110f, 1.0f);
    c[ImGuiCol_ButtonHovered]        = ImVec4(0.000f, 0.737f, 0.831f, 0.20f);
    c[ImGuiCol_ButtonActive]         = ImVec4(0.000f, 0.737f, 0.831f, 1.0f);

    c[ImGuiCol_Header]               = ImVec4(0.000f, 0.737f, 0.831f, 0.20f);
    c[ImGuiCol_HeaderHovered]        = ImVec4(0.000f, 0.737f, 0.831f, 0.30f);
    c[ImGuiCol_HeaderActive]         = ImVec4(0.000f, 0.737f, 0.831f, 0.50f);

    c[ImGuiCol_Separator]            = ImVec4(0.180f, 0.200f, 0.251f, 1.0f);
    c[ImGuiCol_SeparatorHovered]     = ImVec4(0.000f, 0.737f, 0.831f, 0.60f);
    c[ImGuiCol_SeparatorActive]      = ImVec4(0.000f, 0.737f, 0.831f, 1.0f);

    c[ImGuiCol_ResizeGrip]           = ImVec4(0.000f, 0.737f, 0.831f, 0.20f);
    c[ImGuiCol_ResizeGripHovered]    = ImVec4(0.000f, 0.737f, 0.831f, 0.60f);
    c[ImGuiCol_ResizeGripActive]     = ImVec4(0.000f, 0.737f, 0.831f, 1.0f);

    c[ImGuiCol_Tab]                  = ImVec4(0.094f, 0.106f, 0.133f, 1.0f);
    c[ImGuiCol_TabHovered]           = ImVec4(0.000f, 0.737f, 0.831f, 0.30f);
    c[ImGuiCol_TabSelected]          = ImVec4(0.000f, 0.737f, 0.831f, 0.20f);

    c[ImGuiCol_PlotLines]            = ImVec4(0.000f, 0.737f, 0.831f, 1.0f);
    c[ImGuiCol_PlotLinesHovered]     = ImVec4(0.100f, 0.850f, 0.950f, 1.0f);
    c[ImGuiCol_PlotHistogram]        = ImVec4(0.000f, 0.737f, 0.831f, 1.0f);

    c[ImGuiCol_Text]                 = ImVec4(0.886f, 0.902f, 0.941f, 1.0f); // #e2e6f0
    c[ImGuiCol_TextDisabled]         = ImVec4(0.290f, 0.322f, 0.408f, 1.0f); // #4a5268

    c[ImGuiCol_NavHighlight]         = ImVec4(0.000f, 0.737f, 0.831f, 1.0f);

    c[ImGuiCol_ModalWindowDimBg]     = ImVec4(0.0f, 0.0f, 0.0f, 0.6f);
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to init GLFW\n";
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(
        1280, 720, "Machining Force Analyzer", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetScrollCallback(window, scroll_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to init GLAD\n";
        return -1;
    }
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_PROGRAM_POINT_SIZE);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGui::StyleColorsDark();
    applyCAMTheme();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Simulation objects
    MathSim::Bounds stockBounds = {
        -50.0, 50.0,
        -50.0, 50.0,
        -20.0,  0.0
    };
    MathSim::VoxelGrid     grid(stockBounds, 1.0);
    MathSim::ForceModel    forceModel;
    MathSim::GCodeParser   parser;
    MathSim::Simulator     simulator(grid, forceModel);
    MathSim::VoxelRenderer renderer;
    MathSim::UIManager     ui(grid, simulator, forceModel, parser);

    // Default tool so forces work without clicking Apply Tool
    ui.currentTool.diameter   = 10.0;
    ui.currentTool.height     = 30.0;
    ui.currentTool.numFlutes  = 4;
    ui.currentTool.rpm        = 5000.0;
    ui.currentTool.helixAngle = 30.0;
    ui.currentTool.rakeAngle  = 10.0;

    renderer.updateBuffer(grid);
    glfwGetCursorPos(window, &g_lastMouseX, &g_lastMouseY);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Camera orbit
        double mx, my;
        glfwGetCursorPos(window, &mx, &my);
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
            float dx = static_cast<float>(mx - g_lastMouseX);
            float dy = static_cast<float>(my - g_lastMouseY);
            g_cam.yaw   += dx * 0.4f;
            g_cam.pitch -= dy * 0.4f;
            if (g_cam.pitch >  89.0f) g_cam.pitch =  89.0f;
            if (g_cam.pitch < -89.0f) g_cam.pitch = -89.0f;
        }
        g_lastMouseX = mx;
        g_lastMouseY = my;

        // Run simulation steps this frame
        int stepsPerFrame = static_cast<int>(ui.simulationSpeed * 1);
        auto results = simulator.step(stepsPerFrame);

        // Feed all steps to force chart and check if voxels changed
        bool voxelsChanged = false;
        for (auto& r : results) {
            ui.addForceData(r.timeOffset, r.fx, r.fy, r.fz);
            if (r.voxelsRemoved > 0) voxelsChanged = true;
        }
        if (voxelsChanged) {
            renderer.updateBuffer(grid);
        }

        glClearColor(0.075f, 0.082f, 0.110f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ui.render();
        renderer.draw(g_cam);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}