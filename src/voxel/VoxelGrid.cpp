#include "voxel/VoxelGrid.h"
#include <cmath>
#include <algorithm>
#include <numeric>

namespace MathSim {

VoxelGrid::VoxelGrid(Bounds bounds, double resolution)
    : m_bounds(bounds), m_resolution(resolution)
{
    m_dimX = static_cast<int>(std::ceil((bounds.maxX - bounds.minX) / resolution));
    m_dimY = static_cast<int>(std::ceil((bounds.maxY - bounds.minY) / resolution));
    m_dimZ = static_cast<int>(std::ceil((bounds.maxZ - bounds.minZ) / resolution));

    m_data.assign(static_cast<size_t>(m_dimX) * m_dimY * m_dimZ, 1); // 1 = solid
}

size_t VoxelGrid::getIndex(int x, int y, int z) const {
    return static_cast<size_t>(x) * m_dimY * m_dimZ
         + static_cast<size_t>(y) * m_dimZ
         + z;
}

bool VoxelGrid::isSolid(int x, int y, int z) const {
    if (x < 0 || x >= m_dimX || y < 0 || y >= m_dimY || z < 0 || z >= m_dimZ)
        return false;
    return m_data[getIndex(x, y, z)] == 1;
}

uint64_t VoxelGrid::subtractCylinder(Vector3 center, double radius, double height) {
    uint64_t removedCount = 0;

    int minX = std::max(0, static_cast<int>((center.x - radius - m_bounds.minX) / m_resolution));
    int maxX = std::min(m_dimX - 1, static_cast<int>((center.x + radius - m_bounds.minX) / m_resolution));
    int minY = std::max(0, static_cast<int>((center.y - radius - m_bounds.minY) / m_resolution));
    int maxY = std::min(m_dimY - 1, static_cast<int>((center.y + radius - m_bounds.minY) / m_resolution));
    int minZ = std::max(0, static_cast<int>((center.z - m_bounds.minZ) / m_resolution));
    int maxZ = std::min(m_dimZ - 1, static_cast<int>((center.z + height - m_bounds.minZ) / m_resolution));

    for (int x = minX; x <= maxX; ++x) {
        for (int y = minY; y <= maxY; ++y) {
            double dx = (x * m_resolution + m_bounds.minX) - center.x;
            double dy = (y * m_resolution + m_bounds.minY) - center.y;
            if (dx*dx + dy*dy <= radius*radius) {
                for (int z = minZ; z <= maxZ; ++z) {
                    size_t idx = getIndex(x, y, z);
                    if (m_data[idx] == 1) {
                        m_data[idx] = 0;
                        removedCount++;
                    }
                }
            }
        }
    }
    return removedCount;
}

void VoxelGrid::reset() {
    m_data.assign(m_data.size(), 1); // restore all voxels to solid
}

uint64_t VoxelGrid::getSolidCount() const {
    uint64_t count = 0;
    for (auto v : m_data) count += v;
    return count;
}

} // namespace MathSim