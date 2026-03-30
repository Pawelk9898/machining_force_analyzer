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
#include "toolpath/ToolpathEngine.h"
#include "gcode/GCodeParser.h"
#include "force/ForceModel.h"
#include "renderer/VoxelRenderer.h"
#include "ui/UIManager.h"

// Global camera and mouse state
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

int main() {
    // --- 1. GLFW init ---
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

    // --- 2. GLAD init ---
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to init GLAD\n";
        return -1;
    }
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_PROGRAM_POINT_SIZE);

    // --- 3. ImGui init ---
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // --- 4. Simulation objects ---
    MathSim::Bounds stockBounds = {
        -50.0, 50.0,
        -50.0, 50.0,
        -20.0,  0.0
    };
    MathSim::VoxelGrid      grid(stockBounds, 1.0);
    MathSim::ToolpathEngine engine(grid);
    MathSim::ForceModel     forceModel;
    MathSim::GCodeParser    parser(engine);
    MathSim::VoxelRenderer  renderer;
    MathSim::UIManager      ui(grid, engine, forceModel, parser);

    // Initial buffer fill
    renderer.updateBuffer(grid);

    // Seed mouse position
    glfwGetCursorPos(window, &g_lastMouseX, &g_lastMouseY);

    // --- 5. Main loop ---
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Camera orbit — right mouse button drag
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

        // Clear
        glClearColor(0.12f, 0.12f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ui.render();

        // Draw voxels
        renderer.draw(g_cam);

        // Finalize ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // --- 6. Cleanup ---
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}