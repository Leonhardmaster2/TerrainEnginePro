#pragma once

#include "Core/Types.h"
#include "Terrain/Heightfield.h"
#include <vector>
#include <glm/glm.hpp>

namespace Terrain {

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
    glm::vec3 color;
};

enum class RenderMode {
    Solid,
    Wireframe,
    Clay,
    Unlit,
    Normals
};

class TerrainMesh {
public:
    TerrainMesh();
    ~TerrainMesh();

    // Generation
    void GenerateFromHeightfield(const Heightfield& heightfield, float32 heightScale = 100.0f);
    void CalculateNormals();

    // Rendering
    void Upload();
    void Render(RenderMode mode);

    // Getters
    const std::vector<Vertex>& GetVertices() const { return m_Vertices; }
    const std::vector<uint32>& GetIndices() const { return m_Indices; }

    // Stats
    uint32 GetVertexCount() const { return static_cast<uint32>(m_Vertices.size()); }
    uint32 GetTriangleCount() const { return static_cast<uint32>(m_Indices.size() / 3); }

    // Export
    bool ExportOBJ(const String& filepath);
    bool ExportFBX(const String& filepath);

private:
    std::vector<Vertex> m_Vertices;
    std::vector<uint32> m_Indices;

    // OpenGL buffers
    uint32 m_VAO = 0;
    uint32 m_VBO = 0;
    uint32 m_EBO = 0;

    bool m_Uploaded = false;
};

} // namespace Terrain
