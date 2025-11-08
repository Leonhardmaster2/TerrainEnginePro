#pragma once

#include "Types.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <mutex>

namespace Terrain {

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error
};

class Logger {
public:
    static Logger& Get() {
        static Logger instance;
        return instance;
    }

    void SetLogLevel(LogLevel level) {
        m_MinLevel = level;
    }

    void SetLogFile(const String& filepath) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        if (m_FileStream.is_open()) {
            m_FileStream.close();
        }
        m_FileStream.open(filepath, std::ios::out | std::ios::app);
    }

    template<typename... Args>
    void Log(LogLevel level, const char* format, Args... args) {
        if (level < m_MinLevel) return;

        std::lock_guard<std::mutex> lock(m_Mutex);

        // Format message
        char buffer[4096];
        snprintf(buffer, sizeof(buffer), format, args...);

        // Get timestamp
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        std::ostringstream oss;
        oss << "[" << std::put_time(std::localtime(&time_t), "%H:%M:%S")
            << "." << std::setfill('0') << std::setw(3) << ms.count() << "] ";

        // Level prefix
        switch (level) {
            case LogLevel::Debug:   oss << "[DEBUG] "; break;
            case LogLevel::Info:    oss << "[INFO]  "; break;
            case LogLevel::Warning: oss << "[WARN]  "; break;
            case LogLevel::Error:   oss << "[ERROR] "; break;
        }

        oss << buffer;

        // Output to console
        if (level == LogLevel::Error) {
            std::cerr << oss.str() << std::endl;
        } else {
            std::cout << oss.str() << std::endl;
        }

        // Output to file
        if (m_FileStream.is_open()) {
            m_FileStream << oss.str() << std::endl;
            m_FileStream.flush();
        }
    }

    ~Logger() {
        if (m_FileStream.is_open()) {
            m_FileStream.close();
        }
    }

private:
    Logger() : m_MinLevel(LogLevel::Debug) {}
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    LogLevel m_MinLevel;
    std::ofstream m_FileStream;
    std::mutex m_Mutex;
};

// Convenience macros
#define LOG_DEBUG(...) Terrain::Logger::Get().Log(Terrain::LogLevel::Debug, __VA_ARGS__)
#define LOG_INFO(...)  Terrain::Logger::Get().Log(Terrain::LogLevel::Info, __VA_ARGS__)
#define LOG_WARN(...)  Terrain::Logger::Get().Log(Terrain::LogLevel::Warning, __VA_ARGS__)
#define LOG_ERROR(...) Terrain::Logger::Get().Log(Terrain::LogLevel::Error, __VA_ARGS__)

} // namespace Terrain
