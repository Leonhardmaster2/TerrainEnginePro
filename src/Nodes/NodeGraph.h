#pragma once

#include "Core/Types.h"
#include "Node.h"
#include "Terrain/TerrainGenerator.h"
#include <vector>
#include <unordered_map>

namespace Terrain {

// Node graph manages all nodes and connections
class NodeGraph {
public:
    NodeGraph();
    ~NodeGraph();

    // Node management
    template<typename T, typename... Args>
    T* CreateNode(Args&&... args) {
        uint32 id = m_NextNodeID++;
        auto node = MakeUnique<T>(id, std::forward<Args>(args)...);
        T* ptr = node.get();
        m_Nodes[id] = std::move(node);
        return ptr;
    }

    // Create node with specific ID (for deserialization)
    template<typename T, typename... Args>
    T* CreateNodeWithID(uint32 id, Args&&... args) {
        auto node = MakeUnique<T>(id, std::forward<Args>(args)...);
        T* ptr = node.get();
        m_Nodes[id] = std::move(node);
        // Update next ID if necessary
        if (id >= m_NextNodeID) {
            m_NextNodeID = id + 1;
        }
        return ptr;
    }

    void DeleteNode(uint32 nodeId);
    Node* GetNode(uint32 nodeId);
    const std::unordered_map<uint32, Unique<Node>>& GetNodes() const { return m_Nodes; }

    // Connection management
    bool CreateConnection(uint32 outputPinId, uint32 inputPinId);
    void DeleteConnection(uint32 inputPinId);
    bool IsConnected(uint32 pinId) const;
    bool ConnectPins(Pin* outputPin, Pin* inputPin);

    // Execution
    bool ExecuteNode(Node* node);
    bool ExecuteGraph();
    void MarkAllDirty();

    // Output node
    void SetOutputNode(Node* node) { m_OutputNode = node; }
    Node* GetOutputNode() const { return m_OutputNode; }

    // Get result
    Unique<Heightfield> GetResult();

    // Terrain generator (for nodes that need it)
    TerrainGenerator* GetGenerator() { return m_Generator.get(); }

    // Clear all
    void Clear();

private:
    std::unordered_map<uint32, Unique<Node>> m_Nodes;
    uint32 m_NextNodeID = 1;

    Node* m_OutputNode = nullptr;
    Unique<TerrainGenerator> m_Generator;
};

} // namespace Terrain
