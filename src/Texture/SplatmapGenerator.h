#pragma once

#include "Core/Types.h"
#include "Terrain/Heightfield.h"
#include "Texture.h"

namespace Terrain {

// Material layer definition
struct MaterialLayer {
    String name;
    float32 heightMin = 0.0f;      // Minimum height for this material
    float32 heightMax = 1.0f;      // Maximum height for this material
    float32 slopeMin = 0.0f;       // Minimum slope (0-90 degrees)
    float32 slopeMax = 90.0f;      // Maximum slope (0-90 degrees)
    float32 blendRange = 0.1f;     // Blend range with other materials
    float32 noiseScale = 0.0f;     // Noise variation (0 = none, 1 = lots)
    uint32 seed = 12345;           // Random seed for noise
};

struct SplatmapParams {
    MaterialLayer layers[4];       // Up to 4 layers (RGBA channels)
    uint32 layerCount = 4;
    float32 heightScale = 1.0f;    // Height scale for calculations
};

class SplatmapGenerator {
public:
    SplatmapGenerator();
    ~SplatmapGenerator();

    // Generate splatmap from heightfield
    Unique<Texture> Generate(const Heightfield& heightfield, const SplatmapParams& params = SplatmapParams());

    // Create default mountain splatmap params
    static SplatmapParams CreateMountainPreset();
    static SplatmapParams CreateDesertPreset();
    static SplatmapParams CreateArcticPreset();

    // Get/set default parameters
    const SplatmapParams& GetParams() const { return m_Params; }
    void SetParams(const SplatmapParams& params) { m_Params = params; }

private:
    float32 CalculateSlope(const Heightfield& heightfield, uint32 x, uint32 y);
    float32 CalculateLayerWeight(const Heightfield& heightfield, uint32 x, uint32 y, const MaterialLayer& layer, float32 slope);
    float32 SmoothStep(float32 edge0, float32 edge1, float32 x);
    float32 SimpleNoise(uint32 x, uint32 y, uint32 seed);

    SplatmapParams m_Params;
};

} // namespace Terrain
