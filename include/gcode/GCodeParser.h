#pragma once
#include <string>
#include <vector>
#include "Common.h"
#include "toolpath/ToolpathEngine.h"

namespace MathSim {

class GCodeParser {
public:
    GCodeParser(ToolpathEngine& engine);

    // Main entry point: reads the .nc or .gcode file
    bool parseFile(const std::string& filePath, const Tool& tool);

private:
    // Internal helper to update the machine state line-by-line
    void processLine(const std::string& line, const Tool& tool);

    // Extraction helpers
    double parseValue(const std::string& line, char key, double defaultValue);
    bool   hasKey(const std::string& line, char key);

    ToolpathEngine& m_engine;

    // Modal state — persists across lines
    Vector3 m_currentPos    = { 0.0, 0.0, 0.0 };
    double  m_currentFeed   = 100.0;
    double  m_spindleSpeed  = 0.0;
    int     m_motionMode    = 0;    // 0=G0, 1=G1, 2=G2, 3=G3
    bool    m_absoluteMode  = true; // G90=true, G91=false
    bool    m_metricMode    = true; // G21=true, G20=false
    bool    m_spindleOn     = false;
};

} // namespace MathSim