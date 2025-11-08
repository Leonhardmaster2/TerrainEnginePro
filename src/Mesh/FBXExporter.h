#pragma once

#include "Core/Types.h"
#include "Mesh.h"

namespace Terrain {

// FBX export format
enum class FBXFormat {
    ASCII,   // Human-readable ASCII format (recommended)
    Binary   // Binary format (not implemented yet)
};

// FBX export options
struct FBXExportParams {
    FBXFormat format = FBXFormat::ASCII;
    bool exportNormals = true;
    bool exportUVs = true;
    bool exportColors = false;
    String meshName = "TerrainMesh";
    String creatorName = "Terrain Engine Pro";

    // FBX version settings
    int32 fbxVersion = 7400; // FBX 2014 (widely compatible)
};

class FBXExporter {
public:
    FBXExporter();
    ~FBXExporter();

    // Export mesh to FBX file
    // Returns true on success
    bool Export(const Mesh& mesh, const String& filepath, const FBXExportParams& params = FBXExportParams());

private:
    // ASCII FBX export
    bool ExportASCII(const Mesh& mesh, const String& filepath, const FBXExportParams& params);

    // Write FBX header
    void WriteHeader(std::ofstream& file, const FBXExportParams& params);

    // Write FBX definitions
    void WriteDefinitions(std::ofstream& file, const FBXExportParams& params);

    // Write FBX objects (geometry, model, etc.)
    void WriteObjects(std::ofstream& file, const Mesh& mesh, const FBXExportParams& params);

    // Write FBX connections
    void WriteConnections(std::ofstream& file, const FBXExportParams& params);

    // Helper: Write vertex array
    void WriteVertexArray(std::ofstream& file, const Mesh& mesh);

    // Helper: Write index array
    void WriteIndexArray(std::ofstream& file, const Mesh& mesh);

    // Helper: Write normal array
    void WriteNormalArray(std::ofstream& file, const Mesh& mesh);

    // Helper: Write UV array
    void WriteUVArray(std::ofstream& file, const Mesh& mesh);

    // Helper: Write color array
    void WriteColorArray(std::ofstream& file, const Mesh& mesh);
};

} // namespace Terrain
