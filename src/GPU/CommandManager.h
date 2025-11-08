#pragma once

#include "Core/Types.h"
#include <vulkan/vulkan.h>

namespace Terrain {

class VulkanContext;

class CommandManager {
public:
    CommandManager(VulkanContext* context);
    ~CommandManager();

    bool Initialize();

    // Single-time commands
    VkCommandBuffer BeginSingleTimeCommands();
    void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

    // Get command pool
    VkCommandPool GetCommandPool() const { return m_CommandPool; }

private:
    VulkanContext* m_Context;
    VkCommandPool m_CommandPool = VK_NULL_HANDLE;
};

} // namespace Terrain
