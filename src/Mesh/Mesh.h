#pragma once

#include "Core/Types.h"
#include <glm/glm.hpp>
#include <vector>

namespace Terrain {

// Vertex structure for mesh export
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
    glm::vec4 color;    // Optional vertex colors

    Vertex()
        : position(0.0f), normal(0.0f, 1.0f, 0.0f), texCoord(0.0f), color(1.0f) {}

    Vertex(const glm::vec3& pos, const glm::vec3& norm, const glm::vec2& uv)
        : position(pos), normal(norm), texCoord(uv), color(1.0f) {}
};

// Triangle face (indices into vertex array)
struct Face {
    uint32 indices[3];

    Face() : indices{0, 0, 0} {}
    Face(uint32 i0, uint32 i1, uint32 i2) : indices{i0, i1, i2} {}
};

// Mesh data structure
class Mesh {
public:
    Mesh();
    ~Mesh();

    // Vertex and face management
    void AddVertex(const Vertex& vertex);
    void AddFace(uint32 i0, uint32 i1, uint32 i2);
    void Clear();

    // Recalculate normals from faces (smooth or flat)
    void RecalculateNormals(bool smooth = true);

    // Recalculate tangents for normal mapping
    void RecalculateTangents();

    // Getters
    const std::vector<Vertex>& GetVertices() const { return m_Vertices; }
    const std::vector<Face>& GetFaces() const { return m_Faces; }
    uint32 GetVertexCount() const { return static_cast<uint32>(m_Vertices.size()); }
    uint32 GetFaceCount() const { return static_cast<uint32>(m_Faces.size()); }
    uint32 GetTriangleCount() const { return GetFaceCount(); }

    // Bounding box
    glm::vec3 GetMin() const { return m_BoundsMin; }
    glm::vec3 GetMax() const { return m_BoundsMax; }
    void RecalculateBounds();

    // Direct access (for export)
    std::vector<Vertex>& GetVerticesMutable() { return m_Vertices; }
    std::vector<Face>& GetFacesMutable() { return m_Faces; }

private:
    std::vector<Vertex> m_Vertices;
    std::vector<Face> m_Faces;
    glm::vec3 m_BoundsMin;
    glm::vec3 m_BoundsMax;
};

} // namespace Terrain
