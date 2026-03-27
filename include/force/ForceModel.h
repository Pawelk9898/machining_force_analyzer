#pragma once
#include "Common.h"
#include <cmath>
#include <string>

namespace MathSim {

struct ForceVector {
    double x = 0.0; // Fx — feed direction (N)
    double y = 0.0; // Fy — cross-feed direction (N)
    double z = 0.0; // Fz — axial direction (N)

    double magnitude() const {
        return std::sqrt(x*x + y*y + z*z);
    }
};

// Altintas mechanistic cutting coefficients for a specific material
struct CuttingCoefficients {
    // Cutting force coefficients (N/mm²)
    double ktc; // Tangential
    double krc; // Radial
    double kac; // Axial

    // Edge force coefficients (N/mm) — rubbing/friction at cutting edge
    double kte;
    double kre;
    double kae;

    std::string materialName;
};

class ForceModel {
public:
    ForceModel();

    // Set material coefficients
    void setMaterial(const CuttingCoefficients& coeffs);

    // Core Altintas calculation
    // chipThickness (h) in mm, axialDepth (b) in mm, angleRad = engagement angle
    ForceVector calculateInstantaneousForce(double chipThickness,
                                            double axialDepth,
                                            double angleRad) const;

    // Check if a flute is engaged at a given angle
    bool isEngaged(double angleRad, double entryAngle, double exitAngle) const;

    // Material presets
    static CuttingCoefficients getAluminum7075();
    static CuttingCoefficients getSteel4140();
    static CuttingCoefficients getTitaniumTi6Al4V();
    static CuttingCoefficients getStainlessSteel316();

private:
    CuttingCoefficients m_coeffs;
};

} // namespace MathSim