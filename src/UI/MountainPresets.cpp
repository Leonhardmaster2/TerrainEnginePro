#include "MountainPresets.h"
#include "Core/Logger.h"

namespace Terrain {

void MountainPresets::CreatePreset(NodeGraph* graph, MountainPreset preset, uint32 resolution) {
    // Clear existing graph
    graph->Clear();

    switch (preset) {
        case MountainPreset::Alps:
            CreateAlps(graph, resolution);
            break;
        case MountainPreset::Appalachians:
            CreateAppalachians(graph, resolution);
            break;
        case MountainPreset::Himalayas:
            CreateHimalayas(graph, resolution);
            break;
        case MountainPreset::RockyMountains:
            CreateRockyMountains(graph, resolution);
            break;
        case MountainPreset::Andes:
            CreateAndes(graph, resolution);
            break;
        default:
            break;
    }
}

const char* MountainPresets::GetPresetName(MountainPreset preset) {
    switch (preset) {
        case MountainPreset::Alps: return "Alps";
        case MountainPreset::Appalachians: return "Appalachians";
        case MountainPreset::Himalayas: return "Himalayas";
        case MountainPreset::RockyMountains: return "Rocky Mountains";
        case MountainPreset::Andes: return "Andes";
        default: return "Custom";
    }
}

const char* MountainPresets::GetPresetDescription(MountainPreset preset) {
    switch (preset) {
        case MountainPreset::Alps:
            return "Sharp peaks, deep U-shaped valleys, glacial features. Height: 4,000-4,800m";
        case MountainPreset::Appalachians:
            return "Rolling ridges, weathered peaks, gentle slopes. Height: 1,200-2,000m";
        case MountainPreset::Himalayas:
            return "Extreme jagged peaks, dramatic elevation. Height: 6,000-8,800m";
        case MountainPreset::RockyMountains:
            return "Mixed terrain with sharp peaks and rolling hills. Height: 3,000-4,400m";
        case MountainPreset::Andes:
            return "Long volcanic ridges, high plateaus. Height: 4,000-6,900m";
        default:
            return "Custom terrain setup";
    }
}

void MountainPresets::CreateAlps(NodeGraph* graph, uint32 resolution) {
    LOG_INFO("Creating Alps preset...");

    // Base mountain structure - large scale features
    auto* baseNoise = graph->CreateNode<PerlinNode>();
    baseNoise->SetPosition(glm::vec2(50, 100));
    baseNoise->width = resolution;
    baseNoise->height = resolution;
    baseNoise->params.frequency = 0.003f;  // Large features
    baseNoise->params.amplitude = 1.5f;
    baseNoise->params.octaves = 6;
    baseNoise->params.lacunarity = 2.2f;
    baseNoise->params.persistence = 0.55f;
    baseNoise->params.seed = 4807; // Alps-specific seed

    // Sharp ridges and peaks
    auto* ridges = graph->CreateNode<RidgedNode>();
    ridges->SetPosition(glm::vec2(50, 250));
    ridges->width = resolution;
    ridges->height = resolution;
    ridges->frequency = 0.005f;
    ridges->amplitude = 1.2f;
    ridges->octaves = 7;
    ridges->lacunarity = 2.3f;
    ridges->persistence = 0.6f;
    ridges->ridgeOffset = 1.2f; // Sharper ridges
    ridges->seed = 4808;

    // Combine base and ridges
    auto* combineNode = graph->CreateNode<AddNode>();
    combineNode->SetPosition(glm::vec2(300, 175));

    // Scale to appropriate height
    auto* scaleNode = graph->CreateNode<ScaleNode>();
    scaleNode->SetPosition(glm::vec2(500, 175));
    scaleNode->scale = 0.8f; // Alps are very tall

    // Add some terracing for glacial valleys
    auto* terraceNode = graph->CreateNode<TerraceNode>();
    terraceNode->SetPosition(glm::vec2(700, 175));
    terraceNode->steps = 15;
    terraceNode->blend = 0.85f; // Smooth terracing

    // Slight smoothing to simulate glacial smoothing
    auto* smoothNode = graph->CreateNode<SmoothNode>();
    smoothNode->SetPosition(glm::vec2(900, 175));
    smoothNode->iterations = 1;
    smoothNode->strength = 0.2f;

    // Thermal erosion for realistic slopes
    auto* thermalNode = graph->CreateNode<ThermalErosionNode>();
    thermalNode->SetPosition(glm::vec2(1100, 175));
    thermalNode->params.iterations = 8;
    thermalNode->params.talusAngle = 0.6f; // Steeper slopes
    thermalNode->params.strength = 0.4f;

    // Output
    auto* outputNode = graph->CreateNode<OutputNode>();
    outputNode->SetPosition(glm::vec2(1300, 175));
    graph->SetOutputNode(outputNode);

    // Create connections
    graph->CreateConnection(baseNoise->GetOutputPin("Output")->id, combineNode->GetInputPin("A")->id);
    graph->CreateConnection(ridges->GetOutputPin("Output")->id, combineNode->GetInputPin("B")->id);
    graph->CreateConnection(combineNode->GetOutputPin("Output")->id, scaleNode->GetInputPin("Input")->id);
    graph->CreateConnection(scaleNode->GetOutputPin("Output")->id, terraceNode->GetInputPin("Input")->id);
    graph->CreateConnection(terraceNode->GetOutputPin("Output")->id, smoothNode->GetInputPin("Input")->id);
    graph->CreateConnection(smoothNode->GetOutputPin("Output")->id, thermalNode->GetInputPin("Input")->id);
    graph->CreateConnection(thermalNode->GetOutputPin("Output")->id, outputNode->GetInputPin("Input")->id);

    LOG_INFO("Alps preset created");
}

void MountainPresets::CreateAppalachians(NodeGraph* graph, uint32 resolution) {
    LOG_INFO("Creating Appalachians preset...");

    // Weathered, rolling mountains - older mountain range
    auto* baseNoise = graph->CreateNode<PerlinNode>();
    baseNoise->SetPosition(glm::vec2(50, 150));
    baseNoise->width = resolution;
    baseNoise->height = resolution;
    baseNoise->params.frequency = 0.006f;  // Medium scale features
    baseNoise->params.amplitude = 1.0f;
    baseNoise->params.octaves = 5;
    baseNoise->params.lacunarity = 2.0f;   // Less variation
    baseNoise->params.persistence = 0.45f; // Softer features
    baseNoise->params.seed = 1800; // Appalachians-specific seed

    // Add gentle rolling detail
    auto* detailNoise = graph->CreateNode<PerlinNode>();
    detailNoise->SetPosition(glm::vec2(50, 300));
    detailNoise->width = resolution;
    detailNoise->height = resolution;
    detailNoise->params.frequency = 0.015f;
    detailNoise->params.amplitude = 0.3f;
    detailNoise->params.octaves = 3;
    detailNoise->params.lacunarity = 2.0f;
    detailNoise->params.persistence = 0.4f;
    detailNoise->params.seed = 1801;

    // Combine
    auto* combineNode = graph->CreateNode<AddNode>();
    combineNode->SetPosition(glm::vec2(300, 225));

    // Lower height scale - Appalachians are not as tall
    auto* scaleNode = graph->CreateNode<ScaleNode>();
    scaleNode->SetPosition(glm::vec2(500, 225));
    scaleNode->scale = 0.35f; // Much lower than Alps

    // Heavy smoothing - weathered over millions of years
    auto* smoothNode = graph->CreateNode<SmoothNode>();
    smoothNode->SetPosition(glm::vec2(700, 225));
    smoothNode->iterations = 3;
    smoothNode->strength = 0.6f;

    // Gentle thermal erosion
    auto* thermalNode = graph->CreateNode<ThermalErosionNode>();
    thermalNode->SetPosition(glm::vec2(900, 225));
    thermalNode->params.iterations = 15;
    thermalNode->params.talusAngle = 0.9f; // Gentler slopes
    thermalNode->params.strength = 0.6f;

    // Output
    auto* outputNode = graph->CreateNode<OutputNode>();
    outputNode->SetPosition(glm::vec2(1100, 225));
    graph->SetOutputNode(outputNode);

    // Connections
    graph->CreateConnection(baseNoise->GetOutputPin("Output")->id, combineNode->GetInputPin("A")->id);
    graph->CreateConnection(detailNoise->GetOutputPin("Output")->id, combineNode->GetInputPin("B")->id);
    graph->CreateConnection(combineNode->GetOutputPin("Output")->id, scaleNode->GetInputPin("Input")->id);
    graph->CreateConnection(scaleNode->GetOutputPin("Output")->id, smoothNode->GetInputPin("Input")->id);
    graph->CreateConnection(smoothNode->GetOutputPin("Output")->id, thermalNode->GetInputPin("Input")->id);
    graph->CreateConnection(thermalNode->GetOutputPin("Output")->id, outputNode->GetInputPin("Input")->id);

    LOG_INFO("Appalachians preset created");
}

void MountainPresets::CreateHimalayas(NodeGraph* graph, uint32 resolution) {
    LOG_INFO("Creating Himalayas preset...");

    // Extreme peaks - highest mountains on Earth
    auto* baseNoise = graph->CreateNode<RidgedNode>();
    baseNoise->SetPosition(glm::vec2(50, 100));
    baseNoise->width = resolution;
    baseNoise->height = resolution;
    baseNoise->frequency = 0.002f;  // Very large features
    baseNoise->amplitude = 2.0f;
    baseNoise->octaves = 9;         // High detail
    baseNoise->lacunarity = 2.5f;   // Sharp variations
    baseNoise->persistence = 0.65f;
    baseNoise->ridgeOffset = 1.5f;  // Very sharp ridges
    baseNoise->seed = 8848; // Everest height in meters

    // Secondary sharp peaks
    auto* peaks = graph->CreateNode<RidgedNode>();
    peaks->SetPosition(glm::vec2(50, 250));
    peaks->width = resolution;
    peaks->height = resolution;
    peaks->frequency = 0.004f;
    peaks->amplitude = 1.5f;
    peaks->octaves = 7;
    peaks->lacunarity = 2.4f;
    peaks->persistence = 0.7f;
    peaks->ridgeOffset = 1.3f;
    peaks->seed = 8849;

    // Jagged detail
    auto* detail = graph->CreateNode<VoronoiNode>();
    detail->SetPosition(glm::vec2(50, 400));
    detail->width = resolution;
    detail->height = resolution;
    detail->cellCount = 40;
    detail->amplitude = 0.4f;
    detail->seed = 8850;
    detail->invert = true;

    // Combine base and peaks
    auto* combineBase = graph->CreateNode<MaxNode>();
    combineBase->SetPosition(glm::vec2(300, 150));

    // Add detail
    auto* addDetail = graph->CreateNode<AddNode>();
    addDetail->SetPosition(glm::vec2(500, 200));

    // Extreme height scale
    auto* scaleNode = graph->CreateNode<ScaleNode>();
    scaleNode->SetPosition(glm::vec2(700, 200));
    scaleNode->scale = 1.5f; // Himalayas are extremely tall

    // Minimal thermal erosion - young mountains
    auto* thermalNode = graph->CreateNode<ThermalErosionNode>();
    thermalNode->SetPosition(glm::vec2(900, 200));
    thermalNode->params.iterations = 3;
    thermalNode->params.talusAngle = 0.5f; // Very steep slopes
    thermalNode->params.strength = 0.3f;

    // Output
    auto* outputNode = graph->CreateNode<OutputNode>();
    outputNode->SetPosition(glm::vec2(1100, 200));
    graph->SetOutputNode(outputNode);

    // Connections
    graph->CreateConnection(baseNoise->GetOutputPin("Output")->id, combineBase->GetInputPin("A")->id);
    graph->CreateConnection(peaks->GetOutputPin("Output")->id, combineBase->GetInputPin("B")->id);
    graph->CreateConnection(combineBase->GetOutputPin("Output")->id, addDetail->GetInputPin("A")->id);
    graph->CreateConnection(detail->GetOutputPin("Output")->id, addDetail->GetInputPin("B")->id);
    graph->CreateConnection(addDetail->GetOutputPin("Output")->id, scaleNode->GetInputPin("Input")->id);
    graph->CreateConnection(scaleNode->GetOutputPin("Output")->id, thermalNode->GetInputPin("Input")->id);
    graph->CreateConnection(thermalNode->GetOutputPin("Output")->id, outputNode->GetInputPin("Input")->id);

    LOG_INFO("Himalayas preset created");
}

void MountainPresets::CreateRockyMountains(NodeGraph* graph, uint32 resolution) {
    LOG_INFO("Creating Rocky Mountains preset...");

    // Mixed terrain - combination of sharp peaks and rolling hills
    auto* mountains = graph->CreateNode<RidgedNode>();
    mountains->SetPosition(glm::vec2(50, 100));
    mountains->width = resolution;
    mountains->height = resolution;
    mountains->frequency = 0.004f;
    mountains->amplitude = 1.3f;
    mountains->octaves = 7;
    mountains->lacunarity = 2.2f;
    mountains->persistence = 0.58f;
    mountains->ridgeOffset = 1.1f;
    mountains->seed = 4400; // ~Average height

    // Rolling hills
    auto* hills = graph->CreateNode<PerlinNode>();
    hills->SetPosition(glm::vec2(50, 250));
    hills->width = resolution;
    hills->height = resolution;
    hills->params.frequency = 0.008f;
    hills->params.amplitude = 0.8f;
    hills->params.octaves = 5;
    hills->params.lacunarity = 2.1f;
    hills->params.persistence = 0.5f;
    hills->params.seed = 4401;

    // Blend mountains and hills
    auto* blendNode = graph->CreateNode<BlendNode>();
    blendNode->SetPosition(glm::vec2(300, 175));
    blendNode->blend = 0.6f; // More mountains than hills

    // Scale to appropriate height
    auto* scaleNode = graph->CreateNode<ScaleNode>();
    scaleNode->SetPosition(glm::vec2(500, 175));
    scaleNode->scale = 0.7f;

    // Moderate thermal erosion
    auto* thermalNode = graph->CreateNode<ThermalErosionNode>();
    thermalNode->SetPosition(glm::vec2(700, 175));
    thermalNode->params.iterations = 10;
    thermalNode->params.talusAngle = 0.7f;
    thermalNode->params.strength = 0.5f;

    // Output
    auto* outputNode = graph->CreateNode<OutputNode>();
    outputNode->SetPosition(glm::vec2(900, 175));
    graph->SetOutputNode(outputNode);

    // Connections
    graph->CreateConnection(mountains->GetOutputPin("Output")->id, blendNode->GetInputPin("A")->id);
    graph->CreateConnection(hills->GetOutputPin("Output")->id, blendNode->GetInputPin("B")->id);
    graph->CreateConnection(blendNode->GetOutputPin("Output")->id, scaleNode->GetInputPin("Input")->id);
    graph->CreateConnection(scaleNode->GetOutputPin("Output")->id, thermalNode->GetInputPin("Input")->id);
    graph->CreateConnection(thermalNode->GetOutputPin("Output")->id, outputNode->GetInputPin("Input")->id);

    LOG_INFO("Rocky Mountains preset created");
}

void MountainPresets::CreateAndes(NodeGraph* graph, uint32 resolution) {
    LOG_INFO("Creating Andes preset...");

    // Long volcanic ridge - linear mountain range
    auto* baseRidge = graph->CreateNode<RidgedNode>();
    baseRidge->SetPosition(glm::vec2(50, 100));
    baseRidge->width = resolution;
    baseRidge->height = resolution;
    baseRidge->frequency = 0.003f;
    baseRidge->amplitude = 1.4f;
    baseRidge->octaves = 8;
    baseRidge->lacunarity = 2.3f;
    baseRidge->persistence = 0.6f;
    baseRidge->ridgeOffset = 1.2f;
    baseRidge->seed = 6900; // Aconcagua height

    // Volcanic peaks (using Voronoi)
    auto* volcanoes = graph->CreateNode<VoronoiNode>();
    volcanoes->SetPosition(glm::vec2(50, 250));
    volcanoes->width = resolution;
    volcanoes->height = resolution;
    volcanoes->cellCount = 25;
    volcanoes->amplitude = 0.6f;
    volcanoes->seed = 6901;
    volcanoes->invert = true; // Peaks instead of cells

    // Combine ridge and volcanoes
    auto* combineNode = graph->CreateNode<MaxNode>();
    combineNode->SetPosition(glm::vec2(300, 175));

    // High plateaus
    auto* plateaus = graph->CreateNode<TerraceNode>();
    plateaus->SetPosition(glm::vec2(500, 175));
    plateaus->steps = 8;
    plateaus->blend = 0.7f;

    // Scale to height
    auto* scaleNode = graph->CreateNode<ScaleNode>();
    scaleNode->SetPosition(glm::vec2(700, 175));
    scaleNode->scale = 1.1f; // Very tall

    // Moderate thermal erosion
    auto* thermalNode = graph->CreateNode<ThermalErosionNode>();
    thermalNode->SetPosition(glm::vec2(900, 175));
    thermalNode->params.iterations = 8;
    thermalNode->params.talusAngle = 0.65f;
    thermalNode->params.strength = 0.45f;

    // Output
    auto* outputNode = graph->CreateNode<OutputNode>();
    outputNode->SetPosition(glm::vec2(1100, 175));
    graph->SetOutputNode(outputNode);

    // Connections
    graph->CreateConnection(baseRidge->GetOutputPin("Output")->id, combineNode->GetInputPin("A")->id);
    graph->CreateConnection(volcanoes->GetOutputPin("Output")->id, combineNode->GetInputPin("B")->id);
    graph->CreateConnection(combineNode->GetOutputPin("Output")->id, plateaus->GetInputPin("Input")->id);
    graph->CreateConnection(plateaus->GetOutputPin("Output")->id, scaleNode->GetInputPin("Input")->id);
    graph->CreateConnection(scaleNode->GetOutputPin("Output")->id, thermalNode->GetInputPin("Input")->id);
    graph->CreateConnection(thermalNode->GetOutputPin("Output")->id, outputNode->GetInputPin("Input")->id);

    LOG_INFO("Andes preset created");
}

} // namespace Terrain
