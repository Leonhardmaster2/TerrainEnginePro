#pragma once

#include "Core/Types.h"
#include <GLFW/glfw3.h>

namespace Terrain {

class TerrainEditor;

class Application {
public:
    Application();
    ~Application();

    bool Initialize();
    void Run();
    void Shutdown();

    GLFWwindow* GetWindow() const { return m_Window; }
    int GetWidth() const { return m_Width; }
    int GetHeight() const { return m_Height; }

private:
    bool InitializeGLFW();
    void InitializeImGui();
    void ProcessInput();
    void Update(float deltaTime);
    void Render();

    GLFWwindow* m_Window = nullptr;
    int m_Width = 1920;
    int m_Height = 1080;
    bool m_Running = true;

    Unique<TerrainEditor> m_Editor;
};

} // namespace Terrain
