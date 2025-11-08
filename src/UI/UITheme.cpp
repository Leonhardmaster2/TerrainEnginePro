#include "UITheme.h"
#include "Core/Logger.h"
#include <imnodes.h>

namespace Terrain {

// Initialize static colors
ImVec4 UITheme::ColorAccent = ImVec4(0.26f, 0.59f, 0.98f, 1.0f);
ImVec4 UITheme::ColorAccentHovered = ImVec4(0.36f, 0.69f, 1.0f, 1.0f);
ImVec4 UITheme::ColorAccentActive = ImVec4(0.16f, 0.49f, 0.88f, 1.0f);
ImVec4 UITheme::ColorBackground = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
ImVec4 UITheme::ColorBackgroundDark = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
ImVec4 UITheme::ColorText = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
ImVec4 UITheme::ColorTextDim = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
ImVec4 UITheme::ColorSuccess = ImVec4(0.2f, 0.8f, 0.3f, 1.0f);
ImVec4 UITheme::ColorWarning = ImVec4(0.9f, 0.7f, 0.2f, 1.0f);
ImVec4 UITheme::ColorError = ImVec4(0.9f, 0.2f, 0.2f, 1.0f);
ImVec4 UITheme::ColorInfo = ImVec4(0.3f, 0.7f, 0.9f, 1.0f);

UITheme::UITheme() {
}

UITheme::~UITheme() {
}

void UITheme::Apply(UIThemePreset preset) {
    m_CurrentTheme = preset;

    switch (preset) {
        case UIThemePreset::Dark:
            ApplyDarkTheme();
            break;
        case UIThemePreset::Light:
            ApplyLightTheme();
            break;
        case UIThemePreset::Blue:
            ApplyBlueTheme();
            break;
        case UIThemePreset::Custom:
            // Custom colors already set
            break;
    }

    SetupStyles();
    LOG_INFO("UI theme applied: %d", static_cast<int>(preset));
}

void UITheme::ApplyCustomColors(const ImVec4& accent, const ImVec4& background) {
    ColorAccent = accent;
    ColorBackground = background;

    // Derive other colors
    ColorAccentHovered = ImVec4(
        accent.x * 1.2f,
        accent.y * 1.2f,
        accent.z * 1.2f,
        accent.w
    );
    ColorAccentActive = ImVec4(
        accent.x * 0.8f,
        accent.y * 0.8f,
        accent.z * 0.8f,
        accent.w
    );
    ColorBackgroundDark = ImVec4(
        background.x * 0.7f,
        background.y * 0.7f,
        background.z * 0.7f,
        background.w
    );

    m_CurrentTheme = UIThemePreset::Custom;
    SetupStyles();
}

void UITheme::ApplyDarkTheme() {
    // Professional dark theme
    ColorAccent = ImVec4(0.26f, 0.59f, 0.98f, 1.0f);
    ColorAccentHovered = ImVec4(0.36f, 0.69f, 1.0f, 1.0f);
    ColorAccentActive = ImVec4(0.16f, 0.49f, 0.88f, 1.0f);
    ColorBackground = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
    ColorBackgroundDark = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
    ColorText = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    ColorTextDim = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
    ColorSuccess = ImVec4(0.2f, 0.8f, 0.3f, 1.0f);
    ColorWarning = ImVec4(0.9f, 0.7f, 0.2f, 1.0f);
    ColorError = ImVec4(0.9f, 0.2f, 0.2f, 1.0f);
    ColorInfo = ImVec4(0.3f, 0.7f, 0.9f, 1.0f);
}

void UITheme::ApplyLightTheme() {
    // Professional light theme
    ColorAccent = ImVec4(0.26f, 0.59f, 0.98f, 1.0f);
    ColorAccentHovered = ImVec4(0.36f, 0.69f, 1.0f, 1.0f);
    ColorAccentActive = ImVec4(0.16f, 0.49f, 0.88f, 1.0f);
    ColorBackground = ImVec4(0.95f, 0.95f, 0.95f, 1.0f);
    ColorBackgroundDark = ImVec4(0.85f, 0.85f, 0.85f, 1.0f);
    ColorText = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
    ColorTextDim = ImVec4(0.4f, 0.4f, 0.4f, 1.0f);
    ColorSuccess = ImVec4(0.1f, 0.7f, 0.2f, 1.0f);
    ColorWarning = ImVec4(0.8f, 0.6f, 0.1f, 1.0f);
    ColorError = ImVec4(0.8f, 0.1f, 0.1f, 1.0f);
    ColorInfo = ImVec4(0.2f, 0.6f, 0.8f, 1.0f);
}

void UITheme::ApplyBlueTheme() {
    // Blue-accented dark theme
    ColorAccent = ImVec4(0.2f, 0.5f, 0.9f, 1.0f);
    ColorAccentHovered = ImVec4(0.3f, 0.6f, 1.0f, 1.0f);
    ColorAccentActive = ImVec4(0.1f, 0.4f, 0.8f, 1.0f);
    ColorBackground = ImVec4(0.12f, 0.14f, 0.18f, 1.0f);
    ColorBackgroundDark = ImVec4(0.08f, 0.09f, 0.12f, 1.0f);
    ColorText = ImVec4(0.95f, 0.96f, 0.98f, 1.0f);
    ColorTextDim = ImVec4(0.55f, 0.58f, 0.62f, 1.0f);
    ColorSuccess = ImVec4(0.2f, 0.8f, 0.3f, 1.0f);
    ColorWarning = ImVec4(0.9f, 0.7f, 0.2f, 1.0f);
    ColorError = ImVec4(0.9f, 0.2f, 0.2f, 1.0f);
    ColorInfo = ImVec4(0.3f, 0.7f, 0.9f, 1.0f);
}

void UITheme::SetupStyles() {
    ImGuiStyle& style = ImGui::GetStyle();

    // Window
    style.WindowRounding = 4.0f;
    style.WindowBorderSize = 1.0f;
    style.WindowPadding = ImVec2(10.0f, 10.0f);
    style.WindowMinSize = ImVec2(100.0f, 100.0f);

    // Frame
    style.FrameRounding = 3.0f;
    style.FrameBorderSize = 0.0f;
    style.FramePadding = ImVec2(8.0f, 4.0f);

    // Scrollbar
    style.ScrollbarSize = 14.0f;
    style.ScrollbarRounding = 3.0f;

    // Grabber
    style.GrabRounding = 3.0f;
    style.GrabMinSize = 10.0f;

    // Tab
    style.TabRounding = 3.0f;
    style.TabBorderSize = 0.0f;

    // Items
    style.ItemSpacing = ImVec2(8.0f, 6.0f);
    style.ItemInnerSpacing = ImVec2(6.0f, 4.0f);
    style.IndentSpacing = 20.0f;

    // Apply colors
    ImVec4* colors = style.Colors;

    // Background colors
    colors[ImGuiCol_WindowBg] = ColorBackground;
    colors[ImGuiCol_ChildBg] = ColorBackground;
    colors[ImGuiCol_PopupBg] = ColorBackgroundDark;
    colors[ImGuiCol_MenuBarBg] = ColorBackgroundDark;

    // Text
    colors[ImGuiCol_Text] = ColorText;
    colors[ImGuiCol_TextDisabled] = ColorTextDim;

    // Borders
    colors[ImGuiCol_Border] = ImVec4(0.3f, 0.3f, 0.3f, 0.5f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

    // Frame (inputs, etc.)
    colors[ImGuiCol_FrameBg] = ColorBackgroundDark;
    colors[ImGuiCol_FrameBgHovered] = ImVec4(ColorBackgroundDark.x * 1.2f, ColorBackgroundDark.y * 1.2f, ColorBackgroundDark.z * 1.2f, 1.0f);
    colors[ImGuiCol_FrameBgActive] = ColorAccent;

    // Title
    colors[ImGuiCol_TitleBg] = ColorBackgroundDark;
    colors[ImGuiCol_TitleBgActive] = ColorAccent;
    colors[ImGuiCol_TitleBgCollapsed] = ColorBackgroundDark;

    // Scrollbar
    colors[ImGuiCol_ScrollbarBg] = ColorBackgroundDark;
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.4f, 0.4f, 0.4f, 1.0f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
    colors[ImGuiCol_ScrollbarGrabActive] = ColorAccent;

    // Checkmark
    colors[ImGuiCol_CheckMark] = ColorAccent;

    // Slider
    colors[ImGuiCol_SliderGrab] = ColorAccent;
    colors[ImGuiCol_SliderGrabActive] = ColorAccentActive;

    // Button
    colors[ImGuiCol_Button] = ColorAccent;
    colors[ImGuiCol_ButtonHovered] = ColorAccentHovered;
    colors[ImGuiCol_ButtonActive] = ColorAccentActive;

    // Header (tree nodes, etc.)
    colors[ImGuiCol_Header] = ColorAccent;
    colors[ImGuiCol_HeaderHovered] = ColorAccentHovered;
    colors[ImGuiCol_HeaderActive] = ColorAccentActive;

    // Separator
    colors[ImGuiCol_Separator] = ImVec4(0.3f, 0.3f, 0.3f, 0.5f);
    colors[ImGuiCol_SeparatorHovered] = ColorAccentHovered;
    colors[ImGuiCol_SeparatorActive] = ColorAccentActive;

    // Resize grip
    colors[ImGuiCol_ResizeGrip] = ColorAccent;
    colors[ImGuiCol_ResizeGripHovered] = ColorAccentHovered;
    colors[ImGuiCol_ResizeGripActive] = ColorAccentActive;

    // Tab
    colors[ImGuiCol_Tab] = ColorBackgroundDark;
    colors[ImGuiCol_TabHovered] = ColorAccentHovered;
    colors[ImGuiCol_TabActive] = ColorAccent;
    colors[ImGuiCol_TabUnfocused] = ColorBackgroundDark;
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(ColorAccent.x * 0.7f, ColorAccent.y * 0.7f, ColorAccent.z * 0.7f, 1.0f);

    // Docking
    colors[ImGuiCol_DockingPreview] = ImVec4(ColorAccent.x, ColorAccent.y, ColorAccent.z, 0.7f);
    colors[ImGuiCol_DockingEmptyBg] = ColorBackgroundDark;

    // Plot
    colors[ImGuiCol_PlotLines] = ColorAccent;
    colors[ImGuiCol_PlotLinesHovered] = ColorAccentHovered;
    colors[ImGuiCol_PlotHistogram] = ColorAccent;
    colors[ImGuiCol_PlotHistogramHovered] = ColorAccentHovered;

    // Table
    colors[ImGuiCol_TableHeaderBg] = ColorBackgroundDark;
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.4f, 0.4f, 0.4f, 1.0f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
    colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.03f);

    // Text selection
    colors[ImGuiCol_TextSelectedBg] = ImVec4(ColorAccent.x, ColorAccent.y, ColorAccent.z, 0.35f);

    // Drag drop
    colors[ImGuiCol_DragDropTarget] = ColorAccentHovered;

    // Nav
    colors[ImGuiCol_NavHighlight] = ColorAccent;
    colors[ImGuiCol_NavWindowingHighlight] = ColorAccent;
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.5f);

    // Modal
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.5f);

    // ImNodes styling
    ImNodesStyle& nodeStyle = ImNodes::GetStyle();
    nodeStyle.GridSpacing = 32.0f;
    nodeStyle.NodeCornerRounding = 4.0f;
    nodeStyle.NodePadding = ImVec2(8.0f, 8.0f);
    nodeStyle.NodeBorderThickness = 1.0f;
    nodeStyle.LinkThickness = 3.0f;
    nodeStyle.LinkLineSegmentsPerLength = 0.1f;
    nodeStyle.PinCircleRadius = 4.0f;
    nodeStyle.PinQuadSideLength = 7.0f;
    nodeStyle.PinTriangleSideLength = 9.5f;
}

// ============================================================================
// NodeColors Implementation
// ============================================================================

ImVec4 NodeColors::GetNodeColor(NodeCategory category) {
    switch (category) {
        case NodeCategory::Generator: return ColorGenerator;
        case NodeCategory::Modifier: return ColorModifier;
        case NodeCategory::Filter: return ColorFilter;
        case NodeCategory::Erosion: return ColorErosion;
        case NodeCategory::Combiner: return ColorCombiner;
        case NodeCategory::Texture: return ColorTexture;
        case NodeCategory::Export: return ColorExport;
        case NodeCategory::Output: return ColorOutput;
        default: return ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
    }
}

ImVec4 NodeColors::GetNodeColorHovered(NodeCategory category) {
    ImVec4 base = GetNodeColor(category);
    return ImVec4(base.x * 1.2f, base.y * 1.2f, base.z * 1.2f, base.w);
}

ImVec4 NodeColors::GetPinColor(PinType type) {
    switch (type) {
        case PinType::Heightfield: return ImVec4(0.9f, 0.7f, 0.3f, 1.0f);  // Orange/gold
        case PinType::Texture: return ImVec4(0.3f, 0.9f, 0.7f, 1.0f);      // Cyan
        case PinType::Data: return ImVec4(0.7f, 0.3f, 0.9f, 1.0f);         // Purple
        default: return ImVec4(0.8f, 0.8f, 0.8f, 1.0f);
    }
}

} // namespace Terrain
