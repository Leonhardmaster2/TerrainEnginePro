# Node Graph System Design

## Overview

The node graph system is the core of the Terrain Engine's visual programming interface. It provides a non-destructive, procedural workflow for terrain generation through a directed acyclic graph (DAG) of processing nodes.

---

## Core Concepts

### Node

A **Node** is a discrete processing unit that:
- Takes zero or more inputs (heightfields, masks, parameters)
- Produces one or more outputs (heightfields, masks, textures)
- Executes a specific operation (noise generation, erosion, filtering, etc.)
- Has configurable parameters exposed to the UI

### Graph

A **Graph** is a collection of nodes with connections defining data flow:
- Nodes are vertices
- Connections are directed edges
- Must be acyclic (DAG constraint)
- Has one or more output nodes for final results

### Execution Plan

The **Execution Plan** is the computed order of node execution:
- Topologically sorted based on dependencies
- Optimized for minimal redundant computation
- Supports caching and dirty propagation

---

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                       Node Graph                             │
│  ┌──────────┐      ┌──────────┐      ┌──────────┐          │
│  │  Perlin  │──────│  Erosion │──────│  Output  │          │
│  │  Noise   │      │          │      │          │          │
│  └──────────┘      └──────────┘      └──────────┘          │
│       │                                                      │
│       └────────────┐                                        │
│                    │                                        │
│              ┌──────────┐                                   │
│              │ Combine  │                                   │
│              └──────────┘                                   │
└─────────────────────────────────────────────────────────────┘
                           │
                           ▼
              ┌────────────────────────┐
              │  Execution Planner      │
              │  - Topological Sort     │
              │  - Dependency Analysis  │
              │  - Cache Management     │
              └────────────┬────────────┘
                           │
                           ▼
              ┌────────────────────────┐
              │  Execution Engine       │
              │  - GPU Dispatch         │
              │  - Resource Management  │
              │  - Result Caching       │
              └─────────────────────────┘
```

---

## Node Interface

### Base Node Class

```cpp
enum class NodeCategory {
    Generator,      // Creates heightfields from scratch
    Modifier,       // Modifies existing heightfields
    Filter,         // Applies filters (blur, sharpen, etc.)
    Erosion,        // Geological simulation
    Mask,           // Creates/modifies masks
    Texture,        // Generates texture maps
    Combiner,       // Combines multiple inputs
    IO,             // Input/output operations
    Utility         // Misc utilities
};

enum class PinType {
    Heightfield,    // float heightfield data
    Mask,           // single-channel mask
    Texture,        // multi-channel texture
    Float,          // scalar value
    Int,            // integer value
    Vector2,        // 2D vector
    Vector3,        // 3D vector
    Color           // RGBA color
};

struct NodePin {
    std::string name;
    PinType type;
    bool isInput;
    int index;
    std::any defaultValue; // For inputs with defaults
};

class Node {
public:
    // Identification
    NodeID id;                          // Unique identifier
    std::string typeName;               // "PerlinNoise", "HydraulicErosion", etc.
    NodeCategory category;

    // Connections
    std::vector<NodePin> inputPins;
    std::vector<NodePin> outputPins;

    // Parameters
    ParameterSet parameters;            // Exposed to UI

    // State
    bool isDirty = true;                // Needs recomputation
    uint64_t cacheHash = 0;             // For change detection

    // Visual
    glm::vec2 editorPosition;           // Position in graph editor
    glm::vec4 color;                    // Category color

public:
    virtual ~Node() = default;

    // Execution
    virtual void Execute(ExecutionContext& ctx) = 0;

    // Data access
    virtual void* GetOutput(int pinIndex) = 0;
    virtual void SetInput(int pinIndex, void* data) = 0;

    // Metadata
    virtual const char* GetDisplayName() const = 0;
    virtual const char* GetDescription() const = 0;

    // Serialization
    virtual void Serialize(JsonWriter& writer) const;
    virtual void Deserialize(const JsonValue& value);

    // Cache management
    uint64_t ComputeHash() const;
    bool IsCacheValid() const;
    void InvalidateCache();

protected:
    // Subclasses implement specific operations
};
```

### Example: Perlin Noise Node

```cpp
class PerlinNoiseNode : public Node {
private:
    // Output
    HeightfieldBuffer* output = nullptr;

    // Parameters
    float frequency = 1.0f;
    float amplitude = 1.0f;
    int octaves = 6;
    float lacunarity = 2.0f;
    float persistence = 0.5f;
    uint32_t seed = 0;

public:
    PerlinNoiseNode() {
        typeName = "PerlinNoise";
        category = NodeCategory::Generator;

        // Define outputs
        outputPins.push_back({"Heightfield", PinType::Heightfield, false, 0});

        // Define parameters
        parameters.Add("Frequency", &frequency, 0.01f, 10.0f);
        parameters.Add("Amplitude", &amplitude, 0.0f, 10.0f);
        parameters.Add("Octaves", &octaves, 1, 12);
        parameters.Add("Lacunarity", &lacunarity, 1.5f, 4.0f);
        parameters.Add("Persistence", &persistence, 0.0f, 1.0f);
        parameters.Add("Seed", &seed, 0u, 999999u);

        color = glm::vec4(0.2f, 0.8f, 0.4f, 1.0f); // Green for generators
    }

    void Execute(ExecutionContext& ctx) override {
        // Allocate output if needed
        if (!output) {
            output = ctx.AllocateHeightfield();
        }

        // Get compute pipeline
        auto pipeline = ctx.GetPipeline("perlin_noise");

        // Bind descriptors
        VkDescriptorSet descriptors = ctx.CreateDescriptorSet();
        ctx.BindStorageBuffer(descriptors, 0, output->GetGPUBuffer());

        // Push constants
        struct PushConstants {
            float frequency;
            float amplitude;
            int octaves;
            float lacunarity;
            float persistence;
            uint32_t seed;
        } pushConstants = {
            frequency, amplitude, octaves,
            lacunarity, persistence, seed
        };

        // Execute on GPU
        VkCommandBuffer cmd = ctx.GetCommandBuffer();
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
        vkCmdBindDescriptorSets(cmd, ...);
        vkCmdPushConstants(cmd, ..., &pushConstants, sizeof(pushConstants));

        uint32_t groupsX = (ctx.GetResolution() + 15) / 16;
        uint32_t groupsY = (ctx.GetResolution() + 15) / 16;
        vkCmdDispatch(cmd, groupsX, groupsY, 1);

        isDirty = false;
        cacheHash = ComputeHash();
    }

    void* GetOutput(int pinIndex) override {
        if (pinIndex == 0) return output;
        return nullptr;
    }

    const char* GetDisplayName() const override {
        return "Perlin Noise";
    }

    const char* GetDescription() const override {
        return "Generates Perlin noise heightfield with fractal layering";
    }
};
```

### Example: Hydraulic Erosion Node

```cpp
class HydraulicErosionNode : public Node {
private:
    // Inputs
    HeightfieldBuffer* inputHeightfield = nullptr;

    // Outputs
    HeightfieldBuffer* outputHeightfield = nullptr;
    HeightfieldBuffer* flowMap = nullptr; // Optional

    // Parameters
    int iterations = 100;
    float rainRate = 0.01f;
    float evaporationRate = 0.01f;
    float sedimentCapacity = 4.0f;
    float erosionStrength = 0.3f;
    float depositionStrength = 0.3f;

    // Internal state buffers (GPU)
    VkBuffer waterBuffer = VK_NULL_HANDLE;
    VkBuffer sedimentBuffer = VK_NULL_HANDLE;
    VkBuffer velocityBuffer = VK_NULL_HANDLE;

public:
    HydraulicErosionNode() {
        typeName = "HydraulicErosion";
        category = NodeCategory::Erosion;

        // Inputs
        inputPins.push_back({"Heightfield", PinType::Heightfield, true, 0});

        // Outputs
        outputPins.push_back({"Eroded", PinType::Heightfield, false, 0});
        outputPins.push_back({"Flow Map", PinType::Heightfield, false, 1});

        // Parameters
        parameters.Add("Iterations", &iterations, 1, 5000);
        parameters.Add("Rain Rate", &rainRate, 0.0f, 1.0f);
        parameters.Add("Evaporation", &evaporationRate, 0.0f, 0.1f);
        parameters.Add("Sediment Capacity", &sedimentCapacity, 0.1f, 10.0f);
        parameters.Add("Erosion Strength", &erosionStrength, 0.0f, 1.0f);
        parameters.Add("Deposition Strength", &depositionStrength, 0.0f, 1.0f);

        color = glm::vec4(0.6f, 0.3f, 0.1f, 1.0f); // Brown for erosion
    }

    void Execute(ExecutionContext& ctx) override {
        if (!inputHeightfield) {
            throw std::runtime_error("Erosion node missing input heightfield");
        }

        // Allocate outputs and state buffers
        if (!outputHeightfield) {
            outputHeightfield = ctx.AllocateHeightfield();
            flowMap = ctx.AllocateHeightfield();

            waterBuffer = ctx.AllocateBuffer(ctx.GetResolution() * ctx.GetResolution() * sizeof(float));
            sedimentBuffer = ctx.AllocateBuffer(ctx.GetResolution() * ctx.GetResolution() * sizeof(float));
            velocityBuffer = ctx.AllocateBuffer(ctx.GetResolution() * ctx.GetResolution() * sizeof(glm::vec2));

            // Initialize to zero
            ctx.ClearBuffer(waterBuffer);
            ctx.ClearBuffer(sedimentBuffer);
            ctx.ClearBuffer(velocityBuffer);
        }

        // Copy input to output (we modify in-place)
        ctx.CopyBuffer(inputHeightfield->GetGPUBuffer(), outputHeightfield->GetGPUBuffer());

        // Get erosion pipelines
        auto pipelineWater = ctx.GetPipeline("erosion_water");
        auto pipelineFlow = ctx.GetPipeline("erosion_flow");
        auto pipelineErode = ctx.GetPipeline("erosion_erode");
        auto pipelineEvaporate = ctx.GetPipeline("erosion_evaporate");

        VkCommandBuffer cmd = ctx.GetCommandBuffer();

        // Erosion iterations
        for (int i = 0; i < iterations; ++i) {
            // Step 1: Add water
            ctx.BindPipeline(pipelineWater);
            ctx.BindBuffer(0, outputHeightfield->GetGPUBuffer());
            ctx.BindBuffer(1, waterBuffer);
            ctx.PushConstant("rainRate", rainRate);
            ctx.Dispatch();

            ctx.PipelineBarrier(); // Sync

            // Step 2: Compute flow velocity
            ctx.BindPipeline(pipelineFlow);
            ctx.BindBuffer(0, outputHeightfield->GetGPUBuffer());
            ctx.BindBuffer(1, waterBuffer);
            ctx.BindBuffer(2, velocityBuffer);
            ctx.Dispatch();

            ctx.PipelineBarrier();

            // Step 3: Erosion/deposition
            ctx.BindPipeline(pipelineErode);
            ctx.BindBuffer(0, outputHeightfield->GetGPUBuffer());
            ctx.BindBuffer(1, waterBuffer);
            ctx.BindBuffer(2, sedimentBuffer);
            ctx.BindBuffer(3, velocityBuffer);
            ctx.PushConstant("sedimentCapacity", sedimentCapacity);
            ctx.PushConstant("erosionStrength", erosionStrength);
            ctx.PushConstant("depositionStrength", depositionStrength);
            ctx.Dispatch();

            ctx.PipelineBarrier();

            // Step 4: Evaporation
            ctx.BindPipeline(pipelineEvaporate);
            ctx.BindBuffer(0, waterBuffer);
            ctx.PushConstant("evaporationRate", evaporationRate);
            ctx.Dispatch();

            ctx.PipelineBarrier();
        }

        // Copy velocity to flow map output
        ctx.CopyBuffer(velocityBuffer, flowMap->GetGPUBuffer());

        isDirty = false;
        cacheHash = ComputeHash();
    }

    void SetInput(int pinIndex, void* data) override {
        if (pinIndex == 0) {
            inputHeightfield = static_cast<HeightfieldBuffer*>(data);
            InvalidateCache();
        }
    }

    void* GetOutput(int pinIndex) override {
        if (pinIndex == 0) return outputHeightfield;
        if (pinIndex == 1) return flowMap;
        return nullptr;
    }

    const char* GetDisplayName() const override {
        return "Hydraulic Erosion";
    }

    const char* GetDescription() const override {
        return "Simulates water-based erosion with sediment transport";
    }
};
```

---

## Node Graph Management

### Graph Class

```cpp
struct Connection {
    NodeID sourceNode;
    int sourcePin;
    NodeID targetNode;
    int targetPin;
};

class NodeGraph {
private:
    std::unordered_map<NodeID, std::unique_ptr<Node>> nodes;
    std::vector<Connection> connections;

    NodeID nextNodeId = 1;

public:
    // Node management
    NodeID AddNode(std::unique_ptr<Node> node) {
        NodeID id = nextNodeId++;
        node->id = id;
        nodes[id] = std::move(node);
        return id;
    }

    void RemoveNode(NodeID id) {
        // Remove all connections involving this node
        connections.erase(
            std::remove_if(connections.begin(), connections.end(),
                [id](const Connection& conn) {
                    return conn.sourceNode == id || conn.targetNode == id;
                }),
            connections.end()
        );

        nodes.erase(id);
    }

    Node* GetNode(NodeID id) {
        auto it = nodes.find(id);
        return it != nodes.end() ? it->second.get() : nullptr;
    }

    // Connection management
    bool CanConnect(NodeID sourceNode, int sourcePin, NodeID targetNode, int targetPin) {
        // Check types match
        Node* src = GetNode(sourceNode);
        Node* tgt = GetNode(targetNode);

        if (!src || !tgt) return false;

        if (sourcePin >= src->outputPins.size() || targetPin >= tgt->inputPins.size()) {
            return false;
        }

        PinType srcType = src->outputPins[sourcePin].type;
        PinType tgtType = tgt->inputPins[targetPin].type;

        if (srcType != tgtType) return false;

        // Check for cycles
        if (WouldCreateCycle(sourceNode, targetNode)) {
            return false;
        }

        return true;
    }

    void Connect(NodeID sourceNode, int sourcePin, NodeID targetNode, int targetPin) {
        if (!CanConnect(sourceNode, sourcePin, targetNode, targetPin)) {
            throw std::runtime_error("Invalid connection");
        }

        // Disconnect existing connection to target pin (single input)
        Disconnect(targetNode, targetPin);

        connections.push_back({sourceNode, sourcePin, targetNode, targetPin});

        // Mark target node as dirty
        GetNode(targetNode)->InvalidateCache();
    }

    void Disconnect(NodeID targetNode, int targetPin) {
        connections.erase(
            std::remove_if(connections.begin(), connections.end(),
                [=](const Connection& conn) {
                    return conn.targetNode == targetNode && conn.targetPin == targetPin;
                }),
            connections.end()
        );

        GetNode(targetNode)->InvalidateCache();
    }

    // Execution
    void Execute() {
        ExecutionPlanner planner;
        auto plan = planner.CreateExecutionPlan(*this);

        ExecutionContext ctx;
        for (Node* node : plan) {
            if (node->isDirty) {
                node->Execute(ctx);
            }
        }
    }

    // Dirty propagation
    void MarkDirty(NodeID id) {
        Node* node = GetNode(id);
        if (!node) return;

        node->InvalidateCache();

        // Propagate to dependent nodes
        for (const auto& conn : connections) {
            if (conn.sourceNode == id) {
                MarkDirty(conn.targetNode);
            }
        }
    }

    // Cycle detection
    bool WouldCreateCycle(NodeID sourceNode, NodeID targetNode) {
        // DFS from targetNode to see if we can reach sourceNode
        std::unordered_set<NodeID> visited;
        std::stack<NodeID> stack;
        stack.push(targetNode);

        while (!stack.empty()) {
            NodeID current = stack.top();
            stack.pop();

            if (current == sourceNode) {
                return true; // Cycle detected
            }

            if (visited.count(current)) continue;
            visited.insert(current);

            // Add all nodes this one outputs to
            for (const auto& conn : connections) {
                if (conn.sourceNode == current) {
                    stack.push(conn.targetNode);
                }
            }
        }

        return false;
    }

    // Serialization
    void SaveToFile(const char* path) {
        JsonWriter writer;
        writer.StartObject();

        // Save nodes
        writer.Key("nodes");
        writer.StartArray();
        for (const auto& [id, node] : nodes) {
            writer.StartObject();
            writer.Key("id");
            writer.Uint64(id);
            writer.Key("type");
            writer.String(node->typeName);
            writer.Key("position");
            writer.StartArray();
            writer.Double(node->editorPosition.x);
            writer.Double(node->editorPosition.y);
            writer.EndArray();
            node->Serialize(writer);
            writer.EndObject();
        }
        writer.EndArray();

        // Save connections
        writer.Key("connections");
        writer.StartArray();
        for (const auto& conn : connections) {
            writer.StartObject();
            writer.Key("from_node");
            writer.Uint64(conn.sourceNode);
            writer.Key("from_pin");
            writer.Int(conn.sourcePin);
            writer.Key("to_node");
            writer.Uint64(conn.targetNode);
            writer.Key("to_pin");
            writer.Int(conn.targetPin);
            writer.EndObject();
        }
        writer.EndArray();

        writer.EndObject();

        std::ofstream file(path);
        file << writer.GetString();
    }

    void LoadFromFile(const char* path) {
        // Parse JSON and reconstruct graph
        // Implementation details omitted for brevity
    }
};
```

---

## Execution Planner

### Topological Sort

```cpp
class ExecutionPlanner {
public:
    std::vector<Node*> CreateExecutionPlan(const NodeGraph& graph) {
        // Kahn's algorithm for topological sort

        // Compute in-degree for each node
        std::unordered_map<NodeID, int> inDegree;
        for (const auto& [id, node] : graph.nodes) {
            inDegree[id] = 0;
        }

        for (const auto& conn : graph.connections) {
            inDegree[conn.targetNode]++;
        }

        // Queue for nodes with in-degree 0
        std::queue<NodeID> queue;
        for (const auto& [id, degree] : inDegree) {
            if (degree == 0) {
                queue.push(id);
            }
        }

        // Topological ordering
        std::vector<Node*> plan;

        while (!queue.empty()) {
            NodeID current = queue.front();
            queue.pop();

            plan.push_back(graph.GetNode(current));

            // Reduce in-degree of neighbors
            for (const auto& conn : graph.connections) {
                if (conn.sourceNode == current) {
                    inDegree[conn.targetNode]--;
                    if (inDegree[conn.targetNode] == 0) {
                        queue.push(conn.targetNode);
                    }
                }
            }
        }

        // Check for cycles (shouldn't happen if validation works)
        if (plan.size() != graph.nodes.size()) {
            throw std::runtime_error("Graph contains cycles!");
        }

        return plan;
    }

    // Optimize plan by grouping independent nodes
    std::vector<std::vector<Node*>> CreateParallelExecutionPlan(const NodeGraph& graph) {
        auto linearPlan = CreateExecutionPlan(graph);

        // Group nodes by depth level (can execute in parallel)
        std::unordered_map<Node*, int> depth;
        for (Node* node : linearPlan) {
            depth[node] = 0;
        }

        // Compute depth (longest path from root)
        for (Node* node : linearPlan) {
            for (const auto& conn : graph.connections) {
                if (conn.targetNode == node->id) {
                    Node* source = graph.GetNode(conn.sourceNode);
                    depth[node] = std::max(depth[node], depth[source] + 1);
                }
            }
        }

        // Group by depth
        int maxDepth = 0;
        for (const auto& [node, d] : depth) {
            maxDepth = std::max(maxDepth, d);
        }

        std::vector<std::vector<Node*>> parallelPlan(maxDepth + 1);
        for (Node* node : linearPlan) {
            parallelPlan[depth[node]].push_back(node);
        }

        return parallelPlan;
    }
};
```

---

## Cache Management

### Hash-Based Caching

```cpp
class Node {
    uint64_t ComputeHash() const {
        uint64_t hash = 0xcbf29ce484222325; // FNV-1a offset

        // Hash node type
        hash = HashString(typeName, hash);

        // Hash parameters
        hash = parameters.ComputeHash(hash);

        // Hash input hashes (dependency chain)
        for (const auto& input : inputs) {
            if (input.sourceNode) {
                hash ^= input.sourceNode->cacheHash;
                hash *= 0x100000001b3; // FNV prime
            }
        }

        return hash;
    }

    bool IsCacheValid() const {
        uint64_t currentHash = ComputeHash();
        return currentHash == cacheHash && !isDirty;
    }
};

class CacheManager {
    struct CacheEntry {
        uint64_t hash;
        HeightfieldBuffer* result;
        size_t sizeBytes;
        std::chrono::time_point<std::chrono::steady_clock> lastAccess;
    };

    std::unordered_map<NodeID, CacheEntry> cache;
    size_t maxCacheSizeMB = 2048; // 2 GB default
    size_t currentCacheSizeMB = 0;

public:
    HeightfieldBuffer* Get(Node* node) {
        if (cache.count(node->id)) {
            auto& entry = cache[node->id];

            if (entry.hash == node->ComputeHash()) {
                entry.lastAccess = std::chrono::steady_clock::now();
                return entry.result;
            }
        }

        return nullptr; // Cache miss
    }

    void Store(Node* node, HeightfieldBuffer* result) {
        size_t sizeBytes = result->GetSizeBytes();
        size_t sizeMB = sizeBytes / (1024 * 1024);

        // Evict if needed
        while (currentCacheSizeMB + sizeMB > maxCacheSizeMB) {
            EvictLRU();
        }

        CacheEntry entry;
        entry.hash = node->ComputeHash();
        entry.result = result;
        entry.sizeBytes = sizeBytes;
        entry.lastAccess = std::chrono::steady_clock::now();

        cache[node->id] = entry;
        currentCacheSizeMB += sizeMB;
    }

    void EvictLRU() {
        // Find least recently used entry
        NodeID lruNode = 0;
        auto oldestTime = std::chrono::steady_clock::now();

        for (const auto& [id, entry] : cache) {
            if (entry.lastAccess < oldestTime) {
                oldestTime = entry.lastAccess;
                lruNode = id;
            }
        }

        // Remove
        if (lruNode != 0) {
            auto& entry = cache[lruNode];
            currentCacheSizeMB -= entry.sizeBytes / (1024 * 1024);
            delete entry.result;
            cache.erase(lruNode);
        }
    }
};
```

---

## Node Registry

### Factory Pattern

```cpp
class NodeRegistry {
    using NodeFactory = std::function<std::unique_ptr<Node>()>;

    struct NodeInfo {
        std::string category;
        std::string displayName;
        std::string description;
        NodeFactory factory;
    };

    std::unordered_map<std::string, NodeInfo> registry;

public:
    void Register(const char* typeName, const char* category,
                  const char* displayName, const char* description,
                  NodeFactory factory) {
        registry[typeName] = {category, displayName, description, factory};
    }

    std::unique_ptr<Node> Create(const char* typeName) {
        auto it = registry.find(typeName);
        if (it == registry.end()) {
            throw std::runtime_error("Unknown node type");
        }

        return it->second.factory();
    }

    std::vector<NodeInfo> GetNodesByCategory(const char* category) {
        std::vector<NodeInfo> result;
        for (const auto& [name, info] : registry) {
            if (info.category == category) {
                result.push_back(info);
            }
        }
        return result;
    }
};

// Registration (at startup)
void RegisterNodes(NodeRegistry& registry) {
    registry.Register("PerlinNoise", "Generator", "Perlin Noise",
        "Generates Perlin noise heightfield",
        []() { return std::make_unique<PerlinNoiseNode>(); });

    registry.Register("HydraulicErosion", "Erosion", "Hydraulic Erosion",
        "Water-based erosion simulation",
        []() { return std::make_unique<HydraulicErosionNode>(); });

    // ... register all nodes
}
```

---

## Parameter System

### Parameter Definition

```cpp
enum class ParameterType {
    Float,
    Int,
    Bool,
    Vector2,
    Vector3,
    Color,
    Enum
};

struct Parameter {
    std::string name;
    ParameterType type;
    void* dataPtr;

    // Range constraints
    std::any minValue;
    std::any maxValue;

    // UI hints
    float uiStep = 0.01f;
    bool isLogarithmic = false;

    // Serialization
    std::any GetValue() const;
    void SetValue(const std::any& value);
};

class ParameterSet {
    std::vector<Parameter> parameters;

public:
    template<typename T>
    void Add(const char* name, T* dataPtr, T minVal, T maxVal) {
        Parameter param;
        param.name = name;
        param.type = GetParameterType<T>();
        param.dataPtr = dataPtr;
        param.minValue = minVal;
        param.maxValue = maxVal;

        parameters.push_back(param);
    }

    Parameter* Get(const char* name) {
        for (auto& param : parameters) {
            if (param.name == name) {
                return &param;
            }
        }
        return nullptr;
    }

    uint64_t ComputeHash(uint64_t seed) const {
        uint64_t hash = seed;

        for (const auto& param : parameters) {
            // Hash parameter value
            switch (param.type) {
                case ParameterType::Float:
                    hash = HashFloat(*static_cast<float*>(param.dataPtr), hash);
                    break;
                case ParameterType::Int:
                    hash = HashInt(*static_cast<int*>(param.dataPtr), hash);
                    break;
                // ... other types
            }
        }

        return hash;
    }
};
```

---

## Undo/Redo System

### Command Pattern

```cpp
class Command {
public:
    virtual ~Command() = default;
    virtual void Execute() = 0;
    virtual void Undo() = 0;
    virtual const char* GetDescription() const = 0;
};

class AddNodeCommand : public Command {
    NodeGraph* graph;
    std::unique_ptr<Node> node;
    NodeID nodeId;

public:
    AddNodeCommand(NodeGraph* g, std::unique_ptr<Node> n)
        : graph(g), node(std::move(n)) {}

    void Execute() override {
        nodeId = graph->AddNode(std::move(node));
    }

    void Undo() override {
        // Transfer ownership back
        node = graph->RemoveNodeReturnOwnership(nodeId);
    }

    const char* GetDescription() const override {
        return "Add Node";
    }
};

class CommandHistory {
    std::vector<std::unique_ptr<Command>> history;
    int currentIndex = -1;
    int maxHistorySize = 100;

public:
    void Execute(std::unique_ptr<Command> cmd) {
        // Clear redo stack
        history.erase(history.begin() + currentIndex + 1, history.end());

        cmd->Execute();
        history.push_back(std::move(cmd));
        currentIndex++;

        // Limit history size
        if (history.size() > maxHistorySize) {
            history.erase(history.begin());
            currentIndex--;
        }
    }

    void Undo() {
        if (currentIndex >= 0) {
            history[currentIndex]->Undo();
            currentIndex--;
        }
    }

    void Redo() {
        if (currentIndex < (int)history.size() - 1) {
            currentIndex++;
            history[currentIndex]->Execute();
        }
    }

    bool CanUndo() const { return currentIndex >= 0; }
    bool CanRedo() const { return currentIndex < (int)history.size() - 1; }
};
```

---

## Preset System

### JSON-Based Presets

```json
{
  "name": "Alpine Mountains",
  "description": "Realistic mountain range with erosion",
  "nodes": [
    {
      "type": "PerlinNoise",
      "position": [100, 100],
      "parameters": {
        "frequency": 0.5,
        "amplitude": 2.0,
        "octaves": 8,
        "seed": 12345
      }
    },
    {
      "type": "HydraulicErosion",
      "position": [400, 100],
      "parameters": {
        "iterations": 500,
        "rainRate": 0.01,
        "erosionStrength": 0.4
      }
    },
    {
      "type": "Output",
      "position": [700, 100]
    }
  ],
  "connections": [
    {"from": 0, "from_pin": 0, "to": 1, "to_pin": 0},
    {"from": 1, "from_pin": 0, "to": 2, "to_pin": 0}
  ]
}
```

### Preset Loader

```cpp
class PresetManager {
    std::vector<std::string> presetPaths;

public:
    void LoadPresetsFromDirectory(const char* dir) {
        // Scan directory for .json files
        for (const auto& entry : std::filesystem::directory_iterator(dir)) {
            if (entry.path().extension() == ".json") {
                presetPaths.push_back(entry.path().string());
            }
        }
    }

    void ApplyPreset(const char* presetPath, NodeGraph& graph) {
        // Parse JSON
        std::ifstream file(presetPath);
        Json::Value root;
        file >> root;

        graph.Clear();

        // Create nodes
        std::unordered_map<int, NodeID> indexToId;
        for (size_t i = 0; i < root["nodes"].size(); ++i) {
            const auto& nodeJson = root["nodes"][i];

            auto node = nodeRegistry.Create(nodeJson["type"].asString());
            node->editorPosition = glm::vec2(
                nodeJson["position"][0].asFloat(),
                nodeJson["position"][1].asFloat()
            );

            // Set parameters
            const auto& params = nodeJson["parameters"];
            for (const auto& key : params.getMemberNames()) {
                auto param = node->parameters.Get(key.c_str());
                if (param) {
                    // Set value based on type
                    // ...
                }
            }

            NodeID id = graph.AddNode(std::move(node));
            indexToId[i] = id;
        }

        // Create connections
        for (const auto& connJson : root["connections"]) {
            int fromIndex = connJson["from"].asInt();
            int fromPin = connJson["from_pin"].asInt();
            int toIndex = connJson["to"].asInt();
            int toPin = connJson["to_pin"].asInt();

            graph.Connect(indexToId[fromIndex], fromPin,
                          indexToId[toIndex], toPin);
        }
    }
};
```

---

## Performance Considerations

### Incremental Execution

Instead of executing the entire graph, only recompute dirty nodes:

```cpp
void IncrementalExecute(NodeGraph& graph, NodeID changedNode) {
    // Mark affected subgraph as dirty
    graph.MarkDirty(changedNode);

    // Execute only dirty nodes in topological order
    auto plan = planner.CreateExecutionPlan(graph);
    for (Node* node : plan) {
        if (node->isDirty && IsCacheValid(node)) {
            // Use cached result
            continue;
        }

        if (node->isDirty) {
            node->Execute(ctx);
        }
    }
}
```

### Multi-Resolution Preview

```cpp
void UpdatePreview(NodeGraph& graph) {
    // Execute at low resolution for real-time preview
    ExecutionContext previewCtx(512); // 512x512 preview

    auto plan = planner.CreateExecutionPlan(graph);
    for (Node* node : plan) {
        if (node->isDirty) {
            node->Execute(previewCtx);
        }
    }

    UpdateViewport(previewCtx.GetFinalOutput());
}

void RenderFinal(NodeGraph& graph) {
    // Execute at full resolution
    ExecutionContext finalCtx(8192); // 8192x8192 final

    auto plan = planner.CreateExecutionPlan(graph);
    for (Node* node : plan) {
        node->Execute(finalCtx); // Always execute, no cache for final
    }

    ExportOutput(finalCtx.GetFinalOutput());
}
```

---

## Conclusion

The node graph system provides:

- ✅ Flexible visual programming interface
- ✅ Efficient execution with caching
- ✅ Non-destructive workflow
- ✅ Extensible node types
- ✅ Preset support for quick prototyping
- ✅ Robust undo/redo system

**Next Steps**: Implement core node types, test execution planner.

See `ARCHITECTURE.md` for overall system design, `GPU_PIPELINE_DESIGN.md` for GPU execution details.
