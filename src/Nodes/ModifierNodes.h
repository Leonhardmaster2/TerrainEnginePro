#pragma once

#include "Node.h"

namespace Terrain {

// Terrace (step function)
class TerraceNode : public Node {
public:
    TerraceNode(uint32 id);
    bool Execute(NodeGraph* graph) override;

    int32 steps = 5;
    float32 blend = 0.1f;
};

// Clamp values
class ClampNode : public Node {
public:
    ClampNode(uint32 id);
    bool Execute(NodeGraph* graph) override;

    float32 minValue = 0.0f;
    float32 maxValue = 1.0f;
};

// Invert heightfield
class InvertNode : public Node {
public:
    InvertNode(uint32 id);
    bool Execute(NodeGraph* graph) override;
};

// Scale heightfield
class ScaleNode : public Node {
public:
    ScaleNode(uint32 id);
    bool Execute(NodeGraph* graph) override;

    float32 scale = 2.0f;
};

// Curve adjustment
class CurveNode : public Node {
public:
    CurveNode(uint32 id);
    bool Execute(NodeGraph* graph) override;

    float32 power = 2.0f; // Power curve
};

// Smoothing filter
class SmoothNode : public Node {
public:
    SmoothNode(uint32 id);
    bool Execute(NodeGraph* graph) override;

    int32 iterations = 1;
    float32 strength = 0.5f;
};

// Sharpen filter
class SharpenNode : public Node {
public:
    SharpenNode(uint32 id);
    bool Execute(NodeGraph* graph) override;

    float32 strength = 1.0f;
};

// Combiner: Add
class AddNode : public Node {
public:
    AddNode(uint32 id);
    bool Execute(NodeGraph* graph) override;
};

// Combiner: Multiply
class MultiplyNode : public Node {
public:
    MultiplyNode(uint32 id);
    bool Execute(NodeGraph* graph) override;
};

// Combiner: Blend
class BlendNode : public Node {
public:
    BlendNode(uint32 id);
    bool Execute(NodeGraph* graph) override;

    float32 blend = 0.5f;
};

// Combiner: Max
class MaxNode : public Node {
public:
    MaxNode(uint32 id);
    bool Execute(NodeGraph* graph) override;
};

// Combiner: Min
class MinNode : public Node {
public:
    MinNode(uint32 id);
    bool Execute(NodeGraph* graph) override;
};

// Output node (final result)
class OutputNode : public Node {
public:
    OutputNode(uint32 id);
    bool Execute(NodeGraph* graph) override;
};

} // namespace Terrain
