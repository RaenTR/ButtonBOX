#pragma once
#include <fstream>
#include <map>
#include <string>
#include <mutex>
#include "json.hpp"

namespace Config {

using json = nlohmann::json;

enum class ControlType {
    PUSH,   // Tek basım (Korna vb.)
    TOGGLE, // Aç/Kapat (Motor, Beacon)
    ROTARY  // Döner anahtar (Farlar, Silecekler)
};

    // Tip dönüşümü (String -> Enum)
    inline ControlType StringToType(const std::string& type) {
        if (type == "TOGGLE") return ControlType::TOGGLE;
        if (type == "ROTARY") return ControlType::ROTARY;
        return ControlType::PUSH;
    }

    // Tip dönüşümü (Enum -> String)
    inline std::string TypeToString(ControlType type) {
        if (type == ControlType::TOGGLE) return "TOGGLE";
        if (type == ControlType::ROTARY) return "ROTARY";
        return "PUSH";
    }

    struct ButtonConfig {
        std::string id;
        std::string label;
        ControlType type;
        uint16_t scanCode;
        int32_t maxValue;
        int32_t color = -1;       // -1: Varsayılan
        std::string icon = "";    // Boş: Varsayılan
    };

    class ConfigManager {
    public:
        static ConfigManager& GetInstance() {
            static ConfigManager instance;
            return instance;
        }

        bool LoadConfig(const std::string& path) {
            std::lock_guard<std::mutex> lock(m_Mutex);
            m_LastPath = path;
            try {
                std::ifstream file(path);
                if (!file.is_open()) return false;
                json data; file >> data;
                m_Buttons.clear();
                if (data.contains("buttons")) {
                    for (const auto& btn : data["buttons"]) {
                        ButtonConfig config;
                        config.id = btn["id"];
                        config.label = btn["label"];
                        config.type = StringToType(btn["type"]);
                        config.scanCode = (uint16_t)btn["scanCode"];
                        config.maxValue = (btn.contains("maxValue") ? (int)btn["maxValue"] : 0);
                        config.color = (btn.contains("color") ? (int)btn["color"] : -1);
                        config.icon = (btn.contains("icon") ? btn["icon"].get<std::string>() : "");
                        m_Buttons[config.id] = config;
                    }
                }
                return true;
            } catch (...) { return false; }
        }

        bool SaveConfig() {
            std::lock_guard<std::mutex> lock(m_Mutex);
            try {
                json data;
                data["buttons"] = json::array();
                for (auto const& [key, val] : m_Buttons) {
                    json btn;
                    btn["id"] = val.id;
                    btn["label"] = val.label;
                    btn["type"] = TypeToString(val.type);
                    btn["scanCode"] = val.scanCode;
                    btn["maxValue"] = val.maxValue;
                    if (val.color != -1) btn["color"] = val.color;
                    if (!val.icon.empty()) btn["icon"] = val.icon;
                    data["buttons"].push_back(btn);
                }
                data["server"] = { {"port", 8888}, {"theme", "dark"}, {"language", "tr"} };
                std::ofstream file(m_LastPath);
                file << data.dump(4);
                return true;
            } catch (...) { return false; }
        }

        void UpdateButton(const std::string& id, const std::string& label, uint16_t scanCode, int32_t color = -1, const std::string& icon = "") {
            std::lock_guard<std::mutex> lock(m_Mutex);
            if (m_Buttons.contains(id)) {
                m_Buttons[id].label = label;
                m_Buttons[id].scanCode = scanCode;
                if (color != -1) m_Buttons[id].color = color;
                if (!icon.empty()) m_Buttons[id].icon = icon;
            }
        }

        bool SaveConfig(const std::string& path) {
            m_LastPath = path;
            return SaveConfig();
        }

        std::map<std::string, ButtonConfig> GetAllButtons() {
            std::lock_guard<std::mutex> lock(m_Mutex);
            return m_Buttons;
        }

    private:
        ConfigManager() = default;
        std::map<std::string, ButtonConfig> m_Buttons;
        std::string m_LastPath = "buttons.json";
        std::mutex m_Mutex;
    };

} // namespace Config
