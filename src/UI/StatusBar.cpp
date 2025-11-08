#include "StatusBar.h"
#include "UITheme.h"
#include <algorithm>

namespace Terrain {

// ============================================================================
// StatusBar Implementation
// ============================================================================

StatusBar::StatusBar() {
    m_StatusText = "Ready";
}

StatusBar::~StatusBar() {
}

void StatusBar::Render() {
    // Remove expired messages
    m_Messages.erase(
        std::remove_if(m_Messages.begin(), m_Messages.end(),
            [](const StatusMessage& msg) { return msg.IsExpired(); }),
        m_Messages.end()
    );

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + viewport->Size.y - 24));
    ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, 24));
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                              ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                              ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDocking;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 4));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, UITheme::ColorBackgroundDark);

    if (ImGui::Begin("##StatusBar", nullptr, flags)) {
        // Left side: Status text
        ImGui::Text("%s", m_StatusText.c_str());

        // Center: Current messages
        if (!m_Messages.empty()) {
            ImGui::SameLine();
            ImGui::Spacing();
            ImGui::SameLine();
            ImGui::Separator();
            ImGui::SameLine();

            RenderMessage(m_Messages.back());
        }

        // Right side: Statistics
        ImGui::SameLine(ImGui::GetWindowWidth() - 400);

        // File name
        if (!m_CurrentFile.empty()) {
            size_t lastSlash = m_CurrentFile.find_last_of("/\\");
            String filename = (lastSlash != String::npos) ? m_CurrentFile.substr(lastSlash + 1) : m_CurrentFile;
            ImGui::TextColored(UITheme::ColorTextDim, "File:");
            ImGui::SameLine();
            ImGui::Text("%s", filename.c_str());
            ImGui::SameLine();
            ImGui::Separator();
            ImGui::SameLine();
        }

        // Node count
        ImGui::TextColored(UITheme::ColorTextDim, "Nodes:");
        ImGui::SameLine();
        ImGui::Text("%u", m_NodeCount);
        ImGui::SameLine();
        ImGui::Separator();
        ImGui::SameLine();

        // Execution time
        if (m_ExecutionTime > 0.0f) {
            ImGui::TextColored(UITheme::ColorTextDim, "Exec:");
            ImGui::SameLine();
            ImGui::Text("%.1f ms", m_ExecutionTime);
        }
    }
    ImGui::End();

    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
}

void StatusBar::SetStatus(const String& text) {
    m_StatusText = text;
}

void StatusBar::ShowMessage(const String& text, StatusMessageType type, float32 duration) {
    StatusMessage msg;
    msg.text = text;
    msg.type = type;
    msg.timestamp = std::chrono::steady_clock::now();
    msg.duration = duration;

    m_Messages.push_back(msg);

    // Limit message count
    if (m_Messages.size() > MAX_MESSAGES) {
        m_Messages.erase(m_Messages.begin());
    }
}

void StatusBar::ShowInfo(const String& text, float32 duration) {
    ShowMessage(text, StatusMessageType::Info, duration);
}

void StatusBar::ShowSuccess(const String& text, float32 duration) {
    ShowMessage(text, StatusMessageType::Success, duration);
}

void StatusBar::ShowWarning(const String& text, float32 duration) {
    ShowMessage(text, StatusMessageType::Warning, duration);
}

void StatusBar::ShowError(const String& text, float32 duration) {
    ShowMessage(text, StatusMessageType::Error, duration);
}

void StatusBar::SetNodeCount(uint32 count) {
    m_NodeCount = count;
}

void StatusBar::SetExecutionTime(float32 ms) {
    m_ExecutionTime = ms;
}

void StatusBar::SetCurrentFile(const String& filepath) {
    m_CurrentFile = filepath;
}

void StatusBar::ClearMessages() {
    m_Messages.clear();
}

void StatusBar::RenderMessage(const StatusMessage& msg) {
    ImVec4 color = GetMessageColor(msg.type);
    const char* icon = GetMessageIcon(msg.type);

    ImGui::TextColored(color, "%s", icon);
    ImGui::SameLine();
    ImGui::TextColored(color, "%s", msg.text.c_str());
}

ImVec4 StatusBar::GetMessageColor(StatusMessageType type) {
    switch (type) {
        case StatusMessageType::Info: return UITheme::ColorInfo;
        case StatusMessageType::Success: return UITheme::ColorSuccess;
        case StatusMessageType::Warning: return UITheme::ColorWarning;
        case StatusMessageType::Error: return UITheme::ColorError;
        default: return UITheme::ColorText;
    }
}

const char* StatusBar::GetMessageIcon(StatusMessageType type) {
    switch (type) {
        case StatusMessageType::Info: return "[i]";
        case StatusMessageType::Success: return "[✓]";
        case StatusMessageType::Warning: return "[!]";
        case StatusMessageType::Error: return "[X]";
        default: return "";
    }
}

// ============================================================================
// NotificationSystem Implementation
// ============================================================================

NotificationSystem::NotificationSystem() {
}

NotificationSystem::~NotificationSystem() {
}

void NotificationSystem::Render() {
    // Remove expired notifications
    m_Notifications.erase(
        std::remove_if(m_Notifications.begin(), m_Notifications.end(),
            [](const Notification& n) { return n.IsExpired(); }),
        m_Notifications.end()
    );

    // Render active notifications
    float32 yOffset = NOTIFICATION_PADDING;
    for (const auto& notif : m_Notifications) {
        RenderNotification(notif, yOffset);
        yOffset += 80.0f + NOTIFICATION_PADDING; // Height + padding
    }
}

void NotificationSystem::Notify(const String& title, const String& message, StatusMessageType type, float32 duration) {
    Notification notif;
    notif.title = title;
    notif.message = message;
    notif.type = type;
    notif.timestamp = std::chrono::steady_clock::now();
    notif.duration = duration;

    m_Notifications.push_back(notif);

    // Limit notification count
    if (m_Notifications.size() > MAX_NOTIFICATIONS) {
        m_Notifications.erase(m_Notifications.begin());
    }
}

void NotificationSystem::NotifyInfo(const String& title, const String& message, float32 duration) {
    Notify(title, message, StatusMessageType::Info, duration);
}

void NotificationSystem::NotifySuccess(const String& title, const String& message, float32 duration) {
    Notify(title, message, StatusMessageType::Success, duration);
}

void NotificationSystem::NotifyWarning(const String& title, const String& message, float32 duration) {
    Notify(title, message, StatusMessageType::Warning, duration);
}

void NotificationSystem::NotifyError(const String& title, const String& message, float32 duration) {
    Notify(title, message, StatusMessageType::Error, duration);
}

void NotificationSystem::Clear() {
    m_Notifications.clear();
}

void NotificationSystem::RenderNotification(const Notification& notif, float32 yOffset) {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    float32 xPos = viewport->Pos.x + viewport->Size.x - NOTIFICATION_WIDTH - NOTIFICATION_PADDING;
    float32 yPos = viewport->Pos.y + yOffset;

    ImGui::SetNextWindowPos(ImVec2(xPos, yPos));
    ImGui::SetNextWindowSize(ImVec2(NOTIFICATION_WIDTH, 0)); // Auto height

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                              ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                              ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDocking |
                              ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoInputs;

    float32 alpha = notif.GetFadeAlpha();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12, 10));
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);

    ImVec4 bgColor = GetNotificationColor(notif.type);
    bgColor.w = 0.95f * alpha;
    ImGui::PushStyleColor(ImGuiCol_WindowBg, bgColor);
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 1.0f, 1.0f, 0.3f * alpha));

    String windowID = "##Notification" + std::to_string(reinterpret_cast<uintptr_t>(&notif));
    if (ImGui::Begin(windowID.c_str(), nullptr, flags)) {
        const char* icon = GetNotificationIcon(notif.type);

        // Title with icon
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]); // Default font
        ImGui::Text("%s ", icon);
        ImGui::SameLine();
        ImGui::Text("%s", notif.title.c_str());
        ImGui::PopFont();

        // Message
        ImGui::Spacing();
        ImGui::PushTextWrapPos(NOTIFICATION_WIDTH - 24);
        ImGui::TextWrapped("%s", notif.message.c_str());
        ImGui::PopTextWrapPos();
    }
    ImGui::End();

    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(2);
}

ImVec4 NotificationSystem::GetNotificationColor(StatusMessageType type) {
    switch (type) {
        case StatusMessageType::Info: return ImVec4(0.2f, 0.4f, 0.6f, 0.95f);
        case StatusMessageType::Success: return ImVec4(0.2f, 0.6f, 0.3f, 0.95f);
        case StatusMessageType::Warning: return ImVec4(0.7f, 0.5f, 0.2f, 0.95f);
        case StatusMessageType::Error: return ImVec4(0.7f, 0.2f, 0.2f, 0.95f);
        default: return ImVec4(0.3f, 0.3f, 0.3f, 0.95f);
    }
}

const char* NotificationSystem::GetNotificationIcon(StatusMessageType type) {
    switch (type) {
        case StatusMessageType::Info: return "ℹ";
        case StatusMessageType::Success: return "✓";
        case StatusMessageType::Warning: return "⚠";
        case StatusMessageType::Error: return "✖";
        default: return "•";
    }
}

} // namespace Terrain
