#pragma once

#include "Core/Types.h"
#include "Terrain/Heightfield.h"
#include "GPU/VulkanContext.h"
#include "GPU/BufferManager.h"
#include "GPU/ComputePipeline.h"
#include "GPU/CommandManager.h"

namespace Terrain {

struct HydraulicErosionParams {
    uint32 iterations = 100000;     // Number of droplets
    uint32 seed = 12345;

    float32 inertia = 0.05f;        // How much velocity is retained (0-1)
    float32 sedimentCapacity = 4.0f; // Max sediment carried
    float32 minSlope = 0.01f;       // Minimum slope for erosion
    float32 erodeSpeed = 0.3f;      // Erosion rate (0-1)
    float32 depositSpeed = 0.3f;    // Deposition rate (0-1)
    float32 evaporateSpeed = 0.01f; // Water evaporation rate (0-1)
    float32 gravity = 4.0f;         // Gravity strength
    float32 maxDropletLifetime = 30.0f; // Max steps per droplet
};

class HydraulicErosion {
public:
    HydraulicErosion();
    ~HydraulicErosion();

    bool Initialize(VulkanContext* context, BufferManager* bufferManager, CommandManager* commandManager);
    void Shutdown();

    // Apply erosion to heightfield
    bool Erode(Heightfield& heightfield, const HydraulicErosionParams& params);

    // Get/set parameters
    const HydraulicErosionParams& GetParams() const { return m_Params; }
    void SetParams(const HydraulicErosionParams& params) { m_Params = params; }

private:
    VulkanContext* m_VulkanContext = nullptr;
    BufferManager* m_BufferManager = nullptr;
    CommandManager* m_CommandManager = nullptr;

    Unique<ComputePipeline> m_Pipeline;
    HydraulicErosionParams m_Params;
};

} // namespace Terrain
