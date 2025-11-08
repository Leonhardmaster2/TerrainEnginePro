#pragma once

#include "Core/Types.h"
#include <vulkan/vulkan.h>
#include <vector>

namespace Terrain {

class VulkanContext;

struct BufferAllocation {
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkDeviceSize size = 0;
    void* mapped = nullptr;

    bool IsValid() const { return buffer != VK_NULL_HANDLE; }
};

class BufferManager {
public:
    BufferManager(VulkanContext* context);
    ~BufferManager();

    // Buffer creation
    BufferAllocation CreateBuffer(VkDeviceSize size,
                                  VkBufferUsageFlags usage,
                                  VkMemoryPropertyFlags properties);

    BufferAllocation CreateStagingBuffer(VkDeviceSize size);
    BufferAllocation CreateStorageBuffer(VkDeviceSize size);

    // Memory operations
    void* MapBuffer(BufferAllocation& allocation);
    void UnmapBuffer(BufferAllocation& allocation);
    void CopyBuffer(VkCommandBuffer cmd, VkBuffer src, VkBuffer dst, VkDeviceSize size);

    // Cleanup
    void DestroyBuffer(BufferAllocation& allocation);

private:
    VulkanContext* m_Context;
};

} // namespace Terrain
