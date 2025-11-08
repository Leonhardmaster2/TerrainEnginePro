#pragma once

#include "Core/Types.h"
#include <imgui.h>
#include <vector>
#include <chrono>

namespace Terrain {

// Status message type
enum class StatusMessageType {
    Info,
    Success,
    Warning,
    Error
};

// Status message
struct StatusMessage {
    String text;
    StatusMessageType type;
    std::chrono::steady_clock::time_point timestamp;
    float32 duration; // seconds

    bool IsExpired() const {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - timestamp).count() / 1000.0f;
        return elapsed >= duration;
    }
};

// Status bar UI component
class StatusBar {
public:
    StatusBar();
    ~StatusBar();

    // Render status bar
    void Render();

    // Set persistent status text (shown on left side)
    void SetStatus(const String& text);

    // Show temporary message (notification)
    void ShowMessage(const String& text, StatusMessageType type = StatusMessageType::Info, float32 duration = 3.0f);

    // Quick message helpers
    void ShowInfo(const String& text, float32 duration = 3.0f);
    void ShowSuccess(const String& text, float32 duration = 3.0f);
    void ShowWarning(const String& text, float32 duration = 4.0f);
    void ShowError(const String& text, float32 duration = 5.0f);

    // Set statistics (shown on right side)
    void SetNodeCount(uint32 count);
    void SetExecutionTime(float32 ms);
    void SetCurrentFile(const String& filepath);

    // Clear all messages
    void ClearMessages();

private:
    void RenderMessage(const StatusMessage& msg);
    ImVec4 GetMessageColor(StatusMessageType type);
    const char* GetMessageIcon(StatusMessageType type);

    String m_StatusText;
    String m_CurrentFile;
    uint32 m_NodeCount = 0;
    float32 m_ExecutionTime = 0.0f;

    std::vector<StatusMessage> m_Messages;
    static constexpr uint32 MAX_MESSAGES = 5;
};

// Notification toast system (floating notifications)
class NotificationSystem {
public:
    NotificationSystem();
    ~NotificationSystem();

    // Render all active notifications
    void Render();

    // Add notification
    void Notify(const String& title, const String& message, StatusMessageType type = StatusMessageType::Info, float32 duration = 4.0f);

    // Quick notification helpers
    void NotifyInfo(const String& title, const String& message, float32 duration = 4.0f);
    void NotifySuccess(const String& title, const String& message, float32 duration = 4.0f);
    void NotifyWarning(const String& title, const String& message, float32 duration = 5.0f);
    void NotifyError(const String& title, const String& message, float32 duration = 6.0f);

    // Clear all notifications
    void Clear();

private:
    struct Notification {
        String title;
        String message;
        StatusMessageType type;
        std::chrono::steady_clock::time_point timestamp;
        float32 duration;

        bool IsExpired() const {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - timestamp).count() / 1000.0f;
            return elapsed >= duration;
        }

        float32 GetFadeAlpha() const {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - timestamp).count() / 1000.0f;

            // Fade in first 0.2s, fade out last 0.5s
            if (elapsed < 0.2f) {
                return elapsed / 0.2f;
            } else if (elapsed > duration - 0.5f) {
                return (duration - elapsed) / 0.5f;
            }
            return 1.0f;
        }
    };

    void RenderNotification(const Notification& notif, float32 yOffset);
    ImVec4 GetNotificationColor(StatusMessageType type);
    const char* GetNotificationIcon(StatusMessageType type);

    std::vector<Notification> m_Notifications;
    static constexpr uint32 MAX_NOTIFICATIONS = 5;
    static constexpr float32 NOTIFICATION_WIDTH = 350.0f;
    static constexpr float32 NOTIFICATION_PADDING = 10.0f;
};

} // namespace Terrain
