#pragma once

#include "Core/Types.h"
#include "Rendering/Camera.h"
#include "Rendering/Shader.h"
#include "Rendering/TerrainMesh.h"
#include "Terrain/TerrainGenerator.h"
#include "Terrain/Heightfield.h"
#include "NodeGraphEditor.h"

namespace Terrain {

struct EditorState {
    // Generation parameters
    PerlinParams perlinParams;
    uint32 terrainWidth = 512;
    uint32 terrainHeight = 512;
    float32 heightScale = 100.0f;

    // Render settings
    RenderMode renderMode = RenderMode::Solid;
    bool showGrid = true;
    bool showStats = true;
    bool showParameters = true;
    bool showNodeGraph = true;

    // Export settings
    char exportPath[256] = "terrain";
    bool export16BitPNG = true;

    // Camera settings
    float32 cameraSpeed = 1.0f;
    float32 cameraSensitivity = 0.5f;

    // Node graph mode
    bool useNodeGraph = false;
};

class TerrainEditor {
public:
    TerrainEditor();
    ~TerrainEditor();

    bool Initialize();
    void Update(float32 deltaTime);
    void Render();

private:
    // UI Rendering
    void RenderMenuBar();
    void RenderViewport3D();
    void RenderParametersPanel();
    void RenderStatsPanel();
    void RenderExportPanel();

    // Actions
    void GenerateTerrain();
    void ExportHeightmap();
    void ExportMesh();
    void ResetCamera();

    // Input handling
    void HandleViewportInput();

    // Systems
    Unique<TerrainGenerator> m_Generator;
    Unique<Shader> m_TerrainShader;
    Unique<Camera> m_Camera;
    Unique<NodeGraphEditor> m_NodeGraphEditor;

    // Current terrain data
    Unique<Heightfield> m_CurrentHeightfield;
    Unique<TerrainMesh> m_CurrentMesh;

    // State
    EditorState m_State;

    // UI state
    bool m_ViewportHovered = false;
    bool m_ViewportFocused = false;
    glm::vec2 m_LastMousePos;
    bool m_IsDragging = false;

    // Generation state
    bool m_NeedsRegeneration = false;
    bool m_IsGenerating = false;
    float32 m_GenerationTime = 0.0f;

    // Framebuffer for viewport
    uint32 m_ViewportFBO = 0;
    uint32 m_ViewportTexture = 0;
    uint32 m_ViewportDepthBuffer = 0;
    uint32 m_ViewportWidth = 1024;
    uint32 m_ViewportHeight = 768;
};

} // namespace Terrain
