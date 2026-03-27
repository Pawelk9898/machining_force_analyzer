#include <glad/glad.h>   // MUST BE FIRST
#include <GLFW/glfw3.h>

// If the headers above still fail to define it on your system:
#ifndef GL_DEPTH_BIT
    #define GL_DEPTH_BIT 0x00000100
#endif
#ifndef GL_COLOR_BUFFER_BIT
    #define GL_COLOR_BUFFER_BIT 0x00004000
#endif

// Standard Libraries
#include <vector>
#include <iostream>
#include <cmath>

// ImGui
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// Project Headers (Ensure these don't include GLFW before GLAD)
#include "Common.h"
#include "voxel/VoxelGrid.h"
#include "toolpath/ToolpathEngine.h"
#include "gcode/GCodeParser.h"
#include "force/ForceModel.h"
#include "renderer/Renderer.h"

int main() {
    // --- 1. Initialize GLFW & Window ---
    if (!glfwInit()) return -1;
    
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Machining Force Analyzer", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;

    // --- 2. Initialize Simulation Objects ---
    // Using the MathSim namespace to avoid "niezadeklarowany identyfikator"
    MathSim::Bounds stockBounds = {-50.0f, 50.0f, -50.0f, 50.0f, -20.0f, 0.0f};
    MathSim::VoxelGrid grid(stockBounds, 1.0f); // 1.0mm resolution for speed
    
    MathSim::ToolpathEngine engine(grid);
    MathSim::GCodeParser parser(engine);
    MathSim::ForceModel forceCalc;
    MathSim::Renderer renderer; 
    
    // Ensure the Tool struct matches your ToolpathEngine.h definition
    MathSim::Tool myTool = { 10.0f, 30.0f }; 

    // --- 3. Main Loop ---
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Clear Screen (This uses GL_DEPTH_BIT which we fixed with the header order)
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BIT);

        // Render the Voxel Grid
        renderer.render(grid);

        // (Optional: Add ImGui rendering logic here later)

        glfwSwapBuffers(window);
    }

    // --- 4. Cleanup ---
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return 0;
}