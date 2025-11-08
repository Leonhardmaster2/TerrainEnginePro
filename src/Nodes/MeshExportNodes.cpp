#include "MeshExportNodes.h"
#include "NodeGraph.h"
#include "Core/Logger.h"

namespace Terrain {

// ============================================================================
// Mesh Generator Node
// ============================================================================

MeshGeneratorNode::MeshGeneratorNode(uint32 id)
    : Node(id, "Mesh Generator", NodeCategory::Export) {
    AddInputPin("Input", PinType::Heightfield);
    AddOutputPin("Mesh", PinType::Data);

    // Default parameters
    params.scaleX = 1.0f;
    params.scaleY = 100.0f;
    params.scaleZ = 1.0f;
    params.lodLevel = 0;
    params.generateNormals = true;
    params.centerMesh = true;
    params.flipZ = false;
}

bool MeshGeneratorNode::Execute(NodeGraph* graph) {
    if (!m_Dirty && m_GeneratedMesh) {
        return true;
    }

    auto input = GetInputHeightfield("Input", graph);
    if (!input) {
        LOG_ERROR("Mesh generator node: no input heightfield");
        return false;
    }

    LOG_INFO("Generating mesh from heightfield...");

    auto generator = MakeUnique<MeshGenerator>();
    m_GeneratedMesh = generator->Generate(*input, params);

    if (!m_GeneratedMesh) {
        LOG_ERROR("Failed to generate mesh");
        return false;
    }

    m_Dirty = false;
    return true;
}

Unique<Mesh> MeshGeneratorNode::GetMesh() const {
    if (!m_GeneratedMesh) {
        return nullptr;
    }

    // Create a copy
    auto copy = MakeUnique<Mesh>();
    for (const auto& vertex : m_GeneratedMesh->GetVertices()) {
        copy->AddVertex(vertex);
    }
    for (const auto& face : m_GeneratedMesh->GetFaces()) {
        copy->AddFace(face.indices[0], face.indices[1], face.indices[2]);
    }

    return copy;
}

// ============================================================================
// OBJ Export Node
// ============================================================================

OBJExportNode::OBJExportNode(uint32 id)
    : Node(id, "OBJ Export", NodeCategory::Export) {
    AddInputPin("Input", PinType::Heightfield);

    // Default parameters
    params.exportNormals = true;
    params.exportUVs = true;
    params.exportColors = false;
    params.materialName = "TerrainMaterial";
}

bool OBJExportNode::Execute(NodeGraph* graph) {
    if (!m_Dirty && m_ExportSuccess) {
        return true;
    }

    auto input = GetInputHeightfield("Input", graph);
    if (!input) {
        LOG_ERROR("OBJ export node: no input heightfield");
        return false;
    }

    LOG_INFO("Exporting terrain to OBJ: %s", outputPath.c_str());

    // Generate mesh
    MeshGenerationParams meshParams;
    meshParams.scaleX = 1.0f;
    meshParams.scaleY = 100.0f;
    meshParams.scaleZ = 1.0f;
    meshParams.lodLevel = 0;
    meshParams.generateNormals = true;
    meshParams.centerMesh = true;

    auto generator = MakeUnique<MeshGenerator>();
    auto mesh = generator->Generate(*input, meshParams);

    if (!mesh) {
        LOG_ERROR("Failed to generate mesh for OBJ export");
        return false;
    }

    // Export to OBJ
    auto exporter = MakeUnique<OBJExporter>();

    if (exportMaterial) {
        m_ExportSuccess = exporter->ExportWithMaterial(*mesh, outputPath, materialPath, params);
    } else {
        m_ExportSuccess = exporter->Export(*mesh, outputPath, params);
    }

    if (!m_ExportSuccess) {
        LOG_ERROR("Failed to export OBJ");
        return false;
    }

    LOG_INFO("OBJ export complete: %s", outputPath.c_str());
    m_Dirty = false;
    return true;
}

// ============================================================================
// FBX Export Node
// ============================================================================

FBXExportNode::FBXExportNode(uint32 id)
    : Node(id, "FBX Export", NodeCategory::Export) {
    AddInputPin("Input", PinType::Heightfield);

    // Default parameters
    params.format = FBXFormat::ASCII;
    params.exportNormals = true;
    params.exportUVs = true;
    params.exportColors = false;
    params.meshName = "TerrainMesh";
    params.creatorName = "Terrain Engine Pro";
    params.fbxVersion = 7400; // FBX 2014
}

bool FBXExportNode::Execute(NodeGraph* graph) {
    if (!m_Dirty && m_ExportSuccess) {
        return true;
    }

    auto input = GetInputHeightfield("Input", graph);
    if (!input) {
        LOG_ERROR("FBX export node: no input heightfield");
        return false;
    }

    LOG_INFO("Exporting terrain to FBX: %s", outputPath.c_str());

    // Generate mesh
    MeshGenerationParams meshParams;
    meshParams.scaleX = 1.0f;
    meshParams.scaleY = 100.0f;
    meshParams.scaleZ = 1.0f;
    meshParams.lodLevel = 0;
    meshParams.generateNormals = true;
    meshParams.centerMesh = true;

    auto generator = MakeUnique<MeshGenerator>();
    auto mesh = generator->Generate(*input, meshParams);

    if (!mesh) {
        LOG_ERROR("Failed to generate mesh for FBX export");
        return false;
    }

    // Export to FBX
    auto exporter = MakeUnique<FBXExporter>();
    m_ExportSuccess = exporter->Export(*mesh, outputPath, params);

    if (!m_ExportSuccess) {
        LOG_ERROR("Failed to export FBX");
        return false;
    }

    LOG_INFO("FBX export complete: %s", outputPath.c_str());
    m_Dirty = false;
    return true;
}

} // namespace Terrain
