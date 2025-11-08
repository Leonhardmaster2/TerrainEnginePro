cmake_minimum_required(VERSION 3.20)
project(TerrainEngine VERSION 1.0.0 LANGUAGES CXX)

# C++20 Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
if(MSVC)
    add_compile_options(/W4 /WX- /MP)
    add_compile_options($<$<CONFIG:Release>:/O2 /GL /arch:AVX2>)
    add_compile_options($<$<CONFIG:Debug>:/Od /Zi>)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
    add_compile_options($<$<CONFIG:Release>:-O3 -march=native>)
    add_compile_options($<$<CONFIG:Debug>:-g>)
endif()

# Find Vulkan (optional - required for GPU compute)
find_package(Vulkan)

# Find OpenGL
find_package(OpenGL REQUIRED)

# vcpkg packages
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(RapidJSON CONFIG)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/external
    ${Vulkan_INCLUDE_DIRS}
)

# External dependencies (header-only or manual)
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/external/imgui)
set(VMA_DIR ${CMAKE_SOURCE_DIR}/external/VulkanMemoryAllocator)
set(IMNODES_DIR ${CMAKE_SOURCE_DIR}/external/imnodes)
set(STB_DIR ${CMAKE_SOURCE_DIR}/external/stb)

# ImGui sources
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# ImNodes sources
set(IMNODES_SOURCES
    ${IMNODES_DIR}/imnodes.cpp
)

# Engine source files
file(GLOB_RECURSE ENGINE_SOURCES
    ${CMAKE_SOURCE_DIR}/src/Core/*.cpp
    ${CMAKE_SOURCE_DIR}/src/GPU/*.cpp
    ${CMAKE_SOURCE_DIR}/src/Terrain/*.cpp
    ${CMAKE_SOURCE_DIR}/src/Rendering/*.cpp
    ${CMAKE_SOURCE_DIR}/src/Nodes/*.cpp
    ${CMAKE_SOURCE_DIR}/src/Erosion/*.cpp
    ${CMAKE_SOURCE_DIR}/src/Texture/*.cpp
    ${CMAKE_SOURCE_DIR}/src/IO/*.cpp
    ${CMAKE_SOURCE_DIR}/src/UI/*.cpp
)

# Main executable
add_executable(TerrainEngine
    src/main.cpp
    ${ENGINE_SOURCES}
    ${IMGUI_SOURCES}
    ${IMNODES_SOURCES}
)

# Link libraries
target_link_libraries(TerrainEngine
    PRIVATE
        OpenGL::GL
        glfw
        glm::glm
        glad::glad
)

# Add Vulkan if available
if(Vulkan_FOUND)
    target_link_libraries(TerrainEngine PRIVATE Vulkan::Vulkan)
    target_compile_definitions(TerrainEngine PRIVATE VULKAN_AVAILABLE)
endif()

# Include ImGui and VMA
target_include_directories(TerrainEngine PRIVATE
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${VMA_DIR}/include
    ${IMNODES_DIR}
    ${STB_DIR}
)

# Copy shaders to build directory
file(GLOB_RECURSE COMPUTE_SHADER_SOURCES
    ${CMAKE_SOURCE_DIR}/shaders/*.comp
)

# Copy OpenGL shaders (no compilation needed)
file(GLOB_RECURSE OPENGL_SHADER_SOURCES
    ${CMAKE_SOURCE_DIR}/shaders/*.vert
    ${CMAKE_SOURCE_DIR}/shaders/*.frag
    ${CMAKE_SOURCE_DIR}/shaders/*.glsl
)

# Compile Vulkan compute shaders to SPIR-V
find_program(GLSL_VALIDATOR glslangValidator HINTS $ENV{VULKAN_SDK}/Bin)

if(GLSL_VALIDATOR)
    foreach(SHADER ${COMPUTE_SHADER_SOURCES})
        get_filename_component(SHADER_NAME ${SHADER} NAME)
        set(SPIRV_OUTPUT ${CMAKE_BINARY_DIR}/shaders/${SHADER_NAME}.spv)

        add_custom_command(
            OUTPUT ${SPIRV_OUTPUT}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/shaders
            COMMAND ${GLSL_VALIDATOR} -V ${SHADER} -o ${SPIRV_OUTPUT}
            DEPENDS ${SHADER}
            COMMENT "Compiling compute shader: ${SHADER_NAME}"
        )

        list(APPEND SPIRV_SHADERS ${SPIRV_OUTPUT})
    endforeach()

    add_custom_target(ComputeShaders ALL DEPENDS ${SPIRV_SHADERS})
    add_dependencies(TerrainEngine ComputeShaders)
else()
    message(WARNING "glslangValidator not found - compute shaders will not be compiled")
endif()

# Copy OpenGL shaders to build directory (no compilation needed)
foreach(SHADER ${OPENGL_SHADER_SOURCES})
    get_filename_component(SHADER_NAME ${SHADER} NAME)
    set(SHADER_OUTPUT ${CMAKE_BINARY_DIR}/shaders/${SHADER_NAME})

    add_custom_command(
        OUTPUT ${SHADER_OUTPUT}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/shaders
        COMMAND ${CMAKE_COMMAND} -E copy ${SHADER} ${SHADER_OUTPUT}
        DEPENDS ${SHADER}
        COMMENT "Copying OpenGL shader: ${SHADER_NAME}"
    )

    list(APPEND OPENGL_SHADERS ${SHADER_OUTPUT})
endforeach()

add_custom_target(OpenGLShaders ALL DEPENDS ${OPENGL_SHADERS})
add_dependencies(TerrainEngine OpenGLShaders)

# Copy resources to build directory
add_custom_command(TARGET TerrainEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:TerrainEngine>/assets
)

# Installation
install(TARGETS TerrainEngine RUNTIME DESTINATION bin)
install(DIRECTORY ${CMAKE_BINARY_DIR}/shaders DESTINATION bin)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/assets DESTINATION bin)
