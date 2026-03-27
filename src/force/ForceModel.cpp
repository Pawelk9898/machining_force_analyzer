#include "force/ForceModel.h"
namespace MathSim {

ForceModel::ForceModel() {
    m_coeffs = getAluminum7075(); // Default to Aluminum
}

ForceVector ForceModel::calculateInstantaneousForce(double h, double b, double phi) {
    ForceVector f;

    if (h <= 0) return f; // No engagement, no force

    // 1. Calculate Differential Forces (Tangential, Radial, Axial)
    double dFt = (m_coeffs.ktc * h * b) + (m_coeffs.kte * b);
    double dFr = (m_coeffs.krc * h * b) + (m_coeffs.kre * b);
    double dFa = (m_coeffs.kac * h * b) + (m_coeffs.kae * b);

    // 2. Resolve to Global X and Y Coordinates (Altintas Transformation)
    // Fx = -Ft*cos(phi) - Fr*sin(phi)
    // Fy =  Ft*sin(phi) - Fr*cos(phi)
    f.x = -dFt * std::cos(phi) - dFr * std::sin(phi);
    f.y =  dFt * std::sin(phi) - dFr * std::cos(phi);
    f.z = -dFa; // Pushing up against the spindle

    f.magnitude = std::sqrt(f.x*f.x + f.y*f.y + f.z*f.z);

    return f;
}

CuttingCoefficients ForceModel::getAluminum7075() {
    // Standard mechanistic values for Aluminum
    return { 700.0, 210.0, 150.0, 20.0, 10.0, 5.0 };
}

CuttingCoefficients ForceModel::getSteel4140() {
    // Significantly higher coefficients for Steel
    return { 2200.0, 800.0, 600.0, 50.0, 30.0, 15.0 };
}

} // namespace MathSim