#pragma once
#include "Common.h"
#include "voxel/VoxelGrid.h"
#include <glad/glad.h>

namespace MathSim {

struct VoxelVertex {
    float x, y, z;
    float r, g, b;
};

struct Camera {
    float yaw      = 45.0f;
    float pitch    = 35.0f;
    float distance = 200.0f;
    float targetX  = 0.0f;
    float targetY  = 0.0f;
    float targetZ  = -10.0f;
};

class VoxelRenderer {
public:
    VoxelRenderer();
    ~VoxelRenderer();
    void updateBuffer(const VoxelGrid& grid);
    void draw(const Camera& cam);
private:
    GLuint m_vao           = 0;
    GLuint m_vbo           = 0;
    GLuint m_shaderProgram = 0;
    size_t m_vertexCount   = 0;
};

} // namespace MathSim