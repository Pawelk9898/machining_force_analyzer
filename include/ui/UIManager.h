#pragma once
#include "Common.h"
#include "toolpath/ToolpathEngine.h"
#include "force/ForceModel.h"
#include "voxel/VoxelGrid.h"
#include "gcode/GCodeParser.h"
#include <vector>
#include <string>

namespace MathSim {

class UIManager {
public:
    UIManager(VoxelGrid& grid, ToolpathEngine& engine,
              ForceModel& forceModel, GCodeParser& parser);

    // Call once per frame — renders all ImGui panels
    void render();

    // Public state — read by main loop
    bool  simulationRunning = false;
    bool  simulationLoaded  = false;
    float simulationSpeed   = 1.0f;
    Tool  currentTool;

private:
    // Panel renderers
    void renderToolPanel();
    void renderStockPanel();
    void renderGCodePanel();
    void renderSimulationPanel();
    void renderForceChart();
    void renderStatusBar();
    void addForceData(double time, double fx, double fy, double fz);

    // References to core objects
    VoxelGrid&      m_grid;
    ToolpathEngine& m_engine;
    ForceModel&     m_forceModel;
    GCodeParser&    m_parser;

    // Tool panel state
    float m_diameter    = 10.0f;
    float m_height      = 30.0f;
    int   m_numFlutes   = 4;
    float m_rpm         = 5000.0f;
    float m_helixAngle  = 30.0f;
    float m_rakeAngle   = 10.0f;
    int   m_materialIdx = 0;

    // Stock panel state
    float m_stockMinX = -50.0f;
    float m_stockMaxX =  50.0f;
    float m_stockMinY = -50.0f;
    float m_stockMaxY =  50.0f;
    float m_stockMinZ = -20.0f;
    float m_stockMaxZ =   0.0f;
    float m_resolution =  1.0f;

    // G-code panel state
    char        m_gcodePathBuf[512] = {};
    std::string m_gcodeStatus       = "No file loaded";

    // Force history for chart
    std::vector<float> m_fxHistory;
    std::vector<float> m_fyHistory;
    std::vector<float> m_fzHistory;
    std::vector<float> m_timeHistory;
    int                m_maxHistorySize = 2000;

    // Status
    uint64_t m_totalVoxels   = 0;
    uint64_t m_removedVoxels = 0;
};

} // namespace MathSim