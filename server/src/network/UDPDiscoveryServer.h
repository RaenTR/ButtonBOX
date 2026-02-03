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
            Core::Logger::LogError("UDP soket olusturulamadi.");
            return false;
        }

        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(m_ListenPort);

        if (bind(m_Socket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            Core::Logger::LogError("UDP bind islemi basarisiz.");
            closesocket(m_Socket);
            return false;
        }

        m_Running = true;
        m_Thread = std::thread(&UDPDiscoveryServer::DiscoveryLoop, this);
        Core::Logger::LogInfo(std::format("UDP Discovery {} portunda aktif.", m_ListenPort));
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
        int clientAddrSize = sizeof(clientAddr);

        while (m_Running) {
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
