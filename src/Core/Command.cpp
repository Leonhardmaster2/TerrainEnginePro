#include "Command.h"
#include "Logger.h"

namespace Terrain {

CommandHistory::CommandHistory() {
}

void CommandHistory::ExecuteCommand(Unique<ICommand> command) {
    if (!command) {
        LOG_WARNING("Attempted to execute null command");
        return;
    }

    // Try to merge with previous command if possible
    if (m_CurrentIndex > 0 && m_CurrentIndex <= m_History.size()) {
        ICommand* prevCommand = m_History[m_CurrentIndex - 1].get();
        if (prevCommand->CanMerge(command.get())) {
            prevCommand->MergeWith(command.get());
            return;
        }
    }

    // Remove any commands after current index (we're branching history)
    if (m_CurrentIndex < m_History.size()) {
        m_History.erase(m_History.begin() + m_CurrentIndex, m_History.end());
    }

    // Execute the command
    command->Execute();

    // Add to history
    m_History.push_back(std::move(command));
    m_CurrentIndex++;

    // Trim if exceeds max size
    TrimHistory();

    LOG_INFO("Command executed: %s (History: %zu/%zu)",
             m_History.back()->GetDescription().c_str(),
             m_CurrentIndex, m_History.size());
}

bool CommandHistory::CanUndo() const {
    return m_CurrentIndex > 0;
}

bool CommandHistory::CanRedo() const {
    return m_CurrentIndex < m_History.size();
}

void CommandHistory::Undo() {
    if (!CanUndo()) {
        LOG_WARNING("Cannot undo: history is empty");
        return;
    }

    m_CurrentIndex--;
    m_History[m_CurrentIndex]->Undo();

    LOG_INFO("Undo: %s", m_History[m_CurrentIndex]->GetDescription().c_str());
}

void CommandHistory::Redo() {
    if (!CanRedo()) {
        LOG_WARNING("Cannot redo: at end of history");
        return;
    }

    m_History[m_CurrentIndex]->Redo();
    m_CurrentIndex++;

    LOG_INFO("Redo: %s", m_History[m_CurrentIndex - 1]->GetDescription().c_str());
}

void CommandHistory::Clear() {
    m_History.clear();
    m_CurrentIndex = 0;
    LOG_INFO("Command history cleared");
}

String CommandHistory::GetUndoDescription() const {
    if (!CanUndo()) return "";
    return m_History[m_CurrentIndex - 1]->GetDescription();
}

String CommandHistory::GetRedoDescription() const {
    if (!CanRedo()) return "";
    return m_History[m_CurrentIndex]->GetDescription();
}

void CommandHistory::TrimHistory() {
    // Remove oldest commands if we exceed max size
    while (m_History.size() > m_MaxHistorySize) {
        m_History.erase(m_History.begin());
        if (m_CurrentIndex > 0) {
            m_CurrentIndex--;
        }
    }
}

} // namespace Terrain
