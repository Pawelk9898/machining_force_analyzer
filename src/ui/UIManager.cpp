#include "ui/UIManager.h"
#include "imgui.h"
#include "implot.h"
#define _USE_MATH_DEFINES
#include "force/ForceModel.h"
#include <cmath>
#include <algorithm>
namespace MathSim {

UIManager::UIManager(VoxelGrid& grid, ToolpathEngine& engine,
                     ForceModel& forceModel, GCodeParser& parser)
    : m_grid(grid)
    , m_engine(engine)
    , m_forceModel(forceModel)
    , m_parser(parser)
{
    m_totalVoxels = static_cast<uint64_t>(
        m_grid.getDimX()) * m_grid.getDimY() * m_grid.getDimZ();
}

void UIManager::render() {
    renderToolPanel();
    renderStockPanel();
    renderGCodePanel();
    renderSimulationPanel();
    renderForceChart();
    renderStatusBar();
}

void UIManager::renderToolPanel() {
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(280, 280), ImGuiCond_FirstUseEver);
    ImGui::Begin("Tool Definition");

    ImGui::Text("Geometry");
    ImGui::Separator();
    ImGui::SliderFloat("Diameter (mm)",    &m_diameter,   0.5f,  50.0f);
    ImGui::SliderFloat("Flute length (mm)",&m_height,     5.0f, 100.0f);
    ImGui::SliderInt  ("Num flutes",       &m_numFlutes,  1,     8);
    ImGui::SliderFloat("RPM",              &m_rpm,        100.0f, 20000.0f);
    ImGui::SliderFloat("Helix angle (deg)",&m_helixAngle, 0.0f,  60.0f);
    ImGui::SliderFloat("Rake angle (deg)", &m_rakeAngle, -10.0f, 30.0f);

    ImGui::Spacing();
    ImGui::Text("Material");
    ImGui::Separator();
    const char* materials[] = {
        "Aluminum 7075",
        "Steel 4140",
        "Titanium Ti-6Al-4V",
        "Stainless Steel 316"
    };
    if (ImGui::Combo("Material", &m_materialIdx, materials, 4)) {
        switch (m_materialIdx) {
            case 0: m_forceModel.setMaterial(ForceModel::getAluminum7075());      break;
            case 1: m_forceModel.setMaterial(ForceModel::getSteel4140());         break;
            case 2: m_forceModel.setMaterial(ForceModel::getTitaniumTi6Al4V());   break;
            case 3: m_forceModel.setMaterial(ForceModel::getStainlessSteel316()); break;
        }
    }

    ImGui::Spacing();
    if (ImGui::Button("Apply Tool", ImVec2(-1, 0))) {
        currentTool.diameter   = static_cast<double>(m_diameter);
        currentTool.height     = static_cast<double>(m_height);
        currentTool.numFlutes  = m_numFlutes;
        currentTool.rpm        = static_cast<double>(m_rpm);
        currentTool.helixAngle = static_cast<double>(m_helixAngle);
        currentTool.rakeAngle  = static_cast<double>(m_rakeAngle);
    }

    ImGui::End();
}

void UIManager::renderStockPanel() {
    ImGui::SetNextWindowPos(ImVec2(10, 300), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(280, 260), ImGuiCond_FirstUseEver);
    ImGui::Begin("Stock Definition");

    ImGui::Text("Extents (mm)");
    ImGui::Separator();
    ImGui::DragFloat("X min", &m_stockMinX, 1.0f, -500.0f,   0.0f);
    ImGui::DragFloat("X max", &m_stockMaxX, 1.0f,   0.0f,  500.0f);
    ImGui::DragFloat("Y min", &m_stockMinY, 1.0f, -500.0f,   0.0f);
    ImGui::DragFloat("Y max", &m_stockMaxY, 1.0f,   0.0f,  500.0f);
    ImGui::DragFloat("Z min", &m_stockMinZ, 1.0f, -500.0f,   0.0f);
    ImGui::DragFloat("Z max", &m_stockMaxZ, 1.0f, -500.0f, 500.0f);

    ImGui::Spacing();
    ImGui::Text("Resolution");
    ImGui::Separator();
    ImGui::SliderFloat("Voxel size (mm)", &m_resolution, 0.1f, 5.0f);

    ImGui::Spacing();
    if (ImGui::Button("Apply Stock", ImVec2(-1, 0))) {
        Bounds b = {
            m_stockMinX, m_stockMaxX,
            m_stockMinY, m_stockMaxY,
            m_stockMinZ, m_stockMaxZ
        };
        m_grid.~VoxelGrid();
        new (&m_grid) VoxelGrid(b, static_cast<double>(m_resolution));
        m_totalVoxels = static_cast<uint64_t>(
            m_grid.getDimX()) * m_grid.getDimY() * m_grid.getDimZ();
        m_removedVoxels = 0;
        m_gcodeStatus = "Stock reset - reload G-code";
    }

    ImGui::End();
}

void UIManager::renderGCodePanel() {
    ImGui::SetNextWindowPos(ImVec2(10, 570), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(280, 140), ImGuiCond_FirstUseEver);
    ImGui::Begin("G-Code");

    ImGui::Text("File path:");
    ImGui::InputText("##path", m_gcodePathBuf, sizeof(m_gcodePathBuf));

    ImGui::Spacing();
    if (ImGui::Button("Load & Run", ImVec2(-1, 0))) {
        m_grid.reset();
        m_fxHistory.clear();
        m_fyHistory.clear();
        m_fzHistory.clear();
        m_timeHistory.clear();
        m_removedVoxels = 0;

        bool ok = m_parser.parseFile(std::string(m_gcodePathBuf), currentTool);
        if (ok) {
            m_gcodeStatus    = "Loaded successfully";
            simulationLoaded = true;
        } else {
            m_gcodeStatus    = "ERROR: file not found";
            simulationLoaded = false;
        }
    }

    ImGui::Spacing();
    ImGui::TextColored(
        simulationLoaded ? ImVec4(0,1,0,1) : ImVec4(1,0.4f,0.4f,1),
        "%s", m_gcodeStatus.c_str()
    );

    ImGui::End();
}

void UIManager::renderSimulationPanel() {
    ImGui::SetNextWindowPos(ImVec2(300, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(200, 120), ImGuiCond_FirstUseEver);
    ImGui::Begin("Simulation");

    if (ImGui::Button(simulationRunning ? "Pause" : "Play", ImVec2(80, 0)))
        simulationRunning = !simulationRunning;

    ImGui::SameLine();
    if (ImGui::Button("Reset", ImVec2(80, 0))) {
        simulationRunning = false;
        simulationLoaded  = false;
        m_grid.reset();
        m_fxHistory.clear();
        m_fyHistory.clear();
        m_fzHistory.clear();
        m_timeHistory.clear();
        m_removedVoxels = 0;
        m_gcodeStatus   = "No file loaded";
    }

    ImGui::SliderFloat("Speed", &simulationSpeed, 0.1f, 10.0f);
    ImGui::End();
}

void UIManager::renderForceChart() {
    ImGui::SetNextWindowPos(ImVec2(300, 140), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(660, 300), ImGuiCond_FirstUseEver);
    ImGui::Begin("Cutting Forces");

    if (ImPlot::BeginPlot("##forces", ImVec2(-1, -1))) {
        ImPlot::SetupAxes("Time (s)", "Force (N)");
        if (!m_timeHistory.empty()) {
            ImPlot::SetNextLineStyle(ImVec4(0.9f, 0.2f, 0.2f, 1.0f));
            ImPlot::PlotLine("Fx", m_timeHistory.data(),
                             m_fxHistory.data(), (int)m_timeHistory.size());
            ImPlot::SetNextLineStyle(ImVec4(0.2f, 0.8f, 0.2f, 1.0f));
            ImPlot::PlotLine("Fy", m_timeHistory.data(),
                             m_fyHistory.data(), (int)m_timeHistory.size());
            ImPlot::SetNextLineStyle(ImVec4(0.2f, 0.4f, 0.9f, 1.0f));
            ImPlot::PlotLine("Fz", m_timeHistory.data(),
                             m_fzHistory.data(), (int)m_timeHistory.size());
        }
        ImPlot::EndPlot();
    }
    ImGui::End();
}

void UIManager::renderStatusBar() {
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(0, io.DisplaySize.y - 28));
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, 28));
    ImGui::Begin("##statusbar", nullptr,
        ImGuiWindowFlags_NoTitleBar  |
        ImGuiWindowFlags_NoResize    |
        ImGuiWindowFlags_NoMove      |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoBringToFrontOnFocus);

    double pct = m_totalVoxels > 0
        ? 100.0 * m_removedVoxels / m_totalVoxels : 0.0;

    ImGui::Text("Voxels removed: %llu / %llu  (%.1f%%)   |   Material: %s   |   %.1f FPS",
        (unsigned long long)m_removedVoxels,
        (unsigned long long)m_totalVoxels,
        pct,
        m_materialIdx == 0 ? "Al 7075"    :
        m_materialIdx == 1 ? "Steel 4140" :
        m_materialIdx == 2 ? "Ti-6Al-4V"  : "SS 316",
        io.Framerate);

    ImGui::End();
}

void UIManager::addForceData(double time, double fx, double fy, double fz) {
    if ((int)m_timeHistory.size() >= m_maxHistorySize) {
        m_timeHistory.erase(m_timeHistory.begin());
        m_fxHistory.erase(m_fxHistory.begin());
        m_fyHistory.erase(m_fyHistory.begin());
        m_fzHistory.erase(m_fzHistory.begin());
    }
    m_timeHistory.push_back(static_cast<float>(time));
    m_fxHistory.push_back(static_cast<float>(fx));
    m_fyHistory.push_back(static_cast<float>(fy));
    m_fzHistory.push_back(static_cast<float>(fz));
    m_removedVoxels = m_totalVoxels - m_grid.getSolidCount();
}

} // namespace MathSim