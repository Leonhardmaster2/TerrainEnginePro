#include "SplatmapGenerator.h"
#include "Core/Logger.h"
#include <glm/glm.hpp>
#include <algorithm>
#include <cmath>

namespace Terrain {

SplatmapGenerator::SplatmapGenerator() {
    m_Params = CreateMountainPreset();
}

SplatmapGenerator::~SplatmapGenerator() {
}

Unique<Texture> SplatmapGenerator::Generate(const Heightfield& heightfield, const SplatmapParams& params) {
    uint32 width = heightfield.GetWidth();
    uint32 height = heightfield.GetHeight();

    // Create RGBA8 texture for splatmap (4 material layers)
    auto texture = MakeUnique<Texture>(width, height, TextureFormat::RGBA8);

    LOG_INFO("Generating splatmap (%ux%u, %u layers)...", width, height, params.layerCount);

    // Calculate weights for each pixel
    for (uint32 y = 0; y < height; y++) {
        for (uint32 x = 0; x < width; x++) {
            float32 slope = CalculateSlope(heightfield, x, y);
            float32 weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

            // Calculate weight for each layer
            for (uint32 i = 0; i < params.layerCount && i < 4; i++) {
                weights[i] = CalculateLayerWeight(heightfield, x, y, params.layers[i], slope);
            }

            // Normalize weights so they sum to 1.0
            float32 totalWeight = weights[0] + weights[1] + weights[2] + weights[3];
            if (totalWeight > 0.0f) {
                weights[0] /= totalWeight;
                weights[1] /= totalWeight;
                weights[2] /= totalWeight;
                weights[3] /= totalWeight;
            } else {
                // Fallback to first layer if no weights
                weights[0] = 1.0f;
            }

            texture->SetPixel(x, y, weights[0], weights[1], weights[2], weights[3]);
        }

        // Progress logging
        if ((y % (height / 10)) == 0 && y > 0) {
            LOG_INFO("Splatmap generation: %d%%", (y * 100) / height);
        }
    }

    LOG_INFO("Splatmap generated successfully");
    return texture;
}

float32 SplatmapGenerator::CalculateSlope(const Heightfield& heightfield, uint32 x, uint32 y) {
    uint32 width = heightfield.GetWidth();
    uint32 height = heightfield.GetHeight();

    // Get neighboring heights
    float32 hL = heightfield.GetHeight(x > 0 ? x - 1 : x, y);
    float32 hR = heightfield.GetHeight(x < width - 1 ? x + 1 : x, y);
    float32 hD = heightfield.GetHeight(x, y > 0 ? y - 1 : y);
    float32 hU = heightfield.GetHeight(x, y < height - 1 ? y + 1 : y);

    // Calculate gradients
    float32 dx = (hR - hL) / 2.0f;
    float32 dy = (hU - hD) / 2.0f;

    // Calculate slope angle in degrees
    float32 slope = std::atan(std::sqrt(dx * dx + dy * dy)) * (180.0f / 3.14159265f);
    return slope;
}

float32 SplatmapGenerator::CalculateLayerWeight(const Heightfield& heightfield, uint32 x, uint32 y, const MaterialLayer& layer, float32 slope) {
    float32 h = heightfield.GetHeight(x, y);

    // Height factor
    float32 heightFactor = 0.0f;
    if (h >= layer.heightMin - layer.blendRange && h <= layer.heightMax + layer.blendRange) {
        if (h < layer.heightMin) {
            heightFactor = SmoothStep(layer.heightMin - layer.blendRange, layer.heightMin, h);
        } else if (h > layer.heightMax) {
            heightFactor = 1.0f - SmoothStep(layer.heightMax, layer.heightMax + layer.blendRange, h);
        } else {
            heightFactor = 1.0f;
        }
    }

    // Slope factor
    float32 slopeFactor = 0.0f;
    if (slope >= layer.slopeMin - layer.blendRange && slope <= layer.slopeMax + layer.blendRange) {
        if (slope < layer.slopeMin) {
            slopeFactor = SmoothStep(layer.slopeMin - layer.blendRange, layer.slopeMin, slope);
        } else if (slope > layer.slopeMax) {
            slopeFactor = 1.0f - SmoothStep(layer.slopeMax, layer.slopeMax + layer.blendRange, slope);
        } else {
            slopeFactor = 1.0f;
        }
    }

    // Combine factors
    float32 weight = heightFactor * slopeFactor;

    // Add noise variation if needed
    if (layer.noiseScale > 0.0f && weight > 0.0f) {
        float32 noise = SimpleNoise(x, y, layer.seed);
        weight *= (1.0f - layer.noiseScale + noise * layer.noiseScale);
    }

    return std::max(0.0f, weight);
}

float32 SplatmapGenerator::SmoothStep(float32 edge0, float32 edge1, float32 x) {
    if (edge1 <= edge0) return x >= edge1 ? 1.0f : 0.0f;

    float32 t = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

float32 SplatmapGenerator::SimpleNoise(uint32 x, uint32 y, uint32 seed) {
    uint32 n = x + y * 57 + seed * 131;
    n = (n << 13) ^ n;
    return (1.0f - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f) * 0.5f + 0.5f;
}

SplatmapParams SplatmapGenerator::CreateMountainPreset() {
    SplatmapParams params;
    params.layerCount = 4;

    // Layer 0 (R): Grass - low altitude, gentle slopes
    params.layers[0].name = "Grass";
    params.layers[0].heightMin = 0.0f;
    params.layers[0].heightMax = 0.4f;
    params.layers[0].slopeMin = 0.0f;
    params.layers[0].slopeMax = 30.0f;
    params.layers[0].blendRange = 0.15f;
    params.layers[0].noiseScale = 0.2f;
    params.layers[0].seed = 1001;

    // Layer 1 (G): Rock - steep slopes, mid altitude
    params.layers[1].name = "Rock";
    params.layers[1].heightMin = 0.2f;
    params.layers[1].heightMax = 0.9f;
    params.layers[1].slopeMin = 25.0f;
    params.layers[1].slopeMax = 90.0f;
    params.layers[1].blendRange = 0.1f;
    params.layers[1].noiseScale = 0.15f;
    params.layers[1].seed = 1002;

    // Layer 2 (B): Snow - high altitude, any slope
    params.layers[2].name = "Snow";
    params.layers[2].heightMin = 0.65f;
    params.layers[2].heightMax = 1.0f;
    params.layers[2].slopeMin = 0.0f;
    params.layers[2].slopeMax = 90.0f;
    params.layers[2].blendRange = 0.2f;
    params.layers[2].noiseScale = 0.1f;
    params.layers[2].seed = 1003;

    // Layer 3 (A): Dirt - low altitude, steep slopes (cliffs without grass)
    params.layers[3].name = "Dirt";
    params.layers[3].heightMin = 0.0f;
    params.layers[3].heightMax = 0.5f;
    params.layers[3].slopeMin = 30.0f;
    params.layers[3].slopeMax = 90.0f;
    params.layers[3].blendRange = 0.1f;
    params.layers[3].noiseScale = 0.25f;
    params.layers[3].seed = 1004;

    return params;
}

SplatmapParams SplatmapGenerator::CreateDesertPreset() {
    SplatmapParams params;
    params.layerCount = 4;

    // Layer 0 (R): Sand - low areas
    params.layers[0].name = "Sand";
    params.layers[0].heightMin = 0.0f;
    params.layers[0].heightMax = 0.5f;
    params.layers[0].slopeMin = 0.0f;
    params.layers[0].slopeMax = 20.0f;
    params.layers[0].blendRange = 0.15f;
    params.layers[0].noiseScale = 0.3f;

    // Layer 1 (G): Rock - cliffs and peaks
    params.layers[1].name = "Rock";
    params.layers[1].heightMin = 0.3f;
    params.layers[1].heightMax = 1.0f;
    params.layers[1].slopeMin = 20.0f;
    params.layers[1].slopeMax = 90.0f;
    params.layers[1].blendRange = 0.1f;
    params.layers[1].noiseScale = 0.2f;

    // Layer 2 (B): Gravel - mid slopes
    params.layers[2].name = "Gravel";
    params.layers[2].heightMin = 0.2f;
    params.layers[2].heightMax = 0.7f;
    params.layers[2].slopeMin = 10.0f;
    params.layers[2].slopeMax = 30.0f;
    params.layers[2].blendRange = 0.15f;
    params.layers[2].noiseScale = 0.25f;

    // Layer 3 (A): Canyon Floor
    params.layers[3].name = "CanyonFloor";
    params.layers[3].heightMin = 0.0f;
    params.layers[3].heightMax = 0.3f;
    params.layers[3].slopeMin = 0.0f;
    params.layers[3].slopeMax = 10.0f;
    params.layers[3].blendRange = 0.1f;
    params.layers[3].noiseScale = 0.35f;

    return params;
}

SplatmapParams SplatmapGenerator::CreateArcticPreset() {
    SplatmapParams params;
    params.layerCount = 4;

    // Layer 0 (R): Ice - flat areas
    params.layers[0].name = "Ice";
    params.layers[0].heightMin = 0.0f;
    params.layers[0].heightMax = 0.6f;
    params.layers[0].slopeMin = 0.0f;
    params.layers[0].slopeMax = 20.0f;
    params.layers[0].blendRange = 0.1f;
    params.layers[0].noiseScale = 0.15f;

    // Layer 1 (G): Rock - exposed cliffs
    params.layers[1].name = "Rock";
    params.layers[1].heightMin = 0.0f;
    params.layers[1].heightMax = 1.0f;
    params.layers[1].slopeMin = 30.0f;
    params.layers[1].slopeMax = 90.0f;
    params.layers[1].blendRange = 0.1f;
    params.layers[1].noiseScale = 0.1f;

    // Layer 2 (B): Snow - everywhere high
    params.layers[2].name = "Snow";
    params.layers[2].heightMin = 0.4f;
    params.layers[2].heightMax = 1.0f;
    params.layers[2].slopeMin = 0.0f;
    params.layers[2].slopeMax = 90.0f;
    params.layers[2].blendRange = 0.25f;
    params.layers[2].noiseScale = 0.05f;

    // Layer 3 (A): Tundra - low gentle slopes
    params.layers[3].name = "Tundra";
    params.layers[3].heightMin = 0.0f;
    params.layers[3].heightMax = 0.4f;
    params.layers[3].slopeMin = 0.0f;
    params.layers[3].slopeMax = 15.0f;
    params.layers[3].blendRange = 0.15f;
    params.layers[3].noiseScale = 0.2f;

    return params;
}

} // namespace Terrain
