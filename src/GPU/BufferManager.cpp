#include "BufferManager.h"
#include "VulkanContext.h"
#include "Core/Logger.h"

namespace Terrain {

BufferManager::BufferManager(VulkanContext* context)
    : m_Context(context) {
}

BufferManager::~BufferManager() {
}

BufferAllocation BufferManager::CreateBuffer(VkDeviceSize size,
                                             VkBufferUsageFlags usage,
                                             VkMemoryPropertyFlags properties) {
    BufferAllocation allocation;
    allocation.size = size;

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(m_Context->GetDevice(), &bufferInfo, nullptr, &allocation.buffer) != VK_SUCCESS) {
        LOG_ERROR("Failed to create buffer");
        return allocation;
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_Context->GetDevice(), allocation.buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = m_Context->FindMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(m_Context->GetDevice(), &allocInfo, nullptr, &allocation.memory) != VK_SUCCESS) {
        LOG_ERROR("Failed to allocate buffer memory");
        vkDestroyBuffer(m_Context->GetDevice(), allocation.buffer, nullptr);
        allocation.buffer = VK_NULL_HANDLE;
        return allocation;
    }

    vkBindBufferMemory(m_Context->GetDevice(), allocation.buffer, allocation.memory, 0);

    return allocation;
}

BufferAllocation BufferManager::CreateStagingBuffer(VkDeviceSize size) {
    return CreateBuffer(size,
                       VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
}

BufferAllocation BufferManager::CreateStorageBuffer(VkDeviceSize size) {
    return CreateBuffer(size,
                       VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
}

void* BufferManager::MapBuffer(BufferAllocation& allocation) {
    if (allocation.mapped) {
        return allocation.mapped;
    }

    vkMapMemory(m_Context->GetDevice(), allocation.memory, 0, allocation.size, 0, &allocation.mapped);
    return allocation.mapped;
}

void BufferManager::UnmapBuffer(BufferAllocation& allocation) {
    if (allocation.mapped) {
        vkUnmapMemory(m_Context->GetDevice(), allocation.memory);
        allocation.mapped = nullptr;
    }
}

void BufferManager::CopyBuffer(VkCommandBuffer cmd, VkBuffer src, VkBuffer dst, VkDeviceSize size) {
    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(cmd, src, dst, 1, &copyRegion);
}

void BufferManager::DestroyBuffer(BufferAllocation& allocation) {
    if (allocation.buffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(m_Context->GetDevice(), allocation.buffer, nullptr);
        allocation.buffer = VK_NULL_HANDLE;
    }

    if (allocation.memory != VK_NULL_HANDLE) {
        vkFreeMemory(m_Context->GetDevice(), allocation.memory, nullptr);
        allocation.memory = VK_NULL_HANDLE;
    }

    allocation.mapped = nullptr;
    allocation.size = 0;
}

} // namespace Terrain
