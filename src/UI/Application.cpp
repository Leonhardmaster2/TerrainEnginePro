#include "Application.h"
#include "TerrainEditor.h"
#include "Core/Logger.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

namespace Terrain {

// GLFW callbacks
static void GLFWErrorCallback(int error, const char* description) {
    LOG_ERROR("GLFW Error %d: %s", error, description);
}

static void FramebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    if (app) {
        // Update app dimensions
    }
}

Application::Application() {
}

Application::~Application() {
    Shutdown();
}

bool Application::Initialize() {
    LOG_INFO("Initializing Terrain Engine Editor...");

    // Initialize GLFW
    if (!InitializeGLFW()) {
        return false;
    }

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        LOG_ERROR("Failed to initialize GLAD");
        return false;
    }

    LOG_INFO("OpenGL %s", glGetString(GL_VERSION));
    LOG_INFO("GLSL %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

    // OpenGL settings
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // Initialize ImGui
    InitializeImGui();

    // Create editor
    m_Editor = MakeUnique<TerrainEditor>();
    if (!m_Editor->Initialize()) {
        LOG_ERROR("Failed to initialize terrain editor");
        return false;
    }

    LOG_INFO("Terrain Engine Editor initialized successfully");
    return true;
}

bool Application::InitializeGLFW() {
    glfwSetErrorCallback(GLFWErrorCallback);

    if (!glfwInit()) {
        LOG_ERROR("Failed to initialize GLFW");
        return false;
    }

    // OpenGL 3.3 Core
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create window
    m_Window = glfwCreateWindow(m_Width, m_Height, "Terrain Engine Pro", nullptr, nullptr);
    if (!m_Window) {
        LOG_ERROR("Failed to create GLFW window");
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(m_Window);
    glfwSetWindowUserPointer(m_Window, this);
    glfwSetFramebufferSizeCallback(m_Window, FramebufferSizeCallback);
    glfwSwapInterval(1); // VSync

    return true;
}

void Application::InitializeImGui() {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // When viewports are enabled, tweak WindowRounding
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    LOG_INFO("ImGui initialized");
}

void Application::Run() {
    float lastFrame = 0.0f;

    while (!glfwWindowShouldClose(m_Window) && m_Running) {
        // Calculate delta time
        float currentFrame = static_cast<float>(glfwGetTime());
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Process input
        ProcessInput();

        // Update
        Update(deltaTime);

        // Render
        Render();

        // Swap buffers and poll events
        glfwSwapBuffers(m_Window);
        glfwPollEvents();
    }
}

void Application::ProcessInput() {
    if (glfwGetKey(m_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        m_Running = false;
    }
}

void Application::Update(float deltaTime) {
    if (m_Editor) {
        m_Editor->Update(deltaTime);
    }
}

void Application::Render() {
    // Get current window size
    glfwGetFramebufferSize(m_Window, &m_Width, &m_Height);

    // Clear
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Start ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Render editor UI
    if (m_Editor) {
        m_Editor->Render();
    }

    // Render ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Update and Render additional Platform Windows
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
}

void Application::Shutdown() {
    LOG_INFO("Shutting down Terrain Engine Editor...");

    m_Editor.reset();

    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Cleanup GLFW
    if (m_Window) {
        glfwDestroyWindow(m_Window);
    }
    glfwTerminate();

    LOG_INFO("Shutdown complete");
}

} // namespace Terrain
