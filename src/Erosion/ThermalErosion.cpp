#include "ThermalErosion.h"
#include "Core/Logger.h"
#include <algorithm>
#include <cmath>

namespace Terrain {

ThermalErosion::ThermalErosion() {
}

ThermalErosion::~ThermalErosion() {
}

bool ThermalErosion::Erode(Heightfield& heightfield, const ThermalErosionParams& params) {
    uint32 width = heightfield.GetWidth();
    uint32 height = heightfield.GetHeight();

    // Create delta buffer for accumulating changes
    Heightfield delta(width, height);

    for (int32 iteration = 0; iteration < params.iterations; iteration++) {
        // Clear delta
        for (uint32 y = 0; y < height; y++) {
            for (uint32 x = 0; x < width; x++) {
                delta.SetHeight(x, y, 0.0f);
            }
        }

        // Perform erosion pass
        ErodePass(heightfield, delta, params.talusAngle, params.strength);

        // Apply delta
        for (uint32 y = 0; y < height; y++) {
            for (uint32 x = 0; x < width; x++) {
                float h = heightfield.GetHeight(x, y);
                float d = delta.GetHeight(x, y);
                heightfield.SetHeight(x, y, h + d);
            }
        }
    }

    return true;
}

void ThermalErosion::ErodePass(Heightfield& heightfield, Heightfield& delta, float32 talusAngle, float32 strength) {
    uint32 width = heightfield.GetWidth();
    uint32 height = heightfield.GetHeight();

    // For each cell
    for (uint32 y = 1; y < height - 1; y++) {
        for (uint32 x = 1; x < width - 1; x++) {
            float centerHeight = heightfield.GetHeight(x, y);

            // Check all 8 neighbors
            float totalDiff = 0.0f;
            int32 numHigher = 0;

            struct Neighbor {
                int32 dx, dy;
                float height;
                float diff;
            };

            Neighbor neighbors[8] = {
                {-1, -1, 0.0f, 0.0f}, {0, -1, 0.0f, 0.0f}, {1, -1, 0.0f, 0.0f},
                {-1,  0, 0.0f, 0.0f},                       {1,  0, 0.0f, 0.0f},
                {-1,  1, 0.0f, 0.0f}, {0,  1, 0.0f, 0.0f}, {1,  1, 0.0f, 0.0f}
            };

            // Gather neighbor heights
            for (int32 i = 0; i < 8; i++) {
                int32 nx = x + neighbors[i].dx;
                int32 ny = y + neighbors[i].dy;
                neighbors[i].height = heightfield.GetHeight(nx, ny);
                neighbors[i].diff = centerHeight - neighbors[i].height;
            }

            // Find neighbors that are lower by more than talus angle
            for (int32 i = 0; i < 8; i++) {
                float distance = (neighbors[i].dx == 0 || neighbors[i].dy == 0) ? 1.0f : 1.414f; // diagonal
                float maxDiff = talusAngle * distance; // Maximum stable height difference

                if (neighbors[i].diff > maxDiff) {
                    totalDiff += neighbors[i].diff - maxDiff;
                    numHigher++;
                }
            }

            if (numHigher > 0) {
                // Material to move from center
                float materialToMove = totalDiff * strength / static_cast<float>(numHigher);

                // Distribute to lower neighbors
                for (int32 i = 0; i < 8; i++) {
                    float distance = (neighbors[i].dx == 0 || neighbors[i].dy == 0) ? 1.0f : 1.414f;
                    float maxDiff = talusAngle * distance;

                    if (neighbors[i].diff > maxDiff) {
                        int32 nx = x + neighbors[i].dx;
                        int32 ny = y + neighbors[i].dy;

                        float currentDelta = delta.GetHeight(nx, ny);
                        delta.SetHeight(nx, ny, currentDelta + materialToMove);

                        float centerDelta = delta.GetHeight(x, y);
                        delta.SetHeight(x, y, centerDelta - materialToMove);
                    }
                }
            }
        }
    }
}

} // namespace Terrain
