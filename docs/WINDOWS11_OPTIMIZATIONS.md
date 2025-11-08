# Windows 11 Optimizations for Terrain Engine

## Overview

This document details Windows 11 specific optimizations and features to maximize performance on high-end systems with powerful GPUs and multi-core CPUs.

---

## Hardware Assumptions

### Target System Specifications

**GPU**:
- NVIDIA RTX 4080/4090 or AMD RX 7900 XT/XTX
- 12-24 GB VRAM
- Vulkan 1.3 support
- Ray Tracing cores (for future AO/lighting)

**CPU**:
- Intel Core i9 13th/14th gen or AMD Ryzen 9 7000 series
- 16-32 cores / 32-64 threads
- AVX-512 or AVX2 support
- 32+ GB DDR5 RAM

**Storage**:
- NVMe Gen 4/5 SSD
- DirectStorage compatible

**Display**:
- 4K high-DPI display (3840x2160)
- 120+ Hz refresh rate for smooth viewport

---

## Windows 11 Specific Features

### 1. GPU Hardware Scheduling

**Feature**: Windows Display Driver Model (WDDM) 3.0 GPU scheduling

**Benefits**:
- Lower latency between CPU command submission and GPU execution
- Reduced CPU overhead for graphics operations
- Better multi-application GPU sharing

**Implementation**:
```cpp
// Ensure app is DPI-aware and optimized for WDDM 3.0
// Set process priority for GPU work
SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);

// Windows 11 supports fine-grained GPU preemption
// Vulkan automatically benefits, but we can hint:
VkDeviceQueueCreateInfo queueInfo{};
queueInfo.queuePriority = 1.0f; // High priority queue
```

**User Configuration**:
- Verify enabled in Windows Settings → System → Display → Graphics → Change default graphics settings
- Enable "Hardware-accelerated GPU scheduling"

### 2. DirectStorage Integration (Future)

**Feature**: Ultra-fast I/O for texture and heightfield streaming

**Current Status**: Plan for Phase 2 integration

**Benefits**:
- Bypass CPU for texture decompression
- Load 16K heightfields in < 100ms
- GPU-direct DMA from NVMe

**Planned API**:
```cpp
#include <dstorage.h>

class DirectStorageLoader {
    IDStorageFactory* factory;
    IDStorageQueue* queue;

    void LoadHeightfieldAsync(const char* path, VkBuffer destBuffer) {
        // Submit read request directly to GPU buffer
        DSTORAGE_REQUEST request{};
        request.Options.SourceType = DSTORAGE_REQUEST_SOURCE_FILE;
        request.Options.DestinationType = DSTORAGE_REQUEST_DESTINATION_BUFFER;
        request.Source.File.Source = fileHandle;
        request.Destination.Buffer.Resource = GetVkBufferHandle(destBuffer);

        queue->EnqueueRequest(&request);
        queue->Submit();
    }
};
```

### 3. Large Page Support

**Feature**: 2 MB memory pages for heightfield allocations

**Benefits**:
- Reduced TLB misses (up to 20% faster memory access)
- Better cache locality
- Less memory management overhead

**Implementation**:
```cpp
// Enable large pages in Windows
// Requires "Lock Pages in Memory" privilege
void* AllocateLargePage(size_t size) {
    // Round up to 2MB boundary
    size = (size + (2 * 1024 * 1024 - 1)) & ~(2 * 1024 * 1024 - 1);

    void* ptr = VirtualAlloc(
        nullptr,
        size,
        MEM_COMMIT | MEM_RESERVE | MEM_LARGE_PAGES,
        PAGE_READWRITE
    );

    if (!ptr) {
        // Fallback to standard allocation
        return VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    }
    return ptr;
}

// Use for heightfield buffers
struct Heightfield {
    void* data = AllocateLargePage(width * height * sizeof(float));
};
```

**Setup Instructions**:
1. Run `gpedit.msc`
2. Navigate to: Computer Configuration → Windows Settings → Security Settings → Local Policies → User Rights Assignment
3. Find "Lock pages in memory"
4. Add current user account

### 4. Windows Performance Recorder (WPR) Integration

**Feature**: Deep system profiling integration

**Implementation**:
```cpp
#include <evntprov.h>

class TerrainEngineETW {
    REGHANDLE regHandle;

    void Initialize() {
        EventRegister(&TERRAIN_ENGINE_PROVIDER_GUID, nullptr, nullptr, &regHandle);
    }

    void LogNodeExecution(const char* nodeName, float timeMs) {
        EVENT_DESCRIPTOR descriptor;
        EventDescCreate(&descriptor, 1, 0, 0, 4, 0, 0, 0);

        EventWrite(regHandle, &descriptor, 0, nullptr);
    }
};
```

**Usage**:
- Start WPR: `wpr -start GeneralProfile -start GPU`
- Run terrain generation
- Stop WPR: `wpr -stop terrain_trace.etl`
- Analyze in Windows Performance Analyzer (WPA)

### 5. High-DPI Awareness

**Feature**: Per-monitor DPI scaling v2

**Manifest** (`app.manifest`):
```xml
<?xml version="1.0" encoding="utf-8"?>
<assembly manifestVersion="1.0" xmlns="urn:schemas-microsoft-com:asm.v1">
  <application xmlns="urn:schemas-microsoft-com:asm.v3">
    <windowsSettings>
      <dpiAware xmlns="http://schemas.microsoft.com/SMI/2005/WindowsSettings">true/PM</dpiAware>
      <dpiAwareness xmlns="http://schemas.microsoft.com/SMI/2016/WindowsSettings">PerMonitorV2</dpiAwareness>
    </windowsSettings>
  </application>
</assembly>
```

**ImGui Configuration**:
```cpp
// Dynamically scale fonts for DPI
float dpiScale = GetDpiForWindow(hwnd) / 96.0f;
ImGui::GetIO().FontGlobalScale = dpiScale;

// Scale node editor
ImNodes::GetStyle().NodePadding *= dpiScale;
ImNodes::GetStyle().PinRadius *= dpiScale;
```

---

## CPU Optimizations

### 1. Thread Affinity & Priority

**Strategy**: Pin critical threads to performance cores

```cpp
#include <windows.h>

void OptimizeThreading() {
    // Get CPU topology
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    // Identify P-cores vs E-cores (12th gen+)
    // P-cores: 0-15, E-cores: 16-23 (example for i9-13900K)

    // Main render thread → P-core 0
    SetThreadAffinityMask(GetCurrentThread(), 1 << 0);
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

    // GPU command thread → P-core 1
    SetThreadAffinityMask(gpuThread, 1 << 1);
    SetThreadPriority(gpuThread, THREAD_PRIORITY_ABOVE_NORMAL);

    // Worker pool → E-cores for parallel work
    for (int i = 0; i < numWorkers; ++i) {
        SetThreadAffinityMask(workers[i], 1 << (16 + i));
    }
}
```

### 2. SIMD Acceleration

**AVX2/AVX-512**: Vectorized heightfield operations

```cpp
#include <immintrin.h>

// AVX2 heightfield blur (8 floats at once)
void BlurHeightfield_AVX2(float* data, int width, int height) {
    const __m256 kernel = _mm256_set1_ps(0.25f);

    for (int y = 1; y < height - 1; ++y) {
        for (int x = 0; x < width - 8; x += 8) {
            int idx = y * width + x;

            __m256 center = _mm256_loadu_ps(&data[idx]);
            __m256 left = _mm256_loadu_ps(&data[idx - 1]);
            __m256 right = _mm256_loadu_ps(&data[idx + 1]);
            __m256 up = _mm256_loadu_ps(&data[idx - width]);
            __m256 down = _mm256_loadu_ps(&data[idx + width]);

            __m256 sum = _mm256_add_ps(_mm256_add_ps(left, right),
                                       _mm256_add_ps(up, down));
            __m256 result = _mm256_mul_ps(sum, kernel);

            _mm256_storeu_ps(&data[idx], result);
        }
    }
}

// Runtime CPU detection
void DetectSIMD() {
    int cpuInfo[4];
    __cpuid(cpuInfo, 1);

    bool hasAVX2 = (cpuInfo[2] & (1 << 28)) != 0;
    bool hasAVX512 = false;

    __cpuid(cpuInfo, 7);
    hasAVX512 = (cpuInfo[1] & (1 << 16)) != 0;

    if (hasAVX512) {
        g_blurFunction = &BlurHeightfield_AVX512;
    } else if (hasAVX2) {
        g_blurFunction = &BlurHeightfield_AVX2;
    } else {
        g_blurFunction = &BlurHeightfield_Scalar;
    }
}
```

### 3. Cache Optimization

**L3 Cache Blocking**: Process heightfield in cache-sized chunks

```cpp
constexpr size_t L3_CACHE_SIZE = 32 * 1024 * 1024; // 32 MB typical
constexpr size_t BLOCK_SIZE = 512; // Fits in L3

void ProcessHeightfieldBlocked(float* data, int width, int height) {
    for (int yBlock = 0; yBlock < height; yBlock += BLOCK_SIZE) {
        for (int xBlock = 0; xBlock < width; xBlock += BLOCK_SIZE) {
            int yEnd = std::min(yBlock + BLOCK_SIZE, height);
            int xEnd = std::min(xBlock + BLOCK_SIZE, width);

            // Process block (hot in L3 cache)
            for (int y = yBlock; y < yEnd; ++y) {
                for (int x = xBlock; x < xEnd; ++x) {
                    ProcessPixel(data, x, y, width);
                }
            }
        }
    }
}
```

---

## GPU Optimizations

### 1. Vulkan Best Practices for NVIDIA/AMD

**NVIDIA-specific**:
```cpp
// Enable device-local host-visible memory (RTX 4000+)
VkPhysicalDeviceMemoryProperties memProps;
vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProps);

// Look for BAR-enabled memory type
for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i) {
    if ((memProps.memoryTypes[i].propertyFlags &
         (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) ==
        (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {

        // Use this for staging buffers - ultra-fast uploads
        barMemoryTypeIndex = i;
    }
}

// Shader occupancy hints
layout(local_size_x = 16, local_size_y = 16) in; // 256 threads = 8 warps (optimal)
```

**AMD-specific**:
```cpp
// Async compute overlap
VkDeviceQueueCreateInfo queues[2];
queues[0].queueFamilyIndex = graphicsQueueFamily;
queues[1].queueFamilyIndex = asyncComputeQueueFamily; // Separate family

// Execute erosion on async queue while rendering viewport
vkQueueSubmit(asyncComputeQueue, 1, &erosionSubmit, fence);
vkQueueSubmit(graphicsQueue, 1, &viewportSubmit, nullptr);
```

### 2. Shader Compiler Optimizations

**SPIR-V Optimization**:
```bash
# CMake integration
glslangValidator -V shader.comp -o shader.spv --target-env vulkan1.3
spirv-opt --O shader.spv -o shader_opt.spv

# Enable all optimizations:
# - Dead code elimination
# - Constant folding
# - Loop unrolling
# - Instruction combining
```

**Shader Code Patterns**:
```glsl
// Bad: Divergent branching
if (height > threshold) {
    erode();
} else {
    deposit();
}

// Good: Branchless
float erodeMask = step(threshold, height);
height = mix(deposit(), erode(), erodeMask);
```

### 3. Memory Bandwidth Optimization

**Texture Compression**:
```cpp
// Use BC6H for heightfield preview (6:1 compression)
VkImageCreateInfo imageInfo{};
imageInfo.format = VK_FORMAT_BC6H_SFLOAT_BLOCK; // HDR compression
imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

// Upload compressed preview to GPU
// 4K uncompressed: 64 MB
// 4K BC6H: ~11 MB (6x less bandwidth)
```

**Coalesced Memory Access**:
```glsl
// Compute shader work group layout
layout(local_size_x = 16, local_size_y = 16) in;

// Access pattern
ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
float height = texelFetch(heightmap, pixel, 0).r;

// Guarantees coalesced reads (16 threads = 1 cacheline)
```

### 4. Persistent Command Buffers

**Strategy**: Pre-record command buffers for common operations

```cpp
class PersistentPipeline {
    VkCommandBuffer cmdBuffer;
    VkPipeline pipeline;

    void RecordOnce() {
        vkBeginCommandBuffer(cmdBuffer, &beginInfo);

        vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
        vkCmdBindDescriptorSets(cmdBuffer, ...);
        vkCmdDispatch(cmdBuffer, workGroupsX, workGroupsY, 1);

        vkEndCommandBuffer(cmdBuffer);
    }

    void Execute() {
        // Just submit - no recording overhead
        vkQueueSubmit(queue, 1, &submitInfo, fence);
    }
};
```

---

## Power Management

### 1. GPU Power State Control

**Windows Power Plan**:
```cpp
// Force high-performance GPU mode
GUID* scheme = nullptr;
PowerGetActiveScheme(nullptr, &scheme);

// Ensure GPU max performance
GUID gpuPerf = {...}; // GUID_VIDEO_POWERDOWN_TIMEOUT
PowerWriteACValueIndex(nullptr, scheme, &GUID_VIDEO_SUBGROUP, &gpuPerf, 0);
PowerSetActiveScheme(nullptr, scheme);
```

### 2. CPU Governor

**Prevent throttling during long renders**:
```cpp
// Disable CPU parking
SYSTEM_POWER_POLICY policy;
policy.MinThrottle = 100; // 100% performance
CallNtPowerInformation(SystemPowerPolicy, &policy, sizeof(policy), nullptr, 0);

// Request sustained performance
PowerCreateRequest(&powerRequest);
PowerSetRequest(powerRequest, PowerRequestSystemRequired);
PowerSetRequest(powerRequest, PowerRequestDisplayRequired);
```

---

## Profiling & Diagnostics

### 1. NVIDIA Nsight Integration

**GPU Profiling**:
```cpp
// Markers for Nsight
#include <nvtx3/nvToolsExt.h>

void ExecuteNode(Node* node) {
    nvtxRangePushA(node->GetName());

    node->Execute();

    nvtxRangePop();
}

// Launch with: nsight-sys profile -o trace.qdrep terrain-engine.exe
```

### 2. AMD Radeon GPU Profiler

**RGP Capture**:
```cpp
#ifdef AMD_RGP_ENABLED
#include <rgp_client.h>

void TriggerCapture() {
    rgpClientBeginSession();
    ExecuteGraph();
    rgpClientEndSession();
}
#endif
```

### 3. Custom Telemetry

**Real-time Performance HUD**:
```cpp
struct PerformanceMetrics {
    float gpuTimeMs;
    float cpuTimeMs;
    size_t vramUsedMB;
    size_t ramUsedMB;
    float fps;

    void UpdateDisplay(ImGui) {
        ImGui::Text("GPU: %.2f ms", gpuTimeMs);
        ImGui::Text("CPU: %.2f ms", cpuTimeMs);
        ImGui::Text("VRAM: %zu MB", vramUsedMB);
        ImGui::PlotLines("FPS", fpsHistory, 100);
    }
};
```

---

## Installation & Deployment

### 1. Windows 11 Installer Features

**MSIX Package** (Modern App):
```xml
<?xml version="1.0" encoding="utf-8"?>
<Package xmlns="http://schemas.microsoft.com/appx/manifest/foundation/windows10">
  <Identity Name="TerrainEngine" Version="1.0.0.0" Publisher="CN=YourCompany" />
  <Properties>
    <DisplayName>Terrain Engine Pro</DisplayName>
  </Properties>
  <Capabilities>
    <Capability Name="runFullTrust" />
  </Capabilities>
</Package>
```

**Benefits**:
- Auto-updates via Microsoft Store
- Clean install/uninstall
- Sandboxed storage

### 2. Dependency Management

**Vulkan Runtime**:
```batch
@echo off
:: Check for Vulkan support
vulkaninfo > nul 2>&1
if errorlevel 1 (
    echo Installing Vulkan Runtime...
    VulkanRT-Installer.exe /S
)
```

**Visual C++ Redistributable**:
```cpp
// Statically link runtime to avoid dependency
// CMakeLists.txt:
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
```

### 3. GPU Driver Validation

**Startup Check**:
```cpp
void ValidateSystem() {
    // Check Vulkan version
    uint32_t apiVersion;
    vkEnumerateInstanceVersion(&apiVersion);

    if (VK_API_VERSION_MINOR(apiVersion) < 3) {
        ShowWarning("Vulkan 1.3+ required. Please update GPU drivers.");
    }

    // Check VRAM
    VkPhysicalDeviceMemoryProperties memProps;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProps);

    size_t totalVRAM = 0;
    for (uint32_t i = 0; i < memProps.memoryHeapCount; ++i) {
        if (memProps.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
            totalVRAM += memProps.memoryHeaps[i].size;
        }
    }

    if (totalVRAM < 4ULL * 1024 * 1024 * 1024) {
        ShowWarning("Recommended: 8+ GB VRAM for 4K+ terrains.");
    }
}
```

---

## Configuration File

**`config.ini`**:
```ini
[GPU]
EnableHardwareScheduling=true
PreferDedicatedGPU=true
MaxVRAMUsageMB=8192

[CPU]
ThreadPoolSize=0  ; 0 = auto-detect
EnableLargePages=true
PreferPerformanceCores=true

[Performance]
PreviewResolution=512
RealtimeUpdateRate=60
MaxUndoSteps=100

[Windows11]
EnableDirectStorage=false  ; Not yet implemented
UseETWProfiling=false
```

**Loading**:
```cpp
struct Config {
    static Config Load(const char* path) {
        Config cfg;
        INIReader reader(path);

        cfg.maxVRAMMB = reader.GetInteger("GPU", "MaxVRAMUsageMB", 8192);
        cfg.threadPoolSize = reader.GetInteger("CPU", "ThreadPoolSize", 0);

        return cfg;
    }
};
```

---

## Performance Targets

### Benchmarks (RTX 4090 + i9-13900K)

| Operation | Resolution | Target Time | Actual (Projected) |
|-----------|------------|-------------|---------------------|
| Perlin Noise | 4K | < 5 ms | 2 ms |
| Erosion (500 iter) | 4K | < 2.5 s | 2.1 s |
| Normal Map Gen | 4K | < 10 ms | 6 ms |
| Full Graph (10 nodes) | 4K | < 1 s | 0.8 s |
| Final Render | 16K | < 30 s | 22 s |

### Scaling Expectations

| GPU Tier | 4K Preview FPS | 16K Render Time |
|----------|----------------|-----------------|
| RTX 4090 / RX 7900 XTX | 60+ | 20-30s |
| RTX 4070 / RX 7800 XT | 45-60 | 40-60s |
| RTX 3060 / RX 6700 XT | 30-45 | 90-120s |

---

## Troubleshooting

### Common Issues

**1. Stuttering in Viewport**:
- **Cause**: CPU-GPU sync stalls
- **Fix**: Enable async compute, increase command buffer pre-recording

**2. Out of VRAM**:
- **Cause**: Large resolution + many node outputs cached
- **Fix**: Enable automatic cache eviction, reduce preview resolution

**3. Driver Timeout (TDR)**:
- **Cause**: Erosion shader running > 2 seconds
- **Fix**: Split into multiple dispatches with fence synchronization

**Registry Fix for TDR**:
```batch
reg add "HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\GraphicsDrivers" /v TdrDelay /t REG_DWORD /d 60 /f
```

---

## Conclusion

By leveraging Windows 11 features and optimizing for high-end hardware, the Terrain Engine can achieve:

- ✅ Real-time 4K preview at 60 FPS
- ✅ Sub-30 second 16K final renders
- ✅ Efficient multi-core CPU utilization
- ✅ Maximized GPU compute throughput
- ✅ Future-proof architecture (DirectStorage ready)

**Recommended Development Hardware**:
- Windows 11 Pro 23H2+
- RTX 4080 or better
- Ryzen 9 7950X / i9-13900K or better
- 32 GB DDR5 RAM
- 1 TB NVMe Gen 4 SSD

See `ARCHITECTURE.md` for overall system design.
