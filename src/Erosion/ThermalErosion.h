#pragma once

#include "Core/Types.h"
#include "Terrain/Heightfield.h"

namespace Terrain {

struct ThermalErosionParams {
    int32 iterations = 10;              // Number of passes
    float32 talusAngle = 0.7f;          // Angle of repose (in radians, ~40 degrees)
    float32 strength = 0.5f;            // Erosion strength (0-1)
};

class ThermalErosion {
public:
    ThermalErosion();
    ~ThermalErosion();

    // Apply thermal erosion to heightfield (CPU-based)
    bool Erode(Heightfield& heightfield, const ThermalErosionParams& params);

    // Get/set parameters
    const ThermalErosionParams& GetParams() const { return m_Params; }
    void SetParams(const ThermalErosionParams& params) { m_Params = params; }

private:
    void ErodePass(Heightfield& heightfield, Heightfield& delta, float32 talusAngle, float32 strength);

    ThermalErosionParams m_Params;
};

} // namespace Terrain
