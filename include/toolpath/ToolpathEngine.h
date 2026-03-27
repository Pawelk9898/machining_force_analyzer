#pragma once
#include "voxel/VoxelGrid.h"
#include <vector>

namespace MathSim {

struct Tool {
    double diameter;
    double height;
    int numFlutes;
    double rpm;
};

// Data structure to store results of a single micro-step
struct StepResult {
    Vector3 position;
    uint64_t voxelsRemoved;
    double timeOffset; 
};

class ToolpathEngine {
public:
    explicit ToolpathEngine(VoxelGrid& grid);

    // Moves the tool linearly and returns a history of the cut
    std::vector<StepResult> executeG1(Vector3 start, Vector3 end, const Tool& tool, double feedRate);

private:
    VoxelGrid& m_grid;
    double m_totalElapsedSeconds = 0.0;
};

} // namespace MathSim