# UI/UX Design Specification

## Overview

The Terrain Engine UI is designed for professional terrain artists, providing an intuitive node-based workflow with real-time preview and comprehensive parameter control. Built with ImGui (docking branch), it offers a modern, efficient interface optimized for high-DPI displays.

---

## Design Philosophy

### Core Principles

1. **Artist-Friendly**: Intuitive workflows that don't require programming knowledge
2. **Performance-First**: 60 FPS viewport with minimal UI overhead
3. **Professional Aesthetic**: Clean, modern dark theme inspired by industry tools
4. **Contextual**: Show relevant information based on user's current task
5. **Non-Intrusive**: UI elements don't block the creative process

### Target Users

- **Terrain Artists**: Primary users creating landscapes for games/films
- **Technical Artists**: Advanced users leveraging scripting/custom nodes
- **Game Developers**: Integration with game engines
- **Hobbyists**: Accessible to beginners with guided workflows

---

## Layout Design

### Main Window Layout

```
┌─────────────────────────────────────────────────────────────────────┐
│ Menu Bar [File] [Edit] [View] [Tools] [Help]                       │
├─────────────────────────────────────────────────────────────────────┤
│ Toolbar [Save] [Undo] [Redo] [▶ Execute] [Resolution: 4K▼]        │
├───────────┬─────────────────────────────────────┬───────────────────┤
│           │                                     │                   │
│  Node     │       3D Viewport                   │   Property        │
│  Library  │       (Real-time Preview)           │   Inspector       │
│           │                                     │                   │
│  [Search] │                                     │   [Node Name]     │
│           │                                     │                   │
│  Generator│                                     │   ┌─────────────┐ │
│   • Perlin│                                     │   │ Parameters  │ │
│   • Voronoi                                     │   │             │ │
│           │                                     │   │ Frequency:  │ │
│  Modifier │                                     │   │ [====|====] │ │
│   • Blur  │                                     │   │             │ │
│   • Combine                                     │   │ Amplitude:  │ │
│           │                                     │   │ [===|=====] │ │
│  Erosion  │                                     │   └─────────────┘ │
│   • Hydraulic                                   │                   │
│   • Thermal│                                     │   Performance:    │
│           │                                     │   GPU: 2.3 ms     │
│           │                                     │   FPS: 60         │
├───────────┴─────────────────────────────────────┴───────────────────┤
│                                                                      │
│              Node Graph Editor                                       │
│                                                                      │
│     ┌─────────┐      ┌──────────┐      ┌─────────┐                │
│     │ Perlin  │─────▶│ Erosion  │─────▶│ Output  │                │
│     │ Noise   │      │          │      │         │                │
│     └─────────┘      └──────────┘      └─────────┘                │
│                                                                      │
├──────────────────────────────────────────────────────────────────────┤
│ Status Bar: Ready | Last operation: 45ms | VRAM: 1.2GB / 8GB       │
└──────────────────────────────────────────────────────────────────────┘
```

### Panel Descriptions

#### 1. Node Library (Left Panel)
- **Size**: 250px width (resizable)
- **Content**: Categorized list of available nodes
- **Features**:
  - Search/filter functionality
  - Drag-and-drop to graph editor
  - Node preview thumbnails
  - Favorite/recent nodes section

#### 2. 3D Viewport (Top Center)
- **Size**: Flexible, minimum 800x600
- **Content**: Real-time 3D preview of terrain
- **Features**:
  - Orbit camera controls
  - PBR shading with skybox
  - Wireframe overlay toggle
  - LOD visualization
  - Export preview snapshot

#### 3. Property Inspector (Right Panel)
- **Size**: 300px width (resizable)
- **Content**: Selected node parameters
- **Features**:
  - Auto-generated UI from node metadata
  - Real-time parameter adjustment
  - Preset save/load
  - Performance metrics
  - Node documentation

#### 4. Node Graph Editor (Bottom Panel)
- **Size**: Flexible, minimum 600px height
- **Content**: Visual node graph
- **Features**:
  - Bezier curve connections
  - Grid snapping
  - Minimap
  - Multi-select
  - Copy/paste
  - Node grouping/frames

---

## Visual Design

### Color Scheme (Dark Theme)

```cpp
// Base colors
const ImVec4 ColorBackground       = ImVec4(0.12f, 0.12f, 0.13f, 1.0f);
const ImVec4 ColorBackgroundDark   = ImVec4(0.08f, 0.08f, 0.09f, 1.0f);
const ImVec4 ColorPanel            = ImVec4(0.15f, 0.15f, 0.16f, 1.0f);
const ImVec4 ColorBorder           = ImVec4(0.25f, 0.25f, 0.27f, 1.0f);

// Accent colors
const ImVec4 ColorAccent           = ImVec4(0.26f, 0.59f, 0.98f, 1.0f); // Blue
const ImVec4 ColorAccentHover      = ImVec4(0.36f, 0.69f, 1.00f, 1.0f);
const ImVec4 ColorAccentActive     = ImVec4(0.16f, 0.49f, 0.88f, 1.0f);

// Text colors
const ImVec4 ColorText             = ImVec4(0.90f, 0.90f, 0.90f, 1.0f);
const ImVec4 ColorTextDim          = ImVec4(0.60f, 0.60f, 0.60f, 1.0f);
const ImVec4 ColorTextDisabled     = ImVec4(0.40f, 0.40f, 0.40f, 1.0f);

// Node colors by category
const ImVec4 ColorNodeGenerator    = ImVec4(0.20f, 0.80f, 0.40f, 1.0f); // Green
const ImVec4 ColorNodeModifier     = ImVec4(0.40f, 0.60f, 0.90f, 1.0f); // Blue
const ImVec4 ColorNodeErosion      = ImVec4(0.80f, 0.50f, 0.20f, 1.0f); // Orange
const ImVec4 ColorNodeMask         = ImVec4(0.70f, 0.40f, 0.80f, 1.0f); // Purple
const ImVec4 ColorNodeTexture      = ImVec4(0.90f, 0.70f, 0.30f, 1.0f); // Yellow
const ImVec4 ColorNodeIO           = ImVec4(0.60f, 0.60f, 0.60f, 1.0f); // Gray
```

### Typography

```cpp
// Font sizes (scaled for DPI)
const float FontSizeNormal   = 16.0f;
const float FontSizeSmall    = 14.0f;
const float FontSizeLarge    = 18.0f;
const float FontSizeHeading  = 20.0f;

// Font families
const char* FontMain   = "Roboto-Regular.ttf";
const char* FontBold   = "Roboto-Bold.ttf";
const char* FontMono   = "RobotoMono-Regular.ttf"; // For numbers

// Load fonts with DPI scaling
ImGuiIO& io = ImGui::GetIO();
float dpiScale = GetDPIScale();

io.Fonts->AddFontFromFileTTF("fonts/Roboto-Regular.ttf", FontSizeNormal * dpiScale);
io.Fonts->AddFontFromFileTTF("fonts/Roboto-Bold.ttf", FontSizeLarge * dpiScale);
io.Fonts->AddFontFromFileTTF("fonts/RobotoMono-Regular.ttf", FontSizeSmall * dpiScale);
```

### Icons

Use **IconsFontAwesome5** for consistent iconography:

```cpp
#include "IconsFontAwesome5.h"

// Common icons
const char* IconSave       = ICON_FA_SAVE;
const char* IconOpen       = ICON_FA_FOLDER_OPEN;
const char* IconUndo       = ICON_FA_UNDO;
const char* IconRedo       = ICON_FA_REDO;
const char* IconPlay       = ICON_FA_PLAY;
const char* IconStop       = ICON_FA_STOP;
const char* IconSettings   = ICON_FA_COG;
const char* IconCamera     = ICON_FA_CAMERA;
const char* IconEye        = ICON_FA_EYE;
const char* IconTrash      = ICON_FA_TRASH;
```

---

## Window Implementations

### 1. Menu Bar

```cpp
void DrawMenuBar() {
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem(ICON_FA_FILE " New", "Ctrl+N")) {
                NewProject();
            }
            if (ImGui::MenuItem(ICON_FA_FOLDER_OPEN " Open", "Ctrl+O")) {
                OpenProject();
            }
            if (ImGui::MenuItem(ICON_FA_SAVE " Save", "Ctrl+S")) {
                SaveProject();
            }
            if (ImGui::MenuItem(ICON_FA_SAVE " Save As", "Ctrl+Shift+S")) {
                SaveProjectAs();
            }

            ImGui::Separator();

            if (ImGui::BeginMenu(ICON_FA_FILE_IMPORT " Import")) {
                if (ImGui::MenuItem("GeoTIFF/DEM")) ImportHeightfield();
                if (ImGui::MenuItem("RAW 16-bit")) ImportRAW16();
                if (ImGui::MenuItem("EXR 32-bit")) ImportEXR();
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu(ICON_FA_FILE_EXPORT " Export")) {
                if (ImGui::MenuItem("PNG (8-bit)")) ExportPNG();
                if (ImGui::MenuItem("RAW (16-bit)")) ExportRAW16();
                if (ImGui::MenuItem("EXR (32-bit)")) ExportEXR();
                if (ImGui::MenuItem("Unreal Landscape")) ExportUnreal();
                if (ImGui::MenuItem("Unity Terrain")) ExportUnity();
                ImGui::EndMenu();
            }

            ImGui::Separator();

            if (ImGui::MenuItem(ICON_FA_TIMES " Exit", "Alt+F4")) {
                RequestExit();
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem(ICON_FA_UNDO " Undo", "Ctrl+Z", false, CanUndo())) {
                Undo();
            }
            if (ImGui::MenuItem(ICON_FA_REDO " Redo", "Ctrl+Y", false, CanRedo())) {
                Redo();
            }

            ImGui::Separator();

            if (ImGui::MenuItem(ICON_FA_CUT " Cut", "Ctrl+X")) Cut();
            if (ImGui::MenuItem(ICON_FA_COPY " Copy", "Ctrl+C")) Copy();
            if (ImGui::MenuItem(ICON_FA_PASTE " Paste", "Ctrl+V")) Paste();
            if (ImGui::MenuItem(ICON_FA_TRASH " Delete", "Del")) Delete();

            ImGui::Separator();

            if (ImGui::MenuItem(ICON_FA_COG " Preferences")) {
                ShowPreferences();
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Node Library", nullptr, &showNodeLibrary);
            ImGui::MenuItem("Property Inspector", nullptr, &showPropertyInspector);
            ImGui::MenuItem("3D Viewport", nullptr, &show3DViewport);
            ImGui::MenuItem("Node Graph", nullptr, &showNodeGraph);

            ImGui::Separator();

            ImGui::MenuItem("Performance Metrics", nullptr, &showPerformance);
            ImGui::MenuItem("Console Log", nullptr, &showConsole);

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Tools")) {
            if (ImGui::MenuItem(ICON_FA_PLAY " Execute Graph", "F5")) {
                ExecuteGraph();
            }
            if (ImGui::MenuItem(ICON_FA_IMAGE " Render Final", "F7")) {
                RenderFinal();
            }

            ImGui::Separator();

            if (ImGui::MenuItem(ICON_FA_CHART_BAR " Profiler")) {
                ShowProfiler();
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem(ICON_FA_BOOK " Documentation", "F1")) {
                OpenDocumentation();
            }
            if (ImGui::MenuItem(ICON_FA_GRADUATION_CAP " Tutorials")) {
                OpenTutorials();
            }

            ImGui::Separator();

            if (ImGui::MenuItem(ICON_FA_INFO_CIRCLE " About")) {
                ShowAbout();
            }

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }
}
```

### 2. Node Graph Editor

```cpp
void DrawNodeGraph() {
    ImNodes::BeginNodeEditor();

    // Draw nodes
    for (auto& [id, node] : graph.nodes) {
        ImNodes::BeginNode(id);

        // Node title bar
        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted(node->GetDisplayName());
        ImNodes::EndNodeTitleBar();

        // Input pins
        for (const auto& pin : node->inputPins) {
            ImNodes::BeginInputAttribute(pin.index);
            ImGui::TextUnformatted(pin.name.c_str());
            ImNodes::EndInputAttribute();
        }

        // Node content (small preview or info)
        if (node->HasPreview()) {
            ImGui::Image(node->GetPreviewTexture(), ImVec2(100, 100));
        }

        // Output pins
        for (const auto& pin : node->outputPins) {
            ImNodes::BeginOutputAttribute(pin.index);
            ImGui::Indent(100);
            ImGui::TextUnformatted(pin.name.c_str());
            ImNodes::EndOutputAttribute();
        }

        ImNodes::EndNode();
    }

    // Draw connections
    for (const auto& conn : graph.connections) {
        ImNodes::Link(conn.id, conn.sourcePinId, conn.targetPinId);
    }

    ImNodes::EndNodeEditor();

    // Handle interactions
    int startPinId, endPinId;
    if (ImNodes::IsLinkCreated(&startPinId, &endPinId)) {
        CreateConnection(startPinId, endPinId);
    }

    int hoveredNode;
    if (ImNodes::IsNodeHovered(&hoveredNode)) {
        // Show tooltip
        ImGui::BeginTooltip();
        ImGui::Text("Node: %s", GetNode(hoveredNode)->GetDescription());
        ImGui::EndTooltip();
    }

    // Right-click context menu
    if (ImGui::BeginPopupContextWindow("GraphContextMenu")) {
        if (ImGui::MenuItem("Add Node")) {
            ShowAddNodeMenu();
        }
        if (ImGui::MenuItem("Paste")) {
            PasteNodes();
        }
        ImGui::EndPopup();
    }

    // Selection handling
    const int numSelectedNodes = ImNodes::NumSelectedNodes();
    if (numSelectedNodes > 0 && ImGui::IsKeyPressed(ImGuiKey_Delete)) {
        DeleteSelectedNodes();
    }
}
```

### 3. Property Inspector

```cpp
void DrawPropertyInspector(Node* selectedNode) {
    if (!selectedNode) {
        ImGui::Text("No node selected");
        return;
    }

    ImGui::PushFont(fontBold);
    ImGui::Text("%s", selectedNode->GetDisplayName());
    ImGui::PopFont();

    ImGui::TextDisabled("%s", selectedNode->GetDescription());

    ImGui::Separator();

    ImGui::Text("Parameters");

    // Auto-generate UI for parameters
    for (auto& param : selectedNode->parameters.GetAll()) {
        ImGui::PushID(param.name.c_str());

        switch (param.type) {
            case ParameterType::Float: {
                float* value = static_cast<float*>(param.dataPtr);
                float min = std::any_cast<float>(param.minValue);
                float max = std::any_cast<float>(param.maxValue);

                if (ImGui::SliderFloat(param.name.c_str(), value, min, max)) {
                    selectedNode->InvalidateCache();
                    graph.MarkDirty(selectedNode->id);
                }
                break;
            }

            case ParameterType::Int: {
                int* value = static_cast<int*>(param.dataPtr);
                int min = std::any_cast<int>(param.minValue);
                int max = std::any_cast<int>(param.maxValue);

                if (ImGui::SliderInt(param.name.c_str(), value, min, max)) {
                    selectedNode->InvalidateCache();
                    graph.MarkDirty(selectedNode->id);
                }
                break;
            }

            case ParameterType::Bool: {
                bool* value = static_cast<bool*>(param.dataPtr);

                if (ImGui::Checkbox(param.name.c_str(), value)) {
                    selectedNode->InvalidateCache();
                    graph.MarkDirty(selectedNode->id);
                }
                break;
            }

            case ParameterType::Color: {
                ImVec4* value = static_cast<ImVec4*>(param.dataPtr);

                if (ImGui::ColorEdit3(param.name.c_str(), (float*)value)) {
                    selectedNode->InvalidateCache();
                    graph.MarkDirty(selectedNode->id);
                }
                break;
            }
        }

        ImGui::PopID();
    }

    ImGui::Separator();

    // Preset management
    if (ImGui::Button(ICON_FA_SAVE " Save Preset")) {
        SaveNodePreset(selectedNode);
    }

    ImGui::SameLine();

    if (ImGui::Button(ICON_FA_FOLDER_OPEN " Load Preset")) {
        LoadNodePreset(selectedNode);
    }

    ImGui::Separator();

    // Performance info
    ImGui::Text("Performance");
    ImGui::Text("Last execution: %.2f ms", selectedNode->lastExecutionTimeMs);
    ImGui::Text("Cache: %s", selectedNode->IsCacheValid() ? "Valid" : "Dirty");
}
```

### 4. 3D Viewport

```cpp
void Draw3DViewport() {
    ImGui::BeginChild("Viewport", ImGui::GetContentRegionAvail(), false,
                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    // Get viewport size
    ImVec2 viewportSize = ImGui::GetContentRegionAvail();

    // Resize framebuffer if needed
    if (viewportSize.x != viewportWidth || viewportSize.y != viewportHeight) {
        ResizeViewport(viewportSize.x, viewportSize.y);
    }

    // Render terrain
    RenderTerrain(viewportFBO, camera);

    // Display as ImGui image
    ImGui::Image((ImTextureID)(intptr_t)viewportTexture,
                 viewportSize, ImVec2(0, 1), ImVec2(1, 0));

    // Handle camera controls
    if (ImGui::IsItemHovered()) {
        // Orbit: Right mouse drag
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Right)) {
            ImVec2 delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
            camera.Orbit(delta.x * 0.01f, delta.y * 0.01f);
            ImGui::ResetMouseDragDelta(ImGuiMouseButton_Right);
        }

        // Pan: Middle mouse drag
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle)) {
            ImVec2 delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Middle);
            camera.Pan(delta.x * 0.01f, delta.y * 0.01f);
            ImGui::ResetMouseDragDelta(ImGuiMouseButton_Middle);
        }

        // Zoom: Scroll wheel
        float wheel = ImGui::GetIO().MouseWheel;
        if (wheel != 0.0f) {
            camera.Zoom(wheel * 0.1f);
        }
    }

    // Overlay controls
    ImGui::SetCursorPos(ImVec2(10, 10));
    ImGui::BeginGroup();

    if (ImGui::Button(ICON_FA_HOME)) {
        camera.ResetToHome();
    }

    ImGui::SameLine();

    if (ImGui::Button(wireframeMode ? ICON_FA_SQUARE : ICON_FA_BORDER_ALL)) {
        wireframeMode = !wireframeMode;
    }

    ImGui::SameLine();

    if (ImGui::Button(ICON_FA_CAMERA)) {
        SaveViewportSnapshot();
    }

    ImGui::EndGroup();

    // Performance overlay
    ImGui::SetCursorPos(ImVec2(10, viewportSize.y - 60));
    ImGui::BeginGroup();
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Text("GPU: %.2f ms", gpuTimeMs);
    ImGui::EndGroup();

    ImGui::EndChild();
}
```

### 5. Node Library

```cpp
void DrawNodeLibrary() {
    ImGui::BeginChild("NodeLibrary", ImVec2(250, 0), true);

    ImGui::Text(ICON_FA_SEARCH " Node Library");

    // Search filter
    static char searchBuffer[256] = "";
    ImGui::InputTextWithHint("##search", "Search nodes...", searchBuffer, sizeof(searchBuffer));

    ImGui::Separator();

    // Category tree
    const char* categories[] = {
        "Generator", "Modifier", "Filter", "Erosion", "Mask", "Texture", "Combiner", "I/O"
    };

    for (const char* category : categories) {
        if (ImGui::TreeNode(category)) {
            auto nodes = nodeRegistry.GetNodesByCategory(category);

            for (const auto& nodeInfo : nodes) {
                // Filter by search
                if (searchBuffer[0] != '\0' &&
                    !StrContainsInsensitive(nodeInfo.displayName, searchBuffer)) {
                    continue;
                }

                // Selectable node
                if (ImGui::Selectable(nodeInfo.displayName.c_str())) {
                    // Add node to graph at center
                    auto node = nodeRegistry.Create(nodeInfo.typeName.c_str());
                    node->editorPosition = GetGraphEditorCenter();
                    graph.AddNode(std::move(node));
                }

                // Tooltip
                if (ImGui::IsItemHovered()) {
                    ImGui::BeginTooltip();
                    ImGui::Text("%s", nodeInfo.description.c_str());
                    ImGui::EndTooltip();
                }
            }

            ImGui::TreePop();
        }
    }

    ImGui::EndChild();
}
```

---

## Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| **Ctrl+N** | New project |
| **Ctrl+O** | Open project |
| **Ctrl+S** | Save project |
| **Ctrl+Z** | Undo |
| **Ctrl+Y** | Redo |
| **Ctrl+C** | Copy selected nodes |
| **Ctrl+V** | Paste nodes |
| **Ctrl+X** | Cut selected nodes |
| **Del** | Delete selected nodes |
| **F5** | Execute graph |
| **F7** | Render final output |
| **F1** | Help |
| **Space** | Open node search (in graph editor) |
| **Ctrl+D** | Duplicate selected nodes |
| **Ctrl+G** | Group selected nodes |
| **Tab** | Cycle selected node |
| **Ctrl+F** | Find node |

---

## Responsive Behavior

### Window Resizing

- All panels respect minimum sizes
- Graph editor and viewport are primary resize targets
- Side panels collapse to icons at narrow widths
- Automatic layout switching for ultra-wide displays

### High-DPI Support

```cpp
void InitializeDPIScaling() {
    float dpiScale = GetSystemDPIScale();

    // Scale fonts
    ImGui::GetIO().FontGlobalScale = dpiScale;

    // Scale UI elements
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(dpiScale);

    // Scale node editor
    ImNodes::GetStyle().GridSpacing *= dpiScale;
    ImNodes::GetStyle().NodeCornerRounding *= dpiScale;
    ImNodes::GetStyle().NodePadding *= dpiScale;
}
```

---

## Accessibility Features

### Visual Accessibility

- High-contrast mode option
- Colorblind-friendly palette alternatives
- Adjustable font sizes
- Customizable UI scaling

### Workflow Accessibility

- Keyboard-only navigation support
- Search/quick access for all functions
- Undo/redo for all operations
- Autosave and crash recovery

---

## Performance Optimizations

### UI Rendering

```cpp
// Only update UI when needed
if (ImGui::IsWindowFocused() || isDirty) {
    DrawPropertyInspector(selectedNode);
}

// Lazy-load node previews
if (ImGui::IsItemVisible()) {
    ImGui::Image(node->GetOrGeneratePreview());
}

// Cull offscreen nodes in graph
if (IsNodeInViewport(node)) {
    DrawNode(node);
}
```

### Viewport Optimization

- Render at lower resolution during camera movement
- Progressive rendering for high-quality final previews
- Frustum culling for large terrains
- LOD system for distant terrain tiles

---

## User Preferences

### Settings Dialog

```cpp
void DrawPreferences() {
    ImGui::Begin("Preferences", &showPreferences);

    if (ImGui::BeginTabBar("PreferencesTabs")) {
        if (ImGui::BeginTabItem("General")) {
            ImGui::Checkbox("Auto-save enabled", &settings.autoSaveEnabled);
            ImGui::SliderInt("Auto-save interval (minutes)", &settings.autoSaveIntervalMin, 1, 30);

            ImGui::Checkbox("Show splash screen", &settings.showSplashScreen);

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Performance")) {
            ImGui::SliderInt("Preview resolution", &settings.previewResolution, 256, 2048);
            ImGui::SliderInt("Target FPS", &settings.targetFPS, 30, 120);

            ImGui::SliderInt("Max cache size (MB)", &settings.maxCacheSizeMB, 512, 8192);

            ImGui::Checkbox("Enable GPU profiling", &settings.enableGPUProfiling);

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Appearance")) {
            if (ImGui::Combo("Theme", &settings.themeIndex, "Dark\0Light\0Custom\0")) {
                ApplyTheme(settings.themeIndex);
            }

            ImGui::SliderFloat("UI Scale", &settings.uiScale, 0.5f, 2.0f);

            ImGui::ColorEdit3("Accent Color", (float*)&settings.accentColor);

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Hotkeys")) {
            DrawHotkeyEditor();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    if (ImGui::Button("OK")) {
        settings.Save();
        showPreferences = false;
    }

    ImGui::SameLine();

    if (ImGui::Button("Cancel")) {
        settings.Load(); // Revert
        showPreferences = false;
    }

    ImGui::End();
}
```

---

## Tooltips and Help

### Contextual Tooltips

```cpp
void DrawParameterWithTooltip(const Parameter& param) {
    ImGui::SliderFloat(param.name.c_str(), param.valuePtr, param.min, param.max);

    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(param.description.c_str());
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}
```

### Integrated Help System

- F1 opens context-sensitive help
- Built-in tutorial system
- Example projects/presets
- Video tutorial links

---

## Export/Render Dialog

```cpp
void DrawExportDialog() {
    ImGui::Begin("Export Terrain", &showExportDialog);

    ImGui::Text("Resolution");
    static int resolutionIndex = 2;
    const char* resolutions[] = {"1024x1024", "2048x2048", "4096x4096", "8192x8192", "16384x16384"};
    ImGui::Combo("##resolution", &resolutionIndex, resolutions, IM_ARRAYSIZE(resolutions));

    ImGui::Separator();

    ImGui::Text("Format");
    static int formatIndex = 0;
    const char* formats[] = {"PNG (8-bit)", "PNG (16-bit)", "RAW (16-bit)", "EXR (32-bit)", "GeoTIFF"};
    ImGui::Combo("##format", &formatIndex, formats, IM_ARRAYSIZE(formats));

    ImGui::Separator();

    ImGui::Checkbox("Export normal map", &exportSettings.exportNormals);
    ImGui::Checkbox("Export splatmap", &exportSettings.exportSplatmap);
    ImGui::Checkbox("Export flow map", &exportSettings.exportFlowMap);
    ImGui::Checkbox("Export AO map", &exportSettings.exportAO);

    ImGui::Separator();

    if (ImGui::Button("Export", ImVec2(120, 0))) {
        StartExport(resolutionIndex, formatIndex);
        showExportDialog = false;
    }

    ImGui::SameLine();

    if (ImGui::Button("Cancel", ImVec2(120, 0))) {
        showExportDialog = false;
    }

    ImGui::End();
}
```

---

## Conclusion

This UI/UX design provides:

- ✅ Professional, artist-friendly interface
- ✅ Efficient workflow with minimal friction
- ✅ Real-time feedback and preview
- ✅ Comprehensive parameter control
- ✅ Extensible and customizable
- ✅ High-DPI and accessibility support

**Next Steps**: Implement core UI framework, integrate with node graph system.

See `ARCHITECTURE.md` for overall system design, `NODE_SYSTEM_DESIGN.md` for backend integration.
