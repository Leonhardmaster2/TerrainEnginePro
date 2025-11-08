#include "MeshGenerator.h"
#include "Core/Logger.h"
#include <glm/glm.hpp>

namespace Terrain {

MeshGenerator::MeshGenerator() {
    m_Params.scaleX = 1.0f;
    m_Params.scaleY = 100.0f;  // Default height scale
    m_Params.scaleZ = 1.0f;
    m_Params.lodLevel = 0;
    m_Params.generateNormals = true;
    m_Params.centerMesh = true;
    m_Params.flipZ = false;
}

MeshGenerator::~MeshGenerator() {
}

Unique<Mesh> MeshGenerator::Generate(const Heightfield& heightfield, const MeshGenerationParams& params) {
    uint32 heightWidth = heightfield.GetWidth();
    uint32 heightHeight = heightfield.GetHeight();

    // Apply LOD
    uint32 lodStep = 1 << params.lodLevel; // 2^lodLevel
    uint32 meshWidth = (heightWidth - 1) / lodStep + 1;
    uint32 meshHeight = (heightHeight - 1) / lodStep + 1;

    LOG_INFO("Generating mesh from heightfield (%ux%u -> %ux%u, LOD %u)...",
        heightWidth, heightHeight, meshWidth, meshHeight, params.lodLevel);

    auto mesh = MakeUnique<Mesh>();

    // Generate vertices
    GenerateVertices(mesh.get(), heightfield, params);

    // Generate triangle indices
    GenerateIndices(mesh.get(), meshWidth, meshHeight);

    // Calculate normals if requested
    if (params.generateNormals) {
        mesh->RecalculateNormals(true);
    }

    // Calculate bounds
    mesh->RecalculateBounds();

    LOG_INFO("Mesh generated: %u vertices, %u triangles",
        mesh->GetVertexCount(), mesh->GetTriangleCount());

    return mesh;
}

void MeshGenerator::GenerateVertices(Mesh* mesh, const Heightfield& heightfield, const MeshGenerationParams& params) {
    uint32 heightWidth = heightfield.GetWidth();
    uint32 heightHeight = heightfield.GetHeight();
    uint32 lodStep = 1 << params.lodLevel;

    // Calculate mesh dimensions
    uint32 meshWidth = (heightWidth - 1) / lodStep + 1;
    uint32 meshHeight = (heightHeight - 1) / lodStep + 1;

    // Calculate centering offset
    glm::vec3 offset(0.0f);
    if (params.centerMesh) {
        offset.x = -(meshWidth - 1) * params.scaleX * 0.5f;
        offset.z = -(meshHeight - 1) * params.scaleZ * 0.5f;
    }

    // Generate vertices
    for (uint32 z = 0; z < meshHeight; z++) {
        for (uint32 x = 0; x < meshWidth; x++) {
            // Sample heightfield (with LOD)
            uint32 hx = std::min(x * lodStep, heightWidth - 1);
            uint32 hz = std::min(z * lodStep, heightHeight - 1);
            float32 height = heightfield.GetHeight(hx, hz);

            // Calculate position
            glm::vec3 pos;
            pos.x = x * params.scaleX + offset.x;
            pos.y = height * params.scaleY;
            pos.z = z * params.scaleZ + offset.z;

            if (params.flipZ) {
                pos.z = -pos.z;
            }

            // Calculate UV coordinates (0-1 range)
            glm::vec2 uv;
            uv.x = static_cast<float32>(x) / static_cast<float32>(meshWidth - 1);
            uv.y = static_cast<float32>(z) / static_cast<float32>(meshHeight - 1);

            // Create vertex (normal will be calculated later)
            Vertex vertex(pos, glm::vec3(0.0f, 1.0f, 0.0f), uv);
            mesh->AddVertex(vertex);
        }
    }
}

void MeshGenerator::GenerateIndices(Mesh* mesh, uint32 width, uint32 height) {
    // Generate triangle indices for a grid
    for (uint32 z = 0; z < height - 1; z++) {
        for (uint32 x = 0; x < width - 1; x++) {
            // Calculate vertex indices for this quad
            uint32 i0 = z * width + x;           // Bottom-left
            uint32 i1 = z * width + (x + 1);     // Bottom-right
            uint32 i2 = (z + 1) * width + x;     // Top-left
            uint32 i3 = (z + 1) * width + (x + 1); // Top-right

            // Two triangles per quad
            // Triangle 1: Bottom-left, bottom-right, top-left
            mesh->AddFace(i0, i1, i2);

            // Triangle 2: Bottom-right, top-right, top-left
            mesh->AddFace(i1, i3, i2);
        }
    }
}

} // namespace Terrain
