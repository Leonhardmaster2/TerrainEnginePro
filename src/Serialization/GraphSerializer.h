#pragma once

#include "Core/Types.h"
#include "Nodes/NodeGraph.h"
#include <nlohmann/json.hpp>

namespace Terrain {

using json = nlohmann::json;

// Serialization result
struct SerializationResult {
    bool success = false;
    String errorMessage;
};

class GraphSerializer {
public:
    GraphSerializer();
    ~GraphSerializer();

    // Save graph to JSON file
    SerializationResult SaveToFile(const NodeGraph* graph, const String& filepath);

    // Load graph from JSON file
    SerializationResult LoadFromFile(NodeGraph* graph, const String& filepath);

    // Serialize to JSON object (for embedding or debugging)
    json SerializeGraph(const NodeGraph* graph);

    // Deserialize from JSON object
    bool DeserializeGraph(NodeGraph* graph, const json& j);

private:
    // Serialize individual node
    json SerializeNode(const Node* node);

    // Deserialize individual node
    Node* DeserializeNode(NodeGraph* graph, const json& j);

    // Serialize node connections
    json SerializeConnections(const NodeGraph* graph);

    // Deserialize node connections
    bool DeserializeConnections(NodeGraph* graph, const json& j);

    // Type-specific parameter serialization
    json SerializeNodeParams(const Node* node);
    bool DeserializeNodeParams(Node* node, const json& j);

    // Helper: Get node type string
    String GetNodeTypeString(const Node* node);

    // Helper: Create node from type string
    Node* CreateNodeFromType(NodeGraph* graph, const String& type, uint32 id);
};

} // namespace Terrain
