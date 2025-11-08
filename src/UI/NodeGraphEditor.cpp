#include "NodeGraphEditor.h"
#include "Core/Logger.h"
#include <imgui.h>
#include <imnodes.h>

namespace Terrain {

NodeGraphEditor::NodeGraphEditor() {
    m_Graph = MakeUnique<NodeGraph>();
}

NodeGraphEditor::~NodeGraphEditor() {
    ImNodes::DestroyContext();
}

bool NodeGraphEditor::Initialize() {
    if (!m_Graph) {
        return false;
    }

    // Initialize ImNodes
    ImNodes::CreateContext();
    ImNodes::StyleColorsDark();

    // Create default output node
    CreateOutputNode();

    LOG_INFO("Node graph editor initialized");
    return true;
}

void NodeGraphEditor::Render() {
    ImGui::Begin("Node Graph");

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
}

void NodeGraphEditor::RenderMenuBar() {
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Add Node")) {
            if (ImGui::BeginMenu("Generators")) {
                if (ImGui::MenuItem("Perlin Noise")) CreatePerlinNode();
                if (ImGui::MenuItem("Voronoi")) CreateVoronoiNode();
                if (ImGui::MenuItem("Ridged Noise")) CreateRidgedNode();
                if (ImGui::MenuItem("Gradient")) CreateNodeOfType("Gradient");
                if (ImGui::MenuItem("Constant")) CreateNodeOfType("Constant");
                if (ImGui::MenuItem("White Noise")) CreateNodeOfType("WhiteNoise");
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
                if (ImGui::MenuItem("Thermal Erosion")) CreateNodeOfType("ThermalErosion");
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Textures")) {
                if (ImGui::MenuItem("Normal Map")) CreateNodeOfType("NormalMap");
                if (ImGui::MenuItem("Ambient Occlusion")) CreateNodeOfType("AmbientOcclusion");
                if (ImGui::MenuItem("Splatmap")) CreateNodeOfType("Splatmap");
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Mesh Export")) {
                if (ImGui::MenuItem("OBJ Export")) CreateNodeOfType("OBJExport");
                if (ImGui::MenuItem("FBX Export")) CreateNodeOfType("FBXExport");
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
            }
            if (ImGui::MenuItem("Appalachians")) {
                MountainPresets::CreatePreset(m_Graph.get(), MountainPreset::Appalachians);
                m_GraphDirty = true;
            }
            if (ImGui::MenuItem("Himalayas")) {
                MountainPresets::CreatePreset(m_Graph.get(), MountainPreset::Himalayas);
                m_GraphDirty = true;
            }
            if (ImGui::MenuItem("Rocky Mountains")) {
                MountainPresets::CreatePreset(m_Graph.get(), MountainPreset::RockyMountains);
                m_GraphDirty = true;
            }
            if (ImGui::MenuItem("Andes")) {
                MountainPresets::CreatePreset(m_Graph.get(), MountainPreset::Andes);
                m_GraphDirty = true;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Properties", nullptr, &m_ShowProperties);
            ImGui::MenuItem("Auto Execute", nullptr, &m_AutoExecute);
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

} // namespace Terrain
