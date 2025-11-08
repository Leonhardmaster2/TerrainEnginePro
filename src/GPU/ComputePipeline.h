#pragma once

#include "Core/Types.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <string>

namespace Terrain {

class VulkanContext;

struct PushConstantData {
    uint32 resolutionX;
    uint32 resolutionY;
    float32 param1;
    float32 param2;
    int32 param3;
    float32 param4;
    float32 param5;
    uint32 seed;
};

class ComputePipeline {
public:
    ComputePipeline(VulkanContext* context);
    ~ComputePipeline();

    bool LoadShader(const std::string& spirvPath);
    bool CreatePipeline();

    void Bind(VkCommandBuffer cmd);
    void Dispatch(VkCommandBuffer cmd, uint32 groupsX, uint32 groupsY, uint32 groupsZ = 1);
    void SetPushConstants(VkCommandBuffer cmd, const PushConstantData& data);

    void BindBuffer(uint32 binding, VkBuffer buffer);
    void UpdateDescriptorSet();

    VkDescriptorSetLayout GetDescriptorSetLayout() const { return m_DescriptorSetLayout; }

private:
    bool CreateDescriptorSetLayout();
    bool CreateDescriptorPool();
    bool AllocateDescriptorSet();

    VulkanContext* m_Context;

    VkShaderModule m_ShaderModule = VK_NULL_HANDLE;
    VkPipeline m_Pipeline = VK_NULL_HANDLE;
    VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;

    VkDescriptorSetLayout m_DescriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
    VkDescriptorSet m_DescriptorSet = VK_NULL_HANDLE;

    std::vector<VkBuffer> m_BoundBuffers;
};

} // namespace Terrain
