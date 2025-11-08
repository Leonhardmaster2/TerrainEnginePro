#pragma once

#include "Core/Types.h"
#include <vector>

namespace Terrain {

// File dialog filter
struct FileFilter {
    String name;          // Display name (e.g., "Terrain Graph Files")
    String extension;     // Extension (e.g., "*.json")
};

// File dialog result
struct FileDialogResult {
    bool success = false;
    String filepath;
    String errorMessage;
};

// Native file dialog wrapper for Windows
class FileDialog {
public:
    FileDialog();
    ~FileDialog();

    // Open file dialog
    static FileDialogResult OpenFile(
        const String& title = "Open File",
        const std::vector<FileFilter>& filters = {},
        const String& defaultPath = ""
    );

    // Save file dialog
    static FileDialogResult SaveFile(
        const String& title = "Save File",
        const std::vector<FileFilter>& filters = {},
        const String& defaultPath = "",
        const String& defaultExtension = ""
    );

    // Select folder dialog
    static FileDialogResult SelectFolder(
        const String& title = "Select Folder",
        const String& defaultPath = ""
    );

private:
    // Convert filter list to Windows format
    static String BuildFilterString(const std::vector<FileFilter>& filters);
};

// Recent files manager
class RecentFilesManager {
public:
    RecentFilesManager();
    ~RecentFilesManager();

    // Add file to recent list
    void AddRecentFile(const String& filepath);

    // Get recent files (most recent first)
    const std::vector<String>& GetRecentFiles() const { return m_RecentFiles; }

    // Clear recent files
    void ClearRecentFiles();

    // Save/Load recent files list
    void SaveToFile(const String& filepath);
    void LoadFromFile(const String& filepath);

    // Max recent files to track
    static constexpr uint32 MAX_RECENT_FILES = 10;

private:
    std::vector<String> m_RecentFiles;
};

} // namespace Terrain
