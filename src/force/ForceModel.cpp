#include "force/ForceModel.h"
#include <cmath>
#include <algorithm>

namespace MathSim {

ForceModel::ForceModel() {
    // Default to Aluminum 7075 — most common in machining demos
    m_coeffs = getAluminum7075();
}

void ForceModel::setMaterial(const CuttingCoefficients& coeffs) {
    m_coeffs = coeffs;
}

ForceVector ForceModel::calculateInstantaneousForce(double chipThickness,
                                                     double axialDepth,
                                                     double angleRad) const {
    ForceVector forces;

    if (chipThickness < 1e-9 || axialDepth < 1e-9) return forces;

    // --- Altintas Mechanistic Force Model ---
    // Tangential, radial, and axial cutting force components per flute:
    // dFt = (Ktc * h + Kte) * b
    // dFr = (Krc * h + Kre) * b
    // dFa = (Kac * h + Kae) * b
    // where h = chip thickness, b = axial depth of cut

    double dFt = (m_coeffs.ktc * chipThickness + m_coeffs.kte) * axialDepth;
    double dFr = (m_coeffs.krc * chipThickness + m_coeffs.kre) * axialDepth;
    double dFa = (m_coeffs.kac * chipThickness + m_coeffs.kae) * axialDepth;

    // --- Transform from tool frame to machine frame ---
    // Ft and Fr are in the cutting plane, rotated by engagement angle
    // Fx = feed direction, Fy = cross-feed direction
    double cosA = std::cos(angleRad);
    double sinA = std::sin(angleRad);

    forces.x = -dFt * cosA + dFr * sinA; // Fx
    forces.y =  dFt * sinA + dFr * cosA; // Fy
    forces.z =  dFa;                      // Fz (axial, straight up)

    return forces;
}

bool ForceModel::isEngaged(double angleRad,
                            double entryAngle,
                            double exitAngle) const {
    // Normalize angle to [0, 2*pi]
    double twoPi = 2.0 * M_PI;
    double a = std::fmod(angleRad, twoPi);
    if (a < 0) a += twoPi;

    if (entryAngle <= exitAngle) {
        return a >= entryAngle && a <= exitAngle;
    } else {
        // Wraps around 2*pi
        return a >= entryAngle || a <= exitAngle;
    }
}

// --- Material Presets ---
// Coefficients from Altintas "Manufacturing Automation" textbook

CuttingCoefficients ForceModel::getAluminum7075() {
    return {
        796.0,  // Ktc (N/mm²)
        362.0,  // Krc
        235.0,  // Kac
        18.0,   // Kte (N/mm)
        9.0,    // Kre
        6.0,    // Kae
        "Aluminum 7075"
    };
}

CuttingCoefficients ForceModel::getSteel4140() {
    return {
        1800.0, // Ktc
        900.0,  // Krc
        450.0,  // Kac
        35.0,   // Kte
        18.0,   // Kre
        10.0,   // Kae
        "Steel 4140"
    };
}

CuttingCoefficients ForceModel::getTitaniumTi6Al4V() {
    return {
        2100.0, // Ktc
        1050.0, // Krc
        525.0,  // Kac
        45.0,   // Kte
        22.0,   // Kre
        12.0,   // Kae
        "Titanium Ti-6Al-4V"
    };
}

CuttingCoefficients ForceModel::getStainlessSteel316() {
    return {
        2000.0, // Ktc
        980.0,  // Krc
        490.0,  // Kac
        40.0,   // Kte
        20.0,   // Kre
        11.0,   // Kae
        "Stainless Steel 316"
    };
}

} // namespace MathSim