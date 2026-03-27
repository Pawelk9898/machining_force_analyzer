#ifndef RENDERER_H
#define RENDERER_H

// Include glad here so the compiler knows OpenGL constants 
// whenever Renderer.h is included.
#include <glad/glad.h> 

namespace MathSim {
    class VoxelGrid; // Forward declaration

    class Renderer {
    public:
        Renderer();
        ~Renderer();
        void render(const VoxelGrid& grid);

    private:
        unsigned int m_shaderProgram;
        unsigned int m_vbo, m_vao;
        void setupShaders();
        void setupBuffers();
    };
}

#endif