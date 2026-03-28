#include <glad/glad.h>   // MUST BE FIRST
#include <GLFW/glfw3.h>

// ImGui
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"

// Standard libraries
#include <iostream>
#include <vector>
#include <cmath>

// Project headers
#include "Common.h"
#include "voxel/VoxelGrid.h"
#include "toolpath/ToolpathEngine.h"
#include "gcode/GCodeParser.h"
#include "force/ForceModel.h"
#include "renderer/VoxelRenderer.h"
#include "ui/UIManager.h"

// Window resize callback
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

int main() {
    // --- 1. Initialize GLFW ---
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1280, 720,
        "Machining Force Analyzer", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // vsync
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // --- 2. Initialize GLAD ---
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_PROGRAM_POINT_SIZE);

    // --- 3. Initialize ImGui ---
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // --- 4. Initialize simulation objects ---
    MathSim::Bounds stockBounds = {
        -50.0, 50.0,  // X
        -50.0, 50.0,  // Y
        -20.0,  0.0   // Z
    };
    MathSim::VoxelGrid     grid(stockBounds, 1.0);
    MathSim::ToolpathEngine engine(grid);
    MathSim::ForceModel     forceModel;
    MathSim::GCodeParser    parser(engine);
    MathSim::VoxelRenderer  renderer;
    MathSim::UIManager      ui(grid, engine, forceModel, parser);

    // --- 5. Main loop ---
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Clear screen
        glClearColor(0.12f, 0.12f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Render UI panels
        ui.render();

        // Update and draw voxel grid
        renderer.updateBuffer(grid);
        renderer.draw();

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