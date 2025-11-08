#include "GeneratorNodes.h"
#include "NodeGraph.h"
#include "Core/Logger.h"
#include <random>
#include <cmath>

namespace Terrain {

// ============================================================================
// Perlin Node
// ============================================================================

PerlinNode::PerlinNode(uint32 id)
    : Node(id, "Perlin Noise", NodeCategory::Generator) {
    AddOutputPin("Output", PinType::Heightfield);

    // Default parameters
    params.frequency = 0.01f;
    params.amplitude = 1.0f;
    params.octaves = 6;
    params.lacunarity = 2.0f;
    params.persistence = 0.5f;
    params.seed = 12345;
}

bool PerlinNode::Execute(NodeGraph* graph) {
    if (!m_Dirty) {
        return true;
    }

    auto generator = graph->GetGenerator();
    if (!generator) {
        LOG_ERROR("No terrain generator available");
        return false;
    }

    auto heightfield = generator->GeneratePerlin(width, height, params);
    if (!heightfield) {
        LOG_ERROR("Failed to generate Perlin noise");
        return false;
    }

    SetOutputHeightfield("Output", std::move(heightfield));
    return true;
}

// ============================================================================
// Voronoi Node
// ============================================================================

VoronoiNode::VoronoiNode(uint32 id)
    : Node(id, "Voronoi", NodeCategory::Generator) {
    AddOutputPin("Output", PinType::Heightfield);
}

bool VoronoiNode::Execute(NodeGraph* graph) {
    if (!m_Dirty) {
        return true;
    }

    auto heightfield = MakeUnique<Heightfield>(width, height);

    // Generate random cell points
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    std::vector<glm::vec2> cellPoints;
    for (int i = 0; i < cellCount; i++) {
        cellPoints.push_back(glm::vec2(dist(rng), dist(rng)));
    }

    // Calculate distance to nearest cell for each point
    for (uint32 y = 0; y < height; y++) {
        for (uint32 x = 0; x < width; x++) {
            glm::vec2 p(static_cast<float>(x) / width, static_cast<float>(y) / height);

            float minDist = FLT_MAX;
            for (const auto& cell : cellPoints) {
                float dist = glm::length(p - cell);
                minDist = std::min(minDist, dist);
            }

            float value = minDist * amplitude;
            if (invert) {
                value = amplitude - value;
            }

            heightfield->SetHeight(x, y, value);
        }
    }

    heightfield->Normalize(0.0f, 1.0f);
    SetOutputHeightfield("Output", std::move(heightfield));
    return true;
}

// ============================================================================
// Ridged Node
// ============================================================================

RidgedNode::RidgedNode(uint32 id)
    : Node(id, "Ridged Noise", NodeCategory::Generator) {
    AddOutputPin("Output", PinType::Heightfield);
}

bool RidgedNode::Execute(NodeGraph* graph) {
    if (!m_Dirty) {
        return true;
    }

    // Use Perlin but apply ridged transformation
    auto generator = graph->GetGenerator();
    if (!generator) {
        LOG_ERROR("No terrain generator available");
        return false;
    }

    PerlinParams perlinParams;
    perlinParams.frequency = frequency;
    perlinParams.amplitude = amplitude;
    perlinParams.octaves = octaves;
    perlinParams.lacunarity = lacunarity;
    perlinParams.persistence = persistence;
    perlinParams.seed = seed;

    auto heightfield = generator->GeneratePerlin(width, height, perlinParams);
    if (!heightfield) {
        LOG_ERROR("Failed to generate ridged noise");
        return false;
    }

    // Apply ridged transformation: abs(value) and invert
    for (uint32 y = 0; y < height; y++) {
        for (uint32 x = 0; x < width; x++) {
            float value = heightfield->GetHeight(x, y);
            value = ridgeOffset - std::abs(value - 0.5f) * 2.0f;
            heightfield->SetHeight(x, y, value);
        }
    }

    heightfield->Normalize(0.0f, 1.0f);
    SetOutputHeightfield("Output", std::move(heightfield));
    return true;
}

// ============================================================================
// Gradient Node
// ============================================================================

GradientNode::GradientNode(uint32 id)
    : Node(id, "Gradient", NodeCategory::Generator) {
    AddOutputPin("Output", PinType::Heightfield);
}

bool GradientNode::Execute(NodeGraph* graph) {
    if (!m_Dirty) {
        return true;
    }

    auto heightfield = MakeUnique<Heightfield>(width, height);

    glm::vec2 dir = glm::normalize(direction);

    for (uint32 y = 0; y < height; y++) {
        for (uint32 x = 0; x < width; x++) {
            glm::vec2 p(static_cast<float>(x) / width, static_cast<float>(y) / height);
            float value = glm::dot(p, dir) * amplitude;
            heightfield->SetHeight(x, y, value);
        }
    }

    heightfield->Normalize(0.0f, 1.0f);
    SetOutputHeightfield("Output", std::move(heightfield));
    return true;
}

// ============================================================================
// Constant Node
// ============================================================================

ConstantNode::ConstantNode(uint32 id)
    : Node(id, "Constant", NodeCategory::Generator) {
    AddOutputPin("Output", PinType::Heightfield);
}

bool ConstantNode::Execute(NodeGraph* graph) {
    if (!m_Dirty) {
        return true;
    }

    auto heightfield = MakeUnique<Heightfield>(width, height);

    for (uint32 y = 0; y < height; y++) {
        for (uint32 x = 0; x < width; x++) {
            heightfield->SetHeight(x, y, value);
        }
    }

    SetOutputHeightfield("Output", std::move(heightfield));
    return true;
}

// ============================================================================
// White Noise Node
// ============================================================================

WhiteNoiseNode::WhiteNoiseNode(uint32 id)
    : Node(id, "White Noise", NodeCategory::Generator) {
    AddOutputPin("Output", PinType::Heightfield);
}

bool WhiteNoiseNode::Execute(NodeGraph* graph) {
    if (!m_Dirty) {
        return true;
    }

    auto heightfield = MakeUnique<Heightfield>(width, height);

    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> dist(0.0f, amplitude);

    for (uint32 y = 0; y < height; y++) {
        for (uint32 x = 0; x < width; x++) {
            heightfield->SetHeight(x, y, dist(rng));
        }
    }

    SetOutputHeightfield("Output", std::move(heightfield));
    return true;
}

} // namespace Terrain
