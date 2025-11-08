#pragma once

#include "Core/Types.h"
#include "Terrain/Heightfield.h"
#include "Texture.h"
#include <glm/glm.hpp>

namespace Terrain {

struct NormalMapParams {
    float32 strength = 1.0f;     // Normal map strength multiplier
    float32 heightScale = 1.0f;  // Height scale for gradient calculation
    bool invertY = false;        // Invert Y component (OpenGL vs DirectX)
};

class NormalMapGenerator {
public:
    NormalMapGenerator();
    ~NormalMapGenerator();

    // Generate normal map from heightfield
    Unique<Texture> Generate(const Heightfield& heightfield, const NormalMapParams& params = NormalMapParams());

    // Get/set default parameters
    const NormalMapParams& GetParams() const { return m_Params; }
    void SetParams(const NormalMapParams& params) { m_Params = params; }

private:
    glm::vec3 CalculateNormal(const Heightfield& heightfield, uint32 x, uint32 y, float32 heightScale);

    NormalMapParams m_Params;
};

} // namespace Terrain
