#include "TextureNodes.h"
#include "NodeGraph.h"
#include "Core/Logger.h"

namespace Terrain {

// ============================================================================
// Normal Map Node
// ============================================================================

NormalMapNode::NormalMapNode(uint32 id)
    : Node(id, "Normal Map", NodeCategory::Output) {
    AddInputPin("Input", PinType::Heightfield);

    // Default parameters
    params.strength = 1.0f;
    params.heightScale = 1.0f;
    params.invertY = false;
}

bool NormalMapNode::Execute(NodeGraph* graph) {
    if (!m_Dirty) {
        return true;
    }

    auto input = GetInputHeightfield("Input", graph);
    if (!input) {
        LOG_ERROR("Normal map node: no input");
        return false;
    }

    LOG_INFO("Generating normal map...");

    // Generate normal map
    NormalMapGenerator generator;
    m_CachedTexture = generator.Generate(*input, params);

    if (!m_CachedTexture) {
        LOG_ERROR("Failed to generate normal map");
        return false;
    }

    // Auto-export if path is set
    if (!outputPath.empty()) {
        m_CachedTexture->ExportPNG(outputPath);
    }

    m_Dirty = false;
    return true;
}

// ============================================================================
// Ambient Occlusion Node
// ============================================================================

AmbientOcclusionNode::AmbientOcclusionNode(uint32 id)
    : Node(id, "Ambient Occlusion", NodeCategory::Output) {
    AddInputPin("Input", PinType::Heightfield);

    // Default parameters
    params.samples = 16;
    params.radius = 10.0f;
    params.strength = 1.0f;
    params.bias = 0.05f;
    params.heightScale = 1.0f;
}

bool AmbientOcclusionNode::Execute(NodeGraph* graph) {
    if (!m_Dirty) {
        return true;
    }

    auto input = GetInputHeightfield("Input", graph);
    if (!input) {
        LOG_ERROR("Ambient occlusion node: no input");
        return false;
    }

    LOG_INFO("Generating ambient occlusion (this may take a while)...");

    // Generate AO
    AmbientOcclusionGenerator generator;
    m_CachedTexture = generator.Generate(*input, params);

    if (!m_CachedTexture) {
        LOG_ERROR("Failed to generate ambient occlusion");
        return false;
    }

    // Auto-export if path is set
    if (!outputPath.empty()) {
        m_CachedTexture->ExportPNG(outputPath);
    }

    m_Dirty = false;
    return true;
}

// ============================================================================
// Splatmap Node
// ============================================================================

SplatmapNode::SplatmapNode(uint32 id)
    : Node(id, "Splatmap", NodeCategory::Output) {
    AddInputPin("Input", PinType::Heightfield);

    // Use mountain preset by default
    params = SplatmapGenerator::CreateMountainPreset();
}

bool SplatmapNode::Execute(NodeGraph* graph) {
    if (!m_Dirty) {
        return true;
    }

    auto input = GetInputHeightfield("Input", graph);
    if (!input) {
        LOG_ERROR("Splatmap node: no input");
        return false;
    }

    LOG_INFO("Generating splatmap...");

    // Generate splatmap
    SplatmapGenerator generator;
    m_CachedTexture = generator.Generate(*input, params);

    if (!m_CachedTexture) {
        LOG_ERROR("Failed to generate splatmap");
        return false;
    }

    // Auto-export if path is set
    if (!outputPath.empty()) {
        m_CachedTexture->ExportPNG(outputPath);
    }

    m_Dirty = false;
    return true;
}

} // namespace Terrain
