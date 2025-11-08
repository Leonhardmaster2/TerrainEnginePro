#pragma once

#include "Core/Types.h"
#include "Core/Logger.h"
#include <vulkan/vulkan.h>
#include <vector>

namespace Terrain {

struct QueueFamilyIndices {
    std::optional<uint32> graphicsFamily;
    std::optional<uint32> computeFamily;
    std::optional<uint32> transferFamily;

    bool IsComplete() const {
        return graphicsFamily.has_value() && computeFamily.has_value();
    }
};

class VulkanContext {
public:
    VulkanContext();
    ~VulkanContext();

    bool Initialize(bool enableValidation = true);
    void Shutdown();

    // Getters
    VkInstance GetInstance() const { return m_Instance; }
    VkPhysicalDevice GetPhysicalDevice() const { return m_PhysicalDevice; }
    VkDevice GetDevice() const { return m_Device; }
    VkQueue GetGraphicsQueue() const { return m_GraphicsQueue; }
    VkQueue GetComputeQueue() const { return m_ComputeQueue; }
    const QueueFamilyIndices& GetQueueFamilies() const { return m_QueueFamilies; }

    // Helper functions
    uint32 FindMemoryType(uint32 typeFilter, VkMemoryPropertyFlags properties);

private:
    bool CreateInstance(bool enableValidation);
    bool SelectPhysicalDevice();
    bool CreateLogicalDevice();
    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
    int RateDeviceSuitability(VkPhysicalDevice device);

    VkInstance m_Instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
    VkDevice m_Device = VK_NULL_HANDLE;

    VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
    VkQueue m_ComputeQueue = VK_NULL_HANDLE;
    VkQueue m_TransferQueue = VK_NULL_HANDLE;

    QueueFamilyIndices m_QueueFamilies;

    VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;

    std::vector<const char*> m_ValidationLayers;
    std::vector<const char*> m_DeviceExtensions;
};

} // namespace Terrain
