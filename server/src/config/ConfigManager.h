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
                    data["buttons"].push_back(btn);
                }
                data["server"] = { {"port", 8888}, {"theme", "dark"}, {"language", "tr"} };
                std::ofstream file(m_LastPath);
                file << data.dump(4);
                return true;
            } catch (...) { return false; }
        }

        void UpdateButton(const std::string& id, const std::string& label, uint16_t scanCode) {
            std::lock_guard<std::mutex> lock(m_Mutex);
            if (m_Buttons.contains(id)) {
                m_Buttons[id].label = label;
                m_Buttons[id].scanCode = scanCode;
                // SaveConfig'i doğrudan çağırma (re-entrant lock sorunu olmaması için)
                // Ama burada SaveConfig() zaten mutex alıyor. nested lock için recursive_mutex gerekir.
                // Basitlik için kopyalıyoruz veya private bir SaveInternal yapıyoruz.
            }
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
