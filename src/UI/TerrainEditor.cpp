#include "TerrainEditor.h"
#include "Core/Logger.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <chrono>

namespace Terrain {

TerrainEditor::TerrainEditor() {
}

TerrainEditor::~TerrainEditor() {
    if (m_ViewportFBO) glDeleteFramebuffers(1, &m_ViewportFBO);
    if (m_ViewportTexture) glDeleteTextures(1, &m_ViewportTexture);
    if (m_ViewportDepthBuffer) glDeleteRenderbuffers(1, &m_ViewportDepthBuffer);
}

bool TerrainEditor::Initialize() {
    LOG_INFO("Initializing Terrain Editor...");

    // Initialize terrain generator
    m_Generator = MakeUnique<TerrainGenerator>();
    if (!m_Generator->Initialize()) {
        LOG_ERROR("Failed to initialize terrain generator");
        return false;
    }

    // Initialize camera
    m_Camera = MakeUnique<Camera>();
    m_Camera->SetTarget(glm::vec3(0.0f, 0.0f, 0.0f));
    m_Camera->SetDistance(500.0f);

    // Load terrain shader
    m_TerrainShader = MakeUnique<Shader>();
    if (!m_TerrainShader->LoadFromFiles("shaders/terrain.vert", "shaders/terrain.frag")) {
        LOG_ERROR("Failed to load terrain shader");
        return false;
    }

    // Initialize default perlin parameters
    m_State.perlinParams.frequency = 0.01f;
    m_State.perlinParams.amplitude = 1.0f;
    m_State.perlinParams.octaves = 6;
    m_State.perlinParams.lacunarity = 2.0f;
    m_State.perlinParams.persistence = 0.5f;
    m_State.perlinParams.seed = 12345;

    // Create viewport framebuffer
    glGenFramebuffers(1, &m_ViewportFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_ViewportFBO);

    // Create color texture
    glGenTextures(1, &m_ViewportTexture);
    glBindTexture(GL_TEXTURE_2D, m_ViewportTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_ViewportWidth, m_ViewportHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ViewportTexture, 0);

    // Create depth buffer
    glGenRenderbuffers(1, &m_ViewportDepthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_ViewportDepthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_ViewportWidth, m_ViewportHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_ViewportDepthBuffer);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        LOG_ERROR("Viewport framebuffer is not complete");
        return false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Initialize node graph editor
    m_NodeGraphEditor = MakeUnique<NodeGraphEditor>();
    if (!m_NodeGraphEditor->Initialize()) {
        LOG_ERROR("Failed to initialize node graph editor");
        return false;
    }

    // Generate initial terrain
    GenerateTerrain();

    LOG_INFO("Terrain Editor initialized successfully");
    return true;
}

void TerrainEditor::Update(float32 deltaTime) {
    // Handle viewport input if focused
    if (m_ViewportFocused) {
        HandleViewportInput();
    }

    // Regenerate terrain if needed
    if (m_NeedsRegeneration && !m_IsGenerating) {
        GenerateTerrain();
        m_NeedsRegeneration = false;
    }
}

void TerrainEditor::Render() {
    // Main menu bar
    RenderMenuBar();

    // Main docking space
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("DockSpace", nullptr, window_flags);
    ImGui::PopStyleVar(3);

    ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

    ImGui::End();

    // Render panels
    RenderViewport3D();

    if (m_State.showParameters && !m_State.useNodeGraph) {
        RenderParametersPanel();
    }

    if (m_State.showStats) {
        RenderStatsPanel();
    }

    if (m_State.showNodeGraph) {
        m_NodeGraphEditor->Render();
    }

    RenderExportPanel();
}

void TerrainEditor::RenderMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Terrain")) {
                GenerateTerrain();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Export Heightmap...")) {
                ExportHeightmap();
            }
            if (ImGui::MenuItem("Export Mesh (OBJ)...")) {
                ExportMesh();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) {
                // Signal to close application
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Parameters", nullptr, &m_State.showParameters);
            ImGui::MenuItem("Statistics", nullptr, &m_State.showStats);
            ImGui::MenuItem("Node Graph", nullptr, &m_State.showNodeGraph);
            ImGui::MenuItem("Grid", nullptr, &m_State.showGrid);
            ImGui::Separator();
            if (ImGui::MenuItem("Use Node Graph", nullptr, &m_State.useNodeGraph)) {
                if (m_State.useNodeGraph) {
                    // Switch to node graph mode
                    m_NeedsRegeneration = true;
                }
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Reset Camera")) {
                ResetCamera();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Render Mode")) {
            if (ImGui::MenuItem("Solid", nullptr, m_State.renderMode == RenderMode::Solid)) {
                m_State.renderMode = RenderMode::Solid;
            }
            if (ImGui::MenuItem("Wireframe", nullptr, m_State.renderMode == RenderMode::Wireframe)) {
                m_State.renderMode = RenderMode::Wireframe;
            }
            if (ImGui::MenuItem("Clay", nullptr, m_State.renderMode == RenderMode::Clay)) {
                m_State.renderMode = RenderMode::Clay;
            }
            if (ImGui::MenuItem("Unlit", nullptr, m_State.renderMode == RenderMode::Unlit)) {
                m_State.renderMode = RenderMode::Unlit;
            }
            if (ImGui::MenuItem("Normals", nullptr, m_State.renderMode == RenderMode::Normals)) {
                m_State.renderMode = RenderMode::Normals;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About")) {
                LOG_INFO("Terrain Engine Pro v0.3");
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void TerrainEditor::RenderViewport3D() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("3D Viewport");

    m_ViewportHovered = ImGui::IsWindowHovered();
    m_ViewportFocused = ImGui::IsWindowFocused();

    // Get viewport size
    ImVec2 viewportSize = ImGui::GetContentRegionAvail();
    if (viewportSize.x != m_ViewportWidth || viewportSize.y != m_ViewportHeight) {
        m_ViewportWidth = static_cast<uint32>(viewportSize.x);
        m_ViewportHeight = static_cast<uint32>(viewportSize.y);

        // Resize framebuffer
        glBindTexture(GL_TEXTURE_2D, m_ViewportTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_ViewportWidth, m_ViewportHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

        glBindRenderbuffer(GL_RENDERBUFFER, m_ViewportDepthBuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_ViewportWidth, m_ViewportHeight);
    }

    // Render to framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_ViewportFBO);
    glViewport(0, 0, m_ViewportWidth, m_ViewportHeight);

    glClearColor(0.15f, 0.15f, 0.18f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render terrain if available
    if (m_CurrentMesh && m_TerrainShader) {
        m_TerrainShader->Use();

        // Set matrices
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = m_Camera->GetViewMatrix();
        glm::mat4 projection = m_Camera->GetProjectionMatrix(
            static_cast<float32>(m_ViewportWidth) / static_cast<float32>(m_ViewportHeight)
        );

        m_TerrainShader->SetMat4("model", model);
        m_TerrainShader->SetMat4("view", view);
        m_TerrainShader->SetMat4("projection", projection);

        // Set render mode
        m_TerrainShader->SetInt("renderMode", static_cast<int32>(m_State.renderMode));

        // Set lighting
        glm::vec3 lightPos = m_Camera->GetPosition();
        m_TerrainShader->SetVec3("lightPos", lightPos);
        m_TerrainShader->SetVec3("viewPos", m_Camera->GetPosition());

        // Render mesh
        m_CurrentMesh->Render(m_State.renderMode);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Display framebuffer texture in ImGui
    ImGui::Image(
        (void*)(intptr_t)m_ViewportTexture,
        viewportSize,
        ImVec2(0, 1), ImVec2(1, 0) // Flip Y
    );

    ImGui::End();
    ImGui::PopStyleVar();
}

void TerrainEditor::RenderParametersPanel() {
    ImGui::Begin("Parameters", &m_State.showParameters);

    ImGui::Text("Terrain Generation");
    ImGui::Separator();

    bool paramsChanged = false;

    // Resolution
    ImGui::Text("Resolution");
    int resolutions[] = { 128, 256, 512, 1024, 2048, 4096 };
    const char* resolutionNames[] = { "128x128", "256x256", "512x512", "1024x1024", "2048x2048", "4096x4096" };
    int currentRes = 2; // Default to 512
    for (int i = 0; i < 6; i++) {
        if (resolutions[i] == m_State.terrainWidth) {
            currentRes = i;
            break;
        }
    }
    if (ImGui::Combo("##Resolution", &currentRes, resolutionNames, 6)) {
        m_State.terrainWidth = resolutions[currentRes];
        m_State.terrainHeight = resolutions[currentRes];
        paramsChanged = true;
    }

    ImGui::Spacing();

    // Perlin parameters
    ImGui::Text("Perlin Noise");
    paramsChanged |= ImGui::SliderFloat("Frequency", &m_State.perlinParams.frequency, 0.001f, 0.1f, "%.4f");
    paramsChanged |= ImGui::SliderFloat("Amplitude", &m_State.perlinParams.amplitude, 0.1f, 2.0f);
    paramsChanged |= ImGui::SliderInt("Octaves", &m_State.perlinParams.octaves, 1, 10);
    paramsChanged |= ImGui::SliderFloat("Lacunarity", &m_State.perlinParams.lacunarity, 1.5f, 3.0f);
    paramsChanged |= ImGui::SliderFloat("Persistence", &m_State.perlinParams.persistence, 0.1f, 0.9f);

    ImGui::Spacing();

    // Seed
    int seed = static_cast<int>(m_State.perlinParams.seed);
    if (ImGui::InputInt("Seed", &seed)) {
        m_State.perlinParams.seed = static_cast<uint32>(seed);
        paramsChanged = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Random")) {
        m_State.perlinParams.seed = static_cast<uint32>(std::rand());
        paramsChanged = true;
    }

    ImGui::Spacing();

    // Height scale
    paramsChanged |= ImGui::SliderFloat("Height Scale", &m_State.heightScale, 10.0f, 500.0f);

    ImGui::Spacing();
    ImGui::Separator();

    // Generate button
    if (ImGui::Button("Generate Terrain", ImVec2(-1, 40)) || paramsChanged) {
        m_NeedsRegeneration = true;
    }

    ImGui::Spacing();

    // Presets
    ImGui::Text("Presets");
    if (ImGui::Button("Rolling Hills", ImVec2(-1, 0))) {
        m_State.perlinParams.frequency = 0.01f;
        m_State.perlinParams.amplitude = 1.0f;
        m_State.perlinParams.octaves = 4;
        m_State.perlinParams.lacunarity = 2.0f;
        m_State.perlinParams.persistence = 0.5f;
        m_State.heightScale = 50.0f;
        m_NeedsRegeneration = true;
    }
    if (ImGui::Button("Mountains", ImVec2(-1, 0))) {
        m_State.perlinParams.frequency = 0.005f;
        m_State.perlinParams.amplitude = 1.5f;
        m_State.perlinParams.octaves = 8;
        m_State.perlinParams.lacunarity = 2.2f;
        m_State.perlinParams.persistence = 0.6f;
        m_State.heightScale = 200.0f;
        m_NeedsRegeneration = true;
    }
    if (ImGui::Button("Plateaus", ImVec2(-1, 0))) {
        m_State.perlinParams.frequency = 0.015f;
        m_State.perlinParams.amplitude = 1.2f;
        m_State.perlinParams.octaves = 5;
        m_State.perlinParams.lacunarity = 2.5f;
        m_State.perlinParams.persistence = 0.4f;
        m_State.heightScale = 100.0f;
        m_NeedsRegeneration = true;
    }

    ImGui::End();
}

void TerrainEditor::RenderStatsPanel() {
    ImGui::Begin("Statistics", &m_State.showStats);

    ImGui::Text("Terrain Info");
    ImGui::Separator();

    if (m_CurrentHeightfield) {
        ImGui::Text("Resolution: %ux%u", m_CurrentHeightfield->GetWidth(), m_CurrentHeightfield->GetHeight());
    }

    if (m_CurrentMesh) {
        ImGui::Text("Vertices: %u", m_CurrentMesh->GetVertexCount());
        ImGui::Text("Triangles: %u", m_CurrentMesh->GetTriangleCount());
    }

    ImGui::Spacing();
    ImGui::Text("Generation Time: %.2f ms", m_GenerationTime * 1000.0f);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("Camera");
    ImGui::Text("Position: %.1f, %.1f, %.1f",
        m_Camera->GetPosition().x,
        m_Camera->GetPosition().y,
        m_Camera->GetPosition().z);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("Controls");
    ImGui::BulletText("Right Mouse: Orbit");
    ImGui::BulletText("Middle Mouse: Pan");
    ImGui::BulletText("Scroll: Zoom");

    ImGui::End();
}

void TerrainEditor::RenderExportPanel() {
    ImGui::Begin("Export");

    ImGui::Text("Export Options");
    ImGui::Separator();

    ImGui::InputText("Filename", m_State.exportPath, sizeof(m_State.exportPath));

    ImGui::Spacing();

    ImGui::Text("Heightmap Export");
    ImGui::Checkbox("16-bit PNG", &m_State.export16BitPNG);

    if (ImGui::Button("Export PNG", ImVec2(-1, 0))) {
        ExportHeightmap();
    }

    if (ImGui::Button("Export RAW", ImVec2(-1, 0))) {
        if (m_CurrentHeightfield) {
            String filepath = String(m_State.exportPath) + ".raw";
            if (m_Generator->ExportRAW(*m_CurrentHeightfield, filepath)) {
                LOG_INFO("Exported RAW to: %s", filepath.c_str());
            }
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("Mesh Export");

    if (ImGui::Button("Export OBJ", ImVec2(-1, 0))) {
        ExportMesh();
    }

    if (ImGui::Button("Export FBX (TODO)", ImVec2(-1, 0))) {
        if (m_CurrentMesh) {
            String filepath = String(m_State.exportPath) + ".fbx";
            m_CurrentMesh->ExportFBX(filepath);
        }
    }

    ImGui::End();
}

void TerrainEditor::GenerateTerrain() {
    m_IsGenerating = true;

    auto startTime = std::chrono::high_resolution_clock::now();

    LOG_INFO("Generating terrain...");

    // Generate heightfield based on mode
    if (m_State.useNodeGraph) {
        // Execute node graph
        if (m_NodeGraphEditor->ExecuteGraph()) {
            m_CurrentHeightfield = m_NodeGraphEditor->GetResult();
        } else {
            LOG_ERROR("Failed to execute node graph");
            m_IsGenerating = false;
            return;
        }
    } else {
        // Use simple Perlin generation
        m_CurrentHeightfield = m_Generator->GeneratePerlin(
            m_State.terrainWidth,
            m_State.terrainHeight,
            m_State.perlinParams
        );
    }

    if (!m_CurrentHeightfield) {
        LOG_ERROR("Failed to generate heightfield");
        m_IsGenerating = false;
        return;
    }

    // Generate mesh
    m_CurrentMesh = MakeUnique<TerrainMesh>();
    m_CurrentMesh->GenerateFromHeightfield(*m_CurrentHeightfield, m_State.heightScale);
    m_CurrentMesh->Upload();

    auto endTime = std::chrono::high_resolution_clock::now();
    m_GenerationTime = std::chrono::duration<float>(endTime - startTime).count();

    LOG_INFO("Terrain generated in %.2f ms", m_GenerationTime * 1000.0f);

    m_IsGenerating = false;
}

void TerrainEditor::ExportHeightmap() {
    if (!m_CurrentHeightfield) {
        LOG_WARN("No heightfield to export");
        return;
    }

    String filepath = String(m_State.exportPath) + ".png";
    if (m_Generator->ExportPNG(*m_CurrentHeightfield, filepath, m_State.export16BitPNG)) {
        LOG_INFO("Exported heightmap to: %s", filepath.c_str());
    }
}

void TerrainEditor::ExportMesh() {
    if (!m_CurrentMesh) {
        LOG_WARN("No mesh to export");
        return;
    }

    String filepath = String(m_State.exportPath) + ".obj";
    if (m_CurrentMesh->ExportOBJ(filepath)) {
        LOG_INFO("Exported mesh to: %s", filepath.c_str());
    }
}

void TerrainEditor::ResetCamera() {
    m_Camera->SetTarget(glm::vec3(0.0f, 0.0f, 0.0f));
    m_Camera->SetDistance(500.0f);
    m_Camera->SetYaw(45.0f);
    m_Camera->SetPitch(30.0f);
}

void TerrainEditor::HandleViewportInput() {
    ImGuiIO& io = ImGui::GetIO();

    // Get mouse position in viewport
    ImVec2 mousePos = ImGui::GetMousePos();
    glm::vec2 currentMousePos(mousePos.x, mousePos.y);

    // Orbit camera with right mouse button
    if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
        if (m_ViewportHovered) {
            if (!m_IsDragging) {
                m_IsDragging = true;
                m_LastMousePos = currentMousePos;
            } else {
                glm::vec2 delta = currentMousePos - m_LastMousePos;
                m_Camera->Orbit(
                    delta.x * m_State.cameraSensitivity,
                    delta.y * m_State.cameraSensitivity
                );
                m_LastMousePos = currentMousePos;
            }
        }
    } else if (ImGui::IsMouseDown(ImGuiMouseButton_Middle)) {
        // Pan camera with middle mouse button
        if (m_ViewportHovered) {
            if (!m_IsDragging) {
                m_IsDragging = true;
                m_LastMousePos = currentMousePos;
            } else {
                glm::vec2 delta = currentMousePos - m_LastMousePos;
                m_Camera->Pan(
                    -delta.x * m_State.cameraSpeed,
                    delta.y * m_State.cameraSpeed
                );
                m_LastMousePos = currentMousePos;
            }
        }
    } else {
        m_IsDragging = false;
    }

    // Zoom with mouse wheel
    if (m_ViewportHovered && io.MouseWheel != 0.0f) {
        m_Camera->Zoom(-io.MouseWheel * 20.0f);
    }
}

} // namespace Terrain
