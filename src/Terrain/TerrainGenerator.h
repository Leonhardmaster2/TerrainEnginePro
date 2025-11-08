#pragma once

#include "Core/Types.h"
#include "Heightfield.h"
#include "GPU/VulkanContext.h"
#include "GPU/BufferManager.h"
#include "GPU/CommandManager.h"
#include "GPU/ComputePipeline.h"
#include <memory>

namespace Terrain {

struct PerlinParams {
    float32 frequency = 1.0f;
    float32 amplitude = 1.0f;
    int32 octaves = 6;
    float32 lacunarity = 2.0f;
    float32 persistence = 0.5f;
    uint32 seed = 12345;
};

class TerrainGenerator {
public:
    TerrainGenerator();
    ~TerrainGenerator();

    bool Initialize();
    void Shutdown();

    // Generation
    Unique<Heightfield> GeneratePerlin(uint32 width, uint32 height, const PerlinParams& params);

    // Export
    bool ExportPNG(const Heightfield& heightfield, const String& filepath, bool use16Bit = true);
    bool ExportRAW(const Heightfield& heightfield, const String& filepath);

private:
    Unique<VulkanContext> m_VulkanContext;
    Unique<BufferManager> m_BufferManager;
    Unique<CommandManager> m_CommandManager;
    Unique<ComputePipeline> m_PerlinPipeline;
};

} // namespace Terrain
