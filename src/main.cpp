#include "Core/Logger.h"
#include "Core/Types.h"
#include "GPU/VulkanContext.h"

#include <iostream>

using namespace Terrain;

int main(int argc, char** argv) {
    // Initialize logger
    Logger::Get().SetLogLevel(LogLevel::Debug);
    Logger::Get().SetLogFile("terrain_engine.log");

    LOG_INFO("========================================");
    LOG_INFO("Terrain Engine Pro v1.0.0");
    LOG_INFO("========================================");

    // Create Vulkan context
    VulkanContext vulkanContext;
    if (!vulkanContext.Initialize(true)) {
        LOG_ERROR("Failed to initialize Vulkan");
        return -1;
    }

    LOG_INFO("Vulkan initialized successfully!");
    LOG_INFO("Application running...");

    // Main loop placeholder
    LOG_INFO("Press Enter to exit...");
    std::cin.get();

    // Cleanup
    vulkanContext.Shutdown();

    LOG_INFO("Application shutting down");
    return 0;
}
