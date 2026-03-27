#include "gcode/GCodeParser.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace MathSim {

GCodeParser::GCodeParser(ToolpathEngine& engine) : m_engine(engine) {}

bool GCodeParser::parseFile(const std::string& filePath, const Tool& tool) {
    std::ifstream file(filePath);
    if (!file.is_open()) return false;

    std::string line;
    while (std::getline(file, line)) {
        processLine(line, tool);
    }
    return true;
}

void GCodeParser::processLine(const std::string& line, const Tool& tool) {
    // 1. Strip semicolon comments and uppercase the line
    std::string clean = line.substr(0, line.find(';'));
    std::transform(clean.begin(), clean.end(), clean.begin(), ::toupper);

    // 2. Skip empty lines, parenthesis comments, and program markers
    if (clean.empty() || clean[0] == '(' || clean[0] == '%') return;

    // 3. Update units
    if (clean.find("G20") != std::string::npos) m_metricMode = false;
    if (clean.find("G21") != std::string::npos) m_metricMode = true;

    // 4. Update positioning mode
    if (clean.find("G90") != std::string::npos) m_absoluteMode = true;
    if (clean.find("G91") != std::string::npos) m_absoluteMode = false;

    // 5. Update spindle state
    if (clean.find("M3") != std::string::npos || 
        clean.find("M4") != std::string::npos) m_spindleOn = true;
    if (clean.find("M5") != std::string::npos)  m_spindleOn = false;
    if (clean.find("M30") != std::string::npos || 
        clean.find("M2") != std::string::npos)  return; // end of program

    if (hasKey(clean, 'S')) m_spindleSpeed = parseValue(clean, 'S', m_spindleSpeed);

    // 6. Update motion mode — order matters, check G00/G01/G02/G03 first
    if (clean.find("G00") != std::string::npos ||
        clean.find("G0 ") != std::string::npos) m_motionMode = 0;
    else if (clean.find("G01") != std::string::npos ||
             clean.find("G1 ") != std::string::npos) m_motionMode = 1;
    else if (clean.find("G02") != std::string::npos ||
             clean.find("G2 ") != std::string::npos) m_motionMode = 2;
    else if (clean.find("G03") != std::string::npos ||
             clean.find("G3 ") != std::string::npos) m_motionMode = 3;

    // 7. Update feedrate
    if (hasKey(clean, 'F')) m_currentFeed = parseValue(clean, 'F', m_currentFeed);

    // 8. Parse target coordinates
    Vector3 nextPos = m_currentPos;

    if (m_absoluteMode) {
        if (hasKey(clean, 'X')) nextPos.x = parseValue(clean, 'X', m_currentPos.x);
        if (hasKey(clean, 'Y')) nextPos.y = parseValue(clean, 'Y', m_currentPos.y);
        if (hasKey(clean, 'Z')) nextPos.z = parseValue(clean, 'Z', m_currentPos.z);
    } else {
        // Incremental mode — add delta to current position
        if (hasKey(clean, 'X')) nextPos.x = m_currentPos.x + parseValue(clean, 'X', 0.0);
        if (hasKey(clean, 'Y')) nextPos.y = m_currentPos.y + parseValue(clean, 'Y', 0.0);
        if (hasKey(clean, 'Z')) nextPos.z = m_currentPos.z + parseValue(clean, 'Z', 0.0);
    }

    // 9. Convert inches to mm if needed
    if (!m_metricMode) {
        auto toMM = [&](double v) { return v * 25.4; };
        nextPos.x = toMM(nextPos.x);
        nextPos.y = toMM(nextPos.y);
        nextPos.z = toMM(nextPos.z);
    }

    // 10. Execute the move
    switch (m_motionMode) {
        case 0: // G0 — rapid, no cutting
            m_engine.executeG0(m_currentPos, nextPos);
            break;
        case 1: // G1 — linear cutting move
            if (m_spindleOn)
                m_engine.executeG1(m_currentPos, nextPos, tool, m_currentFeed);
            break;
        case 2: // G2/G3 — arc moves (placeholder for now)
        case 3:
            // TODO: implement arc interpolation
            break;
    }

    // 11. Update current position
    m_currentPos = nextPos;
}

double GCodeParser::parseValue(const std::string& line, char key, double defaultValue) {
    size_t pos = line.find(key);
    if (pos == std::string::npos) return defaultValue;

    try {
        size_t end;
        double val = std::stod(line.substr(pos + 1), &end);
        return val;
    } catch (...) {
        return defaultValue;
    }
}

bool GCodeParser::hasKey(const std::string& line, char key) {
    return line.find(key) != std::string::npos;
}

} // namespace MathSim