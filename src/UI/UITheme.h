#pragma once

#include "Core/Types.h"
#include <imgui.h>

namespace Terrain {

// UI theme presets
enum class UIThemePreset {
    Dark,          // Professional dark theme (default)
    Light,         // Light theme
    Blue,          // Blue accent theme
    Custom         // User-defined theme
};

// UI theme manager
class UITheme {
public:
    UITheme();
    ~UITheme();

    // Apply theme to ImGui
    void Apply(UIThemePreset preset = UIThemePreset::Dark);

    // Apply custom colors
    void ApplyCustomColors(const ImVec4& accent, const ImVec4& background);

    // Get current theme
    UIThemePreset GetCurrentTheme() const { return m_CurrentTheme; }

    // Theme colors (accessible for consistent UI)
    static ImVec4 ColorAccent;
    static ImVec4 ColorAccentHovered;
    static ImVec4 ColorAccentActive;
    static ImVec4 ColorBackground;
    static ImVec4 ColorBackgroundDark;
    static ImVec4 ColorText;
    static ImVec4 ColorTextDim;
    static ImVec4 ColorSuccess;
    static ImVec4 ColorWarning;
    static ImVec4 ColorError;
    static ImVec4 ColorInfo;

private:
    void ApplyDarkTheme();
    void ApplyLightTheme();
    void ApplyBlueTheme();

    void SetupFonts();
    void SetupStyles();

    UIThemePreset m_CurrentTheme = UIThemePreset::Dark;
};

// Node colors by category
class NodeColors {
public:
    static ImVec4 GetNodeColor(NodeCategory category);
    static ImVec4 GetNodeColorHovered(NodeCategory category);
    static ImVec4 GetPinColor(PinType type);

    // Generator nodes - Green tones
    static constexpr ImVec4 ColorGenerator = ImVec4(0.2f, 0.7f, 0.3f, 1.0f);

    // Modifier nodes - Orange tones
    static constexpr ImVec4 ColorModifier = ImVec4(0.9f, 0.6f, 0.2f, 1.0f);

    // Filter nodes - Purple tones
    static constexpr ImVec4 ColorFilter = ImVec4(0.7f, 0.3f, 0.9f, 1.0f);

    // Erosion nodes - Brown/Earth tones
    static constexpr ImVec4 ColorErosion = ImVec4(0.7f, 0.5f, 0.3f, 1.0f);

    // Combiner nodes - Blue tones
    static constexpr ImVec4 ColorCombiner = ImVec4(0.3f, 0.5f, 0.9f, 1.0f);

    // Texture nodes - Cyan tones
    static constexpr ImVec4 ColorTexture = ImVec4(0.2f, 0.8f, 0.8f, 1.0f);

    // Export nodes - Red tones
    static constexpr ImVec4 ColorExport = ImVec4(0.9f, 0.3f, 0.3f, 1.0f);

    // Output node - White
    static constexpr ImVec4 ColorOutput = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);
};

} // namespace Terrain
