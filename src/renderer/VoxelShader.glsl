// --- VERTEX SHADER ---
#version 330 core
layout (location = 0) in vec3 aPos;   // Voxel Position
layout (location = 1) in vec3 aColor; // Voxel Color (Force-based)

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float uVoxelSize;

out vec3 vColor;
out float vHeight;

void main() {
    vec4 worldPos = model * vec4(aPos, 1.0);
    gl_Position = projection * view * worldPos;

    // Adjust point size based on distance (Perspective Scaling)
    // This makes voxels look like constant-sized cubes in 3D space
    float dist = length(view * worldPos);
    gl_PointSize = (uVoxelSize * 800.0) / dist;

    vColor = aColor;
    vHeight = aPos.z; // Used for "Top-down" gradient shading
}

// --- FRAGMENT SHADER ---
#version 330 core
out vec4 FragColor;

in vec3 vColor;
in float vHeight;

void main() {
    // Make the points look circular or "rounded square"
    // coords go from 0.0 to 1.0 across the point
    vec2 coords = gl_PointCoord - vec2(0.5);
    if (dot(coords, coords) > 0.25) discard; 

    // Simple "Fake" Lighting: brighten the top surfaces
    float ambient = 0.6;
    float lighing = ambient + (vHeight * 0.05); 

    FragColor = vec4(vColor * lighing, 1.0);
}