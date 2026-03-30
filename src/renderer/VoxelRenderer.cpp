#include "renderer/VoxelRenderer.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <iostream>
#include <cmath>

namespace MathSim {

static const char* vertSrc = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;
uniform mat4 uMVP;
uniform float uVoxelSize;
out vec3 vColor;
void main() {
    vec4 clipPos = uMVP * vec4(aPos, 1.0);
    gl_Position  = clipPos;
    float dist   = length(clipPos.xyz);
    gl_PointSize = max(1.0, (uVoxelSize * 600.0) / dist);
    vColor = aColor;
}
)";

static const char* fragSrc = R"(
#version 330 core
in vec3 vColor;
out vec4 FragColor;
void main() {
    vec2 c = gl_PointCoord - vec2(0.5);
    if (dot(c, c) > 0.25) discard;
    float light = 0.7 + gl_FragCoord.z * 0.3;
    FragColor = vec4(vColor * light, 1.0);
}
)";

static GLuint compileShader(GLenum type, const char* src) {
    GLuint id = glCreateShader(type);
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);
    int ok;
    glGetShaderiv(id, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetShaderInfoLog(id, 512, nullptr, log);
        std::cerr << "Shader error: " << log << "\n";
    }
    return id;
}

VoxelRenderer::VoxelRenderer() {
    GLuint vs = compileShader(GL_VERTEX_SHADER,   vertSrc);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragSrc);
    m_shaderProgram = glCreateProgram();
    glAttachShader(m_shaderProgram, vs);
    glAttachShader(m_shaderProgram, fs);
    glLinkProgram(m_shaderProgram);
    int ok;
    glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetProgramInfoLog(m_shaderProgram, 512, nullptr, log);
        std::cerr << "Shader link error: " << log << "\n";
    }
    glDeleteShader(vs);
    glDeleteShader(fs);

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_DEPTH_TEST);
}

VoxelRenderer::~VoxelRenderer() {
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);
    glDeleteProgram(m_shaderProgram);
}

void VoxelRenderer::updateBuffer(const VoxelGrid& grid) {
    std::vector<VoxelVertex> vertices;
    vertices.reserve(
        static_cast<size_t>(grid.getDimX()) *
        grid.getDimY() * grid.getDimZ() / 2);

    int    dimX   = grid.getDimX();
    int    dimY   = grid.getDimY();
    int    dimZ   = grid.getDimZ();
    double res    = grid.getResolution();
    Bounds bounds = grid.getBounds();

    for (int x = 0; x < dimX; ++x) {
        for (int y = 0; y < dimY; ++y) {
            for (int z = 0; z < dimZ; ++z) {
                if (grid.isSolid(x, y, z)) {
                    VoxelVertex v;
                    v.x = static_cast<float>(bounds.minX + x * res);
                    v.y = static_cast<float>(bounds.minY + y * res);
                    v.z = static_cast<float>(bounds.minZ + z * res);
                    float t = static_cast<float>(z) /
                              static_cast<float>(std::max(dimZ - 1, 1));
                    v.r = 0.3f + t * 0.2f;
                    v.g = 0.5f + t * 0.2f;
                    v.b = 0.8f + t * 0.1f;
                    vertices.push_back(v);
                }
            }
        }
    }

    m_vertexCount = vertices.size();

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 vertices.size() * sizeof(VoxelVertex),
                 vertices.data(), GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          sizeof(VoxelVertex), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          sizeof(VoxelVertex),
                          (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void VoxelRenderer::draw(const Camera& cam) {
    if (m_vertexCount == 0) return;

    glUseProgram(m_shaderProgram);

    float yawRad   = glm::radians(cam.yaw);
    float pitchRad = glm::radians(cam.pitch);

    glm::vec3 eye(
        cam.targetX + cam.distance * std::cos(pitchRad) * std::sin(yawRad),
        cam.targetY + cam.distance * std::cos(pitchRad) * std::cos(yawRad),
        cam.targetZ + cam.distance * std::sin(pitchRad)
    );
    glm::vec3 target(cam.targetX, cam.targetY, cam.targetZ);
    glm::vec3 up(0.0f, 0.0f, 1.0f);

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view  = glm::lookAt(eye, target, up);
    glm::mat4 proj  = glm::perspective(
        glm::radians(45.0f), 1280.0f / 720.0f, 0.1f, 2000.0f);
    glm::mat4 mvp   = proj * view * model;

    glUniformMatrix4fv(
        glGetUniformLocation(m_shaderProgram, "uMVP"),
        1, GL_FALSE, glm::value_ptr(mvp));
    glUniform1f(
        glGetUniformLocation(m_shaderProgram, "uVoxelSize"), 1.0f);

    glBindVertexArray(m_vao);
    glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(m_vertexCount));
    glBindVertexArray(0);
}

} // namespace MathSim