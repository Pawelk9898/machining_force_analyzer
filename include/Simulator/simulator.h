#pragma once
#include "Common.h"
#include "voxel/VoxelGrid.h"
#include "force/ForceModel.h"
#include "toolpath/ToolpathEngine.h"
#include <vector>
namespace MathSim {

// Represents a single parsed G-code move
struct Move {
    enum class Type { RAPID, LINEAR, ARC_CW, ARC_CCW };
    Type     type      = Type::RAPID;
    Vector3  target    = { 0, 0, 0 };
    Vector3  arcCenter = { 0, 0, 0 }; // for G2/G3
    double   feedRate  = 100.0;
    double   spindleSpeed = 0.0;
    bool     spindleOn = false;
};

// Result of a single simulation micro-step
struct SimStepResult {
    Vector3  toolPos       = { 0, 0, 0 };
    double   fx            = 0.0;
    double   fy            = 0.0;
    double   fz            = 0.0;
    uint64_t voxelsRemoved = 0;
    double   timeOffset    = 0.0;
    bool     finished      = false;
};

class Simulator {
public:
    Simulator(VoxelGrid& grid, ForceModel& forceModel);

    // Load parsed moves — does NOT execute them
    void loadMoves(const std::vector<Move>& moves, const Tool& tool);

    // Execute up to N micro-steps this frame
    // Returns results for each step executed
    std::vector<SimStepResult> step(int stepsPerFrame);

    // Playback control
    void play()  { m_playing = true; }
    void pause() { m_playing = false; }
    void reset();

    bool isPlaying()  const { return m_playing; }
    bool isFinished() const { return m_finished; }

    // Current tool position for rendering
    Vector3 getToolPosition() const { return m_currentPos; }

    // Progress 0.0 to 1.0
    float getProgress() const;

private:
    VoxelGrid&  m_grid;
    ForceModel& m_forceModel;

    std::vector<Move> m_moves;
    Tool              m_tool;

    // Playback state
    int     m_moveIndex    = 0;  // which move we are on
    double  m_moveT        = 0.0; // interpolation parameter 0..1 within current move
    Vector3 m_currentPos   = { 0, 0, 0 };
    double  m_elapsedTime  = 0.0;
    bool    m_playing      = false;
    bool    m_finished     = false;

    // Execute one micro-step within the current move
    SimStepResult executeMicroStep();

    // Advance to next move
    void advanceMove();
};

} // namespace MathSim