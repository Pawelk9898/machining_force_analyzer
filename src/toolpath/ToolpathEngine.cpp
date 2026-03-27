#include "toolpath/ToolpathEngine.h"
#include <cmath>
#include <algorithm>

namespace MathSim {

ToolpathEngine::ToolpathEngine(VoxelGrid& grid) : m_grid(grid) {}

std::vector<StepResult> ToolpathEngine::executeG1(Vector3 start, Vector3 end, const Tool& tool, double feedRate) {
    std::vector<StepResult> history;

    // 1. Calculate distances
    double dx = end.x - start.x;
    double dy = end.y - start.y;
    double dz = end.z - start.z;
    double distance = std::sqrt(dx*dx + dy*dy + dz*dz);

    if (distance < 1e-7) return history;

    // 2. Determine Step Count
    // We want at least one step every 0.5 * resolution to ensure no gaps
    double stepSize = m_grid.getResolution() * 0.5;
    int numSteps = static_cast<int>(std::ceil(distance / stepSize));
    
    // Time per step: (distance / feedRate_mm_per_min) * 60 seconds / numSteps
    double moveDuration = (distance / feedRate) * 60.0;
    double dt = moveDuration / numSteps;

    // 3. Interpolation Loop
    for (int i = 0; i <= numSteps; ++i) {
        double t = (numSteps == 0) ? 1.0 : static_cast<double>(i) / numSteps;
        
        Vector3 currentPos = {
            static_cast<float>(start.x + t * dx),
            static_cast<float>(start.y + t * dy),
            static_cast<float>(start.z + t * dz)
        };

        // Subtract from voxel grid and get NEW removal count
        uint64_t removed = m_grid.subtractCylinder(currentPos, tool.diameter / 2.0, tool.height);

        // Record this "instant" for the force analyzer and UI
        StepResult res;
        res.position = currentPos;
        res.voxelsRemoved = removed;
        res.timeOffset = m_totalElapsedSeconds;
        
        history.push_back(res);
        m_totalElapsedSeconds += dt;
    }

    return history;
}

} // namespace MathSim