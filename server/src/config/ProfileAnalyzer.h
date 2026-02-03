#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <regex>
#include <map>
#include <shlobj.h>
#include <cstdint>
#include <format>
#include "../core/Logger.h"

namespace Config {

namespace fs = std::filesystem;

class ProfileAnalyzer {
public:
    static ProfileAnalyzer& GetInstance() {
        static ProfileAnalyzer instance;
        return instance;
    }

    // ETS2 Belge klasörünü bulur
    fs::path GetETS2DocumentsPath() {
        char path[MAX_PATH];
        if (SHGetSpecialFolderPathA(NULL, path, CSIDL_MYDOCUMENTS, FALSE)) {
            fs::path p = fs::path(path) / "Euro Truck Simulator 2";
            if (fs::exists(p)) return p;
        }
        return "";
    }

    // Profil içindeki controls.sii dosyasını analiz eder
    std::map<std::string, uint16_t> ScanLatestProfile() {
        std::map<std::string, uint16_t> keyMap;
        fs::path etsDir = GetETS2DocumentsPath();
        if (etsDir.empty()) {
            Core::Logger::LogWarn("ETS2 belgeler klasörü bulunamadı.");
            return keyMap;
        }

        fs::path profilesDir = etsDir / "profiles";
        if (!fs::exists(profilesDir)) {
            Core::Logger::LogWarn("ETS2 profiller klasörü bulunamadı.");
            return keyMap;
        }

        // En son güncellenen profili bul
        fs::path latestSii = "";
        fs::file_time_type latestTime;
        
        try {
            for (const auto& entry : fs::directory_iterator(profilesDir)) {
                if (entry.is_directory()) {
                    fs::path siiPath = entry.path() / "controls.sii";
                    if (fs::exists(siiPath)) {
                        auto currTime = fs::last_write_time(siiPath);
                        if (latestSii.empty() || currTime > latestTime) {
                            latestTime = currTime;
                            latestSii = siiPath;
                        }
                    }
                }
            }
        } catch (...) {}

        if (latestSii.empty()) {
            Core::Logger::LogWarn("Aktif controls.sii dosyası bulunamadı.");
            return keyMap;
        }

        Core::Logger::LogInfo(std::format("Profil Analiz Ediliyor: {}", latestSii.string()));
        
        std::ifstream file(latestSii);
        std::string line;
        // Örnek: config_lines[131] : "mix light `keyboard.l?0`"
        // Regex: mix (action) `keyboard.(key)?0`
        std::regex reg(R"(mix\s+([a-z0-9_]+)\s+`keyboard\.([a-z0-9_]+)\?0`)");
        
        while (std::getline(file, line)) {
            std::smatch match;
            if (std::regex_search(line, match, reg)) {
                std::string action = match[1];
                std::string key = match[2];
                uint16_t scanCode = KeyToScanCode(key);
                if (scanCode > 0) {
                    keyMap[action] = scanCode;
                }
            }
        }

        return keyMap;
    }

private:
    ProfileAnalyzer() = default;

    // keyboard.key stringini DX Scan Code'a çevirir (Basit eşleştirme)
    uint16_t KeyToScanCode(const std::string& key) {
        static std::map<std::string, uint16_t> dxMap = {
            {"l", 38}, {"p", 25}, {"e", 18}, {"space", 57}, {"h", 35}, 
            {"b", 48}, {"k", 37}, {"j", 36}, {"n", 49}, {"m", 50}, {"g", 34}, {"f", 33},
            {"f1", 59}, {"f2", 60}, {"f3", 61}, {"f4", 62}, {"f5", 63},
            {"num1", 79}, {"num2", 80}, {"num3", 81}, {"num4", 75}, {"num5", 76},
            {"num6", 77}, {"num7", 71}, {"num8", 72}, {"num9", 73}, {"numenter", 156},
            {"up", 200}, {"down", 208}, {"left", 203}, {"right", 205}
        };
        
        if (dxMap.count(key)) return dxMap[key];
        // Tek karakterli harfler a-z (Basit ASCII mapping değil DX mapping lazım)
        if (key.length() == 1) {
            char c = key[0];
            // 'a'->30, 's'->31 vb. (Tam liste eklenebilir, şimdilik kritik olanlar yukarda)
        }
        return 0;
    }
};

} // namespace Config
