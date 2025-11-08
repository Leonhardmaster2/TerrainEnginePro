#include "Mesh.h"
#include <algorithm>
#include <limits>

namespace Terrain {

Mesh::Mesh()
    : m_BoundsMin(std::numeric_limits<float>::max())
    , m_BoundsMax(std::numeric_limits<float>::lowest()) {
}

Mesh::~Mesh() {
}

void Mesh::AddVertex(const Vertex& vertex) {
    m_Vertices.push_back(vertex);

    // Update bounds
    m_BoundsMin = glm::min(m_BoundsMin, vertex.position);
    m_BoundsMax = glm::max(m_BoundsMax, vertex.position);
}

void Mesh::AddFace(uint32 i0, uint32 i1, uint32 i2) {
    m_Faces.push_back(Face(i0, i1, i2));
}

void Mesh::Clear() {
    m_Vertices.clear();
    m_Faces.clear();
    m_BoundsMin = glm::vec3(std::numeric_limits<float>::max());
    m_BoundsMax = glm::vec3(std::numeric_limits<float>::lowest());
}

void Mesh::RecalculateNormals(bool smooth) {
    if (smooth) {
        // Smooth normals - average normals from adjacent faces

        // Reset all normals to zero
        for (auto& vertex : m_Vertices) {
            vertex.normal = glm::vec3(0.0f);
        }

        // Accumulate face normals
        for (const auto& face : m_Faces) {
            const glm::vec3& v0 = m_Vertices[face.indices[0]].position;
            const glm::vec3& v1 = m_Vertices[face.indices[1]].position;
            const glm::vec3& v2 = m_Vertices[face.indices[2]].position;

            glm::vec3 edge1 = v1 - v0;
            glm::vec3 edge2 = v2 - v0;
            glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));

            // Add to all vertices of this face
            m_Vertices[face.indices[0]].normal += faceNormal;
            m_Vertices[face.indices[1]].normal += faceNormal;
            m_Vertices[face.indices[2]].normal += faceNormal;
        }

        // Normalize all normals
        for (auto& vertex : m_Vertices) {
            float len = glm::length(vertex.normal);
            if (len > 0.0001f) {
                vertex.normal /= len;
            } else {
                vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f); // Default up
            }
        }
    } else {
        // Flat normals - each face has its own normal
        // This requires duplicating vertices (not implemented here)
        // For now, just use smooth normals
        RecalculateNormals(true);
    }
}

void Mesh::RecalculateTangents() {
    // Calculate tangents for normal mapping (not implemented yet)
    // This would be used for advanced PBR rendering
}

void Mesh::RecalculateBounds() {
    m_BoundsMin = glm::vec3(std::numeric_limits<float>::max());
    m_BoundsMax = glm::vec3(std::numeric_limits<float>::lowest());

    for (const auto& vertex : m_Vertices) {
        m_BoundsMin = glm::min(m_BoundsMin, vertex.position);
        m_BoundsMax = glm::max(m_BoundsMax, vertex.position);
    }
}

} // namespace Terrain
