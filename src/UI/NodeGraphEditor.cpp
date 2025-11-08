#include "NodeGraphEditor.h"
#include "Core/Logger.h"
#include <imgui.h>
#include <imnodes.h>

namespace Terrain {

NodeGraphEditor::NodeGraphEditor() {
    m_Graph = MakeUnique<NodeGraph>();
    m_Serializer = MakeUnique<GraphSerializer>();
    m_RecentFiles = MakeUnique<RecentFilesManager>();
    m_Theme = MakeUnique<UITheme>();
    m_StatusBar = MakeUnique<StatusBar>();
    m_Notifications = MakeUnique<NotificationSystem>();

    // Load recent files
    m_RecentFiles->LoadFromFile("recent_files.txt");
}

NodeGraphEditor::~NodeGraphEditor() {
    // Save recent files
    m_RecentFiles->SaveToFile("recent_files.txt");

    ImNodes::DestroyContext();
}

bool NodeGraphEditor::Initialize() {
    if (!m_Graph) {
        return false;
    }

    // Apply UI theme
    m_Theme->Apply(UIThemePreset::Dark);

    // Initialize ImNodes
    ImNodes::CreateContext();

    // Create default output node
    CreateOutputNode();

    // Set initial status
    m_StatusBar->SetStatus("Ready");
    m_Notifications->NotifySuccess("Welcome", "Terrain Engine Pro initialized successfully!");

    LOG_INFO("Node graph editor initialized");
    return true;
}

void NodeGraphEditor::Render() {
    // Update status bar
    m_StatusBar->SetNodeCount(static_cast<uint32>(m_Graph->GetNodes().size()));
    if (!m_CurrentFilePath.empty()) {
        m_StatusBar->SetCurrentFile(m_CurrentFilePath);
    }

    // Set window title
    String windowTitle = "Terrain Engine Pro - ";
    if (m_CurrentFilePath.empty()) {
        windowTitle += m_UntitledGraphName;
    } else {
        size_t lastSlash = m_CurrentFilePath.find_last_of("/\\");
        windowTitle += (lastSlash != String::npos) ? m_CurrentFilePath.substr(lastSlash + 1) : m_CurrentFilePath;
    }
    if (m_GraphDirty) {
        windowTitle += " *";
    }

    ImGui::Begin(windowTitle.c_str());

    RenderMenuBar();

    // Split into canvas and properties
    ImGui::Columns(m_ShowProperties ? 2 : 1);

    // Node canvas
    RenderNodeCanvas();

    if (m_ShowProperties) {
        ImGui::NextColumn();
        RenderNodeProperties();
    }

    ImGui::Columns(1);

    ImGui::End();

    // Render status bar
    m_StatusBar->Render();

    // Render notifications
    m_Notifications->Render();
}

void NodeGraphEditor::RenderMenuBar() {
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Graph", "Ctrl+N")) {
                NewGraph();
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Create a new empty terrain graph");
            }
            ImGui::Separator();

            if (ImGui::MenuItem("Open...", "Ctrl+O")) {
                LoadGraphWithDialog();
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Open an existing terrain graph from a file");
            }

            // Recent files submenu
            if (ImGui::BeginMenu("Open Recent")) {
                const auto& recentFiles = m_RecentFiles->GetRecentFiles();
                if (recentFiles.empty()) {
                    ImGui::MenuItem("(No recent files)", nullptr, false, false);
                } else {
                    for (const auto& filepath : recentFiles) {
                        // Extract filename from path
                        size_t lastSlash = filepath.find_last_of("/\\");
                        String filename = (lastSlash != String::npos) ? filepath.substr(lastSlash + 1) : filepath;

                        if (ImGui::MenuItem(filename.c_str())) {
                            LoadGraph(filepath);
                        }
                        if (ImGui::IsItemHovered()) {
                            ImGui::SetTooltip("%s", filepath.c_str());
                        }
                    }

                    ImGui::Separator();
                    if (ImGui::MenuItem("Clear Recent Files")) {
                        m_RecentFiles->ClearRecentFiles();
                    }
                }
                ImGui::EndMenu();
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Save", "Ctrl+S")) {
                if (m_CurrentFilePath.empty()) {
                    SaveGraphAs();
                } else {
                    SaveGraph(m_CurrentFilePath);
                }
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Save the current terrain graph");
            }

            if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S")) {
                SaveGraphAs();
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Save the terrain graph to a new file");
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Add Node")) {
            if (ImGui::BeginMenu("Generators")) {
                if (ImGui::MenuItem("Perlin Noise")) CreatePerlinNode();
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Classic Perlin noise - smooth, natural terrain");

                if (ImGui::MenuItem("Voronoi")) CreateVoronoiNode();
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Cellular/Voronoi noise - cracked, angular patterns");

                if (ImGui::MenuItem("Ridged Noise")) CreateRidgedNode();
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Ridged multifractal noise - sharp mountain ridges");

                if (ImGui::MenuItem("Gradient")) CreateNodeOfType("Gradient");
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Linear gradient from one edge to another");

                if (ImGui::MenuItem("Constant")) CreateNodeOfType("Constant");
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Uniform height value across terrain");

                if (ImGui::MenuItem("White Noise")) CreateNodeOfType("WhiteNoise");
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Random noise - completely random values");

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Modifiers")) {
                if (ImGui::MenuItem("Terrace")) CreateNodeOfType("Terrace");
                if (ImGui::MenuItem("Clamp")) CreateNodeOfType("Clamp");
                if (ImGui::MenuItem("Invert")) CreateNodeOfType("Invert");
                if (ImGui::MenuItem("Scale")) CreateNodeOfType("Scale");
                if (ImGui::MenuItem("Curve")) CreateNodeOfType("Curve");
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Filters")) {
                if (ImGui::MenuItem("Smooth")) CreateNodeOfType("Smooth");
                if (ImGui::MenuItem("Sharpen")) CreateNodeOfType("Sharpen");
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Erosion")) {
                if (ImGui::MenuItem("Hydraulic Erosion")) CreateNodeOfType("HydraulicErosion");
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Simulate water erosion - carves valleys and rivers");

                if (ImGui::MenuItem("Thermal Erosion")) CreateNodeOfType("ThermalErosion");
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Simulate weathering - smooths steep slopes");

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Textures")) {
                if (ImGui::MenuItem("Normal Map")) CreateNodeOfType("NormalMap");
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Generate normal map for PBR rendering");

                if (ImGui::MenuItem("Ambient Occlusion")) CreateNodeOfType("AmbientOcclusion");
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Generate AO map for contact shadows");

                if (ImGui::MenuItem("Splatmap")) CreateNodeOfType("Splatmap");
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Generate material blend map (4 layers)");

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Mesh Export")) {
                if (ImGui::MenuItem("OBJ Export")) CreateNodeOfType("OBJExport");
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Export terrain as OBJ mesh file");

                if (ImGui::MenuItem("FBX Export")) CreateNodeOfType("FBXExport");
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Export terrain as FBX mesh file");

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Combiners")) {
                if (ImGui::MenuItem("Add")) CreateNodeOfType("Add");
                if (ImGui::MenuItem("Multiply")) CreateNodeOfType("Multiply");
                if (ImGui::MenuItem("Blend")) CreateNodeOfType("Blend");
                if (ImGui::MenuItem("Max")) CreateNodeOfType("Max");
                if (ImGui::MenuItem("Min")) CreateNodeOfType("Min");
                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Presets")) {
            if (ImGui::MenuItem("Alps")) {
                MountainPresets::CreatePreset(m_Graph.get(), MountainPreset::Alps);
                m_GraphDirty = true;
                m_Notifications->NotifyInfo("Preset Loaded", "Alps mountain terrain");
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Sharp peaks, glacial features - European Alps");

            if (ImGui::MenuItem("Appalachians")) {
                MountainPresets::CreatePreset(m_Graph.get(), MountainPreset::Appalachians);
                m_GraphDirty = true;
                m_Notifications->NotifyInfo("Preset Loaded", "Appalachian mountain terrain");
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Rolling ridges, weathered - Eastern USA");

            if (ImGui::MenuItem("Himalayas")) {
                MountainPresets::CreatePreset(m_Graph.get(), MountainPreset::Himalayas);
                m_GraphDirty = true;
                m_Notifications->NotifyInfo("Preset Loaded", "Himalayan mountain terrain");
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Extreme peaks, sharp ridges - World's highest");

            if (ImGui::MenuItem("Rocky Mountains")) {
                MountainPresets::CreatePreset(m_Graph.get(), MountainPreset::RockyMountains);
                m_GraphDirty = true;
                m_Notifications->NotifyInfo("Preset Loaded", "Rocky Mountain terrain");
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Mixed terrain, varied features - Western North America");

            if (ImGui::MenuItem("Andes")) {
                MountainPresets::CreatePreset(m_Graph.get(), MountainPreset::Andes);
                m_GraphDirty = true;
                m_Notifications->NotifyInfo("Preset Loaded", "Andes mountain terrain");
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Long ridges, volcanic features - South America");

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Properties", nullptr, &m_ShowProperties);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Toggle node properties panel");

            ImGui::MenuItem("Auto Execute", nullptr, &m_AutoExecute);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Automatically execute graph when nodes change");

            ImGui::EndMenu();
        }

        if (ImGui::Button("Execute")) {
            ExecuteGraph();
        }

        ImGui::EndMenuBar();
    }
}

void NodeGraphEditor::RenderNodeCanvas() {
    ImNodes::BeginNodeEditor();

    // Render all nodes
    for (const auto& [id, node] : m_Graph->GetNodes()) {
        RenderNode(node.get());
    }

    // Render connections
    for (const auto& [id, node] : m_Graph->GetNodes()) {
        for (const auto& input : node->GetInputs()) {
            if (input->connectedPin) {
                ImNodes::Link(
                    static_cast<int>(input->id * 1000), // Unique link ID
                    static_cast<int>(input->connectedPin->id),
                    static_cast<int>(input->id)
                );
            }
        }
    }

    ImNodes::EndNodeEditor();

    // Handle new connections
    int startPin, endPin;
    if (ImNodes::IsLinkCreated(&startPin, &endPin)) {
        m_Graph->CreateConnection(static_cast<uint32>(startPin), static_cast<uint32>(endPin));
        m_GraphDirty = true;

        if (m_AutoExecute) {
            ExecuteGraph();
        }
    }

    // Handle deleted connections
    int linkId;
    if (ImNodes::IsLinkDestroyed(&linkId)) {
        // Find and delete the connection
        uint32 inputPinId = linkId / 1000;
        m_Graph->DeleteConnection(inputPinId);
        m_GraphDirty = true;

        if (m_AutoExecute) {
            ExecuteGraph();
        }
    }

    // Handle node selection
    int selectedNodeId;
    if (ImNodes::IsNodeSelected(&selectedNodeId)) {
        m_SelectedNode = m_Graph->GetNode(static_cast<uint32>(selectedNodeId));
    }

    // Right-click context menu
    if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(1)) {
        ImGui::OpenPopup("node_context_menu");
    }

    if (ImGui::BeginPopup("node_context_menu")) {
        if (ImGui::MenuItem("Add Perlin Noise")) CreatePerlinNode();
        if (ImGui::MenuItem("Add Voronoi")) CreateVoronoiNode();
        if (ImGui::MenuItem("Add Output")) CreateOutputNode();
        ImGui::EndPopup();
    }
}

void NodeGraphEditor::RenderNode(Node* node) {
    ImNodes::BeginNode(static_cast<int>(node->GetID()));

    // Title bar
    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted(node->GetName().c_str());
    ImNodes::EndNodeTitleBar();

    // Input pins
    RenderNodeInputs(node);

    // Output pins
    RenderNodeOutputs(node);

    ImNodes::EndNode();
}

void NodeGraphEditor::RenderNodeInputs(Node* node) {
    for (const auto& input : node->GetInputs()) {
        ImNodes::BeginInputAttribute(static_cast<int>(input->id));
        ImGui::Text("%s", input->name.c_str());
        ImNodes::EndInputAttribute();
    }
}

void NodeGraphEditor::RenderNodeOutputs(Node* node) {
    for (const auto& output : node->GetOutputs()) {
        ImNodes::BeginOutputAttribute(static_cast<int>(output->id));
        ImGui::Indent(80);
        ImGui::Text("%s", output->name.c_str());
        ImNodes::EndOutputAttribute();
    }
}

void NodeGraphEditor::RenderNodeProperties() {
    ImGui::BeginChild("Properties");

    ImGui::Text("Node Properties");
    ImGui::Separator();

    if (m_SelectedNode) {
        ImGui::Text("Node: %s", m_SelectedNode->GetName().c_str());
        ImGui::Text("ID: %u", m_SelectedNode->GetID());

        ImGui::Spacing();

        // Type-specific properties
        if (auto* perlin = dynamic_cast<PerlinNode*>(m_SelectedNode)) {
            ImGui::Text("Perlin Noise Parameters");
            bool changed = false;
            changed |= ImGui::DragInt("Width", reinterpret_cast<int*>(&perlin->width), 1, 128, 4096);
            changed |= ImGui::DragInt("Height", reinterpret_cast<int*>(&perlin->height), 1, 128, 4096);
            changed |= ImGui::SliderFloat("Frequency", &perlin->params.frequency, 0.001f, 0.1f, "%.4f");
            changed |= ImGui::SliderFloat("Amplitude", &perlin->params.amplitude, 0.1f, 2.0f);
            changed |= ImGui::SliderInt("Octaves", &perlin->params.octaves, 1, 10);
            changed |= ImGui::SliderFloat("Lacunarity", &perlin->params.lacunarity, 1.5f, 3.0f);
            changed |= ImGui::SliderFloat("Persistence", &perlin->params.persistence, 0.1f, 0.9f);
            changed |= ImGui::DragInt("Seed", reinterpret_cast<int*>(&perlin->params.seed));

            if (changed) {
                perlin->MarkDirty();
                m_GraphDirty = true;
                if (m_AutoExecute) ExecuteGraph();
            }
        }
        else if (auto* voronoi = dynamic_cast<VoronoiNode*>(m_SelectedNode)) {
            ImGui::Text("Voronoi Parameters");
            bool changed = false;
            changed |= ImGui::DragInt("Width", reinterpret_cast<int*>(&voronoi->width), 1, 128, 4096);
            changed |= ImGui::DragInt("Height", reinterpret_cast<int*>(&voronoi->height), 1, 128, 4096);
            changed |= ImGui::SliderInt("Cell Count", &voronoi->cellCount, 5, 100);
            changed |= ImGui::SliderFloat("Amplitude", &voronoi->amplitude, 0.1f, 2.0f);
            changed |= ImGui::DragInt("Seed", reinterpret_cast<int*>(&voronoi->seed));
            changed |= ImGui::Checkbox("Invert", &voronoi->invert);

            if (changed) {
                voronoi->MarkDirty();
                m_GraphDirty = true;
                if (m_AutoExecute) ExecuteGraph();
            }
        }
        else if (auto* terrace = dynamic_cast<TerraceNode*>(m_SelectedNode)) {
            ImGui::Text("Terrace Parameters");
            bool changed = false;
            changed |= ImGui::SliderInt("Steps", &terrace->steps, 2, 20);
            changed |= ImGui::SliderFloat("Blend", &terrace->blend, 0.0f, 1.0f);

            if (changed) {
                terrace->MarkDirty();
                m_GraphDirty = true;
                if (m_AutoExecute) ExecuteGraph();
            }
        }
        else if (auto* blend = dynamic_cast<BlendNode*>(m_SelectedNode)) {
            ImGui::Text("Blend Parameters");
            bool changed = ImGui::SliderFloat("Blend", &blend->blend, 0.0f, 1.0f);

            if (changed) {
                blend->MarkDirty();
                m_GraphDirty = true;
                if (m_AutoExecute) ExecuteGraph();
            }
        }
        else if (auto* hydraulic = dynamic_cast<HydraulicErosionNode*>(m_SelectedNode)) {
            ImGui::Text("Hydraulic Erosion Parameters");
            ImGui::TextWrapped("Simulates realistic water erosion. Higher iterations = more erosion, but slower.");
            ImGui::Separator();

            bool changed = false;
            changed |= ImGui::DragInt("Droplets", reinterpret_cast<int*>(&hydraulic->params.iterations), 1000, 10000, 500000);
            changed |= ImGui::SliderFloat("Inertia", &hydraulic->params.inertia, 0.0f, 0.3f);
            changed |= ImGui::SliderFloat("Capacity", &hydraulic->params.sedimentCapacity, 1.0f, 10.0f);
            changed |= ImGui::SliderFloat("Erode Speed", &hydraulic->params.erodeSpeed, 0.1f, 1.0f);
            changed |= ImGui::SliderFloat("Deposit Speed", &hydraulic->params.depositSpeed, 0.1f, 1.0f);
            changed |= ImGui::SliderFloat("Evaporation", &hydraulic->params.evaporateSpeed, 0.0f, 0.1f);
            changed |= ImGui::SliderFloat("Gravity", &hydraulic->params.gravity, 1.0f, 10.0f);

            if (changed) {
                hydraulic->MarkDirty();
                m_GraphDirty = true;
                if (m_AutoExecute) ExecuteGraph();
            }
        }
        else if (auto* thermal = dynamic_cast<ThermalErosionNode*>(m_SelectedNode)) {
            ImGui::Text("Thermal Erosion Parameters");
            ImGui::TextWrapped("Simulates material sliding down steep slopes. Creates realistic slope angles.");
            ImGui::Separator();

            bool changed = false;
            changed |= ImGui::SliderInt("Iterations", &thermal->params.iterations, 1, 30);
            changed |= ImGui::SliderFloat("Talus Angle", &thermal->params.talusAngle, 0.3f, 1.5f);
            changed |= ImGui::SliderFloat("Strength", &thermal->params.strength, 0.1f, 1.0f);

            if (changed) {
                thermal->MarkDirty();
                m_GraphDirty = true;
                if (m_AutoExecute) ExecuteGraph();
            }
        }
    } else {
        ImGui::TextDisabled("No node selected");
    }

    ImGui::EndChild();
}

bool NodeGraphEditor::ExecuteGraph() {
    if (!m_Graph->ExecuteGraph()) {
        LOG_ERROR("Failed to execute node graph");
        return false;
    }

    m_GraphDirty = false;
    return true;
}

Unique<Heightfield> NodeGraphEditor::GetResult() {
    return m_Graph->GetResult();
}

void NodeGraphEditor::CreatePerlinNode() {
    auto* node = m_Graph->CreateNode<PerlinNode>();
    node->SetPosition(glm::vec2(100.0f, 100.0f));
    m_GraphDirty = true;
}

void NodeGraphEditor::CreateVoronoiNode() {
    auto* node = m_Graph->CreateNode<VoronoiNode>();
    node->SetPosition(glm::vec2(100.0f, 100.0f));
    m_GraphDirty = true;
}

void NodeGraphEditor::CreateRidgedNode() {
    auto* node = m_Graph->CreateNode<RidgedNode>();
    node->SetPosition(glm::vec2(100.0f, 100.0f));
    m_GraphDirty = true;
}

void NodeGraphEditor::CreateOutputNode() {
    auto* node = m_Graph->CreateNode<OutputNode>();
    node->SetPosition(glm::vec2(400.0f, 200.0f));
    m_Graph->SetOutputNode(node);
    m_GraphDirty = true;
}

Node* NodeGraphEditor::CreateNodeOfType(const String& type) {
    Node* node = nullptr;

    if (type == "Gradient") node = m_Graph->CreateNode<GradientNode>();
    else if (type == "Constant") node = m_Graph->CreateNode<ConstantNode>();
    else if (type == "WhiteNoise") node = m_Graph->CreateNode<WhiteNoiseNode>();
    else if (type == "Terrace") node = m_Graph->CreateNode<TerraceNode>();
    else if (type == "Clamp") node = m_Graph->CreateNode<ClampNode>();
    else if (type == "Invert") node = m_Graph->CreateNode<InvertNode>();
    else if (type == "Scale") node = m_Graph->CreateNode<ScaleNode>();
    else if (type == "Curve") node = m_Graph->CreateNode<CurveNode>();
    else if (type == "Smooth") node = m_Graph->CreateNode<SmoothNode>();
    else if (type == "Sharpen") node = m_Graph->CreateNode<SharpenNode>();
    else if (type == "HydraulicErosion") node = m_Graph->CreateNode<HydraulicErosionNode>();
    else if (type == "ThermalErosion") node = m_Graph->CreateNode<ThermalErosionNode>();
    else if (type == "NormalMap") node = m_Graph->CreateNode<NormalMapNode>();
    else if (type == "AmbientOcclusion") node = m_Graph->CreateNode<AmbientOcclusionNode>();
    else if (type == "Splatmap") node = m_Graph->CreateNode<SplatmapNode>();
    else if (type == "OBJExport") node = m_Graph->CreateNode<OBJExportNode>();
    else if (type == "FBXExport") node = m_Graph->CreateNode<FBXExportNode>();
    else if (type == "Add") node = m_Graph->CreateNode<AddNode>();
    else if (type == "Multiply") node = m_Graph->CreateNode<MultiplyNode>();
    else if (type == "Blend") node = m_Graph->CreateNode<BlendNode>();
    else if (type == "Max") node = m_Graph->CreateNode<MaxNode>();
    else if (type == "Min") node = m_Graph->CreateNode<MinNode>();

    if (node) {
        node->SetPosition(glm::vec2(100.0f, 100.0f));
        m_GraphDirty = true;
    }

    return node;
}

Node* NodeGraphEditor::GetSelectedNode() {
    return m_SelectedNode;
}

void NodeGraphEditor::SelectNode(Node* node) {
    m_SelectedNode = node;
}

void NodeGraphEditor::SaveGraph(const String& filepath) {
    LOG_INFO("Saving node graph...");
    m_StatusBar->SetStatus("Saving graph...");

    auto result = m_Serializer->SaveToFile(m_Graph.get(), filepath);

    if (result.success) {
        m_CurrentFilePath = filepath;
        m_GraphDirty = false;
        m_RecentFiles->AddRecentFile(filepath);

        size_t lastSlash = filepath.find_last_of("/\\");
        String filename = (lastSlash != String::npos) ? filepath.substr(lastSlash + 1) : filepath;

        m_StatusBar->ShowSuccess("Graph saved: " + filename);
        m_Notifications->NotifySuccess("Save Successful", "Graph saved to " + filename);
        LOG_INFO("Graph saved successfully to: %s", filepath.c_str());
    } else {
        m_StatusBar->ShowError("Save failed");
        m_Notifications->NotifyError("Save Failed", result.errorMessage);
        LOG_ERROR("Failed to save graph: %s", result.errorMessage.c_str());
    }
}

void NodeGraphEditor::SaveGraphAs() {
    // Define file filters
    std::vector<FileFilter> filters = {
        {"Terrain Graph Files", "*.json"},
        {"All Files", "*.*"}
    };

    // Show save dialog
    auto result = FileDialog::SaveFile("Save Terrain Graph", filters, m_CurrentFilePath, "json");

    if (result.success) {
        SaveGraph(result.filepath);
    }
}

void NodeGraphEditor::LoadGraph(const String& filepath) {
    LOG_INFO("Loading node graph...");
    m_StatusBar->SetStatus("Loading graph...");

    auto result = m_Serializer->LoadFromFile(m_Graph.get(), filepath);

    if (result.success) {
        m_CurrentFilePath = filepath;
        m_GraphDirty = false;
        m_SelectedNode = nullptr;
        m_RecentFiles->AddRecentFile(filepath);

        // Find and set output node
        for (const auto& [id, node] : m_Graph->GetNodes()) {
            if (node->GetName() == "Output") {
                m_Graph->SetOutputNode(node.get());
                break;
            }
        }

        size_t lastSlash = filepath.find_last_of("/\\");
        String filename = (lastSlash != String::npos) ? filepath.substr(lastSlash + 1) : filepath;

        uint32 nodeCount = static_cast<uint32>(m_Graph->GetNodes().size());
        m_StatusBar->ShowSuccess("Graph loaded: " + filename);
        m_Notifications->NotifySuccess("Load Successful",
            "Loaded " + filename + " with " + std::to_string(nodeCount) + " nodes");
        LOG_INFO("Graph loaded successfully from: %s", filepath.c_str());
    } else {
        m_StatusBar->ShowError("Load failed");
        m_Notifications->NotifyError("Load Failed", result.errorMessage);
        LOG_ERROR("Failed to load graph: %s", result.errorMessage.c_str());
    }
}

void NodeGraphEditor::LoadGraphWithDialog() {
    // Define file filters
    std::vector<FileFilter> filters = {
        {"Terrain Graph Files", "*.json"},
        {"All Files", "*.*"}
    };

    // Show open dialog
    auto result = FileDialog::OpenFile("Open Terrain Graph", filters, m_CurrentFilePath);

    if (result.success) {
        LoadGraph(result.filepath);
    }
}

void NodeGraphEditor::NewGraph() {
    // TODO: Ask to save if dirty

    m_Graph->Clear();
    CreateOutputNode();
    m_CurrentFilePath.clear();
    m_GraphDirty = false;
    m_SelectedNode = nullptr;

    m_StatusBar->SetStatus("Ready");
    m_Notifications->NotifyInfo("New Graph", "Created new empty graph");
    LOG_INFO("Created new graph");
}

} // namespace Terrain
