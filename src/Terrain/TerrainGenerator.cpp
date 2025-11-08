#include "TerrainGenerator.h"
#include "Core/Logger.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#include <fstream>
#include <algorithm>

namespace Terrain {

TerrainGenerator::TerrainGenerator() {
}

TerrainGenerator::~TerrainGenerator() {
    Shutdown();
}

bool TerrainGenerator::Initialize() {
    LOG_INFO("Initializing Terrain Generator...");

    // Initialize Vulkan
    m_VulkanContext = MakeUnique<VulkanContext>();
    if (!m_VulkanContext->Initialize(true)) {
        LOG_ERROR("Failed to initialize Vulkan");
        return false;
    }

    // Create managers
    m_BufferManager = MakeUnique<BufferManager>(m_VulkanContext.get());
    m_CommandManager = MakeUnique<CommandManager>(m_VulkanContext.get());

    if (!m_CommandManager->Initialize()) {
        LOG_ERROR("Failed to initialize command manager");
        return false;
    }

    // Load Perlin pipeline
    m_PerlinPipeline = MakeUnique<ComputePipeline>(m_VulkanContext.get());
    if (!m_PerlinPipeline->LoadShader("shaders/perlin_noise.comp.spv")) {
        LOG_ERROR("Failed to load Perlin shader");
        return false;
    }

    if (!m_PerlinPipeline->CreatePipeline()) {
        LOG_ERROR("Failed to create Perlin pipeline");
        return false;
    }

    LOG_INFO("Terrain Generator initialized successfully");
    return true;
}

void TerrainGenerator::Shutdown() {
    m_PerlinPipeline.reset();
    m_CommandManager.reset();
    m_BufferManager.reset();
    m_VulkanContext.reset();
}

Unique<Heightfield> TerrainGenerator::GeneratePerlin(uint32 width, uint32 height, const PerlinParams& params) {
    LOG_INFO("Generating %dx%d Perlin terrain...", width, height);

    // Create heightfield
    auto heightfield = MakeUnique<Heightfield>(width, height);

    // Allocate GPU buffer
    heightfield->AllocateGPUBuffer(m_BufferManager.get());

    // Bind buffer to pipeline
    m_PerlinPipeline->BindBuffer(0, heightfield->GetGPUBuffer().buffer);
    m_PerlinPipeline->UpdateDescriptorSet();

    // Setup push constants
    PushConstantData pushData{};
    pushData.resolutionX = width;
    pushData.resolutionY = height;
    pushData.param1 = params.frequency;
    pushData.param2 = params.amplitude;
    pushData.param3 = params.octaves;
    pushData.param4 = params.lacunarity;
    pushData.param5 = params.persistence;
    pushData.seed = params.seed;

    // Execute compute shader
    VkCommandBuffer cmd = m_CommandManager->BeginSingleTimeCommands();

    m_PerlinPipeline->Bind(cmd);
    m_PerlinPipeline->SetPushConstants(cmd, pushData);

    uint32 groupsX = (width + 15) / 16;
    uint32 groupsY = (height + 15) / 16;
    m_PerlinPipeline->Dispatch(cmd, groupsX, groupsY, 1);

    // Barrier
    VkBufferMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.buffer = heightfield->GetGPUBuffer().buffer;
    barrier.size = VK_WHOLE_SIZE;

    vkCmdPipelineBarrier(cmd,
                        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                        VK_PIPELINE_STAGE_TRANSFER_BIT,
                        0, 0, nullptr, 1, &barrier, 0, nullptr);

    // Copy to staging buffer for CPU read
    VkDeviceSize bufferSize = width * height * sizeof(float32);
    BufferAllocation staging = m_BufferManager->CreateStagingBuffer(bufferSize);

    m_BufferManager->CopyBuffer(cmd, heightfield->GetGPUBuffer().buffer, staging.buffer, bufferSize);

    m_CommandManager->EndSingleTimeCommands(cmd);

    // Copy from staging to CPU
    void* data = m_BufferManager->MapBuffer(staging);
    std::memcpy(heightfield->GetDataMutable().data(), data, bufferSize);
    m_BufferManager->UnmapBuffer(staging);

    m_BufferManager->DestroyBuffer(staging);

    LOG_INFO("Perlin terrain generated successfully");
    return heightfield;
}

bool TerrainGenerator::ExportPNG(const Heightfield& heightfield, const String& filepath, bool use16Bit) {
    LOG_INFO("Exporting to PNG: %s", filepath.c_str());

    uint32 width = heightfield.GetWidth();
    uint32 height = heightfield.GetHeight();
    const auto& data = heightfield.GetData();

    if (use16Bit) {
        // Export as 16-bit grayscale
        std::vector<uint16> pixels(width * height);

        for (uint32 i = 0; i < pixels.size(); i++) {
            float32 value = std::clamp(data[i], 0.0f, 1.0f);
            pixels[i] = static_cast<uint16>(value * 65535.0f);
        }

        int result = stbi_write_png(filepath.c_str(), width, height, 1, pixels.data(), width * sizeof(uint16));
        if (!result) {
            LOG_ERROR("Failed to write PNG");
            return false;
        }
    } else {
        // Export as 8-bit grayscale
        std::vector<uint8> pixels(width * height);

        for (uint32 i = 0; i < pixels.size(); i++) {
            float32 value = std::clamp(data[i], 0.0f, 1.0f);
            pixels[i] = static_cast<uint8>(value * 255.0f);
        }

        int result = stbi_write_png(filepath.c_str(), width, height, 1, pixels.data(), width);
        if (!result) {
            LOG_ERROR("Failed to write PNG");
            return false;
        }
    }

    LOG_INFO("PNG exported successfully");
    return true;
}

bool TerrainGenerator::ExportRAW(const Heightfield& heightfield, const String& filepath) {
    LOG_INFO("Exporting to RAW: %s", filepath.c_str());

    std::ofstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        LOG_ERROR("Failed to open file for writing");
        return false;
    }

    const auto& data = heightfield.GetData();
    file.write(reinterpret_cast<const char*>(data.data()), data.size() * sizeof(float32));
    file.close();

    LOG_INFO("RAW exported successfully");
    return true;
}

} // namespace Terrain
