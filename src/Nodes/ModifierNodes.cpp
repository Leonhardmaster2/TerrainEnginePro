#include "ModifierNodes.h"
#include "NodeGraph.h"
#include "Core/Logger.h"
#include <cmath>
#include <algorithm>

namespace Terrain {

// ============================================================================
// Terrace Node
// ============================================================================

TerraceNode::TerraceNode(uint32 id)
    : Node(id, "Terrace", NodeCategory::Modifier) {
    AddInputPin("Input", PinType::Heightfield);
    AddOutputPin("Output", PinType::Heightfield);
}

bool TerraceNode::Execute(NodeGraph* graph) {
    if (!m_Dirty) {
        return true;
    }

    auto input = GetInputHeightfield("Input", graph);
    if (!input) {
        LOG_ERROR("Terrace node: no input");
        return false;
    }

    uint32 width = input->GetWidth();
    uint32 height = input->GetHeight();
    auto output = MakeUnique<Heightfield>(width, height);

    for (uint32 y = 0; y < height; y++) {
        for (uint32 x = 0; x < width; x++) {
            float value = input->GetHeight(x, y);

            // Apply terracing
            float stepped = std::floor(value * steps) / steps;

            // Blend between stepped and original
            float finalValue = stepped * (1.0f - blend) + value * blend;

            output->SetHeight(x, y, finalValue);
        }
    }

    SetOutputHeightfield("Output", std::move(output));
    return true;
}

// ============================================================================
// Clamp Node
// ============================================================================

ClampNode::ClampNode(uint32 id)
    : Node(id, "Clamp", NodeCategory::Modifier) {
    AddInputPin("Input", PinType::Heightfield);
    AddOutputPin("Output", PinType::Heightfield);
}

bool ClampNode::Execute(NodeGraph* graph) {
    if (!m_Dirty) {
        return true;
    }

    auto input = GetInputHeightfield("Input", graph);
    if (!input) {
        LOG_ERROR("Clamp node: no input");
        return false;
    }

    uint32 width = input->GetWidth();
    uint32 height = input->GetHeight();
    auto output = MakeUnique<Heightfield>(width, height);

    for (uint32 y = 0; y < height; y++) {
        for (uint32 x = 0; x < width; x++) {
            float value = input->GetHeight(x, y);
            value = std::clamp(value, minValue, maxValue);
            output->SetHeight(x, y, value);
        }
    }

    SetOutputHeightfield("Output", std::move(output));
    return true;
}

// ============================================================================
// Invert Node
// ============================================================================

InvertNode::InvertNode(uint32 id)
    : Node(id, "Invert", NodeCategory::Modifier) {
    AddInputPin("Input", PinType::Heightfield);
    AddOutputPin("Output", PinType::Heightfield);
}

bool InvertNode::Execute(NodeGraph* graph) {
    if (!m_Dirty) {
        return true;
    }

    auto input = GetInputHeightfield("Input", graph);
    if (!input) {
        LOG_ERROR("Invert node: no input");
        return false;
    }

    uint32 width = input->GetWidth();
    uint32 height = input->GetHeight();
    auto output = MakeUnique<Heightfield>(width, height);

    float min = input->GetMin();
    float max = input->GetMax();

    for (uint32 y = 0; y < height; y++) {
        for (uint32 x = 0; x < width; x++) {
            float value = input->GetHeight(x, y);
            output->SetHeight(x, y, max - value + min);
        }
    }

    SetOutputHeightfield("Output", std::move(output));
    return true;
}

// ============================================================================
// Scale Node
// ============================================================================

ScaleNode::ScaleNode(uint32 id)
    : Node(id, "Scale", NodeCategory::Modifier) {
    AddInputPin("Input", PinType::Heightfield);
    AddOutputPin("Output", PinType::Heightfield);
}

bool ScaleNode::Execute(NodeGraph* graph) {
    if (!m_Dirty) {
        return true;
    }

    auto input = GetInputHeightfield("Input", graph);
    if (!input) {
        LOG_ERROR("Scale node: no input");
        return false;
    }

    uint32 width = input->GetWidth();
    uint32 height = input->GetHeight();
    auto output = MakeUnique<Heightfield>(width, height);

    for (uint32 y = 0; y < height; y++) {
        for (uint32 x = 0; x < width; x++) {
            float value = input->GetHeight(x, y);
            output->SetHeight(x, y, value * scale);
        }
    }

    SetOutputHeightfield("Output", std::move(output));
    return true;
}

// ============================================================================
// Curve Node
// ============================================================================

CurveNode::CurveNode(uint32 id)
    : Node(id, "Curve", NodeCategory::Modifier) {
    AddInputPin("Input", PinType::Heightfield);
    AddOutputPin("Output", PinType::Heightfield);
}

bool CurveNode::Execute(NodeGraph* graph) {
    if (!m_Dirty) {
        return true;
    }

    auto input = GetInputHeightfield("Input", graph);
    if (!input) {
        LOG_ERROR("Curve node: no input");
        return false;
    }

    uint32 width = input->GetWidth();
    uint32 height = input->GetHeight();
    auto output = MakeUnique<Heightfield>(width, height);

    for (uint32 y = 0; y < height; y++) {
        for (uint32 x = 0; x < width; x++) {
            float value = input->GetHeight(x, y);
            value = std::pow(value, power);
            output->SetHeight(x, y, value);
        }
    }

    output->Normalize(0.0f, 1.0f);
    SetOutputHeightfield("Output", std::move(output));
    return true;
}

// ============================================================================
// Smooth Node
// ============================================================================

SmoothNode::SmoothNode(uint32 id)
    : Node(id, "Smooth", NodeCategory::Filter) {
    AddInputPin("Input", PinType::Heightfield);
    AddOutputPin("Output", PinType::Heightfield);
}

bool SmoothNode::Execute(NodeGraph* graph) {
    if (!m_Dirty) {
        return true;
    }

    auto input = GetInputHeightfield("Input", graph);
    if (!input) {
        LOG_ERROR("Smooth node: no input");
        return false;
    }

    uint32 width = input->GetWidth();
    uint32 height = input->GetHeight();
    auto output = MakeUnique<Heightfield>(*input); // Copy

    // Box blur
    for (int iter = 0; iter < iterations; iter++) {
        auto temp = MakeUnique<Heightfield>(*output);

        for (uint32 y = 1; y < height - 1; y++) {
            for (uint32 x = 1; x < width - 1; x++) {
                float sum = 0.0f;
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dx = -1; dx <= 1; dx++) {
                        sum += temp->GetHeight(x + dx, y + dy);
                    }
                }
                float smoothed = sum / 9.0f;
                float original = temp->GetHeight(x, y);
                output->SetHeight(x, y, original * (1.0f - strength) + smoothed * strength);
            }
        }
    }

    SetOutputHeightfield("Output", std::move(output));
    return true;
}

// ============================================================================
// Sharpen Node
// ============================================================================

SharpenNode::SharpenNode(uint32 id)
    : Node(id, "Sharpen", NodeCategory::Filter) {
    AddInputPin("Input", PinType::Heightfield);
    AddOutputPin("Output", PinType::Heightfield);
}

bool SharpenNode::Execute(NodeGraph* graph) {
    if (!m_Dirty) {
        return true;
    }

    auto input = GetInputHeightfield("Input", graph);
    if (!input) {
        LOG_ERROR("Sharpen node: no input");
        return false;
    }

    uint32 width = input->GetWidth();
    uint32 height = input->GetHeight();
    auto output = MakeUnique<Heightfield>(width, height);

    // Sharpen kernel
    for (uint32 y = 1; y < height - 1; y++) {
        for (uint32 x = 1; x < width - 1; x++) {
            float center = input->GetHeight(x, y);
            float neighbors =
                input->GetHeight(x - 1, y) + input->GetHeight(x + 1, y) +
                input->GetHeight(x, y - 1) + input->GetHeight(x, y + 1);

            float sharpened = center * (1.0f + 4.0f * strength) - neighbors * strength;
            output->SetHeight(x, y, sharpened);
        }
    }

    // Copy borders
    for (uint32 x = 0; x < width; x++) {
        output->SetHeight(x, 0, input->GetHeight(x, 0));
        output->SetHeight(x, height - 1, input->GetHeight(x, height - 1));
    }
    for (uint32 y = 0; y < height; y++) {
        output->SetHeight(0, y, input->GetHeight(0, y));
        output->SetHeight(width - 1, y, input->GetHeight(width - 1, y));
    }

    SetOutputHeightfield("Output", std::move(output));
    return true;
}

// ============================================================================
// Add Node
// ============================================================================

AddNode::AddNode(uint32 id)
    : Node(id, "Add", NodeCategory::Combiner) {
    AddInputPin("A", PinType::Heightfield);
    AddInputPin("B", PinType::Heightfield);
    AddOutputPin("Output", PinType::Heightfield);
}

bool AddNode::Execute(NodeGraph* graph) {
    if (!m_Dirty) {
        return true;
    }

    auto inputA = GetInputHeightfield("A", graph);
    auto inputB = GetInputHeightfield("B", graph);

    if (!inputA || !inputB) {
        LOG_ERROR("Add node: missing inputs");
        return false;
    }

    uint32 width = inputA->GetWidth();
    uint32 height = inputA->GetHeight();

    if (width != inputB->GetWidth() || height != inputB->GetHeight()) {
        LOG_ERROR("Add node: input dimensions must match");
        return false;
    }

    auto output = MakeUnique<Heightfield>(width, height);

    for (uint32 y = 0; y < height; y++) {
        for (uint32 x = 0; x < width; x++) {
            float a = inputA->GetHeight(x, y);
            float b = inputB->GetHeight(x, y);
            output->SetHeight(x, y, a + b);
        }
    }

    output->Normalize(0.0f, 1.0f);
    SetOutputHeightfield("Output", std::move(output));
    return true;
}

// ============================================================================
// Multiply Node
// ============================================================================

MultiplyNode::MultiplyNode(uint32 id)
    : Node(id, "Multiply", NodeCategory::Combiner) {
    AddInputPin("A", PinType::Heightfield);
    AddInputPin("B", PinType::Heightfield);
    AddOutputPin("Output", PinType::Heightfield);
}

bool MultiplyNode::Execute(NodeGraph* graph) {
    if (!m_Dirty) {
        return true;
    }

    auto inputA = GetInputHeightfield("A", graph);
    auto inputB = GetInputHeightfield("B", graph);

    if (!inputA || !inputB) {
        LOG_ERROR("Multiply node: missing inputs");
        return false;
    }

    uint32 width = inputA->GetWidth();
    uint32 height = inputA->GetHeight();

    if (width != inputB->GetWidth() || height != inputB->GetHeight()) {
        LOG_ERROR("Multiply node: input dimensions must match");
        return false;
    }

    auto output = MakeUnique<Heightfield>(width, height);

    for (uint32 y = 0; y < height; y++) {
        for (uint32 x = 0; x < width; x++) {
            float a = inputA->GetHeight(x, y);
            float b = inputB->GetHeight(x, y);
            output->SetHeight(x, y, a * b);
        }
    }

    output->Normalize(0.0f, 1.0f);
    SetOutputHeightfield("Output", std::move(output));
    return true;
}

// ============================================================================
// Blend Node
// ============================================================================

BlendNode::BlendNode(uint32 id)
    : Node(id, "Blend", NodeCategory::Combiner) {
    AddInputPin("A", PinType::Heightfield);
    AddInputPin("B", PinType::Heightfield);
    AddOutputPin("Output", PinType::Heightfield);
}

bool BlendNode::Execute(NodeGraph* graph) {
    if (!m_Dirty) {
        return true;
    }

    auto inputA = GetInputHeightfield("A", graph);
    auto inputB = GetInputHeightfield("B", graph);

    if (!inputA || !inputB) {
        LOG_ERROR("Blend node: missing inputs");
        return false;
    }

    uint32 width = inputA->GetWidth();
    uint32 height = inputA->GetHeight();

    if (width != inputB->GetWidth() || height != inputB->GetHeight()) {
        LOG_ERROR("Blend node: input dimensions must match");
        return false;
    }

    auto output = MakeUnique<Heightfield>(width, height);

    for (uint32 y = 0; y < height; y++) {
        for (uint32 x = 0; x < width; x++) {
            float a = inputA->GetHeight(x, y);
            float b = inputB->GetHeight(x, y);
            output->SetHeight(x, y, a * (1.0f - blend) + b * blend);
        }
    }

    SetOutputHeightfield("Output", std::move(output));
    return true;
}

// ============================================================================
// Max Node
// ============================================================================

MaxNode::MaxNode(uint32 id)
    : Node(id, "Max", NodeCategory::Combiner) {
    AddInputPin("A", PinType::Heightfield);
    AddInputPin("B", PinType::Heightfield);
    AddOutputPin("Output", PinType::Heightfield);
}

bool MaxNode::Execute(NodeGraph* graph) {
    if (!m_Dirty) {
        return true;
    }

    auto inputA = GetInputHeightfield("A", graph);
    auto inputB = GetInputHeightfield("B", graph);

    if (!inputA || !inputB) {
        LOG_ERROR("Max node: missing inputs");
        return false;
    }

    uint32 width = inputA->GetWidth();
    uint32 height = inputA->GetHeight();

    if (width != inputB->GetWidth() || height != inputB->GetHeight()) {
        LOG_ERROR("Max node: input dimensions must match");
        return false;
    }

    auto output = MakeUnique<Heightfield>(width, height);

    for (uint32 y = 0; y < height; y++) {
        for (uint32 x = 0; x < width; x++) {
            float a = inputA->GetHeight(x, y);
            float b = inputB->GetHeight(x, y);
            output->SetHeight(x, y, std::max(a, b));
        }
    }

    SetOutputHeightfield("Output", std::move(output));
    return true;
}

// ============================================================================
// Min Node
// ============================================================================

MinNode::MinNode(uint32 id)
    : Node(id, "Min", NodeCategory::Combiner) {
    AddInputPin("A", PinType::Heightfield);
    AddInputPin("B", PinType::Heightfield);
    AddOutputPin("Output", PinType::Heightfield);
}

bool MinNode::Execute(NodeGraph* graph) {
    if (!m_Dirty) {
        return true;
    }

    auto inputA = GetInputHeightfield("A", graph);
    auto inputB = GetInputHeightfield("B", graph);

    if (!inputA || !inputB) {
        LOG_ERROR("Min node: missing inputs");
        return false;
    }

    uint32 width = inputA->GetWidth();
    uint32 height = inputA->GetHeight();

    if (width != inputB->GetWidth() || height != inputB->GetHeight()) {
        LOG_ERROR("Min node: input dimensions must match");
        return false;
    }

    auto output = MakeUnique<Heightfield>(width, height);

    for (uint32 y = 0; y < height; y++) {
        for (uint32 x = 0; x < width; x++) {
            float a = inputA->GetHeight(x, y);
            float b = inputB->GetHeight(x, y);
            output->SetHeight(x, y, std::min(a, b));
        }
    }

    SetOutputHeightfield("Output", std::move(output));
    return true;
}

// ============================================================================
// Output Node
// ============================================================================

OutputNode::OutputNode(uint32 id)
    : Node(id, "Output", NodeCategory::Output) {
    AddInputPin("Input", PinType::Heightfield);
}

bool OutputNode::Execute(NodeGraph* graph) {
    if (!m_Dirty) {
        return true;
    }

    auto input = GetInputHeightfield("Input", graph);
    if (!input) {
        LOG_WARN("Output node: no input connected");
        return false;
    }

    // Just pass through
    m_CachedOutput = std::move(input);
    m_Dirty = false;
    return true;
}

} // namespace Terrain
