#pragma once

#include "Core/Types.h"
#include "Terrain/Heightfield.h"
#include "Texture.h"

namespace Terrain {

struct AmbientOcclusionParams {
    uint32 samples = 16;           // Number of samples per pixel
    float32 radius = 10.0f;        // Sampling radius in pixels
    float32 strength = 1.0f;       // AO strength multiplier
    float32 bias = 0.05f;          // Bias to prevent self-occlusion
    float32 heightScale = 1.0f;    // Height scale for occlusion calculation
};

class AmbientOcclusionGenerator {
public:
    AmbientOcclusionGenerator();
    ~AmbientOcclusionGenerator();

    // Generate ambient occlusion map from heightfield
    Unique<Texture> Generate(const Heightfield& heightfield, const AmbientOcclusionParams& params = AmbientOcclusionParams());

    // Get/set default parameters
    const AmbientOcclusionParams& GetParams() const { return m_Params; }
    void SetParams(const AmbientOcclusionParams& params) { m_Params = params; }

private:
    float32 CalculateOcclusion(const Heightfield& heightfield, uint32 x, uint32 y, const AmbientOcclusionParams& params);

    AmbientOcclusionParams m_Params;
};

} // namespace Terrain
