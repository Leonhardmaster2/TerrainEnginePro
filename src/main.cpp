#include "Core/Logger.h"
#include "Core/Types.h"
#include "Terrain/TerrainGenerator.h"

#include <iostream>
#include <chrono>

using namespace Terrain;

void PrintUsage() {
    std::cout << "\n";
    std::cout << "=================================================\n";
    std::cout << "  Terrain Engine Pro v1.0.0 - CLI Generator\n";
    std::cout << "=================================================\n";
    std::cout << "\n";
    std::cout << "Usage: TerrainEngine [options]\n";
    std::cout << "\n";
    std::cout << "Options:\n";
    std::cout << "  --size <width>       Terrain resolution (default: 1024)\n";
    std::cout << "  --frequency <f>      Noise frequency (default: 1.0)\n";
    std::cout << "  --amplitude <a>      Noise amplitude (default: 1.0)\n";
    std::cout << "  --octaves <n>        Number of octaves (default: 6)\n";
    std::cout << "  --seed <s>           Random seed (default: 12345)\n";
    std::cout << "  --output <file>      Output filename (default: terrain.png)\n";
    std::cout << "  --format <fmt>       Format: png, raw (default: png)\n";
    std::cout << "\n";
    std::cout << "Examples:\n";
    std::cout << "  TerrainEngine --size 2048 --output mountain.png\n";
    std::cout << "  TerrainEngine --frequency 2.0 --octaves 8 --seed 42\n";
    std::cout << "  TerrainEngine --size 4096 --format raw --output terrain.raw\n";
    std::cout << "\n";
}

int main(int argc, char** argv) {
    // Initialize logger
    Logger::Get().SetLogLevel(LogLevel::Info);
    Logger::Get().SetLogFile("terrain_engine.log");

    LOG_INFO("========================================");
    LOG_INFO("Terrain Engine Pro v1.0.0");
    LOG_INFO("========================================");

    // Parse command line arguments
    uint32 size = 1024;
    PerlinParams params;
    String outputFile = "terrain.png";
    String format = "png";

    for (int i = 1; i < argc; i++) {
        String arg = argv[i];

        if (arg == "--help" || arg == "-h") {
            PrintUsage();
            return 0;
        }
        else if (arg == "--size" && i + 1 < argc) {
            size = std::atoi(argv[++i]);
        }
        else if (arg == "--frequency" && i + 1 < argc) {
            params.frequency = std::atof(argv[++i]);
        }
        else if (arg == "--amplitude" && i + 1 < argc) {
            params.amplitude = std::atof(argv[++i]);
        }
        else if (arg == "--octaves" && i + 1 < argc) {
            params.octaves = std::atoi(argv[++i]);
        }
        else if (arg == "--seed" && i + 1 < argc) {
            params.seed = std::atoi(argv[++i]);
        }
        else if (arg == "--output" && i + 1 < argc) {
            outputFile = argv[++i];
        }
        else if (arg == "--format" && i + 1 < argc) {
            format = argv[++i];
        }
    }

    // Print configuration
    LOG_INFO("Configuration:");
    LOG_INFO("  Resolution: %dx%d", size, size);
    LOG_INFO("  Frequency: %.2f", params.frequency);
    LOG_INFO("  Amplitude: %.2f", params.amplitude);
    LOG_INFO("  Octaves: %d", params.octaves);
    LOG_INFO("  Lacunarity: %.2f", params.lacunarity);
    LOG_INFO("  Persistence: %.2f", params.persistence);
    LOG_INFO("  Seed: %u", params.seed);
    LOG_INFO("  Output: %s", outputFile.c_str());
    LOG_INFO("  Format: %s", format.c_str());

    // Create terrain generator
    TerrainGenerator generator;
    if (!generator.Initialize()) {
        LOG_ERROR("Failed to initialize terrain generator");
        return -1;
    }

    // Generate terrain
    LOG_INFO("Generating terrain...");
    auto startTime = std::chrono::high_resolution_clock::now();

    auto heightfield = generator.GeneratePerlin(size, size, params);

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    LOG_INFO("Generation completed in %lld ms", duration.count());

    // Get terrain stats
    float minHeight = heightfield->GetMin();
    float maxHeight = heightfield->GetMax();
    LOG_INFO("Terrain statistics:");
    LOG_INFO("  Min height: %.3f", minHeight);
    LOG_INFO("  Max height: %.3f", maxHeight);

    // Export
    bool success = false;
    if (format == "png") {
        success = generator.ExportPNG(*heightfield, outputFile, true);
    } else if (format == "raw") {
        success = generator.ExportRAW(*heightfield, outputFile);
    } else {
        LOG_ERROR("Unknown format: %s", format.c_str());
    }

    if (success) {
        LOG_INFO("Terrain exported successfully to: %s", outputFile.c_str());
        std::cout << "\n";
        std::cout << "SUCCESS! Terrain generated and saved to: " << outputFile << "\n";
        std::cout << "Resolution: " << size << "x" << size << "\n";
        std::cout << "Generation time: " << duration.count() << " ms\n";
        std::cout << "\n";
    } else {
        LOG_ERROR("Failed to export terrain");
        return -1;
    }

    // Cleanup
    generator.Shutdown();

    LOG_INFO("Application shutting down");
    return 0;
}
