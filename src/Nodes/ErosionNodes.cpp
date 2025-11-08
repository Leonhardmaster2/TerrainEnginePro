#include "ErosionNodes.h"
#include "NodeGraph.h"
#include "Core/Logger.h"

namespace Terrain {

// ============================================================================
// Hydraulic Erosion Node
// ============================================================================

HydraulicErosionNode::HydraulicErosionNode(uint32 id)
    : Node(id, "Hydraulic Erosion", NodeCategory::Filter) {
    AddInputPin("Input", PinType::Heightfield);
    AddOutputPin("Output", PinType::Heightfield);

    // Default parameters for realistic erosion
    params.iterations = 100000;
    params.seed = 12345;
    params.inertia = 0.05f;
    params.sedimentCapacity = 4.0f;
    params.minSlope = 0.01f;
    params.erodeSpeed = 0.3f;
    params.depositSpeed = 0.3f;
    params.evaporateSpeed = 0.01f;
    params.gravity = 4.0f;
    params.maxDropletLifetime = 30.0f;
}

bool HydraulicErosionNode::Execute(NodeGraph* graph) {
    if (!m_Dirty) {
        return true;
    }

    auto input = GetInputHeightfield("Input", graph);
    if (!input) {
        LOG_ERROR("Hydraulic erosion node: no input");
        return false;
    }

    // Create erosion instance
    auto erosion = MakeUnique<HydraulicErosion>();

    // Get Vulkan context from generator
    auto generator = graph->GetGenerator();
    if (!generator) {
        LOG_ERROR("No generator available for erosion");
        return false;
    }

    // Initialize erosion (we need access to Vulkan context)
    // For now, we'll skip GPU erosion if Vulkan is not available
    // This is a simplified version - in practice, you'd get these from the generator
    LOG_WARN("Hydraulic erosion node: GPU erosion not yet fully integrated, using placeholder");

    // For now, just pass through
    // TODO: Properly integrate with VulkanContext from TerrainGenerator
    m_CachedOutput = std::move(input);
    m_Dirty = false;

    return true;
}

// ============================================================================
// Thermal Erosion Node
// ============================================================================

ThermalErosionNode::ThermalErosionNode(uint32 id)
    : Node(id, "Thermal Erosion", NodeCategory::Filter) {
    AddInputPin("Input", PinType::Heightfield);
    AddOutputPin("Output", PinType::Heightfield);

    // Default parameters
    params.iterations = 10;
    params.talusAngle = 0.7f; // ~40 degrees
    params.strength = 0.5f;
}

bool ThermalErosionNode::Execute(NodeGraph* graph) {
    if (!m_Dirty) {
        return true;
    }

    auto input = GetInputHeightfield("Input", graph);
    if (!input) {
        LOG_ERROR("Thermal erosion node: no input");
        return false;
    }

    // Create erosion instance
    auto erosion = MakeUnique<ThermalErosion>();

    // Apply erosion
    if (!erosion->Erode(*input, params)) {
        LOG_ERROR("Failed to apply thermal erosion");
        return false;
    }

    SetOutputHeightfield("Output", std::move(input));
    return true;
}

} // namespace Terrain
