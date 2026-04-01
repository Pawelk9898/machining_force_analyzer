#pragma once
#include "Common.h"
#include "force/ForceModel.h"
#include "voxel/VoxelGrid.h"
#include "gcode/GCodeParser.h"
#include "simulator/Simulator.h"
#include <vector>
#include <string>

namespace MathSim {

class UIManager {
public:
    UIManager(VoxelGrid& grid, Simulator& simulator,
              ForceModel& forceModel, GCodeParser& parser);

    void render();
    void addForceData(double time, double fx, double fy, double fz);

    // Public state — read by main loop
    bool  simulationLoaded = false;
    bool  stockChanged     = false;
    float simulationSpeed  = 1.0f;
    Tool  currentTool;

private:
    void renderToolPanel();
    void renderStockPanel();
    void renderGCodePanel();
    void renderSimulationPanel();
    void renderForceChart();
    void renderStatusBar();

    VoxelGrid&   m_grid;
    Simulator&   m_simulator;
    ForceModel&  m_forceModel;
    GCodeParser& m_parser;

    // Tool panel
    float m_diameter    = 10.0f;
    float m_height      = 30.0f;
    int   m_numFlutes   = 4;
    float m_rpm         = 5000.0f;
    float m_helixAngle  = 30.0f;
    float m_rakeAngle   = 10.0f;
    int   m_materialIdx = 0;

    // Stock panel
    float m_stockMinX  = -50.0f;
    float m_stockMaxX  =  50.0f;
    float m_stockMinY  = -50.0f;
    float m_stockMaxY  =  50.0f;
    float m_stockMinZ  = -20.0f;
    float m_stockMaxZ  =   0.0f;
    float m_resolution =   1.0f;

    // G-code panel
    char        m_gcodePathBuf[512] = {};
    std::string m_gcodeStatus       = "No file loaded";

    // Force chart history
    std::vector<float> m_fxHistory;
    std::vector<float> m_fyHistory;
    std::vector<float> m_fzHistory;
    std::vector<float> m_timeHistory;
    int                m_maxHistorySize = 2000;

    // Status bar
    uint64_t m_totalVoxels   = 0;
    uint64_t m_removedVoxels = 0;
};

} // namespace MathSim