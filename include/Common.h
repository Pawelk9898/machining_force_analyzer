#ifndef COMMON_H
#define COMMON_H

#include <cmath>
#include <cstdint>

namespace MathSim {

    struct Vector3 {
        double x, y, z;
        Vector3(double _x = 0.0, double _y = 0.0, double _z = 0.0)
            : x(_x), y(_y), z(_z) {}
    };

    struct Bounds {
        double minX, maxX;
        double minY, maxY;
        double minZ, maxZ;
    };

    // Vector3 math operators
    inline Vector3 operator+(const Vector3& a, const Vector3& b) {
        return { a.x + b.x, a.y + b.y, a.z + b.z };
    }
    inline Vector3 operator-(const Vector3& a, const Vector3& b) {
        return { a.x - b.x, a.y - b.y, a.z - b.z };
    }
    inline Vector3 operator*(const Vector3& a, double s) {
        return { a.x * s, a.y * s, a.z * s };
    }
    inline double dot(const Vector3& a, const Vector3& b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }
    inline double length(const Vector3& v) {
        return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    }
    inline Vector3 normalize(const Vector3& v) {
        double len = length(v);
        if (len < 1e-10) return { 0, 0, 0 };
        return v * (1.0 / len);
    }

} // namespace MathSim

#endif