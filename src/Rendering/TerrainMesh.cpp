#include "TerrainMesh.h"
#include "Core/Logger.h"
#include <glad/glad.h>
#include <fstream>
#include <sstream>

namespace Terrain {

TerrainMesh::TerrainMesh() {
}

TerrainMesh::~TerrainMesh() {
    if (m_VAO) glDeleteVertexArrays(1, &m_VAO);
    if (m_VBO) glDeleteBuffers(1, &m_VBO);
    if (m_EBO) glDeleteBuffers(1, &m_EBO);
}

void TerrainMesh::GenerateFromHeightfield(const Heightfield& heightfield, float32 heightScale) {
    uint32 width = heightfield.GetWidth();
    uint32 height = heightfield.GetHeight();

    m_Vertices.clear();
    m_Indices.clear();

    // Generate vertices
    m_Vertices.reserve(width * height);

    for (uint32 y = 0; y < height; y++) {
        for (uint32 x = 0; x < width; x++) {
            Vertex vertex;

            // Position
            vertex.position.x = static_cast<float32>(x) - width * 0.5f;
            vertex.position.z = static_cast<float32>(y) - height * 0.5f;
            vertex.position.y = heightfield.GetHeight(x, y) * heightScale;

            // Texture coordinates
            vertex.texCoord.x = static_cast<float32>(x) / (width - 1);
            vertex.texCoord.y = static_cast<float32>(y) / (height - 1);

            // Color based on height (for clay rendering)
            float32 normalizedHeight = heightfield.GetHeight(x, y);
            vertex.color = glm::vec3(normalizedHeight);

            // Normal (calculated later)
            vertex.normal = glm::vec3(0, 1, 0);

            m_Vertices.push_back(vertex);
        }
    }

    // Generate indices
    m_Indices.reserve((width - 1) * (height - 1) * 6);

    for (uint32 y = 0; y < height - 1; y++) {
        for (uint32 x = 0; x < width - 1; x++) {
            uint32 topLeft = y * width + x;
            uint32 topRight = topLeft + 1;
            uint32 bottomLeft = (y + 1) * width + x;
            uint32 bottomRight = bottomLeft + 1;

            // First triangle
            m_Indices.push_back(topLeft);
            m_Indices.push_back(bottomLeft);
            m_Indices.push_back(topRight);

            // Second triangle
            m_Indices.push_back(topRight);
            m_Indices.push_back(bottomLeft);
            m_Indices.push_back(bottomRight);
        }
    }

    CalculateNormals();

    LOG_INFO("Generated terrain mesh: %u vertices, %u triangles",
             GetVertexCount(), GetTriangleCount());
}

void TerrainMesh::CalculateNormals() {
    // Reset normals
    for (auto& vertex : m_Vertices) {
        vertex.normal = glm::vec3(0, 0, 0);
    }

    // Accumulate face normals
    for (size_t i = 0; i < m_Indices.size(); i += 3) {
        uint32 i0 = m_Indices[i];
        uint32 i1 = m_Indices[i + 1];
        uint32 i2 = m_Indices[i + 2];

        glm::vec3 v0 = m_Vertices[i0].position;
        glm::vec3 v1 = m_Vertices[i1].position;
        glm::vec3 v2 = m_Vertices[i2].position;

        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;
        glm::vec3 normal = glm::cross(edge1, edge2);

        m_Vertices[i0].normal += normal;
        m_Vertices[i1].normal += normal;
        m_Vertices[i2].normal += normal;
    }

    // Normalize
    for (auto& vertex : m_Vertices) {
        vertex.normal = glm::normalize(vertex.normal);
    }
}

void TerrainMesh::Upload() {
    if (m_Uploaded) {
        glDeleteVertexArrays(1, &m_VAO);
        glDeleteBuffers(1, &m_VBO);
        glDeleteBuffers(1, &m_EBO);
    }

    // Create VAO
    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);

    // Create VBO
    glGenBuffers(1, &m_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(Vertex), m_Vertices.data(), GL_STATIC_DRAW);

    // Create EBO
    glGenBuffers(1, &m_EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indices.size() * sizeof(uint32), m_Indices.data(), GL_STATIC_DRAW);

    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

    // Normal attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

    // TexCoord attribute
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));

    // Color attribute
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));

    glBindVertexArray(0);

    m_Uploaded = true;
    LOG_INFO("Terrain mesh uploaded to GPU");
}

void TerrainMesh::Render(RenderMode mode) {
    if (!m_Uploaded) {
        LOG_WARN("Attempting to render non-uploaded mesh");
        return;
    }

    glBindVertexArray(m_VAO);

    switch (mode) {
        case RenderMode::Wireframe:
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            break;
        case RenderMode::Solid:
        case RenderMode::Clay:
        case RenderMode::Unlit:
        case RenderMode::Normals:
        default:
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            break;
    }

    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_Indices.size()), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

bool TerrainMesh::ExportOBJ(const String& filepath) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        LOG_ERROR("Failed to open OBJ file for writing: %s", filepath.c_str());
        return false;
    }

    // Write header
    file << "# Terrain Engine Pro - OBJ Export\n";
    file << "# Vertices: " << m_Vertices.size() << "\n";
    file << "# Triangles: " << (m_Indices.size() / 3) << "\n\n";

    // Write vertices
    for (const auto& vertex : m_Vertices) {
        file << "v " << vertex.position.x << " "
             << vertex.position.y << " "
             << vertex.position.z << "\n";
    }

    file << "\n";

    // Write normals
    for (const auto& vertex : m_Vertices) {
        file << "vn " << vertex.normal.x << " "
             << vertex.normal.y << " "
             << vertex.normal.z << "\n";
    }

    file << "\n";

    // Write texture coordinates
    for (const auto& vertex : m_Vertices) {
        file << "vt " << vertex.texCoord.x << " "
             << vertex.texCoord.y << "\n";
    }

    file << "\n";

    // Write faces
    for (size_t i = 0; i < m_Indices.size(); i += 3) {
        uint32 i0 = m_Indices[i] + 1;      // OBJ indices start at 1
        uint32 i1 = m_Indices[i + 1] + 1;
        uint32 i2 = m_Indices[i + 2] + 1;

        file << "f " << i0 << "/" << i0 << "/" << i0 << " "
             << i1 << "/" << i1 << "/" << i1 << " "
             << i2 << "/" << i2 << "/" << i2 << "\n";
    }

    file.close();

    LOG_INFO("Exported terrain mesh to OBJ: %s", filepath.c_str());
    return true;
}

bool TerrainMesh::ExportFBX(const String& filepath) {
    LOG_WARN("FBX export requires FBX SDK integration (not yet implemented)");
    LOG_INFO("Use OBJ export as alternative");
    return false;
}

} // namespace Terrain
