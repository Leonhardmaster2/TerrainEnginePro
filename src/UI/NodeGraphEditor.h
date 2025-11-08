#pragma once

#include "Core/Types.h"
#include "Nodes/NodeGraph.h"
#include "Nodes/GeneratorNodes.h"
#include "Nodes/ModifierNodes.h"
#include "Nodes/ErosionNodes.h"
#include "Nodes/TextureNodes.h"
#include "Nodes/MeshExportNodes.h"
#include "Serialization/GraphSerializer.h"
#include "MountainPresets.h"

namespace Terrain {

// Node editor state
struct NodeEditorState {
    bool showNodeList = false;
    glm::vec2 nodeListPos;
};

class NodeGraphEditor {
public:
    NodeGraphEditor();
    ~NodeGraphEditor();

    bool Initialize();
    void Render();

    // Get the node graph
    NodeGraph* GetGraph() { return m_Graph.get(); }

    // Execute and get result
    bool ExecuteGraph();
    Unique<Heightfield> GetResult();

    // Node creation shortcuts
    void CreatePerlinNode();
    void CreateVoronoiNode();
    void CreateRidgedNode();
    void CreateOutputNode();

    // Save/Load
    void SaveGraph(const String& filepath);
    void LoadGraph(const String& filepath);

private:
    void RenderMenuBar();
    void RenderNodeList();
    void RenderNodeCanvas();
    void RenderNodeProperties();

    // Node creation
    void ShowNodeCreationPopup();
    Node* CreateNodeOfType(const String& type);

    // Node rendering
    void RenderNode(Node* node);
    void RenderNodeInputs(Node* node);
    void RenderNodeOutputs(Node* node);

    // Selection
    Node* GetSelectedNode();
    void SelectNode(Node* node);

    Unique<NodeGraph> m_Graph;
    Unique<GraphSerializer> m_Serializer;
    NodeEditorState m_State;

    // UI state
    Node* m_SelectedNode = nullptr;
    bool m_ShowProperties = true;
    bool m_AutoExecute = true;
    bool m_GraphDirty = false;
    String m_CurrentFilePath;
};

} // namespace Terrain
