#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <windows.h>
#include <string>
#include <chrono>
#include "../core/Logger.h"
#include "../state/StateManager.h"
#include "../gui/Dashboard.h"

namespace Telemetry {

// SCS Telemetry SDK yapı taşları (Sadeleştirilmiş Altyapı)
struct PACKED_TelemetryData {
    uint32_t sdk_version;
    uint32_t plugin_version;
    uint32_t game_version;
    uint32_t paused;
    
    // Tır Bilgileri
    float speed;
    float cruise_speed;
    int32_t gear;
    int32_t gears_forward;
    
    // Işıklar ve Kontroller
    uint32_t lights_dashboard;
    uint32_t lights_low_beam;
    uint32_t lights_high_beam;
    uint32_t wipers;
    uint32_t beacon;
    uint32_t parking_brake;
    uint32_t motor_running;

    // Yakıt ve Adblue
    float fuel;
    float fuel_capacity;
    float fuel_consumption; // Yakıt tüketimi L/km
    float adblue;
    float adblue_capacity;

    // Hasar (0.0 - 1.0)
    float wear_engine;
    float wear_transmission;
    float wear_chassis;
    float wear_wheels;
    float wear_cabin;

    // Navigasyon
    float navigation_distance; // metre
    float navigation_time;     // saniye

    // Koordinatlar
    float posX;
    float posY;
    float posZ;
    float heading; 

    // Oyun Zamanı ve Dinlenme
    uint32_t rest_stop; // saniye
    float game_time;    // oyun içi saat (saniye cinsinden mutlak)

    // Metin Verileri (Faz 4)
    char city[64];
    char cargo[64];
    char destination[64];
    char truck_brand[32];
    char truck_model[32];
};

class TelemetryListener {
public:
    static TelemetryListener& GetInstance() {
        static TelemetryListener instance;
        return instance;
    }

    bool Initialize() {
        if (m_MapFile != NULL) return true;

        // Farklı plugin versiyonları için olası tüm isimleri dene
        const wchar_t* mapNames[] = { 
            L"SCS/ETS2", 
            L"Local\\SCS/ETS2", 
            L"Global\\SCS/ETS2", 
            L"SCSTelemetry",
            L"Global\\SCSTelemetry" 
        };

        for (const wchar_t* name : mapNames) {
            m_MapFile = OpenFileMappingW(FILE_MAP_READ, FALSE, name);
            if (m_MapFile != NULL) {
                m_Data = (PACKED_TelemetryData*)MapViewOfFile(m_MapFile, FILE_MAP_READ, 0, 0, sizeof(PACKED_TelemetryData));
                if (m_Data != NULL) {
                    Core::Logger::LogInfo("ETS2 Telemetry bağlantısı başarılı.");
                    SetGameStatus(true);
                    return true;
                }
                CloseHandle(m_MapFile);
                m_MapFile = NULL;
            }
        }
        Core::Logger::LogWarn("ETS2 Shared Memory alanı bulunamadı! SCS Telemetry Plugin (scs-telemetry.dll) yüklü mü?");
        return false;
    }

    void Update() {
        if (m_MapFile == NULL) {
            SetGameStatus(false);
            static auto lastRetry = std::chrono::steady_clock::now();
            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::seconds>(now - lastRetry).count() >= 2) {
                Initialize();
                lastRetry = now;
            }
            return;
        }

        // Oyun kapandıysa (Heartbeat kontrolü veya SDK sürümü ile)
        // SCS Telemetry'de veri 0lanmaz ama sdk_version kontrolü yapılabilir
        if (m_Data->sdk_version == 0) {
            Shutdown();
            return;
        }

        auto& sm = State::StateManager::GetInstance();
        
        // Temel Veriler
        sm.UpdateState("speed", m_Data->speed * 3.6f); 
        sm.UpdateState("engine_running", m_Data->motor_running > 0);
        sm.UpdateState("lights_low", m_Data->lights_low_beam > 0);
        sm.UpdateState("lights_high", m_Data->lights_high_beam > 0);
        sm.UpdateState("beacon", m_Data->beacon > 0);
        sm.UpdateState("wipers", (int)m_Data->wipers);
        sm.UpdateState("parking_brake", m_Data->parking_brake > 0);

        // Genişletilmiş Veriler
        sm.UpdateState("fuel", m_Data->fuel);
        sm.UpdateState("fuel_capacity", m_Data->fuel_capacity);
        sm.UpdateState("fuel_consumption", m_Data->fuel_consumption);
        sm.UpdateState("adblue", m_Data->adblue);
        
        // Detaylı Hasar
        sm.UpdateState("wear_engine", m_Data->wear_engine);
        sm.UpdateState("wear_transmission", m_Data->wear_transmission);
        sm.UpdateState("wear_chassis", m_Data->wear_chassis);
        sm.UpdateState("wear_wheels", m_Data->wear_wheels);
        sm.UpdateState("wear_cabin", m_Data->wear_cabin);

        // Navigasyon ve Konum
        sm.UpdateState("nav_distance", m_Data->navigation_distance);
        sm.UpdateState("nav_time", m_Data->navigation_time);
        sm.UpdateState("rest_stop", (int)m_Data->rest_stop);
        sm.UpdateState("posX", m_Data->posX);
        sm.UpdateState("posZ", m_Data->posZ);
        sm.UpdateState("heading", m_Data->heading);

        // Metin Verileri (Şehir, İş)
        sm.UpdateState("city", std::string(m_Data->city));
        sm.UpdateState("cargo", std::string(m_Data->cargo));
        sm.UpdateState("destination", std::string(m_Data->destination));
        sm.UpdateState("truck_name", std::string(m_Data->truck_brand) + " " + std::string(m_Data->truck_model));
    }

    void Shutdown() {
        if (m_Data) UnmapViewOfFile(m_Data);
        if (m_MapFile) CloseHandle(m_MapFile);
        m_Data = nullptr;
        m_MapFile = NULL;
        SetGameStatus(false);
    }

private:
    TelemetryListener() : m_MapFile(NULL), m_Data(nullptr), m_IsGameRunning(false) {}

    void SetGameStatus(bool running) {
        if (m_IsGameRunning != running) {
            m_IsGameRunning = running;
            GUI::Dashboard::GetInstance().UpdateGame(running ? "AÇIK 🟢" : "KAPALI 🔴");
        }
    }

    HANDLE m_MapFile;
    PACKED_TelemetryData* m_Data;
    bool m_IsGameRunning;
};

} // namespace Telemetry
