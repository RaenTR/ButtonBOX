#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <thread>
#include <atomic>
#include "../core/Logger.h"

namespace Network {

class UDPDiscoveryServer {
public:
    UDPDiscoveryServer(int listenPort, int tcpPort) 
        : m_ListenPort(listenPort), m_TcpPort(tcpPort), m_Socket(INVALID_SOCKET), m_Running(false) {}
    
    ~UDPDiscoveryServer() { Stop(); }

    bool Start() {
        m_Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (m_Socket == INVALID_SOCKET) {
            Core::Logger::LogError("UDP soket oluşturulamadı.");
            return false;
        }

        // Broadcast izni ver
        BOOL broadcast = TRUE;
        if (setsockopt(m_Socket, SOL_SOCKET, SO_BROADCAST, (const char*)&broadcast, sizeof(broadcast)) == SOCKET_ERROR) {
            Core::Logger::LogWarn("UDP Broadcast modu aktif edilemedi.");
        }

        sockaddr_in serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(m_ListenPort);

        if (bind(m_Socket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            Core::Logger::LogError(std::format("UDP bağ (bind) işlemi başarısız (Port: {}). Hata: {}", m_ListenPort, WSAGetLastError()));
            closesocket(m_Socket);
            return false;
        }

        m_Running = true;
        m_Thread = std::thread(&UDPDiscoveryServer::DiscoveryLoop, this);
        Core::Logger::LogInfo(std::format("UDP Discovery (IPv4) {} portunda aktif.", m_ListenPort));
        return true;
    }

    void Stop() {
        m_Running = false;
        if (m_Socket != INVALID_SOCKET) {
            closesocket(m_Socket);
            m_Socket = INVALID_SOCKET;
        }
        if (m_Thread.joinable()) m_Thread.join();
    }

private:
    void DiscoveryLoop() {
        char buffer[1024];
        sockaddr_in clientAddr;

        while (m_Running) {
            int clientAddrSize = sizeof(clientAddr);
            int bytesReceived = recvfrom(m_Socket, buffer, sizeof(buffer) - 1, 0, (sockaddr*)&clientAddr, &clientAddrSize);
            
            if (bytesReceived > 0) {
                buffer[bytesReceived] = '\0';
                std::string msg(buffer);

                if (msg == "DISCOVER_BUTTONBOX_SERVER") {
                    std::string response = std::format(
                        "{{\"type\":\"SERVER_INFO\",\"name\":\"ETS2_ButtonBox_Pro\",\"port\":{},\"version\":\"2.0\"}}",
                        m_TcpPort
                    );
                    sendto(m_Socket, response.c_str(), (int)response.length(), 0, (sockaddr*)&clientAddr, clientAddrSize);
                }
            } else if (bytesReceived < 0) {
                int error = WSAGetLastError();
                if (error != WSAEINTR && m_Running) {
                    Core::Logger::LogWarn(std::format("UDP Discovery soket hatası (Hata: {}). Döngü durduruluyor.", error));
                }
                break; // Hata durumunda busy loop engellemek için döngüyü bitir
            }
        }
    }

    int m_ListenPort;
    int m_TcpPort;
    SOCKET m_Socket;
    std::atomic<bool> m_Running;
    std::thread m_Thread;
};

} // namespace Network
