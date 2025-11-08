#pragma once

#include "Core/Types.h"
#include "GPU/BufferManager.h"
#include <vector>

namespace Terrain {

class Heightfield {
public:
    Heightfield(uint32 width, uint32 height);
    ~Heightfield();

    // Dimensions
    uint32 GetWidth() const { return m_Width; }
    uint32 GetHeight() const { return m_Height; }
    uint32 GetPixelCount() const { return m_Width * m_Height; }

    // CPU data
    const std::vector<float32>& GetData() const { return m_Data; }
    std::vector<float32>& GetDataMutable() { return m_Data; }
    float32 GetHeight(uint32 x, uint32 y) const;
    void SetHeight(uint32 x, uint32 y, float32 height);

    // GPU buffer
    void AllocateGPUBuffer(BufferManager* bufferMgr);
    void UploadToGPU(BufferManager* bufferMgr);
    void DownloadFromGPU(BufferManager* bufferMgr);
    BufferAllocation& GetGPUBuffer() { return m_GPUBuffer; }

    // Operations
    void Clear(float32 value = 0.0f);
    void Normalize(float32 minVal = 0.0f, float32 maxVal = 1.0f);
    float32 GetMin() const;
    float32 GetMax() const;

private:
    uint32 m_Width;
    uint32 m_Height;
    std::vector<float32> m_Data;
    BufferAllocation m_GPUBuffer;
};

} // namespace Terrain
