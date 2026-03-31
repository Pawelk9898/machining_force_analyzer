#include "gcode/GCodeParser.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace MathSim {

GCodeParser::GCodeParser() {}

bool GCodeParser::parseFile(const std::string& filePath) {
    m_moves.clear();
    m_currentPos   = { 0.0, 0.0, 0.0 };
    m_currentFeed  = 100.0;
    m_spindleSpeed = 0.0;
    m_motionMode   = 0;
    m_absoluteMode = true;
    m_metricMode   = true;
    m_spindleOn    = false;

    std::ifstream file(filePath);
    if (!file.is_open()) return false;

    std::string line;
    while (std::getline(file, line)) {
        processLine(line);
    }
    return true;
}

void GCodeParser::processLine(const std::string& line) {
    // Strip semicolon comments and uppercase
    std::string clean = line.substr(0, line.find(';'));
    std::transform(clean.begin(), clean.end(), clean.begin(), ::toupper);
    if (clean.empty() || clean[0] == '(' || clean[0] == '%') return;

    // Units
    if (clean.find("G20") != std::string::npos) m_metricMode = false;
    if (clean.find("G21") != std::string::npos) m_metricMode = true;

    // Positioning mode
    if (clean.find("G90") != std::string::npos) m_absoluteMode = true;
    if (clean.find("G91") != std::string::npos) m_absoluteMode = false;

    // Spindle
    if (clean.find("M3")  != std::string::npos ||
        clean.find("M4")  != std::string::npos) m_spindleOn = true;
    if (clean.find("M5")  != std::string::npos) m_spindleOn = false;
    if (clean.find("M30") != std::string::npos ||
        clean.find("M2")  != std::string::npos) return;

    if (hasKey(clean, 'S'))
        m_spindleSpeed = parseValue(clean, 'S', m_spindleSpeed);

    // Motion mode
    if      (clean.find("G00") != std::string::npos ||
             clean.find("G0 ") != std::string::npos) m_motionMode = 0;
    else if (clean.find("G01") != std::string::npos ||
             clean.find("G1 ") != std::string::npos) m_motionMode = 1;
    else if (clean.find("G02") != std::string::npos ||
             clean.find("G2 ") != std::string::npos) m_motionMode = 2;
    else if (clean.find("G03") != std::string::npos ||
             clean.find("G3 ") != std::string::npos) m_motionMode = 3;

    // Feedrate
    if (hasKey(clean, 'F'))
        m_currentFeed = parseValue(clean, 'F', m_currentFeed);

    // Target coordinates
    Vector3 nextPos = m_currentPos;
    if (m_absoluteMode) {
        if (hasKey(clean, 'X')) nextPos.x = parseValue(clean, 'X', m_currentPos.x);
        if (hasKey(clean, 'Y')) nextPos.y = parseValue(clean, 'Y', m_currentPos.y);
        if (hasKey(clean, 'Z')) nextPos.z = parseValue(clean, 'Z', m_currentPos.z);
    } else {
        if (hasKey(clean, 'X')) nextPos.x = m_currentPos.x + parseValue(clean, 'X', 0.0);
        if (hasKey(clean, 'Y')) nextPos.y = m_currentPos.y + parseValue(clean, 'Y', 0.0);
        if (hasKey(clean, 'Z')) nextPos.z = m_currentPos.z + parseValue(clean, 'Z', 0.0);
    }

    // Inch to mm conversion
    if (!m_metricMode) {
        nextPos.x *= 25.4;
        nextPos.y *= 25.4;
        nextPos.z *= 25.4;
    }

    // Build move and push to list
    Move m;
    m.target      = nextPos;
    m.feedRate    = m_currentFeed;
    m.spindleOn   = m_spindleOn;
    m.spindleSpeed = m_spindleSpeed;

    switch (m_motionMode) {
        case 0: m.type = Move::Type::RAPID;   break;
        case 1: m.type = Move::Type::LINEAR;  break;
        case 2: m.type = Move::Type::ARC_CW;  break;
        case 3: m.type = Move::Type::ARC_CCW; break;
    }

    // Only push if position actually changed
    if (nextPos.x != m_currentPos.x ||
        nextPos.y != m_currentPos.y ||
        nextPos.z != m_currentPos.z) {
        m_moves.push_back(m);
    }

    m_currentPos = nextPos;
}

double GCodeParser::parseValue(const std::string& line, char key, double defaultValue) {
    size_t pos = line.find(key);
    if (pos == std::string::npos) return defaultValue;
    try {
        size_t end;
        return std::stod(line.substr(pos + 1), &end);
    } catch (...) {
        return defaultValue;
    }
}

bool GCodeParser::hasKey(const std::string& line, char key) {
    return line.find(key) != std::string::npos;
}

} // namespace MathSim