#pragma once

#include "Core/Types.h"
#include "Mesh.h"

namespace Terrain {

// OBJ export options
struct OBJExportParams {
    bool exportNormals = true;
    bool exportUVs = true;
    bool exportColors = false;
    String materialName = "TerrainMaterial";
};

class OBJExporter {
public:
    OBJExporter();
    ~OBJExporter();

    // Export mesh to OBJ file
    // Returns true on success
    bool Export(const Mesh& mesh, const String& filepath, const OBJExportParams& params = OBJExportParams());

    // Export with separate MTL file
    bool ExportWithMaterial(const Mesh& mesh, const String& filepath, const String& mtlFilepath, const OBJExportParams& params = OBJExportParams());

private:
    // Write MTL file
    bool WriteMTL(const String& mtlFilepath, const String& materialName);
};

} // namespace Terrain
