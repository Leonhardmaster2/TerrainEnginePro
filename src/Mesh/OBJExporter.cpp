#include "OBJExporter.h"
#include "Core/Logger.h"
#include <fstream>
#include <iomanip>

namespace Terrain {

OBJExporter::OBJExporter() {
}

OBJExporter::~OBJExporter() {
}

bool OBJExporter::Export(const Mesh& mesh, const String& filepath, const OBJExportParams& params) {
    LOG_INFO("Exporting mesh to OBJ: %s", filepath.c_str());

    std::ofstream file(filepath);
    if (!file.is_open()) {
        LOG_ERROR("Failed to open file for writing: %s", filepath.c_str());
        return false;
    }

    // Write header
    file << "# Terrain Engine Pro - OBJ Export\n";
    file << "# Vertices: " << mesh.GetVertexCount() << "\n";
    file << "# Faces: " << mesh.GetFaceCount() << "\n";
    file << "\n";

    // Write vertices
    const auto& vertices = mesh.GetVertices();
    for (const auto& vertex : vertices) {
        file << "v " << std::fixed << std::setprecision(6)
             << vertex.position.x << " "
             << vertex.position.y << " "
             << vertex.position.z;

        // Optional vertex colors (as RGB after position)
        if (params.exportColors) {
            file << " " << vertex.color.r << " " << vertex.color.g << " " << vertex.color.b;
        }

        file << "\n";
    }

    // Write texture coordinates
    if (params.exportUVs) {
        file << "\n";
        for (const auto& vertex : vertices) {
            file << "vt " << std::fixed << std::setprecision(6)
                 << vertex.texCoord.x << " "
                 << vertex.texCoord.y << "\n";
        }
    }

    // Write normals
    if (params.exportNormals) {
        file << "\n";
        for (const auto& vertex : vertices) {
            file << "vn " << std::fixed << std::setprecision(6)
                 << vertex.normal.x << " "
                 << vertex.normal.y << " "
                 << vertex.normal.z << "\n";
        }
    }

    // Write faces
    file << "\n";
    file << "# Faces\n";
    const auto& faces = mesh.GetFaces();
    for (const auto& face : faces) {
        file << "f ";
        for (int i = 0; i < 3; i++) {
            uint32 idx = face.indices[i] + 1; // OBJ is 1-indexed
            file << idx;

            if (params.exportUVs || params.exportNormals) {
                file << "/";
                if (params.exportUVs) {
                    file << idx;
                }
                if (params.exportNormals) {
                    file << "/" << idx;
                }
            }

            if (i < 2) file << " ";
        }
        file << "\n";
    }

    file.close();
    LOG_INFO("OBJ export complete: %s", filepath.c_str());
    return true;
}

bool OBJExporter::ExportWithMaterial(const Mesh& mesh, const String& filepath, const String& mtlFilepath, const OBJExportParams& params) {
    // Write MTL file
    if (!WriteMTL(mtlFilepath, params.materialName)) {
        return false;
    }

    // Export OBJ with MTL reference
    LOG_INFO("Exporting mesh to OBJ with material: %s", filepath.c_str());

    std::ofstream file(filepath);
    if (!file.is_open()) {
        LOG_ERROR("Failed to open file for writing: %s", filepath.c_str());
        return false;
    }

    // Write header with MTL reference
    file << "# Terrain Engine Pro - OBJ Export\n";
    file << "# Vertices: " << mesh.GetVertexCount() << "\n";
    file << "# Faces: " << mesh.GetFaceCount() << "\n";

    // Extract MTL filename from path
    size_t lastSlash = mtlFilepath.find_last_of("/\\");
    String mtlFilename = (lastSlash != String::npos) ? mtlFilepath.substr(lastSlash + 1) : mtlFilepath;
    file << "mtllib " << mtlFilename << "\n";
    file << "\n";

    // Write vertices
    const auto& vertices = mesh.GetVertices();
    for (const auto& vertex : vertices) {
        file << "v " << std::fixed << std::setprecision(6)
             << vertex.position.x << " "
             << vertex.position.y << " "
             << vertex.position.z;

        if (params.exportColors) {
            file << " " << vertex.color.r << " " << vertex.color.g << " " << vertex.color.b;
        }

        file << "\n";
    }

    // Write texture coordinates
    if (params.exportUVs) {
        file << "\n";
        for (const auto& vertex : vertices) {
            file << "vt " << std::fixed << std::setprecision(6)
                 << vertex.texCoord.x << " "
                 << vertex.texCoord.y << "\n";
        }
    }

    // Write normals
    if (params.exportNormals) {
        file << "\n";
        for (const auto& vertex : vertices) {
            file << "vn " << std::fixed << std::setprecision(6)
                 << vertex.normal.x << " "
                 << vertex.normal.y << " "
                 << vertex.normal.z << "\n";
        }
    }

    // Write faces with material
    file << "\n";
    file << "# Faces\n";
    file << "usemtl " << params.materialName << "\n";
    const auto& faces = mesh.GetFaces();
    for (const auto& face : faces) {
        file << "f ";
        for (int i = 0; i < 3; i++) {
            uint32 idx = face.indices[i] + 1;
            file << idx;

            if (params.exportUVs || params.exportNormals) {
                file << "/";
                if (params.exportUVs) {
                    file << idx;
                }
                if (params.exportNormals) {
                    file << "/" << idx;
                }
            }

            if (i < 2) file << " ";
        }
        file << "\n";
    }

    file.close();
    LOG_INFO("OBJ export with material complete: %s", filepath.c_str());
    return true;
}

bool OBJExporter::WriteMTL(const String& mtlFilepath, const String& materialName) {
    LOG_INFO("Writing MTL file: %s", mtlFilepath.c_str());

    std::ofstream file(mtlFilepath);
    if (!file.is_open()) {
        LOG_ERROR("Failed to open MTL file for writing: %s", mtlFilepath.c_str());
        return false;
    }

    // Write basic material definition
    file << "# Terrain Engine Pro - MTL Material\n";
    file << "newmtl " << materialName << "\n";
    file << "Ka 0.8 0.8 0.8\n";  // Ambient color
    file << "Kd 0.8 0.8 0.8\n";  // Diffuse color
    file << "Ks 0.2 0.2 0.2\n";  // Specular color
    file << "Ns 10.0\n";          // Specular exponent
    file << "d 1.0\n";            // Dissolve (opacity)
    file << "illum 2\n";          // Illumination model (2 = highlight on)
    file << "\n";

    file.close();
    LOG_INFO("MTL file written: %s", mtlFilepath.c_str());
    return true;
}

} // namespace Terrain
