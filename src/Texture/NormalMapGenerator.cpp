#include "NormalMapGenerator.h"
#include "Core/Logger.h"
#include <glm/glm.hpp>

namespace Terrain {

NormalMapGenerator::NormalMapGenerator() {
}

NormalMapGenerator::~NormalMapGenerator() {
}

Unique<Texture> NormalMapGenerator::Generate(const Heightfield& heightfield, const NormalMapParams& params) {
    uint32 width = heightfield.GetWidth();
    uint32 height = heightfield.GetHeight();

    // Create RGB8 texture for normal map
    auto texture = MakeUnique<Texture>(width, height, TextureFormat::RGB8);

    LOG_INFO("Generating normal map (%ux%u)...", width, height);

    // Calculate normals for each pixel
    for (uint32 y = 0; y < height; y++) {
        for (uint32 x = 0; x < width; x++) {
            glm::vec3 normal = CalculateNormal(heightfield, x, y, params.heightScale);

            // Apply strength
            normal.x *= params.strength;
            normal.y *= params.strength;
            normal = glm::normalize(normal);

            // Invert Y if needed (OpenGL vs DirectX)
            if (params.invertY) {
                normal.y = -normal.y;
            }

            // Convert from [-1, 1] to [0, 1] range
            float32 r = normal.x * 0.5f + 0.5f;
            float32 g = normal.y * 0.5f + 0.5f;
            float32 b = normal.z * 0.5f + 0.5f;

            texture->SetPixel(x, y, r, g, b, 1.0f);
        }
    }

    LOG_INFO("Normal map generated successfully");
    return texture;
}

glm::vec3 NormalMapGenerator::CalculateNormal(const Heightfield& heightfield, uint32 x, uint32 y, float32 heightScale) {
    uint32 width = heightfield.GetWidth();
    uint32 height = heightfield.GetHeight();

    // Get neighboring heights using Sobel operator
    float32 hL = heightfield.GetHeight(x > 0 ? x - 1 : x, y);
    float32 hR = heightfield.GetHeight(x < width - 1 ? x + 1 : x, y);
    float32 hD = heightfield.GetHeight(x, y > 0 ? y - 1 : y);
    float32 hU = heightfield.GetHeight(x, y < height - 1 ? y + 1 : y);

    // Central differences
    float32 dx = (hR - hL) * heightScale;
    float32 dy = (hU - hD) * heightScale;

    // Normal vector
    glm::vec3 normal(-dx, -dy, 1.0f);
    return glm::normalize(normal);
}

} // namespace Terrain
