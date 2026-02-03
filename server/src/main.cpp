#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <windows.h>
#include "gui/Dashboard.h"
#include "core/Logger.h"
#include "state/StateManager.h"
#include "network/TCPServer.h"
#include "network/UDPDiscoveryServer.h"
#include "input/InputController.h"
#include "telemetry/TelemetryListener.h"
#include "config/ConfigManager.h"
#include <shellapi.h>
#include <vector>
#include <cstdlib>
#include <thread>
#include <format>
#include <filesystem>
#include <atomic>

namespace fs = std::filesystem;

// --- GÖRESEL KONSOL TASARIMI ---
void PrintBanner() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    
    // Mavi/Cyan Renk Seti
    SetConsoleTextAttribute(hConsole, 11);
    std::cout << R"(
    ============================================================
      ______ _______ _____ ___  _____                               
     |  ____|__   __/ ____|__ \|  __ \                              
     | |__     | | | (___    ) | |__) |                             
     |  __|    | |  \___ \  / /|  _  /                              
     | |____   | |  ____) |/ /_| | \ \                              
     |______|  |_| |_____/|____|_|  \_\                             
                                                                    
      PROFESSIONAL BUTTON BOX SERVER - v2026.1
    ============================================================
    )" << std::endl;

    SetConsoleTextAttribute(hConsole, 10); // Yesil
    std::cout << " [SYSTEM] Mimari: x64 | Protokol: TCP/UDP IPv6 DualStack" << std::endl;
    std::cout << " [SYSTEM] Durum: Sistem baslatiliyor..." << std::endl;
    
    SetConsoleTextAttribute(hConsole, 7); // Normal Beyaz
}

// Global sunucu nesneleri (Kapatma handler'ı için)
Network::TCPServer* g_TcpServer = nullptr;
Network::UDPDiscoveryServer* g_UdpServer = nullptr;
std::atomic<bool> g_Running(true);

// Güvenli Kapatma Handler'ı
BOOL WINAPI ConsoleHandler(DWORD signal) {
    if (signal == CTRL_C_EVENT || signal == CTRL_CLOSE_EVENT) {
        Core::Logger::LogWarn("Kapatma sinyali alindi. Kaynaklar temizleniyor...");
        g_Running = false;
        if (g_TcpServer) g_TcpServer->Stop();
        if (g_UdpServer) g_UdpServer->Stop();
        
        Input::InputController::GetInstance().Shutdown();
        Telemetry::TelemetryListener::GetInstance().Shutdown();
        
        Core::Logger::LogInfo("Sunucu güvenli bir sekilde kapatildi. Bye! 👋");
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        WSACleanup();
        return TRUE;
    }
    return FALSE;
}

// Yerel IP adresini bulma (Modern & Robust)
std::string GetLocalIP() {
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) != 0) return "127.0.0.1";

    addrinfo hints = {}, *res = nullptr;
    hints.ai_family = AF_UNSPEC; // IPv4 veya IPv6
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(hostname, nullptr, &hints, &res) != 0) return "127.0.0.1";

    std::string ip = "127.0.0.1";
    for (addrinfo* ptr = res; ptr != nullptr; ptr = ptr->ai_next) {
        if (ptr->ai_family == AF_INET) { // IPv4 tercih et
            char buf[INET_ADDRSTRLEN];
            sockaddr_in* v4 = (sockaddr_in*)ptr->ai_addr;
            inet_ntop(AF_INET, &v4->sin_addr, buf, sizeof(buf));
            ip = buf;
            if (ip.find("192.168.") == 0 || ip.find("10.") == 0) break; // Yerel ağ IP'si bulursan dur
        }
    }

    freeaddrinfo(res);
    return ip;
}

// Güvenlik duvarını otomatik yapılandırma
void SetupFirewall() {
    std::string ruleName = "ETS2_ButtonBox_Server";
    
    // Önce varsa eski kuralı temizle
    std::string deleteCmd = "netsh advfirewall firewall delete rule name=\"" + ruleName + "\" > nul 2>&1";
    system(deleteCmd.c_str());

    // Yeni kuralı ekle (TCP 8888 & UDP 8889)
    std::string addTcp = "netsh advfirewall firewall add rule name=\"" + ruleName + 
                         "\" dir=in action=allow protocol=TCP localport=8888 profile=any description=\"ETS2 ButtonBox TCP Baglanti\" > nul 2>&1";
    
    std::string addUdp = "netsh advfirewall firewall add rule name=\"" + ruleName + "_UDP" +
                         "\" dir=in action=allow protocol=UDP localport=8889 profile=any description=\"ETS2 ButtonBox UDP Discovery\" > nul 2>&1";
    
    system(addTcp.c_str());
    int result = system(addUdp.c_str());

    if (result == 0) {
        Core::Logger::LogInfo("Güvenlik duvarı izinleri otomatik olarak tanımlandı. ✅");
    } else {
        Core::Logger::LogWarn("Güvenlik duvarı izni tanımlanamadı. Lütfen yönetici olarak çalıştırdığınızdan emin olun.");
    }
}

// Yönetici yetkisi kontrolü
bool IsRunAsAdmin() {
    BOOL isAdmin = FALSE;
    PSID adminGroup = NULL;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    if (AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                 DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroup)) {
        CheckTokenMembership(NULL, adminGroup, &isAdmin);
        FreeSid(adminGroup);
    }
    return isAdmin == TRUE;
}

int main(int argc, char* argv[]) {
    // UTF-8 ve Dil desteği
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    PrintBanner();

    // --- GUI INITIALIZATION ---
    auto& dashboard = GUI::Dashboard::GetInstance();
    dashboard.Create(GetModuleHandle(NULL));
    
    // Logger'ı Dashboard'a bağla (Logların pencerede görünmesi için şart)
    Core::Logger::SetCallback([&](const std::string& log) {
        dashboard.AddLog(log);
    });

    // Banner'ı bir an görmek için kısa bekleme
    std::this_thread::sleep_for(std::chrono::milliseconds(800));
    dashboard.HideConsole();

    // --- ADMIN CHECK & AUTO RESTART ---
    if (!IsRunAsAdmin()) {
        Core::Logger::LogWarn("Yönetici yetkisi yok. Yeniden başlatılıyor...");
        char szPath[MAX_PATH];
        GetModuleFileNameA(NULL, szPath, MAX_PATH);
        
        SHELLEXECUTEINFOA sei = { sizeof(sei) };
        sei.lpVerb = "runas";
        sei.lpFile = szPath;
        sei.hwnd = NULL;
        sei.nShow = SW_NORMAL;

        if (ShellExecuteExA(&sei)) {
            return 0; // Yetkili kopya başladı, bunu kapat.
        } else {
            Core::Logger::LogError("Yönetici yetkisi alınamadı. Bazı özellikler çalışmayabilir.");
        }
    }

    if (!SetConsoleCtrlHandler(ConsoleHandler, TRUE)) {
        Core::Logger::LogError("Kapatma handler'i kurulamadi.");
    }

    SetupFirewall();

    Core::Logger::LogInfo("========================================");
    Core::Logger::LogInfo("   ETS2 Professional Button Box Server  ");
    Core::Logger::LogInfo("========================================");
    
    auto& stateManager = State::StateManager::GetInstance();
    auto& telemetry = Telemetry::TelemetryListener::GetInstance();
    auto& input = Input::InputController::GetInstance();
    auto& config = Config::ConfigManager::GetInstance();
    
    input.Initialize();
    config.LoadConfig("buttons.json");
    
    fs::file_time_type lastConfigTime;
    try { lastConfigTime = fs::last_write_time("buttons.json"); } catch(...) {}

    Network::TCPServer server(8888);
    Network::UDPDiscoveryServer discovery(8889, 8888);
    g_TcpServer = &server;
    g_UdpServer = &discovery;

    if (!server.Start()) {
        Core::Logger::LogError("TCP Sunucu baslatilamadi.");
        return 1;
    }

    if (!discovery.Start()) {
        Core::Logger::LogWarn("UDP Discovery baslatilamadi.");
    }

    telemetry.Initialize();

    std::string myIP = GetLocalIP();
    dashboard.UpdateStatus("ÇALIŞIYOR 🟢");
    dashboard.UpdateClient("YOK 🔴");
    dashboard.UpdateLatency(0, 0); // Başlangıçta boş
    Core::Logger::LogInfo(std::format("🚀 SUNUCU AKTİF | IP: {} | Port: 8888", myIP));

    bool isDiscoveryMode = false;
    std::string currentDiscoveryId = "";

    std::string lastState;
    auto lastHeartbeat = std::chrono::steady_clock::now();
    auto lastStatsTime = std::chrono::steady_clock::now();
    auto lastStateUpdateTime = std::chrono::steady_clock::now();
    long long lastMeasuredLatency = 0; // μs

    server.SetOnMessageCallback([&](const std::string& msg) {
        auto arrivalTime = std::chrono::high_resolution_clock::now();
        try {
            auto j = nlohmann::json::parse(msg);
            
            if (j.contains("type") && j["type"] == "START_DISCOVERY") {
                isDiscoveryMode = true;
                currentDiscoveryId = j["id"];
                Core::Logger::LogInfo("Tuş algılama modu aktif...");
                return;
            }

            if (j.contains("type") && j["type"] == "CONFIG_UPDATE") {
                config.UpdateButton(j["id"], j["label"], (uint16_t)j["scanCode"]);
                config.SaveConfig();
                server.SendToClient("{\"type\":\"CONFIG_SUCCESS\"}");
                return;
            }

            if (j.contains("id")) {
                std::string id = j["id"];
                auto constButtons = config.GetAllButtons();
                if (constButtons.contains(id)) {
                    input.SendKeyPress(constButtons.at(id).scanCode);
                    
                    auto processTime = std::chrono::high_resolution_clock::now();
                    lastMeasuredLatency = std::chrono::duration_cast<std::chrono::microseconds>(processTime - arrivalTime).count();
                    // loopDuration henüz hesaplanmadığı için 0 yerine son değeri koru diyemeyiz, 
                    // ana döngü zaten 30ms sonra güncelleyeceği için burada sadece pingi logla
                    Core::Logger::LogDebug(std::format("Buton tetiklendi: {} | İşleme Gecikmesi: {} μs", id, lastMeasuredLatency));
                }
            }
        } catch (...) {}
    });

    while (g_Running && dashboard.IsOpen()) {
        auto loopStart = std::chrono::high_resolution_clock::now();
        
        dashboard.ProcessGUI();
        telemetry.Update(); 
        
        // --- CONFIG HOT-RELOAD (1sn) ---
        static auto lastConfigCheck = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - lastConfigCheck).count() >= 1) {
            try {
                if (fs::exists("buttons.json")) {
                    auto currentTime = fs::last_write_time("buttons.json");
                    if (currentTime != lastConfigTime) {
                        Core::Logger::LogInfo("Yapılandırma değişti, yeniden yükleniyor...");
                        if (config.LoadConfig("buttons.json")) {
                            lastConfigTime = currentTime;
                            server.SendToClient("{\"type\":\"CONFIG_RELOADED\"}");
                        }
                    }
                }
            } catch (...) {}
            lastConfigCheck = now;
        }

        // --- HEARTBEAT (5sn) ---
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - lastHeartbeat).count() >= 5) {
            server.SendToClient("{\"type\":\"HEARTBEAT\"}");
            lastHeartbeat = now;
        }

        // --- NETWORK STATS (10sn) ---
        if (std::chrono::duration_cast<std::chrono::seconds>(now - lastStatsTime).count() >= 10) {
            float sentKB = server.GetBytesSent() / 1024.0f;
            float recvKB = server.GetBytesReceived() / 1024.0f;
            if (sentKB > 0 || recvKB > 0) {
                Core::Logger::LogDebug(std::format("Ağ Trafiği (Son 10sn): Gönderilen: {:.2f} KB | Alınan: {:.2f} KB", sentKB, recvKB));
            }
            lastStatsTime = now;
        }

        // --- DISCOVERY MODE ---
        if (isDiscoveryMode) {
            for (int i = 8; i <= 255; i++) {
                if (GetAsyncKeyState(i) & 0x8000) {
                    UINT scanCode = MapVirtualKey(i, MAPVK_VK_TO_VSC);
                    if (scanCode > 0) {
                        server.SendToClient(std::format("{{\"type\":\"DISCOVERY_RESULT\",\"scanCode\":{}}}", scanCode));
                        isDiscoveryMode = false;
                        std::this_thread::sleep_for(std::chrono::milliseconds(300));
                        break;
                    }
                }
            }
        }
        
        // --- TELEMETRY STATE SYNC (Throttled to 50ms / 20Hz) ---
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastStateUpdateTime).count() >= 50) {
            nlohmann::json stateJson;
            stateJson["type"] = "STATE";
            stateJson["speed"] = stateManager.GetState<float>("speed", 0.0f);
            stateJson["engine"] = stateManager.GetState<bool>("engine_running", false);
            stateJson["fuel"] = stateManager.GetState<float>("fuel", 0.0f);
            stateJson["fuel_max"] = stateManager.GetState<float>("fuel_capacity", 0.0f);
            
            // Faz 8: Ağ Kalitesi
            stateJson["latency"] = lastMeasuredLatency;

            // Faz 7: İnteraktif Durumlar (Görsel Senkronizasyon)
            stateJson["blinker_left"] = stateManager.GetState<bool>("blinker_left", false);
            stateJson["blinker_right"] = stateManager.GetState<bool>("blinker_right", false);
            stateJson["blinker_hazard"] = stateManager.GetState<bool>("blinker_hazard", false);
            stateJson["beacon"] = stateManager.GetState<bool>("beacon", false);
            stateJson["retarder"] = stateManager.GetState<int>("retarder", 0);
            stateJson["diff_lock"] = stateManager.GetState<bool>("diff_lock", false);
            stateJson["cruise_control"] = stateManager.GetState<bool>("cruise_control", false);
            stateJson["parking_brake"] = stateManager.GetState<bool>("parking_brake", false);
            stateJson["lights_low"] = stateManager.GetState<bool>("lights_low", false);
            stateJson["lights_high"] = stateManager.GetState<bool>("lights_high", false);

            // Detaylı Hasar
            stateJson["wear_engine"] = stateManager.GetState<float>("wear_engine", 0.0f);
            stateJson["wear_transmission"] = stateManager.GetState<float>("wear_transmission", 0.0f);
            stateJson["wear_chassis"] = stateManager.GetState<float>("wear_chassis", 0.0f);
            stateJson["wear_wheels"] = stateManager.GetState<float>("wear_wheels", 0.0f);
            stateJson["wear_cabin"] = stateManager.GetState<float>("wear_cabin", 0.0f);

            // Navigasyon ve Konum
            stateJson["nav_dist"] = stateManager.GetState<float>("nav_distance", 0.0f);
            stateJson["posX"] = stateManager.GetState<float>("posX", 0.0f);
            stateJson["posZ"] = stateManager.GetState<float>("posZ", 0.0f);
            stateJson["heading"] = stateManager.GetState<float>("heading", 0.0f);

            // Metin Verileri
            stateJson["city"] = stateManager.GetState<std::string>("city", "");
            stateJson["cargo"] = stateManager.GetState<std::string>("cargo", "");
            stateJson["destination"] = stateManager.GetState<std::string>("destination", "");
            stateJson["truck"] = stateManager.GetState<std::string>("truck_name", "");

            // Lojistik Verileri
            stateJson["consumption"] = stateManager.GetState<float>("fuel_consumption", 0.0f);
            stateJson["nav_time"] = stateManager.GetState<float>("nav_time", 0.0f);
            stateJson["rest_stop"] = stateManager.GetState<int>("rest_stop", 0);

            std::string currentState = stateJson.dump();
            if (currentState != lastState) {
                server.SendToClient(currentState);
                lastState = currentState;
            }
            lastStateUpdateTime = now;
        }
        
        auto loopEnd = std::chrono::high_resolution_clock::now();
        auto loopDuration = std::chrono::duration_cast<std::chrono::milliseconds>(loopEnd - loopStart).count();
        
        // Her döngüde sistemi GUI'de güncelle (Anlık ms takibi)
        dashboard.UpdateLatency(lastMeasuredLatency, loopDuration);

        if (loopDuration > 50) {
            Core::Logger::LogWarn(std::format("Sunucu döngüsü yavaşladı! Süre: {}ms", loopDuration));
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }

    return 0;
}
