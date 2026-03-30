#include "force/ForceModel.h"
#include <cmath>
#include <algorithm>

namespace MathSim {

ForceModel::ForceModel() {
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

    double dFt = (m_coeffs.ktc * chipThickness + m_coeffs.kte) * axialDepth;
    double dFr = (m_coeffs.krc * chipThickness + m_coeffs.kre) * axialDepth;
    double dFa = (m_coeffs.kac * chipThickness + m_coeffs.kae) * axialDepth;

    double cosA = std::cos(angleRad);
    double sinA = std::sin(angleRad);

    forces.x = -dFt * cosA + dFr * sinA;
    forces.y =  dFt * sinA + dFr * cosA;
    forces.z =  dFa;

    return forces;
}

bool ForceModel::isEngaged(double angleRad,
                            double entryAngle,
                            double exitAngle) const {
    constexpr double TWO_PI = 6.28318530717958647;
    double a = std::fmod(angleRad, TWO_PI);
    if (a < 0) a += TWO_PI;
    if (entryAngle <= exitAngle)
        return a >= entryAngle && a <= exitAngle;
    else
        return a >= entryAngle || a <= exitAngle;
}

CuttingCoefficients ForceModel::getAluminum7075() {
    return { 796.0, 362.0, 235.0, 18.0, 9.0, 6.0, "Aluminum 7075" };
}

CuttingCoefficients ForceModel::getSteel4140() {
    return { 1800.0, 900.0, 450.0, 35.0, 18.0, 10.0, "Steel 4140" };
}

CuttingCoefficients ForceModel::getTitaniumTi6Al4V() {
    return { 2100.0, 1050.0, 525.0, 45.0, 22.0, 12.0, "Titanium Ti-6Al-4V" };
}

CuttingCoefficients ForceModel::getStainlessSteel316() {
    return { 2000.0, 980.0, 490.0, 40.0, 20.0, 11.0, "Stainless Steel 316" };
}

} // namespace MathSim