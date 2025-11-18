# 🚀 Terrain Engine Pro - Installation Guide

Complete step-by-step installation guide for Windows 11, Windows 10, and Linux.

---

## 📋 Table of Contents

1. [System Requirements](#system-requirements)
2. [Prerequisites](#prerequisites)
3. [Installation Steps - Windows](#windows-installation)
4. [Installation Steps - Linux](#linux-installation)
5. [Building from Source](#building-from-source)
6. [Troubleshooting](#troubleshooting)
7. [First Run](#first-run)

---

## 💻 System Requirements

### Minimum Requirements
- **OS**: Windows 10 (64-bit) or newer, Ubuntu 20.04+ or equivalent
- **CPU**: Intel Core i5 / AMD Ryzen 5 or equivalent (4 cores)
- **RAM**: 8 GB
- **GPU**: OpenGL 4.5 compatible, 2 GB VRAM
- **Storage**: 500 MB free space
- **Display**: 1920x1080 resolution

### Recommended Requirements
- **OS**: Windows 11 (64-bit), Ubuntu 22.04 LTS
- **CPU**: Intel Core i7 / AMD Ryzen 7 or better (8 cores)
- **RAM**: 16 GB or more
- **GPU**: NVIDIA GTX 1060 / AMD RX 580 or better, 4+ GB VRAM
  - **With Vulkan**: NVIDIA RTX series or AMD RX 6000 series (for GPU-accelerated erosion)
- **Storage**: 2 GB free space (for projects and cache)
- **Display**: 2560x1440 or higher

### GPU Acceleration (Optional but Recommended)
- **Vulkan SDK**: Required for GPU-accelerated erosion and noise generation
- **Compute Shaders**: Vulkan 1.2 or higher
- **Driver**: Latest graphics drivers from NVIDIA/AMD/Intel

---

## 📦 Prerequisites

You'll need to install these tools before building Terrain Engine Pro:

### 1. **CMake** (Version 3.20 or higher)

#### Windows:
Download from [cmake.org/download](https://cmake.org/download/)
- Choose "Windows x64 Installer"
- During installation, select **"Add CMake to system PATH"**
- Verify installation:
  ```cmd
  cmake --version
  ```
  Should show: `cmake version 3.20.0` or higher

#### Linux (Ubuntu/Debian):
```bash
sudo apt update
sudo apt install cmake
cmake --version
```

---

### 2. **vcpkg** (C++ Package Manager)

vcpkg provides all C++ dependencies automatically.

#### Windows:
```cmd
# Clone vcpkg to C:\
cd C:\
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg

# Bootstrap vcpkg
.\bootstrap-vcpkg.bat

# Add to PATH (optional but recommended)
setx PATH "%PATH%;C:\vcpkg"
```

#### Linux:
```bash
# Clone vcpkg to home directory
cd ~
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg

# Bootstrap vcpkg
./bootstrap-vcpkg.sh

# Add to PATH
echo 'export PATH="$HOME/vcpkg:$PATH"' >> ~/.bashrc
source ~/.bashrc
```

**Verify vcpkg installation:**
```bash
vcpkg version
```

---

### 3. **Visual Studio 2022** (Windows Only)

Download **Visual Studio 2022 Community** (free): https://visualstudio.microsoft.com/downloads/

During installation, select these workloads:
- ✅ **Desktop development with C++**
- ✅ **C++ CMake tools for Windows**
- ✅ **Windows 10/11 SDK**

Individual components to add:
- ✅ MSVC v143 - VS 2022 C++ x64/x86 build tools
- ✅ C++ CMake tools for Windows
- ✅ C++ ATL for latest v143 build tools

**OR** Install Build Tools only (lighter):
Download **Build Tools for Visual Studio 2022**:
```cmd
# Same workloads as above, but no IDE
```

---

### 4. **GCC/Clang** (Linux Only)

```bash
# Ubuntu/Debian
sudo apt install build-essential g++ clang

# Fedora
sudo dnf install gcc-c++ clang

# Arch
sudo pacman -S base-devel clang
```

Verify:
```bash
g++ --version  # Should be 9.0 or higher
```

---

### 5. **Git** (Version Control)

#### Windows:
Download from [git-scm.com](https://git-scm.com/download/win)
- Use default settings during installation

#### Linux:
```bash
sudo apt install git  # Ubuntu/Debian
```

Verify:
```bash
git --version
```

---

### 6. **Vulkan SDK** (Optional - for GPU Acceleration)

Only needed if you want GPU-accelerated erosion and noise generation.

#### Windows:
1. Download from [vulkan.lunarg.com](https://vulkan.lunarg.com/sdk/home#windows)
2. Choose latest SDK (1.3.x recommended)
3. Run installer, use default options
4. Verify installation:
   ```cmd
   echo %VULKAN_SDK%
   ```
   Should show: `C:\VulkanSDK\1.3.xxx.x`

#### Linux:
```bash
# Ubuntu/Debian 22.04+
sudo apt install vulkan-sdk

# OR download from LunarG
wget https://sdk.lunarg.com/sdk/download/latest/linux/vulkan-sdk.tar.gz
tar -xzf vulkan-sdk.tar.gz
cd vulkan-sdk-*
./setup-env.sh
```

Verify:
```bash
vulkaninfo | grep "API Version"
```

---

## 🪟 Windows Installation

### Step 1: Install Dependencies with vcpkg

```cmd
cd C:\vcpkg

# Install required packages (this will take 10-30 minutes first time)
vcpkg install glfw3:x64-windows
vcpkg install glm:x64-windows
vcpkg install glad:x64-windows
vcpkg install rapidjson:x64-windows

# Integrate with Visual Studio
vcpkg integrate install
```

**Expected output:**
```
Applied user-wide integration for this vcpkg root.
All MSBuild projects can now #include any installed libraries.
```

---

### Step 2: Clone the Repository

```cmd
cd C:\Users\YourName\Documents
git clone https://github.com/YourUsername/TerrainEnginePro.git
cd TerrainEnginePro
```

---

### Step 3: Configure with CMake

```cmd
# Create build directory
mkdir build
cd build

# Configure (first time will download dependencies)
cmake .. -G "Visual Studio 17 2022" -A x64 ^
    -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake

# This takes 5-10 minutes first time
```

**What this does:**
- `-G "Visual Studio 17 2022"`: Use VS 2022 generator
- `-A x64`: Build for 64-bit
- `-DCMAKE_TOOLCHAIN_FILE`: Tell CMake to use vcpkg dependencies

---

### Step 4: Build the Project

```cmd
# Build Release version (faster, optimized)
cmake --build . --config Release

# OR Debug version (slower, for development)
cmake --build . --config Debug
```

**Build time:** 5-15 minutes depending on your CPU

**Output location:**
- Release: `build\bin\Release\TerrainEngine.exe`
- Debug: `build\bin\Debug\TerrainEngine.exe`

---

### Step 5: Run the Application

```cmd
# From build directory
cd bin\Release
TerrainEngine.exe

# OR double-click TerrainEngine.exe in File Explorer
```

---

## 🐧 Linux Installation

### Step 1: Install System Dependencies

```bash
# Ubuntu/Debian 20.04+
sudo apt update
sudo apt install -y build-essential cmake git \
    libglfw3-dev libglm-dev libgl1-mesa-dev \
    libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev

# Fedora
sudo dnf install cmake gcc-c++ glfw-devel glm-devel mesa-libGL-devel

# Arch
sudo pacman -S cmake gcc glfw-wayland glm mesa
```

---

### Step 2: Install vcpkg Dependencies

```bash
cd ~/vcpkg

# Install packages
./vcpkg install glfw3 glm glad rapidjson

# Take note of the installation path
vcpkg integrate install
```

---

### Step 3: Clone Repository

```bash
cd ~/Documents
git clone https://github.com/YourUsername/TerrainEnginePro.git
cd TerrainEnginePro
```

---

### Step 4: Build

```bash
# Create build directory
mkdir build && cd build

# Configure
cmake .. -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=~/vcpkg/scripts/buildsystems/vcpkg.cmake

# Build (use all CPU cores)
cmake --build . -j$(nproc)
```

**Build time:** 3-10 minutes

---

### Step 5: Run

```bash
# From build directory
./bin/TerrainEngine

# OR install system-wide
sudo cmake --install .
terrainengine
```

---

## 🔧 Building from Source (Advanced)

###External Dependencies (Manual Setup)

If you don't want to use vcpkg, you can build dependencies manually:

#### ImGui (included in `external/imgui/`)
```bash
# Already included, no action needed
```

#### ImNodes (included in `external/imnodes/`)
```bash
# Already included, no action needed
```

#### STB (included in `external/stb/`)
```bash
# Already included, no action needed
```

#### VulkanMemoryAllocator (included in `external/VulkanMemoryAllocator/`)
```bash
# Already included, no action needed
```

---

## 🐛 Troubleshooting

### Problem: "CMake could not find vcpkg toolchain file"

**Solution:**
```cmd
# Windows
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake

# Linux
cmake .. -DCMAKE_TOOLCHAIN_FILE=~/vcpkg/scripts/buildsystems/vcpkg.cmake
```

---

### Problem: "LINK : fatal error LNK1104: cannot open file 'glfw3.lib'"

**Solution:** vcpkg packages not installed correctly
```cmd
cd C:\vcpkg
vcpkg remove glfw3:x64-windows
vcpkg install glfw3:x64-windows
vcpkg integrate install
```

---

### Problem: "Vulkan SDK not found"

**Solution:** Vulkan is optional. To disable:
```cmd
cmake .. -DVULKAN_AVAILABLE=OFF
```

Or install Vulkan SDK and set environment variable:
```cmd
# Windows
setx VULKAN_SDK "C:\VulkanSDK\1.3.xxx.x"

# Linux
export VULKAN_SDK="/path/to/vulkan/sdk"
```

---

### Problem: "OpenGL version 4.5 not supported"

**Solution:** Update graphics drivers
- **NVIDIA**: Download from [nvidia.com/drivers](https://www.nvidia.com/drivers)
- **AMD**: Download from [amd.com/support](https://www.amd.com/support)
- **Intel**: Update via Windows Update or [intel.com/support](https://www.intel.com/support)

---

### Problem: "Application crashes on startup"

**Solutions:**
1. **Check GPU drivers:** Ensure latest drivers installed
2. **Run in Debug mode:**
   ```cmd
   cd build\bin\Debug
   TerrainEngine.exe
   # Check console output for error messages
   ```
3. **Check shaders:** Ensure `shaders/` folder exists in same directory as exe
4. **Disable Vulkan:** If Vulkan is causing issues:
   - Delete `hydraulic_erosion.comp.spv` from shaders folder
   - App will fall back to CPU erosion

---

### Problem: "Missing DLL errors on Windows"

**Solution:** Copy DLLs to exe directory
```cmd
cd build\bin\Release

# Copy vcpkg DLLs
copy C:\vcpkg\installed\x64-windows\bin\*.dll .

# OR use vcpkg deploy
cd ..\..\..
vcpkg integrate project
```

---

## 🎯 First Run

### What to Expect

1. **Welcome Window**: Shows on first launch
2. **3D Viewport**: Empty gray viewport (no terrain yet)
3. **Node Graph Editor**: Centered panel with "Add Node" menu
4. **Parameters Panel** (classic mode): Left sidebar
5. **Stats Panel**: Bottom-right corner

### Quick Start Tutorial

#### Classic Mode (Simple):
1. Click **File → Generate** in menu bar
2. Adjust sliders in Parameters panel
3. Click **Generate Terrain** button
4. View result in 3D Viewport
5. Export via **File → Export Heightmap**

#### Node Graph Mode (Advanced):
1. Enable: **View → Node Graph**
2. Right-click in graph → **Add Node → Generators → Perlin Noise**
3. Right-click again → **Add Node → Output → Output**
4. Drag from Perlin Noise output to Output input
5. Select Perlin node → adjust parameters in Inspector
6. Click **Execute Graph** in menu
7. View result in 3D Viewport

### Example Workflow
```
Perlin Noise (512x512, freq=0.01)
  ↓
Hydraulic Erosion (1000 iterations)
  ↓
Thermal Erosion (500 iterations)
  ↓
Output
```

---

## 📁 Project Structure

```
TerrainEnginePro/
├── build/                 # Build files (generated)
│   ├── bin/              # Executables
│   │   ├── Release/
│   │   │   └── TerrainEngine.exe
│   │   └── Debug/
│   └── shaders/          # Compiled shaders (auto-copied)
├── src/                  # Source code
│   ├── Core/            # Core utilities
│   ├── GPU/             # Vulkan compute
│   ├── Nodes/           # Node graph system
│   ├── Rendering/       # OpenGL rendering
│   ├── Terrain/         # Terrain generation
│   ├── Texture/         # Texture generation
│   ├── UI/              # ImGui interface
│   └── main.cpp
├── shaders/             # GLSL shaders (source)
├── external/            # Third-party libraries
├── assets/              # Icons, fonts (optional)
├── examples/            # Example terrain graphs
└── CMakeLists.txt       # Build configuration
```

---

## 🔄 Updating

To update to the latest version:

```bash
cd TerrainEnginePro
git pull origin main

# Rebuild
cd build
cmake --build . --config Release
```

---

## 📞 Support

### Common Issues
- Check [Troubleshooting](#troubleshooting) section above
- Check GitHub Issues: `https://github.com/YourUsername/TerrainEnginePro/issues`

### Reporting Bugs
Include this information:
- OS version (Windows 11, Ubuntu 22.04, etc.)
- GPU model and driver version
- CMake version
- Build configuration (Release/Debug)
- Error messages from console

### Community
- Discord: [Link here]
- Forum: [Link here]
- Wiki: [Link here]

---

## 📄 License

MIT License - See LICENSE file for details

---

## 🎉 You're Ready!

Your Terrain Engine Pro installation is complete. Start creating amazing terrains!

**Next Steps:**
- Read [USER_GUIDE.md](USER_GUIDE.md) for detailed feature documentation
- Try example graphs in `examples/` folder
- Watch video tutorials: [Link]

---

**Last Updated:** November 2025
**Version:** 1.0.0
**Minimum CMake:** 3.20
**Supported Compilers:** MSVC 2022, GCC 9+, Clang 10+
