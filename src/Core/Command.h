#pragma once

#include "Core/Types.h"
#include <memory>
#include <vector>

namespace Terrain {

// Forward declaration
class NodeGraph;

// Base command interface using Command pattern
class ICommand {
public:
    virtual ~ICommand() = default;

    virtual void Execute() = 0;
    virtual void Undo() = 0;
    virtual void Redo() { Execute(); } // By default, redo = execute

    virtual String GetDescription() const = 0;
    virtual bool CanMerge(const ICommand* other) const { return false; }
    virtual void MergeWith(const ICommand* other) {}
};

// Command history manager
class CommandHistory {
public:
    CommandHistory();
    ~CommandHistory() = default;

    // Execute and add command to history
    void ExecuteCommand(Unique<ICommand> command);

    // Undo/Redo
    bool CanUndo() const;
    bool CanRedo() const;
    void Undo();
    void Redo();

    // Clear history
    void Clear();

    // Get history info
    String GetUndoDescription() const;
    String GetRedoDescription() const;
    size_t GetHistorySize() const { return m_History.size(); }
    size_t GetCurrentIndex() const { return m_CurrentIndex; }

    // Limits
    void SetMaxHistorySize(size_t size) { m_MaxHistorySize = size; }
    size_t GetMaxHistorySize() const { return m_MaxHistorySize; }

private:
    void TrimHistory();

    std::vector<Unique<ICommand>> m_History;
    size_t m_CurrentIndex = 0; // Points to next command to execute
    size_t m_MaxHistorySize = 100;
};

} // namespace Terrain
