#include "Heightfield.h"
#include "Core/Logger.h"
#include <algorithm>
#include <cstring>

namespace Terrain {

Heightfield::Heightfield(uint32 width, uint32 height)
    : m_Width(width), m_Height(height) {
    m_Data.resize(width * height, 0.0f);
}

Heightfield::~Heightfield() {
}

float32 Heightfield::GetHeight(uint32 x, uint32 y) const {
    if (x >= m_Width || y >= m_Height) return 0.0f;
    return m_Data[y * m_Width + x];
}

void Heightfield::SetHeight(uint32 x, uint32 y, float32 height) {
    if (x >= m_Width || y >= m_Height) return;
    m_Data[y * m_Width + x] = height;
}

void Heightfield::AllocateGPUBuffer(BufferManager* bufferMgr) {
    VkDeviceSize bufferSize = m_Width * m_Height * sizeof(float32);
    m_GPUBuffer = bufferMgr->CreateStorageBuffer(bufferSize);
    LOG_INFO("Allocated GPU buffer for %dx%d heightfield (%zu MB)",
             m_Width, m_Height, bufferSize / (1024 * 1024));
}

void Heightfield::UploadToGPU(BufferManager* bufferMgr) {
    if (!m_GPUBuffer.IsValid()) {
        LOG_ERROR("GPU buffer not allocated");
        return;
    }

    // Create staging buffer
    VkDeviceSize bufferSize = m_Width * m_Height * sizeof(float32);
    BufferAllocation staging = bufferMgr->CreateStagingBuffer(bufferSize);

    // Copy data to staging
    void* data = bufferMgr->MapBuffer(staging);
    std::memcpy(data, m_Data.data(), bufferSize);
    bufferMgr->UnmapBuffer(staging);

    // TODO: Copy staging to device buffer (needs command buffer)
    // For now, we'll handle this in the generator

    bufferMgr->DestroyBuffer(staging);
}

void Heightfield::DownloadFromGPU(BufferManager* bufferMgr) {
    if (!m_GPUBuffer.IsValid()) {
        LOG_ERROR("GPU buffer not allocated");
        return;
    }

    // Create staging buffer
    VkDeviceSize bufferSize = m_Width * m_Height * sizeof(float32);
    BufferAllocation staging = bufferMgr->CreateStagingBuffer(bufferSize);

    // TODO: Copy device buffer to staging (needs command buffer)

    // Copy from staging to CPU
    void* data = bufferMgr->MapBuffer(staging);
    std::memcpy(m_Data.data(), data, bufferSize);
    bufferMgr->UnmapBuffer(staging);

    bufferMgr->DestroyBuffer(staging);

    LOG_INFO("Downloaded heightfield from GPU");
}

void Heightfield::Clear(float32 value) {
    std::fill(m_Data.begin(), m_Data.end(), value);
}

void Heightfield::Normalize(float32 minVal, float32 maxVal) {
    float32 currentMin = GetMin();
    float32 currentMax = GetMax();

    if (currentMax - currentMin < 0.0001f) {
        Clear((minVal + maxVal) * 0.5f);
        return;
    }

    for (auto& height : m_Data) {
        height = minVal + (height - currentMin) / (currentMax - currentMin) * (maxVal - minVal);
    }
}

float32 Heightfield::GetMin() const {
    return *std::min_element(m_Data.begin(), m_Data.end());
}

float32 Heightfield::GetMax() const {
    return *std::max_element(m_Data.begin(), m_Data.end());
}

} // namespace Terrain
