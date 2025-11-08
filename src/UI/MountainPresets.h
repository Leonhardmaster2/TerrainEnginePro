#pragma once

#include "Core/Types.h"
#include "Nodes/NodeGraph.h"
#include "Nodes/GeneratorNodes.h"
#include "Nodes/ModifierNodes.h"
#include "Nodes/ErosionNodes.h"

namespace Terrain {

enum class MountainPreset {
    Alps,           // Sharp peaks, deep valleys, glacial features
    Appalachians,   // Rolling ridges, weathered peaks
    Himalayas,      // Extreme peaks, sharp ridges
    RockyMountains, // Mixed sharp and rolling terrain
    Andes,          // Long ridges with volcanic features
    Custom
};

class MountainPresets {
public:
    // Create a node graph for a specific mountain preset
    static void CreatePreset(NodeGraph* graph, MountainPreset preset, uint32 resolution = 1024);

    // Get preset name
    static const char* GetPresetName(MountainPreset preset);

    // Get preset description
    static const char* GetPresetDescription(MountainPreset preset);

private:
    // Individual preset creators
    static void CreateAlps(NodeGraph* graph, uint32 resolution);
    static void CreateAppalachians(NodeGraph* graph, uint32 resolution);
    static void CreateHimalayas(NodeGraph* graph, uint32 resolution);
    static void CreateRockyMountains(NodeGraph* graph, uint32 resolution);
    static void CreateAndes(NodeGraph* graph, uint32 resolution);
};

} // namespace Terrain
