#pragma once

#include "Node.h"
#include "Texture/NormalMapGenerator.h"
#include "Texture/AmbientOcclusionGenerator.h"
#include "Texture/SplatmapGenerator.h"

namespace Terrain {

// Note: Texture nodes are special - they don't output heightfields,
// but generate textures that are saved separately

// Normal Map Generator Node
class NormalMapNode : public Node {
public:
    NormalMapNode(uint32 id);
    bool Execute(NodeGraph* graph) override;

    NormalMapParams params;
    String outputPath = "normal_map.png";

    // Cached texture result
    Unique<Texture> GetTexture() const { return m_CachedTexture ? MakeUnique<Texture>(*m_CachedTexture) : nullptr; }

private:
    Unique<Texture> m_CachedTexture;
};

// Ambient Occlusion Generator Node
class AmbientOcclusionNode : public Node {
public:
    AmbientOcclusionNode(uint32 id);
    bool Execute(NodeGraph* graph) override;

    AmbientOcclusionParams params;
    String outputPath = "ambient_occlusion.png";

    Unique<Texture> GetTexture() const { return m_CachedTexture ? MakeUnique<Texture>(*m_CachedTexture) : nullptr; }

private:
    Unique<Texture> m_CachedTexture;
};

// Splatmap Generator Node
class SplatmapNode : public Node {
public:
    SplatmapNode(uint32 id);
    bool Execute(NodeGraph* graph) override;

    SplatmapParams params;
    String outputPath = "splatmap.png";

    Unique<Texture> GetTexture() const { return m_CachedTexture ? MakeUnique<Texture>(*m_CachedTexture) : nullptr; }

private:
    Unique<Texture> m_CachedTexture;
};

} // namespace Terrain
