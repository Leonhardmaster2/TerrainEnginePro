#include "GraphSerializer.h"
#include "Core/Logger.h"
#include "Nodes/GeneratorNodes.h"
#include "Nodes/ModifierNodes.h"
#include "Nodes/ErosionNodes.h"
#include "Nodes/TextureNodes.h"
#include "Nodes/MeshExportNodes.h"
#include <fstream>
#include <sstream>

namespace Terrain {

GraphSerializer::GraphSerializer() {
}

GraphSerializer::~GraphSerializer() {
}

SerializationResult GraphSerializer::SaveToFile(const NodeGraph* graph, const String& filepath) {
    SerializationResult result;

    try {
        LOG_INFO("Saving node graph to: %s", filepath.c_str());

        // Serialize to JSON
        json j = SerializeGraph(graph);

        // Write to file
        std::ofstream file(filepath);
        if (!file.is_open()) {
            result.success = false;
            result.errorMessage = "Failed to open file for writing: " + filepath;
            LOG_ERROR("%s", result.errorMessage.c_str());
            return result;
        }

        file << j.dump(2); // Pretty print with 2-space indent
        file.close();

        result.success = true;
        LOG_INFO("Graph saved successfully");
    }
    catch (const std::exception& e) {
        result.success = false;
        result.errorMessage = String("Exception during save: ") + e.what();
        LOG_ERROR("%s", result.errorMessage.c_str());
    }

    return result;
}

SerializationResult GraphSerializer::LoadFromFile(NodeGraph* graph, const String& filepath) {
    SerializationResult result;

    try {
        LOG_INFO("Loading node graph from: %s", filepath.c_str());

        // Read file
        std::ifstream file(filepath);
        if (!file.is_open()) {
            result.success = false;
            result.errorMessage = "Failed to open file for reading: " + filepath;
            LOG_ERROR("%s", result.errorMessage.c_str());
            return result;
        }

        // Parse JSON
        json j;
        file >> j;
        file.close();

        // Clear existing graph
        graph->Clear();

        // Deserialize
        result.success = DeserializeGraph(graph, j);
        if (result.success) {
            LOG_INFO("Graph loaded successfully");
        } else {
            result.errorMessage = "Failed to deserialize graph";
            LOG_ERROR("%s", result.errorMessage.c_str());
        }
    }
    catch (const std::exception& e) {
        result.success = false;
        result.errorMessage = String("Exception during load: ") + e.what();
        LOG_ERROR("%s", result.errorMessage.c_str());
    }

    return result;
}

json GraphSerializer::SerializeGraph(const NodeGraph* graph) {
    json j;

    j["version"] = "1.0";
    j["generator"] = "Terrain Engine Pro";

    // Serialize nodes
    json nodesArray = json::array();
    for (const auto& [id, node] : graph->GetNodes()) {
        nodesArray.push_back(SerializeNode(node.get()));
    }
    j["nodes"] = nodesArray;

    // Serialize connections
    j["connections"] = SerializeConnections(graph);

    return j;
}

bool GraphSerializer::DeserializeGraph(NodeGraph* graph, const json& j) {
    try {
        // Check version
        if (!j.contains("version")) {
            LOG_ERROR("Graph JSON missing version field");
            return false;
        }

        // Deserialize nodes
        if (j.contains("nodes")) {
            for (const auto& nodeJson : j["nodes"]) {
                Node* node = DeserializeNode(graph, nodeJson);
                if (!node) {
                    LOG_ERROR("Failed to deserialize node");
                    return false;
                }
            }
        }

        // Deserialize connections
        if (j.contains("connections")) {
            if (!DeserializeConnections(graph, j["connections"])) {
                LOG_ERROR("Failed to deserialize connections");
                return false;
            }
        }

        return true;
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception during deserialization: %s", e.what());
        return false;
    }
}

json GraphSerializer::SerializeNode(const Node* node) {
    json j;

    j["id"] = node->GetID();
    j["type"] = GetNodeTypeString(node);
    j["name"] = node->GetName();
    j["category"] = static_cast<int>(node->GetCategory());

    // Position
    glm::vec2 pos = node->GetPosition();
    j["position"] = {pos.x, pos.y};

    // Parameters
    j["params"] = SerializeNodeParams(node);

    return j;
}

Node* GraphSerializer::DeserializeNode(NodeGraph* graph, const json& j) {
    try {
        uint32 id = j["id"];
        String type = j["type"];

        // Create node
        Node* node = CreateNodeFromType(graph, type, id);
        if (!node) {
            LOG_ERROR("Failed to create node of type: %s", type.c_str());
            return nullptr;
        }

        // Set position
        if (j.contains("position")) {
            float x = j["position"][0];
            float y = j["position"][1];
            node->SetPosition(glm::vec2(x, y));
        }

        // Deserialize parameters
        if (j.contains("params")) {
            DeserializeNodeParams(node, j["params"]);
        }

        return node;
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception deserializing node: %s", e.what());
        return nullptr;
    }
}

json GraphSerializer::SerializeConnections(const NodeGraph* graph) {
    json connections = json::array();

    for (const auto& [id, node] : graph->GetNodes()) {
        // Serialize input connections
        for (const auto& pin : node->GetInputPins()) {
            if (pin.connectedTo) {
                json conn;
                conn["from_node"] = pin.connectedTo->ownerNode->GetID();
                conn["from_pin"] = pin.connectedTo->name;
                conn["to_node"] = node->GetID();
                conn["to_pin"] = pin.name;
                connections.push_back(conn);
            }
        }
    }

    return connections;
}

bool GraphSerializer::DeserializeConnections(NodeGraph* graph, const json& j) {
    try {
        for (const auto& conn : j) {
            uint32 fromNode = conn["from_node"];
            String fromPin = conn["from_pin"];
            uint32 toNode = conn["to_node"];
            String toPin = conn["to_pin"];

            // Find nodes
            Node* from = graph->GetNode(fromNode);
            Node* to = graph->GetNode(toNode);

            if (!from || !to) {
                LOG_ERROR("Connection references non-existent nodes");
                continue;
            }

            // Find pins
            Pin* outputPin = from->GetOutputPin(fromPin);
            Pin* inputPin = to->GetInputPin(toPin);

            if (!outputPin || !inputPin) {
                LOG_ERROR("Connection references non-existent pins");
                continue;
            }

            // Create connection
            graph->ConnectPins(outputPin, inputPin);
        }

        return true;
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception deserializing connections: %s", e.what());
        return false;
    }
}

String GraphSerializer::GetNodeTypeString(const Node* node) {
    // Use node name as type identifier
    // This is a simplified approach; a more robust system would use RTTI or a type registry
    String name = node->GetName();

    // Remove spaces for cleaner type names
    String type = name;
    type.erase(std::remove(type.begin(), type.end(), ' '), type.end());

    return type;
}

Node* GraphSerializer::CreateNodeFromType(NodeGraph* graph, const String& type, uint32 id) {
    Node* node = nullptr;

    // Generator nodes
    if (type == "PerlinNoise") node = graph->CreateNodeWithID<PerlinNoiseNode>(id);
    else if (type == "VoronoiNoise") node = graph->CreateNodeWithID<VoronoiNoiseNode>(id);
    else if (type == "RidgedNoise") node = graph->CreateNodeWithID<RidgedNoiseNode>(id);
    else if (type == "Gradient") node = graph->CreateNodeWithID<GradientNode>(id);
    else if (type == "Constant") node = graph->CreateNodeWithID<ConstantNode>(id);
    else if (type == "WhiteNoise") node = graph->CreateNodeWithID<WhiteNoiseNode>(id);

    // Modifier nodes
    else if (type == "Terrace") node = graph->CreateNodeWithID<TerraceNode>(id);
    else if (type == "Clamp") node = graph->CreateNodeWithID<ClampNode>(id);
    else if (type == "Invert") node = graph->CreateNodeWithID<InvertNode>(id);
    else if (type == "Scale") node = graph->CreateNodeWithID<ScaleNode>(id);
    else if (type == "Curve") node = graph->CreateNodeWithID<CurveNode>(id);

    // Filter nodes
    else if (type == "Smooth") node = graph->CreateNodeWithID<SmoothNode>(id);
    else if (type == "Sharpen") node = graph->CreateNodeWithID<SharpenNode>(id);

    // Erosion nodes
    else if (type == "HydraulicErosion") node = graph->CreateNodeWithID<HydraulicErosionNode>(id);
    else if (type == "ThermalErosion") node = graph->CreateNodeWithID<ThermalErosionNode>(id);

    // Texture nodes
    else if (type == "NormalMap") node = graph->CreateNodeWithID<NormalMapNode>(id);
    else if (type == "AmbientOcclusion") node = graph->CreateNodeWithID<AmbientOcclusionNode>(id);
    else if (type == "Splatmap") node = graph->CreateNodeWithID<SplatmapNode>(id);

    // Mesh export nodes
    else if (type == "OBJExport") node = graph->CreateNodeWithID<OBJExportNode>(id);
    else if (type == "FBXExport") node = graph->CreateNodeWithID<FBXExportNode>(id);

    // Combiner nodes
    else if (type == "Add") node = graph->CreateNodeWithID<AddNode>(id);
    else if (type == "Multiply") node = graph->CreateNodeWithID<MultiplyNode>(id);
    else if (type == "Blend") node = graph->CreateNodeWithID<BlendNode>(id);
    else if (type == "Max") node = graph->CreateNodeWithID<MaxNode>(id);
    else if (type == "Min") node = graph->CreateNodeWithID<MinNode>(id);

    // Output node
    else if (type == "Output") node = graph->CreateNodeWithID<OutputNode>(id);

    return node;
}

json GraphSerializer::SerializeNodeParams(const Node* node) {
    json params;

    String type = GetNodeTypeString(node);

    // Perlin Noise
    if (type == "PerlinNoise") {
        auto* perlin = static_cast<const PerlinNoiseNode*>(node);
        params["octaves"] = perlin->params.octaves;
        params["frequency"] = perlin->params.frequency;
        params["lacunarity"] = perlin->params.lacunarity;
        params["persistence"] = perlin->params.persistence;
        params["seed"] = perlin->params.seed;
    }
    // Voronoi Noise
    else if (type == "VoronoiNoise") {
        auto* voronoi = static_cast<const VoronoiNoiseNode*>(node);
        params["frequency"] = voronoi->params.frequency;
        params["seed"] = voronoi->params.seed;
        params["distanceType"] = static_cast<int>(voronoi->params.distanceType);
    }
    // Ridged Noise
    else if (type == "RidgedNoise") {
        auto* ridged = static_cast<const RidgedNoiseNode*>(node);
        params["octaves"] = ridged->params.octaves;
        params["frequency"] = ridged->params.frequency;
        params["lacunarity"] = ridged->params.lacunarity;
        params["gain"] = ridged->params.gain;
        params["seed"] = ridged->params.seed;
    }
    // Terrace
    else if (type == "Terrace") {
        auto* terrace = static_cast<const TerraceNode*>(node);
        params["levels"] = terrace->params.levels;
        params["smoothness"] = terrace->params.smoothness;
    }
    // Scale
    else if (type == "Scale") {
        auto* scale = static_cast<const ScaleNode*>(node);
        params["scale"] = scale->params.scale;
        params["bias"] = scale->params.bias;
    }
    // Add more node types as needed...

    return params;
}

bool GraphSerializer::DeserializeNodeParams(Node* node, const json& j) {
    String type = GetNodeTypeString(node);

    try {
        // Perlin Noise
        if (type == "PerlinNoise") {
            auto* perlin = static_cast<PerlinNoiseNode*>(node);
            if (j.contains("octaves")) perlin->params.octaves = j["octaves"];
            if (j.contains("frequency")) perlin->params.frequency = j["frequency"];
            if (j.contains("lacunarity")) perlin->params.lacunarity = j["lacunarity"];
            if (j.contains("persistence")) perlin->params.persistence = j["persistence"];
            if (j.contains("seed")) perlin->params.seed = j["seed"];
        }
        // Voronoi Noise
        else if (type == "VoronoiNoise") {
            auto* voronoi = static_cast<VoronoiNoiseNode*>(node);
            if (j.contains("frequency")) voronoi->params.frequency = j["frequency"];
            if (j.contains("seed")) voronoi->params.seed = j["seed"];
            if (j.contains("distanceType")) voronoi->params.distanceType = static_cast<VoronoiDistanceType>(j["distanceType"].get<int>());
        }
        // Ridged Noise
        else if (type == "RidgedNoise") {
            auto* ridged = static_cast<RidgedNoiseNode*>(node);
            if (j.contains("octaves")) ridged->params.octaves = j["octaves"];
            if (j.contains("frequency")) ridged->params.frequency = j["frequency"];
            if (j.contains("lacunarity")) ridged->params.lacunarity = j["lacunarity"];
            if (j.contains("gain")) ridged->params.gain = j["gain"];
            if (j.contains("seed")) ridged->params.seed = j["seed"];
        }
        // Terrace
        else if (type == "Terrace") {
            auto* terrace = static_cast<TerraceNode*>(node);
            if (j.contains("levels")) terrace->params.levels = j["levels"];
            if (j.contains("smoothness")) terrace->params.smoothness = j["smoothness"];
        }
        // Scale
        else if (type == "Scale") {
            auto* scale = static_cast<ScaleNode*>(node);
            if (j.contains("scale")) scale->params.scale = j["scale"];
            if (j.contains("bias")) scale->params.bias = j["bias"];
        }
        // Add more node types as needed...

        return true;
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception deserializing node params: %s", e.what());
        return false;
    }
}

} // namespace Terrain
