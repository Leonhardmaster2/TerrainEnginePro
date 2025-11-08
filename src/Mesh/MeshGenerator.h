#pragma once

#include "Core/Types.h"
#include "Terrain/Heightfield.h"
#include "Mesh.h"

namespace Terrain {

// Parameters for mesh generation
struct MeshGenerationParams {
    float32 scaleX = 1.0f;          // Horizontal scale (X axis)
    float32 scaleY = 1.0f;          // Vertical scale (height)
    float32 scaleZ = 1.0f;          // Horizontal scale (Z axis)
    uint32 lodLevel = 0;            // Level of detail (0 = full resolution, 1 = half, etc.)
    bool generateNormals = true;    // Calculate smooth normals
    bool centerMesh = true;         // Center mesh at origin
    bool flipZ = false;             // Flip Z axis (for different coordinate systems)
};

class MeshGenerator {
public:
    MeshGenerator();
    ~MeshGenerator();

    // Generate mesh from heightfield
    Unique<Mesh> Generate(const Heightfield& heightfield, const MeshGenerationParams& params);

    // Get/Set default parameters
    const MeshGenerationParams& GetParams() const { return m_Params; }
    void SetParams(const MeshGenerationParams& params) { m_Params = params; }

private:
    // Generate vertices and UVs
    void GenerateVertices(Mesh* mesh, const Heightfield& heightfield, const MeshGenerationParams& params);

    // Generate triangle indices
    void GenerateIndices(Mesh* mesh, uint32 width, uint32 height);

    MeshGenerationParams m_Params;
};

} // namespace Terrain
