#pragma once
#include "Common.h"
#include "voxel/VoxelGrid.h"
#include <vector>

namespace MathSim {

struct Tool {
    // Basic geometry
    double diameter   = 10.0;  // mm
    double height     = 30.0;  // mm (flute length)
    int    numFlutes  = 4;
    double rpm        = 5000.0;

    // Advanced geometry (needed for Altintas force model)
    double helixAngle  = 30.0; // degrees
    double rakeAngle   = 10.0; // degrees
    double reliefAngle = 8.0;  // degrees
};

struct StepResult {
    Vector3  position;
    uint64_t voxelsRemoved  = 0;
    double   timeOffset     = 0.0;

    // Force output — filled by ForceModel
    double fx               = 0.0; // N
    double fy               = 0.0; // N
    double fz               = 0.0; // N

    // Engagement data — useful for debugging and validation
    double chipThickness    = 0.0; // mm
    double engagementAngle  = 0.0; // radians
};

class ToolpathEngine {
public:
    explicit ToolpathEngine(VoxelGrid& grid);

    // Rapid move — no cutting, just updates position
    void executeG0(Vector3 start, Vector3 end);

    // Linear cutting move — returns history of micro-steps
    std::vector<StepResult> executeG1(Vector3 start, Vector3 end,
                                      const Tool& tool, double feedRate);

private:
    VoxelGrid& m_grid;
    double     m_totalElapsedSeconds = 0.0;
};

} // namespace MathSim