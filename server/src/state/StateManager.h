#pragma once
#include <string>
#include <unordered_map>
#include <mutex>
#include <shared_mutex>
#include <variant>

namespace State {

using StateValue = std::variant<int, float, bool, std::string>;

class StateManager {
public:
    // Singleton instance
    static StateManager& GetInstance() {
        static StateManager instance;
        return instance;
    }

    // State güncelleme (Thread-safe)
    void UpdateState(const std::string& key, StateValue value) {
        std::unique_lock lock(m_Mutex);
        m_States[key] = value;
    }

    // State okuma (Güvenli Template Erişimi)
    template<typename T>
    T GetState(const std::string& key, T defaultValue) {
        std::shared_lock lock(m_Mutex);
        if (m_States.contains(key)) {
            const auto& val = m_States.at(key);
            if (std::holds_alternative<T>(val)) {
                return std::get<T>(val);
            }
        }
        return defaultValue;
    }

    const std::unordered_map<std::string, StateValue>& GetAllStates() const {
        return m_States;
    }

private:
    StateManager() = default;
    
    std::unordered_map<std::string, StateValue> m_States;
    mutable std::shared_mutex m_Mutex; // Okuma öncelikli kilit
};

} // namespace State
