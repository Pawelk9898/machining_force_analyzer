#include "toolpath/ToolpathEngine.h"
#include "force/ForceModel.h"
#include <cmath>
#include <algorithm>

namespace MathSim {

ToolpathEngine::ToolpathEngine(VoxelGrid& grid) : m_grid(grid) {}

void ToolpathEngine::executeG0(Vector3 start, Vector3 end) {
    // Rapid move — no cutting, no force calculation
    // Position update is handled by GCodeParser tracking m_currentPos
    (void)start;
    (void)end;
}

std::vector<StepResult> ToolpathEngine::executeG1(Vector3 start, Vector3 end,
                                                   const Tool& tool, double feedRate) {
    std::vector<StepResult> history;

    // 1. Calculate move vector and distance
    double dx = end.x - start.x;
    double dy = end.y - start.y;
    double dz = end.z - start.z;
    double distance = std::sqrt(dx*dx + dy*dy + dz*dz);

    if (distance < 1e-7) return history;

    // 2. Step size — half voxel resolution ensures no gaps
    double stepSize = m_grid.getResolution() * 0.5;
    int numSteps = static_cast<int>(std::ceil(distance / stepSize));

    // 3. Time per step
    double moveDuration = (distance / feedRate) * 60.0; // feedRate in mm/min
    double dt = moveDuration / numSteps;

    // 4. Force model instance
    ForceModel forceModel;

    // 5. Precompute chip thickness from feedrate per tooth
    double feedPerTooth = feedRate / (tool.numFlutes * tool.rpm); // mm/tooth
    double radialDepth  = tool.diameter / 2.0; // assume full radial engagement for now
    double axialDepth   = tool.height;

    // 6. Interpolation loop
    for (int i = 0; i <= numSteps; ++i) {
        double t = (numSteps == 0) ? 1.0 : static_cast<double>(i) / numSteps;

        Vector3 currentPos(
            start.x + t * dx,
            start.y + t * dy,
            start.z + t * dz
        );

        // Subtract material and get removal count
        uint64_t removed = m_grid.subtractCylinder(
            currentPos,
            tool.diameter / 2.0,
            tool.height
        );

        // Calculate engagement angle from voxels removed
        // More voxels removed = deeper engagement
        double engagementAngle = 0.0;
        if (removed > 0) {
            // Approximate engagement angle from radial depth
            engagementAngle = std::acos(1.0 - (radialDepth / (tool.diameter / 2.0)));
        }

        // Chip thickness varies with engagement angle
        double chipThickness = feedPerTooth * std::sin(engagementAngle);
        chipThickness = std::max(chipThickness, 0.0);

        // Calculate forces using Altintas model
        ForceVector forces = { 0.0, 0.0, 0.0 };
        if (removed > 0 && chipThickness > 1e-9) {
            forces = forceModel.calculateInstantaneousForce(
                chipThickness,
                axialDepth,
                engagementAngle
            );
        }

        // Record step result
        StepResult res;
        res.position        = currentPos;
        res.voxelsRemoved   = removed;
        res.timeOffset      = m_totalElapsedSeconds;
        res.fx              = forces.x;
        res.fy              = forces.y;
        res.fz              = forces.z;
        res.chipThickness   = chipThickness;
        res.engagementAngle = engagementAngle;

        history.push_back(res);
        m_totalElapsedSeconds += dt;
    }

    return history;
}

} // namespace MathSim