#include "HydraulicErosion.h"
#include "Core/Logger.h"
#include <cstring>

namespace Terrain {

HydraulicErosion::HydraulicErosion() {
}

HydraulicErosion::~HydraulicErosion() {
    Shutdown();
}

bool HydraulicErosion::Initialize(VulkanContext* context, BufferManager* bufferManager, CommandManager* commandManager) {
    m_VulkanContext = context;
    m_BufferManager = bufferManager;
    m_CommandManager = commandManager;

    // Create compute pipeline
    m_Pipeline = MakeUnique<ComputePipeline>(m_VulkanContext, m_BufferManager);

    if (!m_Pipeline->LoadShader("shaders/hydraulic_erosion.comp.spv")) {
        LOG_ERROR("Failed to load hydraulic erosion shader");
        return false;
    }

    if (!m_Pipeline->CreatePipeline()) {
        LOG_ERROR("Failed to create hydraulic erosion pipeline");
        return false;
    }

    LOG_INFO("Hydraulic erosion initialized");
    return true;
}

void HydraulicErosion::Shutdown() {
    m_Pipeline.reset();
}

bool HydraulicErosion::Erode(Heightfield& heightfield, const HydraulicErosionParams& params) {
    if (!m_Pipeline) {
        LOG_ERROR("Hydraulic erosion not initialized");
        return false;
    }

    uint32 width = heightfield.GetWidth();
    uint32 height = heightfield.GetHeight();

    // Ensure heightfield has GPU buffer
    if (!heightfield.GetGPUBuffer().buffer) {
        heightfield.AllocateGPUBuffer(m_BufferManager);
    }

    // Upload heightfield to GPU
    heightfield.UploadToGPU();

    // Prepare push constants
    struct PushConstants {
        uint32 resolutionX;
        uint32 resolutionY;
        uint32 iterations;
        uint32 seed;

        float32 inertia;
        float32 sedimentCapacity;
        float32 minSlope;
        float32 erodeSpeed;
        float32 depositSpeed;
        float32 evaporateSpeed;
        float32 gravity;
        float32 maxDropletLifetime;

        uint32 startSeed;
    } pushConstants;

    pushConstants.resolutionX = width;
    pushConstants.resolutionY = height;
    pushConstants.iterations = params.iterations;
    pushConstants.seed = params.seed;
    pushConstants.inertia = params.inertia;
    pushConstants.sedimentCapacity = params.sedimentCapacity;
    pushConstants.minSlope = params.minSlope;
    pushConstants.erodeSpeed = params.erodeSpeed;
    pushConstants.depositSpeed = params.depositSpeed;
    pushConstants.evaporateSpeed = params.evaporateSpeed;
    pushConstants.gravity = params.gravity;
    pushConstants.maxDropletLifetime = params.maxDropletLifetime;
    pushConstants.startSeed = params.seed;

    // Execute erosion
    VkCommandBuffer cmd = m_CommandManager->BeginSingleTimeCommands();

    m_Pipeline->Bind(cmd);
    m_Pipeline->UpdateDescriptorSet(0, heightfield.GetGPUBuffer());
    m_Pipeline->SetPushConstants(cmd, &pushConstants, sizeof(PushConstants));

    // Each invocation simulates one droplet
    m_Pipeline->Dispatch(cmd, params.iterations, 1, 1);

    // Memory barrier
    VkMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(
        cmd,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        0,
        1, &barrier,
        0, nullptr,
        0, nullptr
    );

    m_CommandManager->EndSingleTimeCommands(cmd);

    // Download result from GPU
    heightfield.DownloadFromGPU();

    return true;
}

} // namespace Terrain
