#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <mutex>
#include <format>
#include <chrono>
#include <functional>
#include <string_view>

namespace Core {

enum class LogLevel {
    LEVEL_INFO,
    LEVEL_DEBUG,
    LEVEL_WARN,
    LEVEL_ERROR
};

class Logger {
public:
    using LogCallback = std::function<void(const std::string&)>;

    static void SetCallback(LogCallback callback) {
        std::lock_guard<std::mutex> lock(GetMutex());
        m_Callback = callback;
    }

    static void Log(LogLevel level, std::string_view message) {
        std::lock_guard<std::mutex> lock(GetMutex());
        
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        struct tm time_info;
        localtime_s(&time_info, &time_t);

        std::string levelStr;
        std::string colorCode;
        
        switch (level) {
            case LogLevel::LEVEL_INFO:  levelStr = "[BILGI]"; break; 
            case LogLevel::LEVEL_DEBUG: levelStr = "[HATA_AYIKLAMA]"; break;
            case LogLevel::LEVEL_WARN:  levelStr = "[UYARI]"; break;
            case LogLevel::LEVEL_ERROR: levelStr = "[HATA]"; break;
        }

        std::string timeStr = std::format("{:02d}:{:02d}:{:02d}", time_info.tm_hour, time_info.tm_min, time_info.tm_sec);
        std::string fullLog = std::format("[{}] {} {}", timeStr, levelStr, message);

        std::cout << fullLog << "\n";

        static std::ofstream logFile("server.log", std::ios::app);
        if (logFile.is_open()) {
            logFile << fullLog << "\n";
            logFile.flush();
        }

        if (m_Callback) {
            m_Callback(fullLog);
        }
    }

    static void LogInfo(std::string_view msg) { Log(LogLevel::LEVEL_INFO, msg); }
    static void LogDebug(std::string_view msg) { Log(LogLevel::LEVEL_DEBUG, msg); }
    static void LogWarn(std::string_view msg) { Log(LogLevel::LEVEL_WARN, msg); }
    static void LogError(std::string_view msg) { Log(LogLevel::LEVEL_ERROR, msg); }

private:
    static std::mutex& GetMutex() {
        static std::mutex mutex;
        return mutex;
    }
    static inline LogCallback m_Callback = nullptr;
};

} // namespace Core
