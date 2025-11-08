# GPU Pipeline Architecture - Vulkan Compute Design

## Overview

This document details the Vulkan compute pipeline architecture for the Terrain Engine, focusing on efficient GPU utilization for heightfield processing, erosion simulation, and texture generation.

---

## Vulkan Architecture

### Device Selection Strategy

```cpp
struct GPUSelector {
    VkPhysicalDevice SelectBestDevice(const std::vector<VkPhysicalDevice>& devices) {
        int bestScore = -1;
        VkPhysicalDevice bestDevice = VK_NULL_HANDLE;

        for (auto device : devices) {
            VkPhysicalDeviceProperties props;
            vkGetPhysicalDeviceProperties(device, &props);

            int score = 0;

            // Prefer discrete GPU
            if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                score += 1000;
            }

            // More VRAM = better
            VkPhysicalDeviceMemoryProperties memProps;
            vkGetPhysicalDeviceMemoryProperties(device, &memProps);
            size_t vramMB = GetDeviceLocalMemoryMB(memProps);
            score += vramMB / 100;

            // Compute queue required
            if (!HasComputeQueue(device)) {
                continue; // Skip
            }

            // Prefer Vulkan 1.3+
            if (props.apiVersion >= VK_API_VERSION_1_3) {
                score += 100;
            }

            if (score > bestScore) {
                bestScore = score;
                bestDevice = device;
            }
        }

        return bestDevice;
    }
};
```

### Queue Architecture

**Queue Families**:
1. **Graphics Queue**: Viewport rendering (3D preview)
2. **Compute Queue**: Terrain processing (dedicated or shared)
3. **Transfer Queue**: Async staging buffer uploads (optional)

**Configuration**:
```cpp
struct QueueConfig {
    uint32_t graphicsFamily;
    uint32_t computeFamily;
    uint32_t transferFamily;

    VkQueue graphicsQueue;
    VkQueue computeQueue;
    VkQueue transferQueue;
};

// Optimal: Dedicated async compute queue
// graphics: family 0
// compute: family 1 (if available)
// transfer: family 2 (if available)

// Fallback: Use graphics queue for all
```

**Benefits of Async Compute**:
- Overlap erosion simulation with viewport rendering
- Hide latency of GPU memory transfers
- Maximize GPU occupancy

---

## Memory Management

### Memory Architecture

```cpp
class VulkanMemoryManager {
public:
    // Use Vulkan Memory Allocator (VMA)
    VmaAllocator allocator;

    struct BufferAllocation {
        VkBuffer buffer;
        VmaAllocation allocation;
        void* mappedPtr; // For host-visible memory
    };

    void Initialize(VkDevice device, VkPhysicalDevice physicalDevice) {
        VmaAllocatorCreateInfo createInfo{};
        createInfo.device = device;
        createInfo.physicalDevice = physicalDevice;
        createInfo.instance = instance;

        vmaCreateAllocator(&createInfo, &allocator);
    }

    BufferAllocation CreateHeightfieldBuffer(uint32_t width, uint32_t height) {
        size_t size = width * height * sizeof(float);

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                           VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                           VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

        BufferAllocation result;
        vmaCreateBuffer(allocator, &bufferInfo, &allocInfo,
                        &result.buffer, &result.allocation, nullptr);

        return result;
    }

    BufferAllocation CreateStagingBuffer(size_t size) {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                           VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        allocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

        BufferAllocation result;
        VmaAllocationInfo vmaInfo;
        vmaCreateBuffer(allocator, &bufferInfo, &allocInfo,
                        &result.buffer, &result.allocation, &vmaInfo);

        result.mappedPtr = vmaInfo.pMappedData;

        return result;
    }
};
```

### Memory Pool Strategy

**Resource Categories**:

| Resource | Type | Size (4K) | Lifetime | Pool |
|----------|------|-----------|----------|------|
| Primary Heightfield | Buffer | 64 MB | Persistent | Static |
| Temporary Heightfield | Buffer | 64 MB | Per-node | Dynamic |
| Mask Layers | Buffer | 16 MB each | Persistent | Static |
| Erosion State (water, sediment) | Buffer | 128 MB | Erosion duration | Transient |
| Staging Buffers | Buffer | 64 MB | Per-transfer | Staging Pool |

**Pool Implementation**:
```cpp
class BufferPool {
    std::vector<BufferAllocation> availableBuffers;
    std::vector<BufferAllocation> inUseBuffers;
    size_t bufferSize;

public:
    BufferPool(size_t size, uint32_t initialCount) : bufferSize(size) {
        for (uint32_t i = 0; i < initialCount; ++i) {
            availableBuffers.push_back(CreateBuffer(size));
        }
    }

    BufferAllocation* Acquire() {
        if (availableBuffers.empty()) {
            // Grow pool
            availableBuffers.push_back(CreateBuffer(bufferSize));
        }

        auto buffer = availableBuffers.back();
        availableBuffers.pop_back();
        inUseBuffers.push_back(buffer);

        return &inUseBuffers.back();
    }

    void Release(BufferAllocation* buffer) {
        auto it = std::find(inUseBuffers.begin(), inUseBuffers.end(), *buffer);
        if (it != inUseBuffers.end()) {
            availableBuffers.push_back(*it);
            inUseBuffers.erase(it);
        }
    }
};
```

---

## Compute Pipeline Design

### Pipeline Layout

**Descriptor Set Layout**:
```cpp
// Set 0: Per-frame global data
layout(set = 0, binding = 0) uniform GlobalParams {
    uint resolutionX;
    uint resolutionY;
    float worldScale;
    float time;
};

// Set 1: Per-node input/output buffers
layout(set = 1, binding = 0) buffer InputHeightfield {
    float heights[];
};

layout(set = 1, binding = 1) buffer OutputHeightfield {
    float outHeights[];
};

layout(set = 1, binding = 2) buffer MaskBuffer {
    float masks[];
};

// Set 2: Per-node parameters (push constants alternative)
layout(set = 2, binding = 0) uniform NodeParams {
    float frequency;
    float amplitude;
    int octaves;
    float lacunarity;
    float persistence;
};
```

**Pipeline Cache**:
```cpp
class PipelineCache {
    std::unordered_map<std::string, VkPipeline> pipelines;
    VkPipelineCache vkCache;

public:
    void Initialize(VkDevice device) {
        // Load cache from disk
        std::vector<char> cacheData = LoadFile("pipeline.cache");

        VkPipelineCacheCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        createInfo.initialDataSize = cacheData.size();
        createInfo.pInitialData = cacheData.data();

        vkCreatePipelineCache(device, &createInfo, nullptr, &vkCache);
    }

    VkPipeline GetOrCreate(const char* shaderName, VkShaderModule shader) {
        if (pipelines.count(shaderName)) {
            return pipelines[shaderName];
        }

        VkComputePipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        pipelineInfo.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        pipelineInfo.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        pipelineInfo.stage.module = shader;
        pipelineInfo.stage.pName = "main";
        pipelineInfo.layout = pipelineLayout;

        VkPipeline pipeline;
        vkCreateComputePipelines(device, vkCache, 1, &pipelineInfo, nullptr, &pipeline);

        pipelines[shaderName] = pipeline;
        return pipeline;
    }

    void SaveToDisk() {
        size_t cacheSize;
        vkGetPipelineCacheData(device, vkCache, &cacheSize, nullptr);

        std::vector<char> cacheData(cacheSize);
        vkGetPipelineCacheData(device, vkCache, &cacheSize, cacheData.data());

        SaveFile("pipeline.cache", cacheData);
    }
};
```

---

## Shader Design

### Shader Organization

```
/Shaders/
  /Noise/
    perlin.comp
    simplex.comp
    voronoi.comp
  /Filters/
    blur.comp
    sharpen.comp
    combine.comp
  /Erosion/
    hydraulic_step1_water.comp
    hydraulic_step2_flow.comp
    hydraulic_step3_erosion.comp
    hydraulic_step4_evaporate.comp
    thermal.comp
  /Textures/
    normal_map.comp
    ambient_occlusion.comp
    splatmap.comp
  /Utilities/
    common.glsl          # Shared functions
    noise_lib.glsl       # Noise implementations
```

### Example Shader: Perlin Noise

```glsl
#version 450

layout(local_size_x = 16, local_size_y = 16) in;

// Global parameters
layout(set = 0, binding = 0) uniform GlobalParams {
    uint resolutionX;
    uint resolutionY;
    float worldScale;
};

// Output buffer
layout(set = 1, binding = 0) buffer OutputBuffer {
    float heights[];
};

// Node parameters
layout(push_constant) uniform PushConstants {
    float frequency;
    float amplitude;
    int octaves;
    float lacunarity;
    float persistence;
    uint seed;
};

// Include noise library
#include "noise_lib.glsl"

void main() {
    uvec2 pixel = gl_GlobalInvocationID.xy;

    if (pixel.x >= resolutionX || pixel.y >= resolutionY) {
        return;
    }

    vec2 uv = vec2(pixel) / vec2(resolutionX, resolutionY);
    vec2 pos = uv * worldScale;

    // FBM (Fractal Brownian Motion)
    float value = 0.0;
    float amp = amplitude;
    float freq = frequency;

    for (int i = 0; i < octaves; ++i) {
        value += perlinNoise(pos * freq + vec2(seed)) * amp;
        freq *= lacunarity;
        amp *= persistence;
    }

    // Write to output
    uint index = pixel.y * resolutionX + pixel.x;
    heights[index] = value;
}
```

### Example Shader: Hydraulic Erosion

```glsl
#version 450

layout(local_size_x = 16, local_size_y = 16) in;

layout(set = 0, binding = 0) uniform GlobalParams {
    uint resolutionX;
    uint resolutionY;
    float deltaTime;
};

// Input/output heightfield
layout(set = 1, binding = 0) buffer HeightBuffer {
    float heights[];
};

// Water layer
layout(set = 1, binding = 1) buffer WaterBuffer {
    float water[];
};

// Sediment layer
layout(set = 1, binding = 2) buffer SedimentBuffer {
    float sediment[];
};

// Velocity field
layout(set = 1, binding = 3) buffer VelocityBuffer {
    vec2 velocity[];
};

layout(push_constant) uniform Params {
    float rainRate;
    float evaporationRate;
    float sedimentCapacity;
    float erosionStrength;
    float depositionStrength;
};

// Shared memory for cache blocking
shared float s_heights[18][18]; // 16+2 border

void main() {
    uvec2 pixel = gl_GlobalInvocationID.xy;
    uvec2 local = gl_LocalInvocationID.xy;

    if (pixel.x >= resolutionX || pixel.y >= resolutionY) {
        return;
    }

    uint index = pixel.y * resolutionX + pixel.x;

    // Load into shared memory (with border)
    s_heights[local.y + 1][local.x + 1] = heights[index];

    // Load borders (boundary threads)
    if (local.x == 0 && pixel.x > 0) {
        s_heights[local.y + 1][0] = heights[index - 1];
    }
    if (local.x == 15 && pixel.x < resolutionX - 1) {
        s_heights[local.y + 1][17] = heights[index + 1];
    }
    // ... similar for y

    barrier();

    // Current state
    float h = s_heights[local.y + 1][local.x + 1];
    float w = water[index];
    float s = sediment[index];
    vec2 v = velocity[index];

    // Add rain
    w += rainRate * deltaTime;

    // Compute gradient (from shared memory - fast!)
    float hL = s_heights[local.y + 1][local.x];
    float hR = s_heights[local.y + 1][local.x + 2];
    float hU = s_heights[local.y][local.x + 1];
    float hD = s_heights[local.y + 2][local.x + 1];

    vec2 gradient = vec2(hR - hL, hD - hU) * 0.5;

    // Update velocity (simplified physics)
    v -= gradient * deltaTime * 9.81; // Gravity acceleration
    v *= 0.95; // Friction

    // Transport water
    vec2 offset = v * deltaTime;
    // ... bilinear sampling for water advection

    // Erosion/deposition
    float capacity = sedimentCapacity * length(v) * w;

    if (s < capacity) {
        // Erode
        float eroded = erosionStrength * (capacity - s) * deltaTime;
        h -= eroded;
        s += eroded;
    } else {
        // Deposit
        float deposited = depositionStrength * (s - capacity) * deltaTime;
        h += deposited;
        s -= deposited;
    }

    // Evaporation
    w *= (1.0 - evaporationRate * deltaTime);

    // Write back
    heights[index] = h;
    water[index] = w;
    sediment[index] = s;
    velocity[index] = v;
}
```

---

## Command Buffer Management

### Command Recording Strategy

**Approach 1: Immediate Recording** (Simple)
```cpp
void ExecuteNode(Node* node) {
    VkCommandBuffer cmd = BeginCommandBuffer();

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, node->GetPipeline());
    vkCmdBindDescriptorSets(cmd, ...);
    vkCmdPushConstants(cmd, ...);

    uint32_t groupsX = (width + 15) / 16;
    uint32_t groupsY = (height + 15) / 16;
    vkCmdDispatch(cmd, groupsX, groupsY, 1);

    EndCommandBuffer(cmd);
    SubmitAndWait(cmd);
}
```

**Approach 2: Batched Recording** (Optimal)
```cpp
void ExecuteGraph(const std::vector<Node*>& nodes) {
    VkCommandBuffer cmd = BeginCommandBuffer();

    for (auto node : nodes) {
        // Pipeline barrier for dependency
        if (node->HasDependency()) {
            VkBufferMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
            barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier.buffer = node->GetInputBuffer();

            vkCmdPipelineBarrier(cmd,
                VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                0, 0, nullptr, 1, &barrier, 0, nullptr);
        }

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, node->GetPipeline());
        vkCmdBindDescriptorSets(cmd, ...);
        vkCmdDispatch(cmd, ...);
    }

    EndCommandBuffer(cmd);
    SubmitAndWait(cmd);
}
```

### Synchronization

**Pipeline Barriers**:
```cpp
// Between dependent nodes
VkBufferMemoryBarrier barrier{};
barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;  // Previous node wrote
barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;   // Next node reads
barrier.buffer = intermediateBuffer;

vkCmdPipelineBarrier(cmd,
    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
    0, 0, nullptr, 1, &barrier, 0, nullptr);
```

**Timeline Semaphores** (Vulkan 1.2+):
```cpp
// For async compute overlap
VkSemaphoreTypeCreateInfo timelineInfo{};
timelineInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
timelineInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
timelineInfo.initialValue = 0;

VkSemaphoreCreateInfo semaphoreInfo{};
semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
semaphoreInfo.pNext = &timelineInfo;

VkSemaphore timelineSemaphore;
vkCreateSemaphore(device, &semaphoreInfo, nullptr, &timelineSemaphore);

// Submit erosion to compute queue
VkSubmitInfo computeSubmit{};
computeSubmit.signalSemaphoreCount = 1;
computeSubmit.pSignalSemaphores = &timelineSemaphore;
uint64_t signalValue = 1;
computeSubmit.pSignalSemaphoreValues = &signalValue;

vkQueueSubmit(computeQueue, 1, &computeSubmit, VK_NULL_HANDLE);

// Submit viewport render to graphics queue (waits for erosion)
VkSubmitInfo graphicsSubmit{};
graphicsSubmit.waitSemaphoreCount = 1;
graphicsSubmit.pWaitSemaphores = &timelineSemaphore;
uint64_t waitValue = 1;
graphicsSubmit.pWaitSemaphoreValues = &waitValue;

vkQueueSubmit(graphicsQueue, 1, &graphicsSubmit, VK_NULL_HANDLE);
```

---

## GPU Profiling & Debugging

### Timestamp Queries

```cpp
class GPUProfiler {
    VkQueryPool queryPool;
    std::vector<std::string> markerNames;

public:
    void Initialize(VkDevice device) {
        VkQueryPoolCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
        createInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
        createInfo.queryCount = 128; // Max markers

        vkCreateQueryPool(device, &createInfo, nullptr, &queryPool);
    }

    void BeginMarker(VkCommandBuffer cmd, const char* name) {
        uint32_t queryIndex = markerNames.size();
        markerNames.push_back(name);

        vkCmdWriteTimestamp(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, queryPool, queryIndex * 2);
    }

    void EndMarker(VkCommandBuffer cmd) {
        uint32_t queryIndex = (markerNames.size() - 1) * 2 + 1;
        vkCmdWriteTimestamp(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, queryPool, queryIndex);
    }

    void GetResults(VkDevice device) {
        std::vector<uint64_t> timestamps(markerNames.size() * 2);
        vkGetQueryPoolResults(device, queryPool, 0, timestamps.size(),
            timestamps.size() * sizeof(uint64_t), timestamps.data(),
            sizeof(uint64_t), VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);

        // Convert to milliseconds
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(physicalDevice, &props);
        float timestampPeriod = props.limits.timestampPeriod; // nanoseconds

        for (size_t i = 0; i < markerNames.size(); ++i) {
            uint64_t start = timestamps[i * 2];
            uint64_t end = timestamps[i * 2 + 1];
            float timeMs = (end - start) * timestampPeriod / 1000000.0f;

            printf("%s: %.3f ms\n", markerNames[i].c_str(), timeMs);
        }
    }
};

// Usage
profiler.BeginMarker(cmd, "Perlin Noise");
vkCmdDispatch(cmd, ...);
profiler.EndMarker(cmd);
```

### Debug Labels (RenderDoc, NSight)

```cpp
#ifdef _DEBUG
    VkDebugUtilsLabelEXT label{};
    label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
    label.pLabelName = "Hydraulic Erosion";
    label.color[0] = 1.0f;
    label.color[1] = 0.5f;
    label.color[2] = 0.0f;
    label.color[3] = 1.0f;

    vkCmdBeginDebugUtilsLabelEXT(cmd, &label);
    vkCmdDispatch(cmd, ...);
    vkCmdEndDebugUtilsLabelEXT(cmd);
#endif
```

---

## Shader Hot-Reload System

```cpp
class ShaderWatcher {
    std::unordered_map<std::string, VkShaderModule> shaderModules;
    std::unordered_map<std::string, time_t> lastModified;

public:
    void WatchDirectory(const char* path) {
        // Use filesystem watch API
#ifdef _WIN32
        HANDLE dirHandle = FindFirstChangeNotification(path, TRUE,
            FILE_NOTIFY_CHANGE_LAST_WRITE);
#endif
    }

    void CheckForChanges() {
        for (auto& [shaderPath, lastTime] : lastModified) {
            time_t currentTime = GetFileModificationTime(shaderPath);

            if (currentTime > lastTime) {
                // Recompile
                RecompileShader(shaderPath);
                lastModified[shaderPath] = currentTime;
            }
        }
    }

    void RecompileShader(const std::string& path) {
        // glslangValidator or shaderc
        std::string cmd = "glslangValidator -V " + path + " -o temp.spv";
        system(cmd.c_str());

        // Load SPIR-V
        std::vector<uint32_t> spirv = LoadSPIRV("temp.spv");

        // Destroy old module
        if (shaderModules.count(path)) {
            vkDestroyShaderModule(device, shaderModules[path], nullptr);
        }

        // Create new module
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = spirv.size() * sizeof(uint32_t);
        createInfo.pCode = spirv.data();

        VkShaderModule newModule;
        vkCreateShaderModule(device, &createInfo, nullptr, &newModule);

        shaderModules[path] = newModule;

        // Rebuild pipeline
        RebuildPipeline(path);
    }
};
```

---

## Optimization Techniques

### 1. Shared Memory for Cache Blocking

```glsl
layout(local_size_x = 16, local_size_y = 16) in;

shared float s_data[18][18]; // 16x16 + 1-pixel border

void main() {
    uvec2 local = gl_LocalInvocationID.xy;
    uvec2 global = gl_GlobalInvocationID.xy;

    // Load center
    s_data[local.y + 1][local.x + 1] = inputBuffer[global.y * width + global.x];

    // Load borders (threads on edges load neighbors)
    if (local.x == 0) {
        s_data[local.y + 1][0] = inputBuffer[global.y * width + global.x - 1];
    }

    barrier(); // Wait for all loads

    // Process using shared memory (fast!)
    float result = ProcessWithNeighbors(s_data, local.x + 1, local.y + 1);

    outputBuffer[global.y * width + global.x] = result;
}
```

### 2. Subgroup Operations (Wave Intrinsics)

```glsl
#extension GL_KHR_shader_subgroup_arithmetic : enable

void main() {
    float value = computeValue();

    // Reduce across waveform (32/64 threads)
    float waveSum = subgroupAdd(value);
    float waveMax = subgroupMax(value);

    if (subgroupElect()) {
        // First thread in wave
        outputSummary[gl_WorkGroupID.x] = waveSum;
    }
}
```

### 3. Async Compute Overlap

```cpp
// While GPU processes erosion on compute queue...
vkQueueSubmit(computeQueue, 1, &erosionSubmit, nullptr);

// ...CPU prepares next frame's commands
PrepareNextFrame();

// ...and graphics queue renders viewport simultaneously
vkQueueSubmit(graphicsQueue, 1, &viewportSubmit, nullptr);
```

---

## Error Handling

### Vulkan Validation Layers

```cpp
const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

VkInstanceCreateInfo createInfo{};
#ifdef _DEBUG
    createInfo.enabledLayerCount = validationLayers.size();
    createInfo.ppEnabledLayerNames = validationLayers.data();
#endif
```

### Robust Buffer Access

```glsl
#extension GL_EXT_robustness : enable

void main() {
    uvec2 pixel = gl_GlobalInvocationID.xy;

    // Vulkan validation will catch out-of-bounds access
    if (pixel.x < resolutionX && pixel.y < resolutionY) {
        uint index = pixel.y * resolutionX + pixel.x;
        heights[index] = computeHeight();
    }
}
```

---

## Performance Targets

### 4K Resolution (4096x4096)

| Operation | Dispatches | Target Time | Notes |
|-----------|------------|-------------|-------|
| Perlin Noise | 1 | < 5 ms | Embarrassingly parallel |
| Gaussian Blur | 2 (separable) | < 8 ms | Bandwidth-bound |
| Erosion (1 iter) | 4 | < 5 ms | Complex dependency |
| Normal Map | 1 | < 3 ms | Simple derivative |
| Splatmap Gen | 1 | < 4 ms | Rule-based |

**Total for 10-node graph**: ~50-100 ms (10-20 FPS preview)

With preview at 512x512: **< 2 ms total** (500+ FPS, limited to 60 FPS display)

---

## Conclusion

This GPU pipeline architecture provides:

- ✅ Efficient Vulkan compute pipeline
- ✅ Flexible descriptor management
- ✅ Optimal memory allocation strategies
- ✅ Comprehensive profiling support
- ✅ Shader hot-reload for rapid iteration
- ✅ Scalable from preview to production

**Next Steps**: Implement shader library, test on reference hardware.

See `ARCHITECTURE.md` for overall system context.
