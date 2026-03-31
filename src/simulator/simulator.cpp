#include "simulator/Simulator.h"
#include <cmath>
#include <algorithm>

namespace MathSim {

Simulator::Simulator(VoxelGrid& grid, ForceModel& forceModel)
    : m_grid(grid)
    , m_forceModel(forceModel)
{}

void Simulator::loadMoves(const std::vector<Move>& moves, const Tool& tool) {
    m_moves      = moves;
    m_tool       = tool;
    m_moveIndex  = 0;
    m_moveT      = 0.0;
    m_currentPos = { 0, 0, 0 };
    m_elapsedTime = 0.0;
    m_playing    = false;
    m_finished   = false;
}

void Simulator::reset() {
    m_moveIndex  = 0;
    m_moveT      = 0.0;
    m_currentPos = { 0, 0, 0 };
    m_elapsedTime = 0.0;
    m_playing    = false;
    m_finished   = false;
    m_grid.reset();
}

float Simulator::getProgress() const {
    if (m_moves.empty()) return 0.0f;
    return static_cast<float>(m_moveIndex) / static_cast<float>(m_moves.size());
}

std::vector<SimStepResult> Simulator::step(int stepsPerFrame) {
    std::vector<SimStepResult> results;
    if (!m_playing || m_finished || m_moves.empty()) return results;

    for (int i = 0; i < stepsPerFrame; ++i) {
        if (m_moveIndex >= static_cast<int>(m_moves.size())) {
            m_finished = true;
            m_playing  = false;
            break;
        }
        SimStepResult res = executeMicroStep();
        results.push_back(res);
        if (m_finished) break;
    }
    return results;
}

SimStepResult Simulator::executeMicroStep() {
    SimStepResult result;

    const Move& move = m_moves[m_moveIndex];

    // Calculate move vector
    double dx = move.target.x - m_currentPos.x;
    double dy = move.target.y - m_currentPos.y;
    double dz = move.target.z - m_currentPos.z;

    // For rapid moves start and finish immediately
    if (move.type == Move::Type::RAPID) {
        m_currentPos = move.target;
        result.toolPos = m_currentPos;
        advanceMove();
        return result;
    }

    // Linear move — step size tied to voxel resolution
    double distance = std::sqrt(dx*dx + dy*dy + dz*dz);
    if (distance < 1e-7) {
        advanceMove();
        return result;
    }

    double stepSize = m_grid.getResolution() * 0.5;
    double tStep    = stepSize / distance;
    m_moveT        += tStep;

    if (m_moveT >= 1.0) {
        m_moveT      = 0.0;
        m_currentPos = move.target;
        advanceMove();
    } else {
        m_currentPos = Vector3(
            m_moves[m_moveIndex].target.x - dx * (1.0 - m_moveT),
            m_moves[m_moveIndex].target.y - dy * (1.0 - m_moveT),
            m_moves[m_moveIndex].target.z - dz * (1.0 - m_moveT)
        );
    }

    // Material removal
    if (move.spindleOn) {
        result.voxelsRemoved = m_grid.subtractCylinder(
            m_currentPos,
            m_tool.diameter / 2.0,
            m_tool.height
        );
    }

    // Force calculation
    if (result.voxelsRemoved > 0) {
        double radialDepth   = m_tool.diameter / 2.0;
        double feedPerTooth  = move.feedRate /
            (m_tool.numFlutes * m_tool.rpm);
        double engAngle      = std::acos(
            std::max(-1.0, std::min(1.0,
                1.0 - radialDepth / (m_tool.diameter / 2.0))));
        double chipThickness = feedPerTooth * std::sin(engAngle);
        chipThickness        = std::max(chipThickness, 0.0);

        ForceVector forces = m_forceModel.calculateInstantaneousForce(
            chipThickness, m_tool.height, engAngle);

        result.fx = forces.x;
        result.fy = forces.y;
        result.fz = forces.z;
    }

    result.toolPos    = m_currentPos;
    result.timeOffset = m_elapsedTime;

    // Advance time
    if (move.feedRate > 0)
        m_elapsedTime += (stepSize / move.feedRate) * 60.0;

    return result;
}

void Simulator::advanceMove() {
    m_moveIndex++;
    m_moveT = 0.0;
    if (m_moveIndex >= static_cast<int>(m_moves.size())) {
        m_finished = true;
        m_playing  = false;
    }
}

} // namespace MathSim