#pragma once

#include "Node.h"
#include "Erosion/HydraulicErosion.h"
#include "Erosion/ThermalErosion.h"

namespace Terrain {

// Hydraulic Erosion Node
class HydraulicErosionNode : public Node {
public:
    HydraulicErosionNode(uint32 id);
    bool Execute(NodeGraph* graph) override;

    HydraulicErosionParams params;
};

// Thermal Erosion Node
class ThermalErosionNode : public Node {
public:
    ThermalErosionNode(uint32 id);
    bool Execute(NodeGraph* graph) override;

    ThermalErosionParams params;
};

} // namespace Terrain
