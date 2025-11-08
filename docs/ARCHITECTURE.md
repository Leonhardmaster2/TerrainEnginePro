# Terrain Engine - System Architecture Design

## Executive Summary

The Terrain Engine is a professional-grade, node-based terrain synthesis system designed to leverage modern GPU compute capabilities for real-time, high-resolution landscape generation. This document outlines the complete system architecture optimized for Windows 11 with high-performance GPU and CPU configurations.

## Design Philosophy

### Core Principles

1. **GPU-First Architecture**: All heavy computations execute on GPU via Vulkan compute shaders
2. **Non-Destructive Workflow**: Complete undo/redo support through graph-based processing
3. **Scalability**: From real-time preview (256x256) to production output (16384x16384+)
4. **Modularity**: Plugin-friendly architecture with clear module boundaries
5. **Performance**: Target 60 FPS for interactive preview, seconds for final rendering

### Key Features

- **Node-Based Procedural Generation**: Visual programming interface for terrain creation
- **Physically-Based Erosion**: Hydraulic, thermal, and sediment transport simulation
- **Multi-Resolution Pipeline**: Automatic LOD generation for preview and final output
- **Material System**: PBR-ready splatmaps, normal maps, and texture outputs
- **Tiled Workflow**: Support for ultra-high resolution terrains (16k+)
- **Cross-Platform Foundation**: Vulkan-based with Windows 11 primary target

---

## System Architecture Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                         Application Layer                        │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────────────┐  │
│  │  UI Layer    │  │  CLI Tool    │  │  Python Scripting    │  │
│  │  (ImGui)     │  │              │  │  API (pybind11)      │  │
│  └──────┬───────┘  └──────┬───────┘  └──────────┬───────────┘  │
└─────────┼──────────────────┼─────────────────────┼──────────────┘
          │                  │                     │
┌─────────┴──────────────────┴─────────────────────┴──────────────┐
│                         Engine Core                              │
│  ┌────────────────────────────────────────────────────────────┐ │
│  │              Node Graph System (Orchestrator)              │ │
│  │  • Graph Execution Planner  • Dependency Resolver          │ │
│  │  • Dirty Propagation        • Cache Management             │ │
│  └────────────┬───────────────────────────────┬────────────────┘ │
│               │                               │                  │
│  ┌────────────┴────────┐         ┌────────────┴─────────────┐   │
│  │   Terrain Modules   │         │    Processing Modules    │   │
│  │  • Data Model       │         │  • Generators            │   │
│  │  • Multi-res System │         │  • Modifiers/Filters     │   │
│  │  • Mask Layers      │         │  • Erosion Solvers       │   │
│  │  • Attachments      │         │  • Texture Generators    │   │
│  └─────────────────────┘         └──────────────────────────┘   │
└──────────────────────────────────────────┬───────────────────────┘
                                           │
┌──────────────────────────────────────────┴───────────────────────┐
│                      GPU Backend (Vulkan)                         │
│  ┌──────────────┐  ┌──────────────┐  ┌────────────────────────┐ │
│  │   Compute    │  │   Memory     │  │   Shader Compiler      │ │
│  │   Pipelines  │  │   Manager    │  │   (GLSL→SPIR-V)        │ │
│  │              │  │   (VMA)      │  │   Hot-Reload System    │ │
│  └──────────────┘  └──────────────┘  └────────────────────────┘ │
│  ┌──────────────────────────────────────────────────────────────┐│
│  │           GPU Resource Pool & Command Buffers                ││
│  └──────────────────────────────────────────────────────────────┘│
└──────────────────────────────────────────────────────────────────┘
                                ▼
                    ┌───────────────────────┐
                    │   Hardware (GPU/CPU)  │
                    │   NVIDIA/AMD/Intel    │
                    │   Multi-core CPU      │
                    └───────────────────────┘
```

---

## Module Breakdown

### 1. Core Systems (`/Core`)

**Responsibility**: Foundation services for all other modules

**Components**:
- **Logger**: Multi-level logging (Debug/Info/Warning/Error) with file output
- **Memory Manager**: Custom allocators for heightfield data, tracking, profiling
- **Profiler**: CPU timing + GPU timestamp queries
- **Threading**: Thread pool using `std::execution` or Intel TBB
- **Platform Layer**: OS-specific abstractions (file I/O, threading primitives)

**Key Interfaces**:
```cpp
class Logger {
    void Log(LogLevel level, const char* format, ...);
    void SetOutputFile(const std::string& path);
};

class Profiler {
    ScopedTimer BeginCPUTimer(const char* name);
    GPUTimer BeginGPUTimer(VkCommandBuffer cmd);
    void GenerateReport();
};
```

### 2. GPU Backend (`/GPU`)

**Responsibility**: Vulkan abstraction and compute pipeline management

**Components**:
- **VulkanContext**: Device initialization, queue management
- **ComputePipeline**: Shader compilation, pipeline cache
- **MemoryManager**: VMA integration for efficient GPU memory
- **BufferPool**: Reusable GPU buffers for heightfield/masks
- **CommandQueue**: Asynchronous command submission
- **ShaderHotReload**: Watch GLSL files, auto-recompile to SPIR-V

**Key Features**:
- Async compute queue for overlapping CPU/GPU work
- Persistent GPU buffers to minimize transfer overhead
- Descriptor set management for dynamic binding
- Timeline semaphores for fine-grained synchronization

**Performance Targets**:
- 4K terrain processing: < 16ms per node (60 FPS preview)
- 16K terrain final render: < 5 seconds for full graph
- Memory bandwidth utilization: > 80% of theoretical peak

### 3. Terrain Data Model (`/Terrain`)

**Responsibility**: Core data structures for heightfield and metadata

**Data Structures**:

```cpp
struct HeightfieldLayer {
    uint32_t width, height;
    VkBuffer gpuBuffer;          // GPU-side float32 data
    std::vector<float> cpuCache; // Optional CPU mirror
    VkImage mipChain;            // Multi-resolution pyramid
    bool isDirty;
};

struct TerrainAttachments {
    HeightfieldLayer* slope;         // Computed slope map
    HeightfieldLayer* curvature;     // Curvature (convex/concave)
    HeightfieldLayer* flowMap;       // Water flow direction/strength
    HeightfieldLayer* sediment;      // Sediment concentration
    HeightfieldLayer* moisture;      // Moisture/wetness map
};

class Terrain {
    HeightfieldLayer primaryHeight;
    TerrainAttachments attachments;
    std::vector<MaskLayer> masks;
    BoundingBox worldBounds;

    void ComputeDerivatives();       // Slope, curvature
    void UpdateMipChain();           // Generate LOD pyramid
    TileGrid* GetTileGrid();         // For 16k+ tiled mode
};
```

**Tiling System**:
- Tile size: 2048x2048 (configurable)
- Overlap region: 32 pixels for seamless blending
- Streaming: Load/unload tiles on demand
- Memory budget: Keep max 4 tiles in GPU memory simultaneously

### 4. Node Graph System (`/Nodes`)

**Responsibility**: Visual programming framework and execution engine

**Architecture**:

```cpp
class Node {
    NodeID id;
    std::string type;               // "Perlin", "Erosion", etc.
    std::vector<NodePin> inputs;
    std::vector<NodePin> outputs;
    ParameterSet parameters;        // Exposed to UI

    virtual void Execute(ExecutionContext& ctx) = 0;
    virtual void* GetOutput(int pinIndex) = 0;
    uint64_t GetCacheHash();        // For change detection
};

class NodeGraph {
    std::vector<Node*> nodes;
    std::vector<Connection> connections;

    ExecutionPlan BuildExecutionPlan();
    void Execute();
    void MarkDirty(NodeID from);
    void InvalidateCache(NodeID id);
};

class ExecutionPlanner {
    // Topological sort for dependency ordering
    std::vector<Node*> TopologicalSort(const NodeGraph& graph);

    // Detect cycles, optimize batching
    void OptimizeExecutionOrder(std::vector<Node*>& plan);
};
```

**Execution Strategy**:
1. **Dirty Propagation**: Only recompute affected nodes
2. **Incremental Execution**: Execute one node at a time with preview updates
3. **Cache Hit Detection**: Compare parameter hashes to skip unchanged nodes
4. **GPU Resource Reuse**: Pool and reuse intermediate buffers

**Node Categories**:

| Category | Nodes |
|----------|-------|
| **Generators** | Perlin, Simplex, Ridged, Billow, FBM, Voronoi, Gradient, Dome |
| **Modifiers** | Combine, Remap, Blur, Sharpen, Terrace, Clamp |
| **Erosion** | Hydraulic, Thermal, Sediment Flow, Weathering |
| **Masks** | Height Mask, Slope Mask, Curvature Mask, Noise Mask |
| **Textures** | Splatmap, Normal Map, AO, Roughness, Flow Map |
| **I/O** | File Input, File Output, Preview |

### 5. Erosion Simulation (`/Erosion`)

**Responsibility**: Physically-inspired geological processes

**Hydraulic Erosion**:
- **Algorithm**: Velocity-based water transport with sediment
- **GPU Kernels**:
  - Water source injection
  - Velocity field computation
  - Sediment erosion/deposition
  - Evaporation
- **Parameters**: Rain rate, evaporation, sediment capacity, erosion strength
- **Iterations**: 100-1000 steps for realistic results
- **Stability**: Semi-implicit integration, timestep limiting

**Thermal Erosion**:
- **Algorithm**: Talus angle relaxation
- **Talus Angle**: Configurable (typically 30-45°)
- **Iterative Relaxation**: Multi-pass until stable

**Sediment Transport**:
- **Flow Routing**: D-infinity algorithm for realistic flow
- **Channel Formation**: Erosion reinforcement along flow paths

**Performance**:
- Hydraulic: ~5ms per iteration @ 4K resolution on RTX 3080
- Target: 500 iterations in ~2.5 seconds

### 6. Texture & Material System (`/Texture`)

**Responsibility**: Generate PBR-ready texture maps

**Outputs**:
1. **Splatmap** (4-channel RGBA):
   - R: Rock (steep slopes)
   - G: Soil (medium slopes)
   - B: Grass (flat areas)
   - A: Snow (high altitude)

2. **Normal Map**: Tangent-space normals from heightfield
3. **Ambient Occlusion**: Multi-scale SSAO-like sampling
4. **Curvature Map**: Convex (ridges) vs. concave (valleys)
5. **Roughness Map**: Derived from slope/curvature
6. **Flow Map**: Water flow direction (R/G channels)

**Generation Rules**:
```cpp
// Pseudo-code for splatmap generation
float rockWeight = smoothstep(30°, 60°, slope);
float snowWeight = smoothstep(0.7, 0.9, normalizedHeight);
float grassWeight = (1.0 - rockWeight) * smoothstep(0.1, 0.3, moisture);
float soilWeight = 1.0 - (rockWeight + snowWeight + grassWeight);
```

**Export Formats**:
- Unreal Engine: Landscape layer info + weighted blend
- Unity: Terrain layer maps + material assets
- Generic: PNG/TGA/EXR with metadata JSON

### 7. I/O System (`/IO`)

**Supported Formats**:

| Format | Import | Export | Bit Depth | Use Case |
|--------|--------|--------|-----------|----------|
| GeoTIFF | ✅ | ✅ | 16/32 | Real-world DEM data |
| RAW | ✅ | ✅ | 16/32 | Simple interchange |
| EXR | ✅ | ✅ | 32 (float) | High dynamic range |
| PNG | ✅ | ✅ | 8/16 | Previews, masks |
| Unreal .r16 | ❌ | ✅ | 16 | Unreal heightmap |
| Unity .raw | ❌ | ✅ | 16 | Unity heightmap |

**Libraries**:
- GDAL: GeoTIFF, DEM
- tinyexr: EXR
- stb_image/write: PNG, TGA, JPG

### 8. UI Layer (`/UI`)

**Technology**: ImGui with docking branch

**Windows**:
1. **Node Graph Editor**:
   - Visual node editing with bezier connections
   - Context menu for node creation
   - Minimap for navigation
   - Grid snapping

2. **3D Viewport**:
   - Orbit camera controls
   - Real-time PBR shading
   - LOD visualization
   - Wireframe overlay

3. **Property Inspector**:
   - Auto-generated from node metadata
   - Real-time parameter adjustment
   - Preset management

4. **Asset Browser**:
   - Preset library
   - Texture preview
   - Import/export management

5. **Performance Panel**:
   - CPU/GPU profiler
   - Memory usage
   - Frametime graph

**Theming**:
- Professional dark theme (inspired by Substance Designer)
- Custom node colors by category
- Scalable fonts for high-DPI displays

### 9. CLI Tool (`/CLI`)

**Use Cases**:
- Batch terrain generation
- Render farm integration
- Automated testing

**Example Usage**:
```bash
terrain-cli --graph terrain.tgraph --output-dir ./renders --resolution 8192
terrain-cli --preset "Alpine Mountains" --seed 42 --export-formats unreal,unity
```

### 10. Python Scripting API (`/Scripting`)

**Technology**: pybind11

**Example API**:
```python
import terrainengine as te

# Create graph
graph = te.Graph()
perlin = graph.add_node("Perlin", octaves=6, frequency=1.0)
erosion = graph.add_node("HydraulicErosion", iterations=500)
output = graph.add_node("Output")

# Connect nodes
graph.connect(perlin, 0, erosion, 0)
graph.connect(erosion, 0, output, 0)

# Execute
graph.execute()
graph.export("terrain.exr", resolution=8192)
```

---

## Data Flow Architecture

### Preview Mode (Real-time)

```
User Edits Node → Mark Dirty → Execution Planner → Execute @ 512x512
     ↓
GPU Compute (< 16ms) → Staging Buffer → CPU Upload → UI Preview
     ↓
Update Viewport (60 FPS target)
```

### Final Render Mode

```
User Clicks "Render" → Select Resolution (up to 16k)
     ↓
Tiled Execution (if > 4k):
  For each tile:
    - Allocate GPU buffer
    - Execute full node graph
    - Transfer to CPU
    - Write to disk (EXR/TIFF)
     ↓
Merge tiles with overlap blending → Final output
```

### Erosion Simulation Flow

```
Initial Heightfield → Upload to GPU
     ↓
For N iterations:
  1. Water source compute shader
  2. Flow velocity compute shader
  3. Erosion/deposition compute shader
  4. Evaporation compute shader
  (Each step = separate Vulkan dispatch)
     ↓
Download final heightfield → Update node output
```

---

## Memory Management Strategy

### GPU Memory Budget (Example: 8GB VRAM)

| Allocation | Size | Purpose |
|------------|------|---------|
| Primary Heightfield (4K) | 64 MB | Main terrain data |
| MIP Pyramid | 21 MB | LOD chain |
| Attachments (slope, etc.) | 256 MB | 4x layers @ 4K |
| Intermediate Buffers | 512 MB | Node outputs |
| Texture Maps | 128 MB | Normals, AO, splatmaps |
| Viewport Render Target | 32 MB | 1080p preview |
| **Reserved** | ~2 GB | Windows/driver overhead |
| **Available for tiling** | 5+ GB | Large resolution work |

### CPU Memory

- **Minimal Mirroring**: Only mirror GPU data when necessary (export, save)
- **Streaming**: Load large input files in tiles
- **Undo Stack**: Store diff deltas, not full copies

---

## Performance Optimization Strategies

### GPU Optimizations

1. **Persistent Buffers**: Allocate once, reuse across executions
2. **Async Compute**: Overlap CPU graph planning with GPU execution
3. **Batch Dispatches**: Group multiple node executes into single submit
4. **Shared Descriptors**: Minimize descriptor set updates
5. **Shader Optimization**:
   - Shared memory for cache blocking
   - Coalesced memory access patterns
   - Workgroup size tuning per kernel

### CPU Optimizations

1. **SIMD**: Use SSE/AVX for CPU-side heightfield operations
2. **Multi-threading**: Parallelize I/O, tile merging, CPU preview
3. **Lock-Free Queues**: For GPU command submission
4. **Memory Pooling**: Avoid allocations in hot paths

### Windows 11 Specific

1. **DirectStorage**: Future integration for ultra-fast texture streaming
2. **Hardware Scheduling**: Enable GPU hardware scheduling for lower latency
3. **Large Pages**: For heightfield allocations (2MB pages)
4. **High Priority Threads**: Real-time priority for viewport update thread

---

## Error Handling & Robustness

### GPU Error Recovery

- **Device Lost**: Detect via `VK_ERROR_DEVICE_LOST`, attempt reset
- **Out of Memory**: Graceful degradation to lower resolution
- **Shader Compile Error**: Show error, fall back to default shader

### Data Validation

- **Node Parameter Ranges**: Clamp/validate before GPU upload
- **Graph Cycles**: Detect and reject cyclic connections
- **File Corruption**: CRC checks on save files

### Undo/Redo

- **Command Pattern**: Each action is a reversible command
- **Lightweight Deltas**: Store only changed parameters
- **Limit**: Max 100 undo steps (configurable)

---

## Security & Stability

### Shader Sandbox

- **No Arbitrary Code**: Only pre-compiled or vetted GLSL shaders
- **Timeout Detection**: Kill infinite-loop shaders after 5s
- **Resource Limits**: Max buffer size, max dispatch dimensions

### File I/O

- **Path Validation**: Sanitize user-provided paths
- **Format Validation**: Check magic numbers, reject malformed files
- **Sandboxing**: Limit file access to project directories

---

## Testing Strategy

### Unit Tests

- Core math functions (noise, filters)
- Graph topology operations
- Memory allocators

### Integration Tests

- Full node graph execution
- I/O round-trip tests
- Vulkan pipeline creation

### Performance Tests

- Benchmark standard graphs @ 4K, 8K
- Memory leak detection (Valgrind, ASAN)
- GPU profiling (NSight, RenderDoc)

### Visual Regression Tests

- Generate reference outputs
- Compare pixel differences
- Alert on unexpected changes

---

## Extensibility

### Plugin System

```cpp
class INodePlugin {
    virtual const char* GetName() = 0;
    virtual NodeMetadata GetMetadata() = 0;
    virtual void Execute(ExecutionContext& ctx) = 0;
};

// Load from DLL
void LoadPlugin(const char* dllPath) {
    auto factory = LoadLibrary(dllPath);
    auto createFn = GetProcAddress(factory, "CreateNode");
    INodePlugin* node = createFn();
    RegisterNode(node);
}
```

### Custom Shader Nodes

- User provides GLSL compute shader
- Engine validates, compiles to SPIR-V
- Auto-generates UI from uniform blocks

---

## Deployment

### Windows 11 Package

- **Installer**: NSIS or WiX
- **Dependencies**: Bundle Vulkan runtime, VC++ redistributable
- **Size**: ~150 MB (includes base assets, shaders)
- **Requirements**:
  - Windows 11 22H2+
  - Vulkan 1.3 compatible GPU
  - 8 GB RAM minimum, 16 GB recommended
  - 500 MB disk space

### Optional Components

- Python scripting module (separate install)
- GDAL plugin (for GeoTIFF support)
- Example projects & presets

---

## Future Enhancements

### Phase 2 (Post-MVP)

- **Multi-GPU**: Distribute tiles across GPUs
- **Vegetation Placement**: Point cloud generation based on terrain
- **Real-time Preview**: Full resolution streaming
- **Animation**: Animate node parameters over time
- **Cloud Rendering**: Offload to cloud GPU instances

### Phase 3 (Advanced)

- **Machine Learning**: Trained erosion models
- **Procedural Roads**: Automatic path generation
- **Biome System**: Advanced ecosystem simulation
- **VR Support**: Immersive terrain sculpting

---

## Conclusion

This architecture provides a solid foundation for a professional terrain generation tool. The modular design allows for incremental development, while the GPU-first approach ensures performance scales with hardware capabilities.

**Key Strengths**:
- ✅ Scalable from preview to production
- ✅ Modern GPU compute pipeline
- ✅ Non-destructive, artist-friendly workflow
- ✅ Extensible plugin architecture
- ✅ Cross-platform foundation (Vulkan)

**Next Steps**: See `IMPLEMENTATION_ROADMAP.md` for phased development plan.
