#pragma once

#include "Core/Command.h"
#include "Core/Types.h"
#include "NodeGraph.h"
#include "Node.h"
#include <glm/glm.hpp>
#include <functional>

namespace Terrain {

// Command: Create Node
class CreateNodeCommand : public ICommand {
public:
    CreateNodeCommand(NodeGraph* graph, const String& nodeType, const glm::vec2& position);
    ~CreateNodeCommand() override = default;

    void Execute() override;
    void Undo() override;
    String GetDescription() const override { return "Create " + m_NodeType; }

private:
    NodeGraph* m_Graph;
    String m_NodeType;
    glm::vec2 m_Position;
    uint32 m_NodeId = 0;
    std::function<Node*()> m_CreateFunc;
};

// Command: Delete Node
class DeleteNodeCommand : public ICommand {
public:
    DeleteNodeCommand(NodeGraph* graph, uint32 nodeId);
    ~DeleteNodeCommand() override = default;

    void Execute() override;
    void Undo() override;
    String GetDescription() const override;

private:
    struct ConnectionData {
        uint32 outputNodeId;
        String outputPinName;
        uint32 inputNodeId;
        String inputPinName;
    };

    NodeGraph* m_Graph;
    uint32 m_NodeId;
    String m_NodeType;
    glm::vec2 m_Position;
    std::vector<ConnectionData> m_Connections; // Connections to restore
    // TODO: Store node parameters
};

// Command: Create Connection
class CreateConnectionCommand : public ICommand {
public:
    CreateConnectionCommand(NodeGraph* graph, uint32 outputPinId, uint32 inputPinId);
    ~CreateConnectionCommand() override = default;

    void Execute() override;
    void Undo() override;
    String GetDescription() const override { return "Connect Nodes"; }

private:
    NodeGraph* m_Graph;
    uint32 m_OutputPinId;
    uint32 m_InputPinId;
    uint32 m_PreviousOutputPinId = 0; // For undo: what was connected before
};

// Command: Delete Connection
class DeleteConnectionCommand : public ICommand {
public:
    DeleteConnectionCommand(NodeGraph* graph, uint32 inputPinId);
    ~DeleteConnectionCommand() override = default;

    void Execute() override;
    void Undo() override;
    String GetDescription() const override { return "Disconnect Nodes"; }

private:
    NodeGraph* m_Graph;
    uint32 m_InputPinId;
    uint32 m_OutputPinId = 0; // For undo
};

// Command: Move Node
class MoveNodeCommand : public ICommand {
public:
    MoveNodeCommand(Node* node, const glm::vec2& newPosition);
    ~MoveNodeCommand() override = default;

    void Execute() override;
    void Undo() override;
    void Redo() override { Execute(); }
    String GetDescription() const override { return "Move Node"; }

    // Allow merging consecutive move commands for the same node
    bool CanMerge(const ICommand* other) const override;
    void MergeWith(const ICommand* other) override;

private:
    Node* m_Node;
    glm::vec2 m_OldPosition;
    glm::vec2 m_NewPosition;
};

// Command: Change Float Parameter
class ChangeFloatParamCommand : public ICommand {
public:
    ChangeFloatParamCommand(float32* param, float32 newValue, const String& description);
    ~ChangeFloatParamCommand() override = default;

    void Execute() override;
    void Undo() override;
    void Redo() override { Execute(); }
    String GetDescription() const override { return m_Description; }

    bool CanMerge(const ICommand* other) const override;
    void MergeWith(const ICommand* other) override;

private:
    float32* m_Param;
    float32 m_OldValue;
    float32 m_NewValue;
    String m_Description;
};

// Command: Change Int Parameter
class ChangeIntParamCommand : public ICommand {
public:
    ChangeIntParamCommand(int32* param, int32 newValue, const String& description);
    ~ChangeIntParamCommand() override = default;

    void Execute() override;
    void Undo() override;
    void Redo() override { Execute(); }
    String GetDescription() const override { return m_Description; }

    bool CanMerge(const ICommand* other) const override;
    void MergeWith(const ICommand* other) override;

private:
    int32* m_Param;
    int32 m_OldValue;
    int32 m_NewValue;
    String m_Description;
};

// Command: Composite (batch multiple commands)
class CompositeCommand : public ICommand {
public:
    CompositeCommand(const String& description);
    ~CompositeCommand() override = default;

    void AddCommand(Unique<ICommand> command);

    void Execute() override;
    void Undo() override;
    void Redo() override;
    String GetDescription() const override { return m_Description; }

private:
    String m_Description;
    std::vector<Unique<ICommand>> m_Commands;
};

} // namespace Terrain
