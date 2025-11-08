#include "NodeGraph.h"
#include "Core/Logger.h"

namespace Terrain {

NodeGraph::NodeGraph() {
    m_Generator = MakeUnique<TerrainGenerator>();
    if (!m_Generator->Initialize()) {
        LOG_ERROR("Failed to initialize terrain generator for node graph");
    }
}

NodeGraph::~NodeGraph() {
    Clear();
}

void NodeGraph::DeleteNode(uint32 nodeId) {
    auto it = m_Nodes.find(nodeId);
    if (it == m_Nodes.end()) {
        return;
    }

    Node* node = it->second.get();

    // Remove all connections to/from this node
    for (auto& input : node->GetInputs()) {
        if (input->connectedPin) {
            // Remove from output's connection list
            auto& connections = input->connectedPin->connections;
            connections.erase(
                std::remove(connections.begin(), connections.end(), input.get()),
                connections.end()
            );
        }
    }

    for (auto& output : node->GetOutputs()) {
        // Disconnect all inputs connected to this output
        for (NodePin* connectedInput : output->connections) {
            connectedInput->connectedPin = nullptr;
        }
    }

    // Remove the node
    m_Nodes.erase(it);

    // Mark all nodes dirty since graph structure changed
    MarkAllDirty();
}

Node* NodeGraph::GetNode(uint32 nodeId) {
    auto it = m_Nodes.find(nodeId);
    if (it != m_Nodes.end()) {
        return it->second.get();
    }
    return nullptr;
}

bool NodeGraph::CreateConnection(uint32 outputPinId, uint32 inputPinId) {
    // Find the pins
    NodePin* outputPin = nullptr;
    NodePin* inputPin = nullptr;

    for (auto& [id, node] : m_Nodes) {
        NodePin* pin = node->GetPin(outputPinId);
        if (pin && pin->isOutput) {
            outputPin = pin;
        }
        pin = node->GetPin(inputPinId);
        if (pin && !pin->isOutput) {
            inputPin = pin;
        }
    }

    if (!outputPin || !inputPin) {
        LOG_ERROR("Invalid pin IDs for connection");
        return false;
    }

    // Type checking
    if (outputPin->type != inputPin->type) {
        LOG_ERROR("Cannot connect pins of different types");
        return false;
    }

    // Remove existing connection from input
    if (inputPin->connectedPin) {
        auto& oldConnections = inputPin->connectedPin->connections;
        oldConnections.erase(
            std::remove(oldConnections.begin(), oldConnections.end(), inputPin),
            oldConnections.end()
        );
    }

    // Create new connection
    inputPin->connectedPin = outputPin;
    outputPin->connections.push_back(inputPin);

    // Mark downstream nodes dirty
    inputPin->node->MarkDirty();

    return true;
}

void NodeGraph::DeleteConnection(uint32 inputPinId) {
    for (auto& [id, node] : m_Nodes) {
        NodePin* pin = node->GetPin(inputPinId);
        if (pin && !pin->isOutput && pin->connectedPin) {
            // Remove from output's connection list
            auto& connections = pin->connectedPin->connections;
            connections.erase(
                std::remove(connections.begin(), connections.end(), pin),
                connections.end()
            );

            // Clear input's connection
            pin->connectedPin = nullptr;

            // Mark node dirty
            node->MarkDirty();
            return;
        }
    }
}

bool NodeGraph::IsConnected(uint32 pinId) const {
    for (const auto& [id, node] : m_Nodes) {
        for (const auto& input : node->GetInputs()) {
            if (input->id == pinId) {
                return input->connectedPin != nullptr;
            }
        }
        for (const auto& output : node->GetOutputs()) {
            if (output->id == pinId) {
                return !output->connections.empty();
            }
        }
    }
    return false;
}

bool NodeGraph::ConnectPins(Pin* outputPin, Pin* inputPin) {
    if (!outputPin || !inputPin) {
        LOG_ERROR("Invalid pins for connection");
        return false;
    }

    if (!outputPin->isOutput || inputPin->isOutput) {
        LOG_ERROR("Invalid pin types for connection");
        return false;
    }

    // Type checking
    if (outputPin->type != inputPin->type) {
        LOG_ERROR("Cannot connect pins of different types");
        return false;
    }

    // Remove existing connection from input
    if (inputPin->connectedTo) {
        auto& oldConnections = inputPin->connectedTo->connections;
        oldConnections.erase(
            std::remove(oldConnections.begin(), oldConnections.end(), inputPin),
            oldConnections.end()
        );
    }

    // Create new connection
    inputPin->connectedTo = outputPin;
    outputPin->connections.push_back(inputPin);

    // Mark downstream node dirty
    inputPin->ownerNode->MarkDirty();

    return true;
}

bool NodeGraph::ExecuteNode(Node* node) {
    if (!node) {
        return false;
    }

    // If node is already clean, skip execution
    if (!node->IsDirty()) {
        return true;
    }

    return node->Execute(this);
}

bool NodeGraph::ExecuteGraph() {
    if (!m_OutputNode) {
        LOG_WARN("No output node set");
        return false;
    }

    return ExecuteNode(m_OutputNode);
}

void NodeGraph::MarkAllDirty() {
    for (auto& [id, node] : m_Nodes) {
        node->MarkDirty();
    }
}

Unique<Heightfield> NodeGraph::GetResult() {
    if (!m_OutputNode || !m_OutputNode->m_CachedOutput) {
        return nullptr;
    }

    // Return a copy
    return MakeUnique<Heightfield>(*m_OutputNode->m_CachedOutput);
}

void NodeGraph::Clear() {
    m_Nodes.clear();
    m_OutputNode = nullptr;
    m_NextNodeID = 1;
}

} // namespace Terrain
