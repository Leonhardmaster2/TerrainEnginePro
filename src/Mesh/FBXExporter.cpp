#include "FBXExporter.h"
#include "Core/Logger.h"
#include <fstream>
#include <iomanip>
#include <sstream>

namespace Terrain {

FBXExporter::FBXExporter() {
}

FBXExporter::~FBXExporter() {
}

bool FBXExporter::Export(const Mesh& mesh, const String& filepath, const FBXExportParams& params) {
    LOG_INFO("Exporting mesh to FBX: %s", filepath.c_str());

    if (params.format == FBXFormat::ASCII) {
        return ExportASCII(mesh, filepath, params);
    } else {
        LOG_ERROR("Binary FBX export not implemented yet");
        return false;
    }
}

bool FBXExporter::ExportASCII(const Mesh& mesh, const String& filepath, const FBXExportParams& params) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        LOG_ERROR("Failed to open file for writing: %s", filepath.c_str());
        return false;
    }

    // Set precision for floating point numbers
    file << std::fixed << std::setprecision(6);

    // Write FBX file structure
    WriteHeader(file, params);
    WriteDefinitions(file, params);
    WriteObjects(file, mesh, params);
    WriteConnections(file, params);

    file.close();
    LOG_INFO("FBX export complete: %s", filepath.c_str());
    return true;
}

void FBXExporter::WriteHeader(std::ofstream& file, const FBXExportParams& params) {
    // FBX header comments
    file << "; FBX " << params.fbxVersion / 100 << "." << (params.fbxVersion % 100) / 10 << "." << params.fbxVersion % 10 << " project file\n";
    file << "; Created by " << params.creatorName << "\n";
    file << "; ----------------------------------------------------\n\n";

    // FBX header section
    file << "FBXHeaderExtension:  {\n";
    file << "\tFBXHeaderVersion: 1003\n";
    file << "\tFBXVersion: " << params.fbxVersion << "\n";
    file << "\tCreator: \"" << params.creatorName << "\"\n";
    file << "}\n\n";

    // Global settings
    file << "GlobalSettings:  {\n";
    file << "\tVersion: 1000\n";
    file << "\tProperties70:  {\n";
    file << "\t\tP: \"UpAxis\", \"int\", \"Integer\", \"\",1\n";
    file << "\t\tP: \"UpAxisSign\", \"int\", \"Integer\", \"\",1\n";
    file << "\t\tP: \"FrontAxis\", \"int\", \"Integer\", \"\",2\n";
    file << "\t\tP: \"FrontAxisSign\", \"int\", \"Integer\", \"\",1\n";
    file << "\t\tP: \"CoordAxis\", \"int\", \"Integer\", \"\",0\n";
    file << "\t\tP: \"CoordAxisSign\", \"int\", \"Integer\", \"\",1\n";
    file << "\t\tP: \"OriginalUpAxis\", \"int\", \"Integer\", \"\",1\n";
    file << "\t\tP: \"OriginalUpAxisSign\", \"int\", \"Integer\", \"\",1\n";
    file << "\t\tP: \"UnitScaleFactor\", \"double\", \"Number\", \"\",1.0\n";
    file << "\t}\n";
    file << "}\n\n";
}

void FBXExporter::WriteDefinitions(std::ofstream& file, const FBXExportParams& params) {
    file << "Definitions:  {\n";
    file << "\tVersion: 100\n";
    file << "\tCount: 3\n\n";

    // Model definition
    file << "\tObjectType: \"Model\" {\n";
    file << "\t\tCount: 1\n";
    file << "\t}\n\n";

    // Geometry definition
    file << "\tObjectType: \"Geometry\" {\n";
    file << "\t\tCount: 1\n";
    file << "\t}\n\n";

    // Material definition
    file << "\tObjectType: \"Material\" {\n";
    file << "\t\tCount: 1\n";
    file << "\t}\n";

    file << "}\n\n";
}

void FBXExporter::WriteObjects(std::ofstream& file, const Mesh& mesh, const FBXExportParams& params) {
    file << "Objects:  {\n";

    // Geometry object (the mesh data)
    file << "\tGeometry: 1000, \"Geometry::\", \"Mesh\" {\n";
    file << "\t\tProperties70:  {\n";
    file << "\t\t}\n";

    // Write vertices
    WriteVertexArray(file, mesh);

    // Write polygon indices
    WriteIndexArray(file, mesh);

    // Write layer 0 (normals, UVs, etc.)
    file << "\t\tLayer: 0 {\n";
    file << "\t\t\tVersion: 100\n";

    // Normals
    if (params.exportNormals) {
        WriteNormalArray(file, mesh);
    }

    // UVs
    if (params.exportUVs) {
        WriteUVArray(file, mesh);
    }

    // Vertex colors
    if (params.exportColors) {
        WriteColorArray(file, mesh);
    }

    file << "\t\t}\n"; // End Layer 0

    file << "\t}\n\n"; // End Geometry

    // Model object
    file << "\tModel: 2000, \"Model::" << params.meshName << "\", \"Mesh\" {\n";
    file << "\t\tVersion: 232\n";
    file << "\t\tProperties70:  {\n";
    file << "\t\t\tP: \"ScalingMax\", \"Vector3D\", \"Vector\", \"\",0,0,0\n";
    file << "\t\t\tP: \"DefaultAttributeIndex\", \"int\", \"Integer\", \"\",0\n";
    file << "\t\t}\n";
    file << "\t\tShading: T\n";
    file << "\t\tCulling: \"CullingOff\"\n";
    file << "\t}\n\n";

    // Material object
    file << "\tMaterial: 3000, \"Material::TerrainMaterial\", \"\" {\n";
    file << "\t\tVersion: 102\n";
    file << "\t\tShadingModel: \"phong\"\n";
    file << "\t\tMultiLayer: 0\n";
    file << "\t\tProperties70:  {\n";
    file << "\t\t\tP: \"DiffuseColor\", \"Color\", \"\", \"A\",0.8,0.8,0.8\n";
    file << "\t\t\tP: \"SpecularColor\", \"Color\", \"\", \"A\",0.2,0.2,0.2\n";
    file << "\t\t\tP: \"Shininess\", \"double\", \"Number\", \"\",20.0\n";
    file << "\t\t}\n";
    file << "\t}\n";

    file << "}\n\n"; // End Objects
}

void FBXExporter::WriteConnections(std::ofstream& file, const FBXExportParams& params) {
    file << "Connections:  {\n";

    // Connect geometry to model
    file << "\tC: \"OO\",1000,2000\n";

    // Connect material to model
    file << "\tC: \"OO\",3000,2000\n";

    file << "}\n";
}

void FBXExporter::WriteVertexArray(std::ofstream& file, const Mesh& mesh) {
    const auto& vertices = mesh.GetVertices();

    file << "\t\tVertices: *" << (vertices.size() * 3) << " {\n\t\t\ta: ";

    for (size_t i = 0; i < vertices.size(); i++) {
        const auto& v = vertices[i];
        file << v.position.x << "," << v.position.y << "," << v.position.z;

        if (i < vertices.size() - 1) {
            file << ",";
        }

        // Line breaks for readability
        if ((i + 1) % 5 == 0 && i < vertices.size() - 1) {
            file << "\n\t\t\t";
        }
    }

    file << "\n\t\t}\n";
}

void FBXExporter::WriteIndexArray(std::ofstream& file, const Mesh& mesh) {
    const auto& faces = mesh.GetFaces();

    file << "\t\tPolygonVertexIndex: *" << (faces.size() * 3) << " {\n\t\t\ta: ";

    for (size_t i = 0; i < faces.size(); i++) {
        const auto& f = faces[i];
        // FBX uses negative index for last vertex of polygon
        file << f.indices[0] << "," << f.indices[1] << "," << -(static_cast<int32>(f.indices[2]) + 1);

        if (i < faces.size() - 1) {
            file << ",";
        }

        // Line breaks for readability
        if ((i + 1) % 5 == 0 && i < faces.size() - 1) {
            file << "\n\t\t\t";
        }
    }

    file << "\n\t\t}\n";
}

void FBXExporter::WriteNormalArray(std::ofstream& file, const Mesh& mesh) {
    const auto& vertices = mesh.GetVertices();

    file << "\t\t\tLayerElementNormal: 0 {\n";
    file << "\t\t\t\tVersion: 101\n";
    file << "\t\t\t\tName: \"\"\n";
    file << "\t\t\t\tMappingInformationType: \"ByVertice\"\n";
    file << "\t\t\t\tReferenceInformationType: \"Direct\"\n";

    file << "\t\t\t\tNormals: *" << (vertices.size() * 3) << " {\n\t\t\t\t\ta: ";

    for (size_t i = 0; i < vertices.size(); i++) {
        const auto& n = vertices[i].normal;
        file << n.x << "," << n.y << "," << n.z;

        if (i < vertices.size() - 1) {
            file << ",";
        }

        if ((i + 1) % 5 == 0 && i < vertices.size() - 1) {
            file << "\n\t\t\t\t\t";
        }
    }

    file << "\n\t\t\t\t}\n";
    file << "\t\t\t}\n";
}

void FBXExporter::WriteUVArray(std::ofstream& file, const Mesh& mesh) {
    const auto& vertices = mesh.GetVertices();

    file << "\t\t\tLayerElementUV: 0 {\n";
    file << "\t\t\t\tVersion: 101\n";
    file << "\t\t\t\tName: \"UVChannel_1\"\n";
    file << "\t\t\t\tMappingInformationType: \"ByVertice\"\n";
    file << "\t\t\t\tReferenceInformationType: \"Direct\"\n";

    file << "\t\t\t\tUV: *" << (vertices.size() * 2) << " {\n\t\t\t\t\ta: ";

    for (size_t i = 0; i < vertices.size(); i++) {
        const auto& uv = vertices[i].texCoord;
        file << uv.x << "," << uv.y;

        if (i < vertices.size() - 1) {
            file << ",";
        }

        if ((i + 1) % 5 == 0 && i < vertices.size() - 1) {
            file << "\n\t\t\t\t\t";
        }
    }

    file << "\n\t\t\t\t}\n";
    file << "\t\t\t}\n";
}

void FBXExporter::WriteColorArray(std::ofstream& file, const Mesh& mesh) {
    const auto& vertices = mesh.GetVertices();

    file << "\t\t\tLayerElementColor: 0 {\n";
    file << "\t\t\t\tVersion: 101\n";
    file << "\t\t\t\tName: \"VertexColors\"\n";
    file << "\t\t\t\tMappingInformationType: \"ByVertice\"\n";
    file << "\t\t\t\tReferenceInformationType: \"Direct\"\n";

    file << "\t\t\t\tColors: *" << (vertices.size() * 4) << " {\n\t\t\t\t\ta: ";

    for (size_t i = 0; i < vertices.size(); i++) {
        const auto& c = vertices[i].color;
        file << c.r << "," << c.g << "," << c.b << "," << c.a;

        if (i < vertices.size() - 1) {
            file << ",";
        }

        if ((i + 1) % 5 == 0 && i < vertices.size() - 1) {
            file << "\n\t\t\t\t\t";
        }
    }

    file << "\n\t\t\t\t}\n";
    file << "\t\t\t}\n";
}

} // namespace Terrain
