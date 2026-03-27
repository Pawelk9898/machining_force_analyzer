#pragma once
#include <cmath>
#include <vector>

namespace MathSim {

struct ForceVector {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    double magnitude = 0.0;
};

// Altintas Mechanistic Coefficients for a specific Material
struct CuttingCoefficients {
    // Tangential, Radial, Axial Cutting Coefficients (N/mm^2)
    double ktc, krc, kac;
    // Edge Coefficients (N/mm) - Represents rubbing/friction
    double kte, kre, kae;
};

class ForceModel {
public:
    ForceModel();

    // The core Altintas calculation
    // chipThickness (h) is in mm, axialDepth (b) is in mm
    ForceVector calculateInstantaneousForce(double chipThickness, double axialDepth, double angleRad);

    // Helper to get presets
    static CuttingCoefficients getAluminum7075();
    static CuttingCoefficients getSteel4140();

private:
    CuttingCoefficients m_coeffs;
};

} // namespace MathSim