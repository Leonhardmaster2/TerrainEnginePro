# Build Instructions - Terrain Engine Pro MVP

## Current Status

This is an MVP (Minimum Viable Product) implementation demonstrating the core architecture of Terrain Engine Pro. Currently implemented:

✅ CMake build system
✅ Core infrastructure (Logger)
✅ Vulkan context and device initialization
✅ Perlin noise compute shader (GLSL)
✅ Basic application framework

## Quick Build (Windows 11)

### Prerequisites

You MUST have these installed first (see [INSTALLATION.md](docs/INSTALLATION.md) for details):

1. **Visual Studio 2022** with C++ workload
2. **Vulkan SDK 1.3.x** - https://vulkan.lunarg.com/
3. **CMake 3.20+** - https://cmake.org/download/
4. **Git** - https://git-scm.com/

### Clone and Build

```cmd
# Clone the repository
git clone https://github.com/Leonhardmaster2/TerrainEnginePro.git
cd TerrainEnginePro

# Create external dependencies directory
mkdir external
cd external

# Clone ImGui (docking branch)
git clone --branch docking https://github.com/ocornut/imgui.git

# Clone Vulkan Memory Allocator
git clone https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git

# Clone ImNodes
git clone https://github.com/Nelarius/imnodes.git

# Clone stb (for image I/O)
git clone https://github.com/nothings/stb.git

# Go back to root
cd ..

# Install vcpkg dependencies
# (If you don't have vcpkg, follow INSTALLATION.md first)
vcpkg install glfw3:x64-windows glm:x64-windows rapidjson:x64-windows

# Configure with CMake
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64 ^
  -DCMAKE_TOOLCHAIN_FILE=C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake

# Build
cmake --build . --config Release

# Run
.\bin\Release\TerrainEngine.exe
```

## Expected Output

When you run the application, you should see:

```
[00:00:00.000] [INFO]  ========================================
[00:00:00.001] [INFO]  Terrain Engine Pro v1.0.0
[00:00:00.001] [INFO]  ========================================
[00:00:00.015] [INFO]  Initializing Vulkan Context...
[00:00:00.120] [INFO]  Selected GPU: NVIDIA GeForce RTX 4090
[00:00:00.125] [INFO]  Vulkan Context initialized successfully
[00:00:00.125] [INFO]  Vulkan initialized successfully!
[00:00:00.125] [INFO]  Application running...
[00:00:00.125] [INFO]  Press Enter to exit...
```

## What's Working

- ✅ Vulkan 1.3 initialization
- ✅ GPU device selection (automatically picks best GPU)
- ✅ Logging system with file output (`terrain_engine.log`)
- ✅ Perlin noise compute shader (compiled to SPIR-V)
- ✅ Basic project structure

## What's Next

The following features are designed but not yet implemented (see docs for full design):

- ⏳ Compute pipeline execution engine
- ⏳ Heightfield data structure
- ⏳ ImGui UI framework
- ⏳ 3D viewport with terrain visualization
- ⏳ Node graph editor
- ⏳ Erosion simulation
- ⏳ Texture generation
- ⏳ Import/Export (PNG, EXR, etc.)

## Development Roadmap

See [docs/IMPLEMENTATION_ROADMAP.md](docs/IMPLEMENTATION_ROADMAP.md) for the full 35-week development plan.

Current phase: **Phase 1 - Vulkan Backend** (Week 3/35)

## Troubleshooting

### "Vulkan SDK not found"

Make sure `VULKAN_SDK` environment variable is set:
```cmd
echo %VULKAN_SDK%
```

Should output something like: `C:\VulkanSDK\1.3.xxx.0`

If not, reinstall Vulkan SDK or set manually:
```cmd
set VULKAN_SDK=C:\VulkanSDK\1.3.xxx.0
```

### "vcpkg not found"

Specify the toolchain file explicitly:
```cmd
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:\Dev\vcpkg\scripts\buildsystems\vcpkg.cmake
```

### "glslangValidator not found"

glslangValidator comes with Vulkan SDK. Make sure it's in your PATH:
```cmd
glslangValidator --version
```

If not found, add Vulkan SDK bin to PATH:
```cmd
set PATH=%PATH%;%VULKAN_SDK%\Bin
```

### "No Vulkan-capable GPUs found"

Your GPU may not support Vulkan 1.3. Update your GPU drivers:
- NVIDIA: https://www.nvidia.com/drivers
- AMD: https://www.amd.com/en/support
- Intel: https://www.intel.com/content/www/us/en/download-center/home.html

Check Vulkan support:
```cmd
vulkaninfo
```

## Contributing

This is an active development project. To contribute:

1. Read the design docs in `docs/`
2. Pick a feature from the roadmap
3. Create a branch and implement
4. Submit a pull request

See [IMPLEMENTATION_ROADMAP.md](docs/IMPLEMENTATION_ROADMAP.md) for task breakdown.

## License

TBD

## Contact

- GitHub Issues: https://github.com/Leonhardmaster2/TerrainEnginePro/issues
- Documentation: [docs/](docs/)

---

**Note**: This is an MVP demonstrating the architecture. Full Gaea-like functionality will be achieved over the 35-week roadmap. See design documentation for complete feature set.
