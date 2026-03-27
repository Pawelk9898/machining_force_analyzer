#include "gcode/GCodeParser.h"
#include <fstream>
#include <sstream>
#include <algorithm>

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
    if (line.empty() || line[0] == '(' || line[0] == '%') return;

    // 1. Update Motion Mode (G0 vs G1)
    if (line.find("G0") != std::string::npos) m_motionMode = 0;
    if (line.find("G1") != std::string::npos) m_motionMode = 1;

    // 2. Capture New Coordinates (only if they exist on this line)
    Vector3 nextPos = m_currentPos;
    if (hasKey(line, 'X')) nextPos.x = parseValue(line, 'X', m_currentPos.x);
    if (hasKey(line, 'Y')) nextPos.y = parseValue(line, 'Y', m_currentPos.y);
    if (hasKey(line, 'Z')) nextPos.z = parseValue(line, 'Z', m_currentPos.z);
    if (hasKey(line, 'F')) m_currentFeed = parseValue(line, 'F', m_currentFeed);

    // 3. Execute the move if it's a cutting move (G1)
    if (m_motionMode == 1) {
        // This triggers the voxel subtraction we wrote earlier
        m_engine.executeG1(m_currentPos, nextPos, tool, m_currentFeed);
    }

    // 4. Update the current "last known" position
    m_currentPos = nextPos;
}

double GCodeParser::parseValue(const std::string& line, char key, double defaultValue) {
    size_t pos = line.find(key);
    if (pos == std::string::npos) return defaultValue;
    
    try {
        return std::stod(line.substr(pos + 1));
    } catch (...) {
        return defaultValue;
    }
}

bool GCodeParser::hasKey(const std::string& line, char key) {
    return line.find(key) != std::string::npos;
}

} // namespace MathSim