#include "FileDialog.h"
#include "Core/Logger.h"
#include <algorithm>
#include <fstream>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>
#include <shlobj.h>
#endif

namespace Terrain {

// ============================================================================
// FileDialog Implementation
// ============================================================================

FileDialog::FileDialog() {
}

FileDialog::~FileDialog() {
}

FileDialogResult FileDialog::OpenFile(
    const String& title,
    const std::vector<FileFilter>& filters,
    const String& defaultPath)
{
    FileDialogResult result;

#ifdef _WIN32
    char filename[MAX_PATH] = {};

    // Copy default path if provided
    if (!defaultPath.empty()) {
        strncpy_s(filename, defaultPath.c_str(), MAX_PATH - 1);
    }

    OPENFILENAMEA ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = title.c_str();
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

    // Build filter string
    String filterStr = BuildFilterString(filters);
    if (!filterStr.empty()) {
        ofn.lpstrFilter = filterStr.c_str();
    }

    if (GetOpenFileNameA(&ofn)) {
        result.success = true;
        result.filepath = filename;
    } else {
        DWORD error = CommDlgExtendedError();
        if (error != 0) {
            result.success = false;
            result.errorMessage = "File dialog error code: " + std::to_string(error);
            LOG_ERROR("%s", result.errorMessage.c_str());
        }
        // If error is 0, user cancelled - don't log error
    }

#else
    result.success = false;
    result.errorMessage = "File dialogs not implemented for this platform";
    LOG_ERROR("%s", result.errorMessage.c_str());
#endif

    return result;
}

FileDialogResult FileDialog::SaveFile(
    const String& title,
    const std::vector<FileFilter>& filters,
    const String& defaultPath,
    const String& defaultExtension)
{
    FileDialogResult result;

#ifdef _WIN32
    char filename[MAX_PATH] = {};

    // Copy default path if provided
    if (!defaultPath.empty()) {
        strncpy_s(filename, defaultPath.c_str(), MAX_PATH - 1);
    }

    OPENFILENAMEA ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = title.c_str();
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

    // Build filter string
    String filterStr = BuildFilterString(filters);
    if (!filterStr.empty()) {
        ofn.lpstrFilter = filterStr.c_str();
    }

    // Default extension
    if (!defaultExtension.empty()) {
        ofn.lpstrDefExt = defaultExtension.c_str();
    }

    if (GetSaveFileNameA(&ofn)) {
        result.success = true;
        result.filepath = filename;
    } else {
        DWORD error = CommDlgExtendedError();
        if (error != 0) {
            result.success = false;
            result.errorMessage = "File dialog error code: " + std::to_string(error);
            LOG_ERROR("%s", result.errorMessage.c_str());
        }
        // If error is 0, user cancelled - don't log error
    }

#else
    result.success = false;
    result.errorMessage = "File dialogs not implemented for this platform";
    LOG_ERROR("%s", result.errorMessage.c_str());
#endif

    return result;
}

FileDialogResult FileDialog::SelectFolder(
    const String& title,
    const String& defaultPath)
{
    FileDialogResult result;

#ifdef _WIN32
    char path[MAX_PATH] = {};

    BROWSEINFOA bi = {};
    bi.hwndOwner = NULL;
    bi.lpszTitle = title.c_str();
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    bi.pszDisplayName = path;

    LPITEMIDLIST pidl = SHBrowseForFolderA(&bi);
    if (pidl != NULL) {
        if (SHGetPathFromIDListA(pidl, path)) {
            result.success = true;
            result.filepath = path;
        }
        CoTaskMemFree(pidl);
    }

#else
    result.success = false;
    result.errorMessage = "Folder dialogs not implemented for this platform";
    LOG_ERROR("%s", result.errorMessage.c_str());
#endif

    return result;
}

String FileDialog::BuildFilterString(const std::vector<FileFilter>& filters) {
    if (filters.empty()) {
        return "";
    }

    std::stringstream ss;
    for (const auto& filter : filters) {
        ss << filter.name << '\0' << filter.extension << '\0';
    }
    ss << '\0'; // Double null terminator

    return ss.str();
}

// ============================================================================
// RecentFilesManager Implementation
// ============================================================================

RecentFilesManager::RecentFilesManager() {
}

RecentFilesManager::~RecentFilesManager() {
}

void RecentFilesManager::AddRecentFile(const String& filepath) {
    // Remove if already exists
    auto it = std::find(m_RecentFiles.begin(), m_RecentFiles.end(), filepath);
    if (it != m_RecentFiles.end()) {
        m_RecentFiles.erase(it);
    }

    // Add to front
    m_RecentFiles.insert(m_RecentFiles.begin(), filepath);

    // Limit to max size
    if (m_RecentFiles.size() > MAX_RECENT_FILES) {
        m_RecentFiles.resize(MAX_RECENT_FILES);
    }
}

void RecentFilesManager::ClearRecentFiles() {
    m_RecentFiles.clear();
}

void RecentFilesManager::SaveToFile(const String& filepath) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        LOG_ERROR("Failed to save recent files to: %s", filepath.c_str());
        return;
    }

    for (const auto& recent : m_RecentFiles) {
        file << recent << "\n";
    }

    file.close();
    LOG_INFO("Recent files saved to: %s", filepath.c_str());
}

void RecentFilesManager::LoadFromFile(const String& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        // Not an error - file might not exist yet
        return;
    }

    m_RecentFiles.clear();

    String line;
    while (std::getline(file, line)) {
        if (!line.empty()) {
            m_RecentFiles.push_back(line);
        }
    }

    file.close();
    LOG_INFO("Recent files loaded from: %s (%zu files)", filepath.c_str(), m_RecentFiles.size());
}

} // namespace Terrain
