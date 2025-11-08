#pragma once

#include "Core/Types.h"
#include "NodeGraph.h"
#include "Mesh/Mesh.h"
#include "Mesh/MeshGenerator.h"
#include "Mesh/OBJExporter.h"
#include "Mesh/FBXExporter.h"

namespace Terrain {

// ============================================================================
// Mesh Generator Node
// ============================================================================

class MeshGeneratorNode : public Node {
public:
    explicit MeshGeneratorNode(uint32 id);

    bool Execute(NodeGraph* graph) override;

    // Parameters
    MeshGenerationParams params;

    // Get generated mesh
    Unique<Mesh> GetMesh() const;

private:
    Unique<Mesh> m_GeneratedMesh;
};

// ============================================================================
// OBJ Export Node
// ============================================================================

class OBJExportNode : public Node {
public:
    explicit OBJExportNode(uint32 id);

    bool Execute(NodeGraph* graph) override;

    // Parameters
    OBJExportParams params;
    String outputPath = "terrain.obj";
    bool exportMaterial = true;
    String materialPath = "terrain.mtl";

private:
    bool m_ExportSuccess = false;
};

// ============================================================================
// FBX Export Node
// ============================================================================

class FBXExportNode : public Node {
public:
    explicit FBXExportNode(uint32 id);

    bool Execute(NodeGraph* graph) override;

    // Parameters
    FBXExportParams params;
    String outputPath = "terrain.fbx";

private:
    bool m_ExportSuccess = false;
};

} // namespace Terrain
