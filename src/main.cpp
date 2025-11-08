#include "Core/Logger.h"
#include "Core/Types.h"
#include "UI/Application.h"

using namespace Terrain;

int main(int argc, char** argv) {
    // Initialize logger
    Logger::Get().SetLogLevel(LogLevel::Info);
    Logger::Get().SetLogFile("terrain_engine.log");

    LOG_INFO("========================================");
    LOG_INFO("Terrain Engine Pro v0.3 - Editor");
    LOG_INFO("========================================");

    // Create and initialize application
    Application app;
    if (!app.Initialize()) {
        LOG_ERROR("Failed to initialize application");
        return -1;
    }

    // Run main loop
    app.Run();

    // Cleanup
    app.Shutdown();

    LOG_INFO("Application shutting down");
    return 0;
}
