#include "ui/UIManager.h"
#include "imgui.h"
#include "implot.h"
#include <algorithm>

namespace MathSim {

// Teal accent color
static const ImVec4 kTeal     = ImVec4(0.000f, 0.737f, 0.831f, 1.0f);
static const ImVec4 kTealDark = ImVec4(0.000f, 0.200f, 0.230f, 1.0f);
static const ImVec4 kDimText  = ImVec4(0.290f, 0.322f, 0.408f, 1.0f);
static const ImVec4 kTile     = ImVec4(0.075f, 0.082f, 0.110f, 1.0f);
static const ImVec4 kBorder   = ImVec4(0.180f, 0.200f, 0.251f, 1.0f);

UIManager::UIManager(VoxelGrid& grid, Simulator& simulator,
                     ForceModel& forceModel, GCodeParser& parser)
    : m_grid(grid)
    , m_simulator(simulator)
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

// Helper: section label
static void SectionLabel(const char* label) {
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.290f, 0.322f, 0.408f, 1.0f));
    ImGui::TextUnformatted(label);
    ImGui::PopStyleColor();
    ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.180f, 0.200f, 0.251f, 1.0f));
    ImGui::Separator();
    ImGui::PopStyleColor();
    ImGui::Spacing();
}

// Helper: value tile (label + big number)
static void ValueTile(const char* label, const char* value, const char* unit = "") {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.075f, 0.082f, 0.110f, 1.0f));
    ImGui::BeginChild(label, ImVec2(0, 52), true);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.290f, 0.322f, 0.408f, 1.0f));
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4);
    ImGui::TextUnformatted(label);
    ImGui::PopStyleColor();
    ImGui::SameLine(0, 0);
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ImGui::CalcTextSize(unit).x - 10);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.290f, 0.322f, 0.408f, 1.0f));
    ImGui::TextUnformatted(unit);
    ImGui::PopStyleColor();
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    ImGui::TextUnformatted(value);
    ImGui::PopStyleVar();
    ImGui::EndChild();
    ImGui::PopStyleColor();
}

void UIManager::renderToolPanel() {
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(272, 370), ImGuiCond_FirstUseEver);
    ImGui::Begin("Tool");

    SectionLabel("GEOMETRY");

    // Two-column input grid
    float colW = (ImGui::GetContentRegionAvail().x - 8) * 0.5f;

    ImGui::PushItemWidth(colW);
    ImGui::InputFloat("##dia",  &m_diameter,   0, 0, "%.1f mm");
    ImGui::SameLine(0, 8);
    ImGui::InputFloat("##ht",   &m_height,     0, 0, "%.1f mm");

    ImGui::InputInt  ("##fl",   &m_numFlutes);
    ImGui::SameLine(0, 8);
    ImGui::InputFloat("##rpm",  &m_rpm,        0, 0, "%.0f rpm");

    ImGui::InputFloat("##hx",  &m_helixAngle,  0, 0, "%.1f deg");
    ImGui::SameLine(0, 8);
    ImGui::InputFloat("##rk",  &m_rakeAngle,   0, 0, "%.1f deg");
    ImGui::PopItemWidth();

    SectionLabel("MATERIAL");

    const char* matNames[4] = { "Al 7075", "Steel 4140", "Ti-6Al-4V", "SS 316" };
    float pillW = (ImGui::GetContentRegionAvail().x - 8) / 2.0f - 2.0f;

    for (int i = 0; i < 4; ++i) {
        if (i % 2 != 0) ImGui::SameLine(0, 5);
        bool selected = (m_materialIdx == i);
        if (selected) {
            ImGui::PushStyleColor(ImGuiCol_Button,        kTeal);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, kTeal);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  kTeal);
            ImGui::PushStyleColor(ImGuiCol_Text,          kTealDark);
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button,        kTile);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.10f, 0.11f, 0.14f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  kTile);
            ImGui::PushStyleColor(ImGuiCol_Text,          kDimText);
        }
        if (ImGui::Button(matNames[i], ImVec2(pillW, 28))) {
            m_materialIdx = i;
            switch (i) {
                case 0: m_forceModel.setMaterial(ForceModel::getAluminum7075());      break;
                case 1: m_forceModel.setMaterial(ForceModel::getSteel4140());         break;
                case 2: m_forceModel.setMaterial(ForceModel::getTitaniumTi6Al4V());   break;
                case 3: m_forceModel.setMaterial(ForceModel::getStainlessSteel316()); break;
            }
        }
        ImGui::PopStyleColor(4);
    }

    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Button,        kTeal);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.10f, 0.85f, 0.95f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.00f, 0.60f, 0.70f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text,          kTealDark);
    if (ImGui::Button("APPLY TOOL", ImVec2(-1, 30))) {
        currentTool.diameter   = static_cast<double>(m_diameter);
        currentTool.height     = static_cast<double>(m_height);
        currentTool.numFlutes  = m_numFlutes;
        currentTool.rpm        = static_cast<double>(m_rpm);
        currentTool.helixAngle = static_cast<double>(m_helixAngle);
        currentTool.rakeAngle  = static_cast<double>(m_rakeAngle);
    }
    ImGui::PopStyleColor(4);
    ImGui::End();
}

void UIManager::renderStockPanel() {
    ImGui::SetNextWindowPos(ImVec2(10, 390), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(272, 260), ImGuiCond_FirstUseEver);
    ImGui::Begin("Stock");

    SectionLabel("EXTENTS (mm)");

    float colW = (ImGui::GetContentRegionAvail().x - 8) * 0.5f;
    ImGui::PushItemWidth(colW);

    ImGui::InputFloat("##xmn", &m_stockMinX, 0, 0, "X  %.0f");
    ImGui::SameLine(0, 8);
    ImGui::InputFloat("##xmx", &m_stockMaxX, 0, 0, "%.0f");

    ImGui::InputFloat("##ymn", &m_stockMinY, 0, 0, "Y  %.0f");
    ImGui::SameLine(0, 8);
    ImGui::InputFloat("##ymx", &m_stockMaxY, 0, 0, "%.0f");

    ImGui::InputFloat("##zmn", &m_stockMinZ, 0, 0, "Z  %.0f");
    ImGui::SameLine(0, 8);
    ImGui::InputFloat("##zmx", &m_stockMaxZ, 0, 0, "%.0f");
    ImGui::PopItemWidth();

    SectionLabel("RESOLUTION");
    ImGui::SetNextItemWidth(-1);
    ImGui::InputFloat("##res", &m_resolution, 0, 0, "%.1f mm/vox");

    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Button,        kTeal);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.10f, 0.85f, 0.95f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.00f, 0.60f, 0.70f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text,          kTealDark);
    if (ImGui::Button("APPLY STOCK", ImVec2(-1, 30))) {
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
        m_gcodeStatus   = "Stock reset";
        stockChanged    = true;
    }
    ImGui::PopStyleColor(4);
    ImGui::End();
}

void UIManager::renderGCodePanel() {
    ImGui::SetNextWindowPos(ImVec2(10, 660), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(272, 110), ImGuiCond_FirstUseEver);
    ImGui::Begin("G-Code");

    ImGui::SetNextItemWidth(-1);
    ImGui::InputText("##path", m_gcodePathBuf, sizeof(m_gcodePathBuf));
    ImGui::Spacing();

    ImGui::PushStyleColor(ImGuiCol_Button,        kTeal);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.10f, 0.85f, 0.95f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.00f, 0.60f, 0.70f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text,          kTealDark);
    if (ImGui::Button("LOAD", ImVec2(-1, 30))) {
        bool ok = m_parser.parseFile(std::string(m_gcodePathBuf));
        if (ok && !m_parser.getMoves().empty()) {
            m_simulator.loadMoves(m_parser.getMoves(), currentTool);
            m_fxHistory.clear();
            m_fyHistory.clear();
            m_fzHistory.clear();
            m_timeHistory.clear();
            m_removedVoxels  = 0;
            m_gcodeStatus    = "Ready";
            simulationLoaded = true;
        } else {
            m_gcodeStatus    = "File not found";
            simulationLoaded = false;
        }
    }
    ImGui::PopStyleColor(4);

    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text,
        simulationLoaded
            ? ImVec4(0.00f, 0.74f, 0.43f, 1.0f)
            : ImVec4(0.80f, 0.25f, 0.25f, 1.0f));
    ImGui::TextUnformatted(m_gcodeStatus.c_str());
    ImGui::PopStyleColor();
    ImGui::End();
}

void UIManager::renderSimulationPanel() {
    ImGui::SetNextWindowPos(ImVec2(292, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(260, 100), ImGuiCond_FirstUseEver);
    ImGui::Begin("Simulation");

    // Play/Pause button
    ImGui::PushStyleColor(ImGuiCol_Button,        kTeal);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.10f, 0.85f, 0.95f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.00f, 0.60f, 0.70f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text,          kTealDark);
    if (ImGui::Button(m_simulator.isPlaying() ? "PAUSE" : "PLAY", ImVec2(110, 30))) {
        if (m_simulator.isPlaying()) m_simulator.pause();
        else                         m_simulator.play();
    }
    ImGui::PopStyleColor(4);

    ImGui::SameLine(0, 8);

    ImGui::PushStyleColor(ImGuiCol_Button,        kTile);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.10f, 0.11f, 0.14f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  kTile);
    ImGui::PushStyleColor(ImGuiCol_Text,          kDimText);
    if (ImGui::Button("RESET", ImVec2(110, 30))) {
        m_simulator.reset();
        m_fxHistory.clear();
        m_fyHistory.clear();
        m_fzHistory.clear();
        m_timeHistory.clear();
        m_removedVoxels = 0;
        stockChanged    = true;
    }
    ImGui::PopStyleColor(4);

    // Speed input
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, kDimText);
    ImGui::TextUnformatted("SPEED");
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-1);
    ImGui::SliderFloat("##spd", &simulationSpeed, 0.1f, 5.0f, "%.1fx");

    // Thin progress bar
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, kTeal);
    ImGui::PushStyleColor(ImGuiCol_FrameBg,       kTile);
    ImGui::ProgressBar(m_simulator.getProgress(), ImVec2(-1, 4), "");
    ImGui::PopStyleColor(2);

    ImGui::End();
}

void UIManager::renderForceChart() {
    ImGui::SetNextWindowPos(ImVec2(292, 120), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(700, 320), ImGuiCond_FirstUseEver);
    ImGui::Begin("Cutting Forces");

    ImPlot::PushStyleVar(ImPlotStyleVar_PlotPadding,  ImVec2(12, 12));
    ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight,   2.5f);
    ImPlot::PushStyleColor(ImPlotCol_PlotBg,     ImVec4(0.075f, 0.082f, 0.110f, 1.0f));
    ImPlot::PushStyleColor(ImPlotCol_PlotBorder, ImVec4(0.180f, 0.200f, 0.251f, 1.0f));
    ImPlot::PushStyleColor(ImPlotCol_FrameBg,    ImVec4(0.094f, 0.106f, 0.133f, 1.0f));
    ImPlot::PushStyleColor(ImPlotCol_AxisText,   ImVec4(0.290f, 0.322f, 0.408f, 1.0f));
    ImPlot::PushStyleColor(ImPlotCol_AxisText,   ImVec4(0.290f, 0.322f, 0.408f, 1.0f));
    ImPlot::PushStyleColor(ImPlotCol_AxisGrid,   ImVec4(0.180f, 0.200f, 0.251f, 0.8f));
    ImPlot::PushStyleColor(ImPlotCol_AxisGrid,   ImVec4(0.180f, 0.200f, 0.251f, 0.8f));

    if (ImPlot::BeginPlot("##forces", ImVec2(-1, -1))) {
        ImPlot::SetupAxes("Time (s)", "Force (kN)");
        ImPlot::SetupAxisLimits(ImAxis_Y1, -2.0, 2.0, ImGuiCond_Once);
        ImPlot::SetupAxisFormat(ImAxis_Y1, "%.2f");

        if (!m_timeHistory.empty()) {
            std::vector<float> fxkN(m_fxHistory.size());
            std::vector<float> fykN(m_fyHistory.size());
            std::vector<float> fzkN(m_fzHistory.size());
            for (size_t i = 0; i < m_fxHistory.size(); ++i) {
                fxkN[i] = m_fxHistory[i] / 1000.0f;
                fykN[i] = m_fyHistory[i] / 1000.0f;
                fzkN[i] = m_fzHistory[i] / 1000.0f;
            }
            ImPlot::SetNextLineStyle(ImVec4(0.95f, 0.35f, 0.35f, 1.0f));
            ImPlot::PlotLine("Fx", m_timeHistory.data(), fxkN.data(), (int)m_timeHistory.size());
            ImPlot::SetNextLineStyle(ImVec4(0.30f, 0.85f, 0.50f, 1.0f));
            ImPlot::PlotLine("Fy", m_timeHistory.data(), fykN.data(), (int)m_timeHistory.size());
            ImPlot::SetNextLineStyle(ImVec4(0.30f, 0.60f, 0.98f, 1.0f));
            ImPlot::PlotLine("Fz", m_timeHistory.data(), fzkN.data(), (int)m_timeHistory.size());
        }
        ImPlot::EndPlot();
    }
    ImPlot::PopStyleColor(7);
    ImPlot::PopStyleVar(2);
    ImGui::End();
}

void UIManager::renderStatusBar() {
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(0, io.DisplaySize.y - 28));
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, 28));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.094f, 0.106f, 0.133f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Border,   ImVec4(0.180f, 0.200f, 0.251f, 1.0f));
    ImGui::Begin("##statusbar", nullptr,
        ImGuiWindowFlags_NoTitleBar  |
        ImGuiWindowFlags_NoResize    |
        ImGuiWindowFlags_NoMove      |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoBringToFrontOnFocus);

    double pct = m_totalVoxels > 0
        ? 100.0 * m_removedVoxels / m_totalVoxels : 0.0;

    ImGui::PushStyleColor(ImGuiCol_Text, kDimText);
    ImGui::Text("VOXELS  ");
    ImGui::PopStyleColor();
    ImGui::SameLine(0, 0);
    ImGui::Text("%llu / %llu", (unsigned long long)m_removedVoxels,
                               (unsigned long long)m_totalVoxels);
    ImGui::SameLine(0, 20);
    ImGui::PushStyleColor(ImGuiCol_Text, kTeal);
    ImGui::Text("%.1f%%", pct);
    ImGui::PopStyleColor();
    ImGui::SameLine(0, 20);
    ImGui::PushStyleColor(ImGuiCol_Text, kDimText);
    ImGui::Text("MATERIAL  ");
    ImGui::PopStyleColor();
    ImGui::SameLine(0, 0);
    const char* matNames[4] = { "Al 7075", "Steel 4140", "Ti-6Al-4V", "SS 316" };
    ImGui::Text("%s", matNames[m_materialIdx]);
    ImGui::SameLine(0, 20);
    ImGui::PushStyleColor(ImGuiCol_Text, kDimText);
    ImGui::Text("FPS  ");
    ImGui::PopStyleColor();
    ImGui::SameLine(0, 0);
    ImGui::Text("%.0f", io.Framerate);

    ImGui::End();
    ImGui::PopStyleColor(2);
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