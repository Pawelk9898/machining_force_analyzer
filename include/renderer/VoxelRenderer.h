#pragma once
#include <vector>
#include "voxel/VoxelGrid.h"
#include <glad/glad.h> // Or glad.h

namespace MathSim {

struct VoxelVertex {
    float x, y, z;
    float r, g, b; // Color based on force or height
};

class VoxelRenderer {
public:
    VoxelRenderer();
    ~VoxelRenderer();

    // Updates the GPU buffer based on the current VoxelGrid state
    void updateBuffer(const VoxelGrid& grid);

    // Draws the stock to the screen
    void draw();

private:
    GLuint m_vbo, m_vao;
    size_t m_vertexCount = 0;
};

} // namespace MathSim