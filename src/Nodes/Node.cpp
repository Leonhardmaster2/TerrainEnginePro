#include "Node.h"
#include "NodeGraph.h"
#include "Core/Logger.h"

namespace Terrain {

uint32 Node::s_NextPinID = 1;

Node::Node(uint32 id, const String& name, NodeCategory category)
    : m_ID(id), m_Name(name), m_Category(category) {
}

void Node::Reset() {
    m_CachedOutput.reset();
    m_Dirty = true;
}

NodePin* Node::AddInputPin(const String& name, PinType type) {
    auto pin = MakeUnique<NodePin>();
    pin->id = s_NextPinID++;
    pin->name = name;
    pin->type = type;
    pin->node = this;
    pin->isOutput = false;

    NodePin* ptr = pin.get();
    m_Inputs.push_back(std::move(pin));
    return ptr;
}

NodePin* Node::AddOutputPin(const String& name, PinType type) {
    auto pin = MakeUnique<NodePin>();
    pin->id = s_NextPinID++;
    pin->name = name;
    pin->type = type;
    pin->node = this;
    pin->isOutput = true;

    NodePin* ptr = pin.get();
    m_Outputs.push_back(std::move(pin));
    return ptr;
}

NodePin* Node::GetInputPin(const String& name) {
    for (auto& pin : m_Inputs) {
        if (pin->name == name) {
            return pin.get();
        }
    }
    return nullptr;
}

NodePin* Node::GetOutputPin(const String& name) {
    for (auto& pin : m_Outputs) {
        if (pin->name == name) {
            return pin.get();
        }
    }
    return nullptr;
}

NodePin* Node::GetPin(uint32 pinId) {
    for (auto& pin : m_Inputs) {
        if (pin->id == pinId) {
            return pin.get();
        }
    }
    for (auto& pin : m_Outputs) {
        if (pin->id == pinId) {
            return pin.get();
        }
    }
    return nullptr;
}

Unique<Heightfield> Node::GetInputHeightfield(const String& pinName, NodeGraph* graph) {
    NodePin* pin = GetInputPin(pinName);
    if (!pin || !pin->connectedPin) {
        return nullptr;
    }

    // Execute the connected node to get its output
    Node* sourceNode = pin->connectedPin->node;
    if (!sourceNode->Execute(graph)) {
        LOG_ERROR("Failed to execute node: %s", sourceNode->GetName().c_str());
        return nullptr;
    }

    // Return a copy of the cached output
    if (sourceNode->m_CachedOutput) {
        return MakeUnique<Heightfield>(*sourceNode->m_CachedOutput);
    }

    return nullptr;
}

float32 Node::GetInputFloat(const String& pinName, float32 defaultValue) {
    NodePin* pin = GetInputPin(pinName);
    if (!pin) {
        return defaultValue;
    }

    if (pin->connectedPin) {
        // TODO: Get value from connected node
        return defaultValue;
    }

    return pin->floatValue;
}

int32 Node::GetInputInt(const String& pinName, int32 defaultValue) {
    NodePin* pin = GetInputPin(pinName);
    if (!pin) {
        return defaultValue;
    }

    if (pin->connectedPin) {
        // TODO: Get value from connected node
        return defaultValue;
    }

    return pin->intValue;
}

glm::vec2 Node::GetInputVec2(const String& pinName, const glm::vec2& defaultValue) {
    NodePin* pin = GetInputPin(pinName);
    if (!pin) {
        return defaultValue;
    }

    if (pin->connectedPin) {
        // TODO: Get value from connected node
        return defaultValue;
    }

    return pin->vec2Value;
}

void Node::SetOutputHeightfield(const String& pinName, Unique<Heightfield> heightfield) {
    NodePin* pin = GetOutputPin(pinName);
    if (!pin) {
        LOG_ERROR("Output pin not found: %s", pinName.c_str());
        return;
    }

    m_CachedOutput = std::move(heightfield);
    m_Dirty = false;
}

} // namespace Terrain
