# Terrain Engine Pro - Installation Guide

## Overview

This guide will walk you through setting up the complete development environment for Terrain Engine Pro on Windows 11, from installing prerequisites to building and running the application.

---

## System Requirements

### Minimum Requirements

- **OS**: Windows 11 22H2 or newer
- **CPU**: Intel Core i5-10600K / AMD Ryzen 5 3600 or equivalent
- **RAM**: 16 GB
- **GPU**: NVIDIA GTX 1660 / AMD RX 5600 XT (Vulkan 1.3 support required)
- **VRAM**: 4 GB
- **Storage**: 10 GB free space (5 GB for dependencies, 5 GB for build artifacts)
- **Internet**: Required for downloading dependencies

### Recommended Requirements

- **OS**: Windows 11 23H2 or newer
- **CPU**: Intel Core i9-13900K / AMD Ryzen 9 7950X
- **RAM**: 32 GB DDR5
- **GPU**: NVIDIA RTX 4080 / AMD RX 7900 XT
- **VRAM**: 12+ GB
- **Storage**: 20 GB free space on NVMe SSD (Gen 4+)

---

## Installation Steps

### Step 1: Install Visual Studio 2022

Visual Studio is required for compiling C++20 code and provides the MSVC compiler.

#### Download and Install

1. Download **Visual Studio 2022 Community** (free) from:
   ```
   https://visualstudio.microsoft.com/downloads/
   ```

2. Run the installer and select the following workloads:
   - ✅ **Desktop development with C++**

3. In the "Individual components" tab, also select:
   - ✅ **C++ CMake tools for Windows**
   - ✅ **C++ Clang tools for Windows** (optional, for static analysis)
   - ✅ **Windows 11 SDK (10.0.22621.0 or newer)**

4. Click **Install** and wait for completion (~10-15 GB download)

5. Restart your computer after installation

#### Verify Installation

Open **Developer Command Prompt for VS 2022** and run:
```cmd
cl
cmake --version
```

You should see the MSVC compiler version and CMake version.

---

### Step 2: Install Git

Git is required for cloning the repository and version control.

#### Download and Install

1. Download **Git for Windows** from:
   ```
   https://git-scm.com/download/win
   ```

2. Run the installer with these recommended settings:
   - Editor: **Visual Studio Code** (or your preferred editor)
   - Path environment: **Git from the command line and also from 3rd-party software**
   - HTTPS backend: **OpenSSL**
   - Line ending: **Checkout Windows-style, commit Unix-style**
   - Terminal emulator: **Use Windows' default console window**

3. Complete installation

#### Verify Installation

Open Command Prompt and run:
```cmd
git --version
```

Should output: `git version 2.x.x`

---

### Step 3: Install Vulkan SDK

The Vulkan SDK provides the Vulkan runtime, headers, and shader compiler tools.

#### Download and Install

1. Download **Vulkan SDK** from:
   ```
   https://vulkan.lunarg.com/sdk/home#windows
   ```

   Get the latest version (1.3.x or newer)

2. Run the installer with these components:
   - ✅ **Vulkan SDK Core** (required)
   - ✅ **Shader Toolchain** (glslangValidator, spirv-tools)
   - ✅ **Vulkan Runtime** (required)
   - ✅ **Validation Layers** (required for debugging)
   - ✅ **SPIR-V Tools** (required)
   - ⬜ **Demos and Examples** (optional)

3. Install to default location: `C:\VulkanSDK\<version>\`

4. The installer will set environment variables automatically

#### Verify Installation

Open Command Prompt and run:
```cmd
echo %VULKAN_SDK%
glslangValidator --version
vulkaninfo
```

You should see:
- Vulkan SDK path
- glslangValidator version
- Detailed information about your GPU's Vulkan capabilities

**Important**: If `vulkaninfo` shows an error, your GPU drivers may not support Vulkan 1.3. Update your GPU drivers:
- **NVIDIA**: https://www.nvidia.com/Download/index.aspx
- **AMD**: https://www.amd.com/en/support
- **Intel**: https://www.intel.com/content/www/us/en/download-center/home.html

---

### Step 4: Install CMake

CMake is the build system generator for the project.

#### Download and Install

1. Download **CMake** from:
   ```
   https://cmake.org/download/
   ```

   Get the Windows x64 Installer (version 3.20 or newer)

2. Run the installer:
   - ✅ **Add CMake to the system PATH for all users**

3. Complete installation

#### Verify Installation

Open Command Prompt and run:
```cmd
cmake --version
```

Should output: `cmake version 3.x.x`

---

### Step 5: Install Python (Optional, for Scripting API)

Python is optional but recommended for using the scripting API in later phases.

#### Download and Install

1. Download **Python 3.10+** from:
   ```
   https://www.python.org/downloads/
   ```

2. Run the installer:
   - ✅ **Add Python to PATH**
   - ✅ **Install pip**

3. Complete installation

#### Verify Installation

```cmd
python --version
pip --version
```

---

### Step 6: Clone the Repository

Now that prerequisites are installed, clone the Terrain Engine repository.

#### Clone via HTTPS

1. Open Command Prompt or PowerShell

2. Navigate to where you want the project:
   ```cmd
   cd C:\Dev
   ```

3. Clone the repository:
   ```cmd
   git clone https://github.com/Leonhardmaster2/TerrainEnginePro.git
   ```

4. Navigate into the project:
   ```cmd
   cd TerrainEnginePro
   ```

5. Switch to the design branch (if needed):
   ```cmd
   git checkout claude/design-app-from-idea-011CUvTroxJbuNmi5DFi5NJ2
   ```

#### Clone via SSH (Advanced)

If you have SSH keys set up:
```cmd
git clone git@github.com:Leonhardmaster2/TerrainEnginePro.git
```

---

### Step 7: Install Project Dependencies

The project uses several third-party libraries. We'll use vcpkg for dependency management.

#### Install vcpkg

1. Navigate to a suitable location:
   ```cmd
   cd C:\Dev
   ```

2. Clone vcpkg:
   ```cmd
   git clone https://github.com/microsoft/vcpkg.git
   cd vcpkg
   ```

3. Bootstrap vcpkg:
   ```cmd
   .\bootstrap-vcpkg.bat
   ```

4. Integrate with Visual Studio:
   ```cmd
   .\vcpkg integrate install
   ```

5. Set environment variable (add to System Environment Variables):
   - Variable: `VCPKG_ROOT`
   - Value: `C:\Dev\vcpkg`

#### Install Dependencies

From the vcpkg directory, install required packages:

```cmd
# Core dependencies
.\vcpkg install glfw3:x64-windows
.\vcpkg install glm:x64-windows
.\vcpkg install rapidjson:x64-windows
.\vcpkg install stb:x64-windows

# Optional dependencies (recommended)
.\vcpkg install gdal:x64-windows
.\vcpkg install tinyexr:x64-windows
.\vcpkg install gtest:x64-windows
```

**Note**: GDAL installation may take 30-60 minutes due to its many dependencies.

#### Manual Dependencies (ImGui, VMA, ImNodes)

Some dependencies need to be manually added as git submodules or downloaded:

1. Navigate to your project directory:
   ```cmd
   cd C:\Dev\TerrainEnginePro
   ```

2. Create `external` directory:
   ```cmd
   mkdir external
   cd external
   ```

3. Clone ImGui (docking branch):
   ```cmd
   git clone --branch docking https://github.com/ocornut/imgui.git
   ```

4. Clone VulkanMemoryAllocator:
   ```cmd
   git clone https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
   ```

5. Clone ImNodes:
   ```cmd
   git clone https://github.com/Nelarius/imnodes.git
   ```

6. Clone imnodes (alternative, for node graph):
   ```cmd
   git clone https://github.com/thedmd/imgui-node-editor.git
   ```

---

### Step 8: Configure the Project with CMake

Now configure the build system.

#### Create Build Directory

```cmd
cd C:\Dev\TerrainEnginePro
mkdir build
cd build
```

#### Configure with CMake

```cmd
cmake .. -G "Visual Studio 17 2022" -A x64 ^
  -DCMAKE_TOOLCHAIN_FILE=C:\Dev\vcpkg\scripts\buildsystems\vcpkg.cmake ^
  -DVULKAN_SDK=%VULKAN_SDK% ^
  -DCMAKE_BUILD_TYPE=Debug
```

**Note**: Adjust paths if you installed vcpkg or Vulkan SDK elsewhere.

#### Expected Output

You should see:
```
-- The C compiler identification is MSVC 19.x.x
-- The CXX compiler identification is MSVC 19.x.x
-- Configuring done
-- Generating done
-- Build files have been written to: C:/Dev/TerrainEnginePro/build
```

---

### Step 9: Build the Project

#### Build with CMake

```cmd
cmake --build . --config Debug
```

For Release build:
```cmd
cmake --build . --config Release
```

#### Build with Visual Studio

Alternatively, open the generated solution file:
```cmd
start TerrainEngine.sln
```

In Visual Studio:
1. Set build configuration to **Debug** or **Release**
2. Press **Ctrl+Shift+B** to build
3. Wait for compilation to complete

#### Expected Build Time

- **Debug build**: 2-5 minutes (first time)
- **Release build**: 3-7 minutes (first time)
- **Incremental builds**: 10-30 seconds

---

### Step 10: Run the Application

#### From Command Line

```cmd
cd C:\Dev\TerrainEnginePro\build\Debug
.\TerrainEngine.exe
```

#### From Visual Studio

1. Open `TerrainEngine.sln`
2. Right-click on **TerrainEngine** project → **Set as Startup Project**
3. Press **F5** to run with debugger
4. Or **Ctrl+F5** to run without debugger

#### Expected First Run

On first launch, you should see:
- Main window with dark theme UI
- Node library on the left
- 3D viewport in the center
- Property inspector on the right
- Node graph editor at the bottom
- Status bar showing GPU information

---

## Development Environment Setup

### Recommended Tools

#### 1. Visual Studio Code (Optional)

For lightweight editing and CMake integration:

```cmd
# Install from:
https://code.visualstudio.com/
```

**Recommended Extensions**:
- C/C++ (Microsoft)
- CMake Tools
- GLSL Lint
- GitLens

#### 2. RenderDoc (GPU Debugging)

For debugging Vulkan rendering:

```cmd
# Download from:
https://renderdoc.org/builds
```

Install and set as Vulkan layer in environment variables.

#### 3. NVIDIA Nsight Graphics (NVIDIA GPUs)

For advanced GPU profiling:

```cmd
# Download from:
https://developer.nvidia.com/nsight-graphics
```

#### 4. AMD Radeon GPU Profiler (AMD GPUs)

For AMD GPU profiling:

```cmd
# Download from:
https://gpuopen.com/rgp/
```

---

## Configuration

### User Configuration File

On first run, the application creates a config file at:
```
%APPDATA%\TerrainEngine\config.ini
```

#### Example Configuration

```ini
[GPU]
EnableHardwareScheduling=true
PreferDedicatedGPU=true
MaxVRAMUsageMB=8192

[CPU]
ThreadPoolSize=0  ; 0 = auto-detect
EnableLargePages=false  ; Requires admin privileges
PreferPerformanceCores=true

[Performance]
PreviewResolution=512
RealtimeUpdateRate=60
MaxUndoSteps=100

[UI]
Theme=Dark
UIScale=1.0
ShowFPS=true
```

### Large Page Support (Optional)

For ~20% better memory performance:

1. Run `gpedit.msc` (Local Group Policy Editor)
2. Navigate to: **Computer Configuration** → **Windows Settings** → **Security Settings** → **Local Policies** → **User Rights Assignment**
3. Open **Lock pages in memory**
4. Add your user account
5. Restart computer
6. Set `EnableLargePages=true` in config

---

## Troubleshooting

### Common Issues

#### Issue: CMake can't find Vulkan SDK

**Solution**:
```cmd
set VULKAN_SDK=C:\VulkanSDK\1.3.xxx.0
cmake .. -DVULKAN_SDK=%VULKAN_SDK%
```

#### Issue: "vcpkg.cmake not found"

**Solution**:
```cmd
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:\Dev\vcpkg\scripts\buildsystems\vcpkg.cmake
```

Adjust path to your vcpkg installation.

#### Issue: MSVC compiler not found

**Solution**:
- Open **Developer Command Prompt for VS 2022** (not regular Command Prompt)
- Or run: `"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"`

#### Issue: Vulkan device not found

**Solution**:
1. Update GPU drivers to latest version
2. Verify Vulkan support: `vulkaninfo`
3. Check GPU supports Vulkan 1.3:
   - NVIDIA: GTX 900 series or newer
   - AMD: RX 400 series or newer
   - Intel: Arc series

#### Issue: Application crashes on startup

**Solution**:
1. Check config file for invalid settings
2. Delete `%APPDATA%\TerrainEngine\config.ini` to reset
3. Run in Debug mode and check logs at `%APPDATA%\TerrainEngine\logs\`
4. Ensure validation layers are installed: `VK_LAYER_KHRONOS_validation`

#### Issue: "Out of Memory" errors

**Solution**:
1. Reduce `PreviewResolution` in config
2. Lower `MaxCacheSizeMB`
3. Close other GPU-intensive applications
4. Upgrade RAM/VRAM if below minimum requirements

#### Issue: Shader compilation errors

**Solution**:
1. Verify `glslangValidator` is in PATH: `glslangValidator --version`
2. Check Vulkan SDK is properly installed
3. Reinstall Vulkan SDK if needed

---

## Updating Dependencies

### Update vcpkg Packages

```cmd
cd C:\Dev\vcpkg
git pull
.\vcpkg upgrade --no-dry-run
```

### Update Git Submodules

```cmd
cd C:\Dev\TerrainEnginePro
git submodule update --remote --merge
```

### Update Vulkan SDK

1. Download latest SDK from LunarG
2. Run installer
3. Update `VULKAN_SDK` environment variable
4. Rebuild project

---

## Performance Validation

### Run Benchmark (Future)

Once implemented:
```cmd
.\TerrainEngine.exe --benchmark
```

Expected results on RTX 4090:
- Perlin 4K: < 5ms ✅
- Erosion 4K (500 iter): < 3s ✅
- Full graph (10 nodes): < 100ms ✅

### GPU Profiling

#### With RenderDoc

1. Launch RenderDoc
2. Set executable to `TerrainEngine.exe`
3. Click **Launch**
4. Press **F12** to capture frame
5. Analyze GPU commands

#### With Nsight

1. Launch Nsight Graphics
2. Open **Connection** tab
3. Browse to `TerrainEngine.exe`
4. Click **Launch**
5. Use **Range Profiler** for detailed timing

---

## Building for Release

### Release Build

```cmd
cd C:\Dev\TerrainEnginePro\build
cmake --build . --config Release
```

### Optimization Flags

The CMake configuration automatically enables:
- `/O2` - Maximum optimization
- `/GL` - Whole program optimization
- `/arch:AVX2` - AVX2 SIMD instructions (if CPU supports)
- `/fp:fast` - Fast floating-point math

### Creating Installer (Future Phase 8)

Once installer is implemented:
```cmd
cd build
cpack -G NSIS -C Release
```

Output: `TerrainEngine-1.0.0-win64.exe`

---

## Development Workflow

### Recommended Workflow

1. **Edit Code**: Use Visual Studio or VS Code
2. **Build**: `cmake --build . --config Debug`
3. **Run**: `.\build\Debug\TerrainEngine.exe`
4. **Debug**: Run in Visual Studio with F5
5. **Profile**: Use RenderDoc/Nsight for GPU analysis
6. **Test**: `ctest --output-on-failure`
7. **Commit**: `git add . && git commit -m "message"`

### Hot-Reload Shaders

The application supports shader hot-reload:
1. Edit `.comp` files in `shaders/` directory
2. Save the file
3. Application automatically recompiles and reloads
4. No need to restart the application

---

## Environment Variables Reference

| Variable | Purpose | Example |
|----------|---------|---------|
| `VULKAN_SDK` | Vulkan SDK path | `C:\VulkanSDK\1.3.xxx.0` |
| `VCPKG_ROOT` | vcpkg installation | `C:\Dev\vcpkg` |
| `PATH` | Executable search path | Includes CMake, Git, Python |

---

## Next Steps

### After Successful Installation

1. **Read Documentation**:
   - `docs/README.md` - Documentation index
   - `docs/ARCHITECTURE.md` - System overview

2. **Try Example Projects** (when available):
   - Basic terrain generation
   - Erosion simulation
   - Texture export workflow

3. **Start Development** (if contributing):
   - Review `docs/IMPLEMENTATION_ROADMAP.md`
   - Check open issues on GitHub
   - Set up development branch

4. **Join Community** (when available):
   - Discord/Forum for discussions
   - Report bugs on GitHub Issues
   - Contribute improvements via Pull Requests

---

## Uninstallation

### Remove Application

1. Delete project directory:
   ```cmd
   rmdir /s /q C:\Dev\TerrainEnginePro
   ```

2. Delete user data:
   ```cmd
   rmdir /s /q %APPDATA%\TerrainEngine
   ```

### Remove Dependencies (Optional)

- **vcpkg**: Delete `C:\Dev\vcpkg`
- **Vulkan SDK**: Use Windows "Add or Remove Programs"
- **Visual Studio**: Use Visual Studio Installer to uninstall
- **CMake**: Use Windows "Add or Remove Programs"

---

## Getting Help

### Resources

- **Documentation**: `docs/` directory
- **GitHub Issues**: https://github.com/Leonhardmaster2/TerrainEnginePro/issues
- **Vulkan Tutorial**: https://vulkan-tutorial.com/
- **ImGui Documentation**: https://github.com/ocornut/imgui

### Reporting Issues

When reporting installation issues, include:
1. Windows version (`winver`)
2. GPU model and driver version
3. Vulkan SDK version (`echo %VULKAN_SDK%`)
4. CMake output (full log)
5. Build error messages (full text)

### Community Support

- **Stack Overflow**: Tag `terrain-engine` or `vulkan`
- **Vulkan Discord**: https://discord.gg/vulkan
- **Graphics Programming Discord**: Various communities

---

## Appendix: Manual Build (Without vcpkg)

If you prefer not to use vcpkg, you can manually download and build dependencies.

### Required Manual Downloads

1. **GLFW**: https://www.glfw.org/download.html
2. **GLM**: https://github.com/g-truc/glm/releases
3. **RapidJSON**: https://github.com/Tencent/rapidjson/releases
4. **stb**: https://github.com/nothings/stb (header-only)

### Configure CMake with Manual Dependencies

```cmd
cmake .. ^
  -DGLFW_DIR=C:\libs\glfw ^
  -DGLM_DIR=C:\libs\glm ^
  -DRapidJSON_DIR=C:\libs\rapidjson
```

---

## Version Information

**Document Version**: 1.0
**Last Updated**: 2025-11-08
**Compatible With**: Terrain Engine Pro v1.0 (in development)

---

**Congratulations!** You should now have a fully functional development environment for Terrain Engine Pro. Happy terrain creating! 🏔️
