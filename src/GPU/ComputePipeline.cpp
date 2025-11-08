#include "ComputePipeline.h"
#include "VulkanContext.h"
#include "Core/Logger.h"
#include <fstream>

namespace Terrain {

ComputePipeline::ComputePipeline(VulkanContext* context)
    : m_Context(context) {
    m_BoundBuffers.resize(4, VK_NULL_HANDLE);
}

ComputePipeline::~ComputePipeline() {
    if (m_Pipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(m_Context->GetDevice(), m_Pipeline, nullptr);
    }
    if (m_PipelineLayout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(m_Context->GetDevice(), m_PipelineLayout, nullptr);
    }
    if (m_ShaderModule != VK_NULL_HANDLE) {
        vkDestroyShaderModule(m_Context->GetDevice(), m_ShaderModule, nullptr);
    }
    if (m_DescriptorPool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(m_Context->GetDevice(), m_DescriptorPool, nullptr);
    }
    if (m_DescriptorSetLayout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(m_Context->GetDevice(), m_DescriptorSetLayout, nullptr);
    }
}

bool ComputePipeline::LoadShader(const std::string& spirvPath) {
    std::ifstream file(spirvPath, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        LOG_ERROR("Failed to open shader file: %s", spirvPath.c_str());
        return false;
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = buffer.size();
    createInfo.pCode = reinterpret_cast<const uint32*>(buffer.data());

    if (vkCreateShaderModule(m_Context->GetDevice(), &createInfo, nullptr, &m_ShaderModule) != VK_SUCCESS) {
        LOG_ERROR("Failed to create shader module");
        return false;
    }

    LOG_INFO("Loaded shader: %s", spirvPath.c_str());
    return true;
}

bool ComputePipeline::CreateDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding bindings[4] = {};

    for (int i = 0; i < 4; i++) {
        bindings[i].binding = i;
        bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        bindings[i].descriptorCount = 1;
        bindings[i].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 4;
    layoutInfo.pBindings = bindings;

    if (vkCreateDescriptorSetLayout(m_Context->GetDevice(), &layoutInfo, nullptr, &m_DescriptorSetLayout) != VK_SUCCESS) {
        LOG_ERROR("Failed to create descriptor set layout");
        return false;
    }

    return true;
}

bool ComputePipeline::CreateDescriptorPool() {
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSize.descriptorCount = 4;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = 1;

    if (vkCreateDescriptorPool(m_Context->GetDevice(), &poolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS) {
        LOG_ERROR("Failed to create descriptor pool");
        return false;
    }

    return true;
}

bool ComputePipeline::AllocateDescriptorSet() {
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_DescriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_DescriptorSetLayout;

    if (vkAllocateDescriptorSets(m_Context->GetDevice(), &allocInfo, &m_DescriptorSet) != VK_SUCCESS) {
        LOG_ERROR("Failed to allocate descriptor set");
        return false;
    }

    return true;
}

bool ComputePipeline::CreatePipeline() {
    if (!CreateDescriptorSetLayout()) return false;
    if (!CreateDescriptorPool()) return false;
    if (!AllocateDescriptorSet()) return false;

    // Push constants
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(PushConstantData);

    // Pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &m_DescriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    if (vkCreatePipelineLayout(m_Context->GetDevice(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS) {
        LOG_ERROR("Failed to create pipeline layout");
        return false;
    }

    // Compute pipeline
    VkComputePipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    pipelineInfo.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    pipelineInfo.stage.module = m_ShaderModule;
    pipelineInfo.stage.pName = "main";
    pipelineInfo.layout = m_PipelineLayout;

    if (vkCreateComputePipelines(m_Context->GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_Pipeline) != VK_SUCCESS) {
        LOG_ERROR("Failed to create compute pipeline");
        return false;
    }

    LOG_INFO("Compute pipeline created successfully");
    return true;
}

void ComputePipeline::Bind(VkCommandBuffer cmd) {
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_Pipeline);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_PipelineLayout, 0, 1, &m_DescriptorSet, 0, nullptr);
}

void ComputePipeline::Dispatch(VkCommandBuffer cmd, uint32 groupsX, uint32 groupsY, uint32 groupsZ) {
    vkCmdDispatch(cmd, groupsX, groupsY, groupsZ);
}

void ComputePipeline::SetPushConstants(VkCommandBuffer cmd, const PushConstantData& data) {
    vkCmdPushConstants(cmd, m_PipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(PushConstantData), &data);
}

void ComputePipeline::BindBuffer(uint32 binding, VkBuffer buffer) {
    if (binding < m_BoundBuffers.size()) {
        m_BoundBuffers[binding] = buffer;
    }
}

void ComputePipeline::UpdateDescriptorSet() {
    std::vector<VkWriteDescriptorSet> descriptorWrites;
    std::vector<VkDescriptorBufferInfo> bufferInfos;

    for (size_t i = 0; i < m_BoundBuffers.size(); i++) {
        if (m_BoundBuffers[i] != VK_NULL_HANDLE) {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = m_BoundBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = VK_WHOLE_SIZE;
            bufferInfos.push_back(bufferInfo);

            VkWriteDescriptorSet write{};
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.dstSet = m_DescriptorSet;
            write.dstBinding = static_cast<uint32>(i);
            write.dstArrayElement = 0;
            write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            write.descriptorCount = 1;
            write.pBufferInfo = &bufferInfos.back();
            descriptorWrites.push_back(write);
        }
    }

    if (!descriptorWrites.empty()) {
        vkUpdateDescriptorSets(m_Context->GetDevice(), static_cast<uint32>(descriptorWrites.size()),
                              descriptorWrites.data(), 0, nullptr);
    }
}

} // namespace Terrain
