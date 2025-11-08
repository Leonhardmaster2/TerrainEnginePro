#pragma once

#include "Core/Types.h"
#include "Terrain/Heightfield.h"
#include <vector>
#include <string>
#include <memory>

namespace Terrain {

// Forward declarations
class Node;
class NodeGraph;

// Node pin types
enum class PinType {
    Heightfield,
    Float,
    Int,
    Vec2,
    Vec3
};

// Node pin (input or output)
struct NodePin {
    uint32 id;
    String name;
    PinType type;
    Node* node = nullptr;
    bool isOutput = false;

    // For inputs: the connected output pin
    NodePin* connectedPin = nullptr;

    // For outputs: list of connected input pins
    std::vector<NodePin*> connections;

    // Cached value for constant inputs
    float32 floatValue = 0.0f;
    int32 intValue = 0;
    glm::vec2 vec2Value = glm::vec2(0.0f);
    glm::vec3 vec3Value = glm::vec3(0.0f);
};

// Node categories
enum class NodeCategory {
    Generator,
    Modifier,
    Combiner,
    Filter,
    Output
};

// Base node class
class Node {
public:
    Node(uint32 id, const String& name, NodeCategory category);
    virtual ~Node() = default;

    // Core interface
    virtual bool Execute(NodeGraph* graph) = 0;
    virtual void Reset();

    // Pin management
    NodePin* AddInputPin(const String& name, PinType type);
    NodePin* AddOutputPin(const String& name, PinType type);
    NodePin* GetInputPin(const String& name);
    NodePin* GetOutputPin(const String& name);
    NodePin* GetPin(uint32 pinId);

    // Getters
    uint32 GetID() const { return m_ID; }
    const String& GetName() const { return m_Name; }
    NodeCategory GetCategory() const { return m_Category; }
    const std::vector<Unique<NodePin>>& GetInputs() const { return m_Inputs; }
    const std::vector<Unique<NodePin>>& GetOutputs() const { return m_Outputs; }

    // Cache management
    bool IsDirty() const { return m_Dirty; }
    void MarkDirty() { m_Dirty = true; }
    void MarkClean() { m_Dirty = false; }

    // UI position
    glm::vec2 GetPosition() const { return m_Position; }
    void SetPosition(const glm::vec2& pos) { m_Position = pos; }

protected:
    // Helper methods for getting input values
    Unique<Heightfield> GetInputHeightfield(const String& pinName, NodeGraph* graph);
    float32 GetInputFloat(const String& pinName, float32 defaultValue = 0.0f);
    int32 GetInputInt(const String& pinName, int32 defaultValue = 0);
    glm::vec2 GetInputVec2(const String& pinName, const glm::vec2& defaultValue = glm::vec2(0.0f));

    // Helper for setting output
    void SetOutputHeightfield(const String& pinName, Unique<Heightfield> heightfield);

    uint32 m_ID;
    String m_Name;
    NodeCategory m_Category;

    std::vector<Unique<NodePin>> m_Inputs;
    std::vector<Unique<NodePin>> m_Outputs;

    bool m_Dirty = true;
    glm::vec2 m_Position = glm::vec2(0.0f);

    // Cached output
    Unique<Heightfield> m_CachedOutput;

private:
    static uint32 s_NextPinID;
};

} // namespace Terrain
