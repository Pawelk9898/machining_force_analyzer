#pragma once
#include <string>
#include <vector>
#include "Common.h"
#include "simulator/Simulator.h"

namespace MathSim {

class GCodeParser {
public:
    GCodeParser();

    // Parse file into move list — does NOT execute anything
    bool parseFile(const std::string& filePath);

    // Get parsed moves for the Simulator
    const std::vector<Move>& getMoves() const { return m_moves; }

private:
    void processLine(const std::string& line);
    double parseValue(const std::string& line, char key, double defaultValue);
    bool   hasKey(const std::string& line, char key);

    std::vector<Move> m_moves;

    // Modal state
    Vector3 m_currentPos   = { 0.0, 0.0, 0.0 };
    double  m_currentFeed  = 100.0;
    double  m_spindleSpeed = 0.0;
    int     m_motionMode   = 0;
    bool    m_absoluteMode = true;
    bool    m_metricMode   = true;
    bool    m_spindleOn    = false;
};

} // namespace MathSim