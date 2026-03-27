#include "renderer/Renderer.h"
#include "voxel/VoxelGrid.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

namespace MathSim {

// Vertex Shader (Stayed the same)
const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;
    void main() {
        gl_Position = projection * view * model * vec4(aPos, 1.0);
    }
)";

// Fragment Shader (Stayed the same)
const char* fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;
    void main() {
        FragColor = vec4(0.8f, 0.4f, 0.1f, 1.0f); 
    }
)";

Renderer::Renderer() : m_shaderProgram(0), m_vbo(0), m_vao(0) {
    setupShaders();
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
}

Renderer::~Renderer() {
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);
    glDeleteProgram(m_shaderProgram);
}

void Renderer::setupShaders() {
    unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertexShaderSource, NULL);
    glCompileShader(vs);

    unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmentShaderSource, NULL);
    glCompileShader(fs);

    m_shaderProgram = glCreateProgram();
    glAttachShader(m_shaderProgram, vs);
    glAttachShader(m_shaderProgram, fs);
    glLinkProgram(m_shaderProgram);
    glDeleteShader(vs);
    glDeleteShader(fs);
}

void Renderer::render(const VoxelGrid& grid) {
    glEnable(GL_DEPTH_TEST);
    glUseProgram(m_shaderProgram);

    // 1. Generate Mesh from VoxelGrid
    std::vector<float> vertices;
    auto dim = grid.getDimensions(); // Assuming you have getDimensions() returning {100, 100, 50}

    for (int x = 0; x < 100; ++x) {
        for (int y = 0; y < 100; ++y) {
            for (int z = 0; z < 50; ++z) {
                if (grid.isSolid(x, y, z)) {
                    // Add a simple point/cube-center for each solid voxel
                    // For performance, we'll just draw points for now
                    vertices.push_back((float)x);
                    vertices.push_back((float)y);
                    vertices.push_back((float)z);
                }
            }
        }
    }

    // 2. Upload to GPU
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // 3. Camera (Zoomed out to see the 100x100 block)
    glm::mat4 view = glm::lookAt(glm::vec3(150, 150, 150), glm::vec3(50, 50, 25), glm::vec3(0, 0, 1));
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), 1280.0f / 720.0f, 0.1f, 1000.0f);
    glm::mat4 model = glm::mat4(1.0f);

    glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(proj));

    // 4. Draw as Points (Fastest way to see the block initially)
    glPointSize(2.0f);
    glDrawArrays(GL_POINTS, 0, (GLsizei)(vertices.size() / 3));
    
    glBindVertexArray(0);
}

} // namespace MathSim