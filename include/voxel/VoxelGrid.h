#ifndef VOXELGRID_H
#define VOXELGRID_H

#include "Common.h"
#include <vector>
#include <cstdint>

namespace MathSim {
    class VoxelGrid {
    public:
        VoxelGrid(Bounds bounds, float resolution);
        
        bool isSolid(int x, int y, int z) const;
        uint64_t subtractCylinder(Vector3 center, float radius, float height);
        
        Bounds getBounds() const { return m_bounds; }
        float getResolution() const { return m_resolution; }
        int getDimX() const { return m_dimX; }
        int getDimY() const { return m_dimY; }
        int getDimZ() const { return m_dimZ; }

    private:
        Bounds m_bounds;
        float m_resolution;
        int m_dimX, m_dimY, m_dimZ;
        std::vector<uint8_t> m_data; 
        size_t getIndex(int x, int y, int z) const;
    };
}

#endif // VOXELGRID_H