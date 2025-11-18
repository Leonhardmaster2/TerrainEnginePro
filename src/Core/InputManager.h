#pragma once

#include "Core/Types.h"
#include <functional>
#include <unordered_map>
#include <string>

struct GLFWwindow;

namespace Terrain {

// Keyboard shortcut structure
struct Shortcut {
    int key;        // GLFW key code
    bool ctrl = false;
    bool shift = false;
    bool alt = false;

    bool Matches(int pressedKey, bool ctrlPressed, bool shiftPressed, bool altPressed) const {
        return key == pressedKey &&
               ctrl == ctrlPressed &&
               shift == shiftPressed &&
               alt == altPressed;
    }
};

// Input action callback
using InputAction = std::function<void()>;

// Centralized input management
class InputManager {
public:
    static InputManager& Get();

    void Initialize(GLFWwindow* window);
    void Update();

    // Register keyboard shortcuts
    void RegisterShortcut(const String& actionName, const Shortcut& shortcut, InputAction action);
    void UnregisterShortcut(const String& actionName);

    // Check if key is pressed
    bool IsKeyPressed(int key) const;
    bool IsKeyDown(int key) const;
    bool IsKeyReleased(int key) const;

    // Modifiers
    bool IsCtrlDown() const;
    bool IsShiftDown() const;
    bool IsAltDown() const;

    // Mouse
    glm::vec2 GetMousePosition() const;
    glm::vec2 GetMouseDelta() const;
    bool IsMouseButtonPressed(int button) const;
    bool IsMouseButtonDown(int button) const;
    bool IsMouseButtonReleased(int button) const;

    // Enable/Disable input (useful for text input fields)
    void SetInputEnabled(bool enabled) { m_InputEnabled = enabled; }
    bool IsInputEnabled() const { return m_InputEnabled; }

private:
    InputManager() = default;
    ~InputManager() = default;
    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;

    void ProcessKeyboardInput();
    void ProcessMouseInput();

    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

    struct ShortcutData {
        Shortcut shortcut;
        InputAction action;
    };

    GLFWwindow* m_Window = nullptr;
    std::unordered_map<String, ShortcutData> m_Shortcuts;

    // Key states
    std::unordered_map<int, bool> m_KeyPressed;    // This frame
    std::unordered_map<int, bool> m_KeyDown;       // Held down
    std::unordered_map<int, bool> m_KeyReleased;   // This frame

    // Mouse states
    std::unordered_map<int, bool> m_MousePressed;
    std::unordered_map<int, bool> m_MouseDown;
    std::unordered_map<int, bool> m_MouseReleased;
    glm::vec2 m_MousePos = glm::vec2(0.0f);
    glm::vec2 m_LastMousePos = glm::vec2(0.0f);
    glm::vec2 m_MouseDelta = glm::vec2(0.0f);
    float32 m_ScrollDelta = 0.0f;

    bool m_InputEnabled = true;
};

} // namespace Terrain
