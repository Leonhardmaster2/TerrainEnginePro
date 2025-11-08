#include "AmbientOcclusionGenerator.h"
#include "Core/Logger.h"
#include <glm/glm.hpp>
#include <cmath>
#include <algorithm>

namespace Terrain {

AmbientOcclusionGenerator::AmbientOcclusionGenerator() {
}

AmbientOcclusionGenerator::~AmbientOcclusionGenerator() {
}

Unique<Texture> AmbientOcclusionGenerator::Generate(const Heightfield& heightfield, const AmbientOcclusionParams& params) {
    uint32 width = heightfield.GetWidth();
    uint32 height = heightfield.GetHeight();

    // Create R8 texture for AO map
    auto texture = MakeUnique<Texture>(width, height, TextureFormat::R8);

    LOG_INFO("Generating ambient occlusion (%ux%u, %u samples)...", width, height, params.samples);

    // Calculate occlusion for each pixel
    for (uint32 y = 0; y < height; y++) {
        for (uint32 x = 0; x < width; x++) {
            float32 occlusion = CalculateOcclusion(heightfield, x, y, params);
            texture->SetPixel(x, y, occlusion, 0.0f, 0.0f, 1.0f);
        }

        // Progress logging every 10%
        if ((y % (height / 10)) == 0 && y > 0) {
            LOG_INFO("AO generation: %d%%", (y * 100) / height);
        }
    }

    LOG_INFO("Ambient occlusion generated successfully");
    return texture;
}

float32 AmbientOcclusionGenerator::CalculateOcclusion(const Heightfield& heightfield, uint32 x, uint32 y, const AmbientOcclusionParams& params) {
    uint32 width = heightfield.GetWidth();
    uint32 height = heightfield.GetHeight();

    float32 centerHeight = heightfield.GetHeight(x, y);
    float32 occlusion = 0.0f;
    uint32 validSamples = 0;

    // Sample in a circle around the point
    for (uint32 i = 0; i < params.samples; i++) {
        float32 angle = (static_cast<float32>(i) / static_cast<float32>(params.samples)) * 2.0f * 3.14159265f;
        float32 distance = params.radius;

        // Calculate sample position
        int32 sampleX = static_cast<int32>(x + std::cos(angle) * distance);
        int32 sampleY = static_cast<int32>(y + std::sin(angle) * distance);

        // Check bounds
        if (sampleX < 0 || sampleX >= static_cast<int32>(width) ||
            sampleY < 0 || sampleY >= static_cast<int32>(height)) {
            continue;
        }

        // Get sample height
        float32 sampleHeight = heightfield.GetHeight(sampleX, sampleY);

        // Calculate height difference
        float32 heightDiff = (sampleHeight - centerHeight) * params.heightScale;

        // Calculate horizon angle
        float32 horizonAngle = std::atan2(heightDiff, distance);

        // Accumulate occlusion (higher nearby points occlude more)
        if (horizonAngle > params.bias) {
            occlusion += horizonAngle;
            validSamples++;
        }
    }

    // Average and normalize
    if (validSamples > 0) {
        occlusion /= static_cast<float32>(validSamples);
    }

    // Apply strength and clamp
    occlusion = 1.0f - (occlusion * params.strength);
    occlusion = std::clamp(occlusion, 0.0f, 1.0f);

    return occlusion;
}

} // namespace Terrain
