#pragma once

#include "Node.h"
#include "Terrain/TerrainGenerator.h"

namespace Terrain {

// Perlin Noise Generator
class PerlinNode : public Node {
public:
    PerlinNode(uint32 id);
    bool Execute(NodeGraph* graph) override;

    PerlinParams params;
    uint32 width = 512;
    uint32 height = 512;
};

// Voronoi Generator
class VoronoiNode : public Node {
public:
    VoronoiNode(uint32 id);
    bool Execute(NodeGraph* graph) override;

    uint32 width = 512;
    uint32 height = 512;
    int32 cellCount = 20;
    float32 amplitude = 1.0f;
    uint32 seed = 12345;
    bool invert = false;
};

// Ridged Noise Generator
class RidgedNode : public Node {
public:
    RidgedNode(uint32 id);
    bool Execute(NodeGraph* graph) override;

    uint32 width = 512;
    uint32 height = 512;
    float32 frequency = 0.01f;
    float32 amplitude = 1.0f;
    int32 octaves = 6;
    float32 lacunarity = 2.0f;
    float32 persistence = 0.5f;
    float32 ridgeOffset = 1.0f;
    uint32 seed = 12345;
};

// Gradient Generator
class GradientNode : public Node {
public:
    GradientNode(uint32 id);
    bool Execute(NodeGraph* graph) override;

    uint32 width = 512;
    uint32 height = 512;
    glm::vec2 direction = glm::vec2(0.0f, 1.0f); // Vertical gradient
    float32 amplitude = 1.0f;
};

// Constant Value Generator
class ConstantNode : public Node {
public:
    ConstantNode(uint32 id);
    bool Execute(NodeGraph* graph) override;

    uint32 width = 512;
    uint32 height = 512;
    float32 value = 0.5f;
};

// White Noise Generator
class WhiteNoiseNode : public Node {
public:
    WhiteNoiseNode(uint32 id);
    bool Execute(NodeGraph* graph) override;

    uint32 width = 512;
    uint32 height = 512;
    float32 amplitude = 1.0f;
    uint32 seed = 12345;
};

} // namespace Terrain
