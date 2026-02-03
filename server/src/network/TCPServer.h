#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <vector>
#include <functional>
#include <atomic>
#include <thread>
#include "../core/Logger.h"

namespace Network {

class TCPServer {
public:
    using MessageCallback = std::function<void(const std::string&)>;

    TCPServer(int port) : m_Port(port), m_ListenSocket(INVALID_SOCKET), m_ClientSocket(INVALID_SOCKET), m_Running(false) {}
    ~TCPServer() { Stop(); }

    bool Start() {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            Core::Logger::LogError("WSAStartup basarisiz.");
            return false;
        }

        // IPv6 soketi oluştur (Dual-stack için)
        m_ListenSocket = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
        if (m_ListenSocket == INVALID_SOCKET) {
            Core::Logger::LogError("Soket olusturulamadi.");
            WSACleanup();
            return false;
        }

        // DUAL-STACK Modu Aktif Et (IPv4 ve IPv6'yı aynı anda kabul et)
        int no = 0;
        if (setsockopt(m_ListenSocket, IPPROTO_IPV6, IPV6_V6ONLY, (const char*)&no, sizeof(no)) == SOCKET_ERROR) {
            Core::Logger::LogWarn("Dual-stack (V4/V6) modu aktif edilemedi, sadece IPv6 çalışabilir.");
        }

        sockaddr_in6 serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin6_family = AF_INET6;
        serverAddr.sin6_addr = in6addr_any;
        serverAddr.sin6_port = htons(m_Port);

        if (bind(m_ListenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            Core::Logger::LogError(std::format("Bind islemi basarisiz (Port: {}). Hata: {}", m_Port, WSAGetLastError()));
            closesocket(m_ListenSocket);
            WSACleanup();
            return false;
        }

        if (listen(m_ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
            Core::Logger::LogError("Listen islemi basarisiz.");
            closesocket(m_ListenSocket);
            WSACleanup();
            return false;
        }

        m_Running = true;
        m_AcceptThread = std::thread(&TCPServer::AcceptLoop, this);
        Core::Logger::LogInfo(std::format("TCP Sunucu {} portunda dinlemede.", m_Port));
        return true;
    }

    void Stop() {
        m_Running = false;
        if (m_ListenSocket != INVALID_SOCKET) {
            closesocket(m_ListenSocket);
            m_ListenSocket = INVALID_SOCKET;
        }
        if (m_ClientSocket != INVALID_SOCKET) {
            closesocket(m_ClientSocket);
            m_ClientSocket = INVALID_SOCKET;
        }
        if (m_AcceptThread.joinable()) m_AcceptThread.join();
        if (m_ClientThread.joinable()) m_ClientThread.join();
        WSACleanup();
    }

    void SetOnMessageCallback(MessageCallback cb) { m_OnMessage = cb; }

    void SendToClient(const std::string& data) {
        if (m_ClientSocket != INVALID_SOCKET) {
            std::string msgWithNewLine = data + "\n";
            int result = send(m_ClientSocket, msgWithNewLine.c_str(), (int)msgWithNewLine.length(), 0);
            if (result != SOCKET_ERROR) {
                m_BytesSent += result;
            }
        }
    }

    // Profiling için istatistikleri al
    uint64_t GetBytesSent() { return m_BytesSent.exchange(0); }
    uint64_t GetBytesReceived() { return m_BytesReceived.exchange(0); }

private:
    void AcceptLoop() {
        while (m_Running) {
            sockaddr_in6 clientAddr;
            int clientAddrSize = sizeof(clientAddr);
            SOCKET clientSocket = accept(m_ListenSocket, (sockaddr*)&clientAddr, &clientAddrSize);

            if (clientSocket == INVALID_SOCKET) {
                if (m_Running) Core::Logger::LogWarn("İstemci kabul edilemedi.");
                continue;
            }

            // Yeni bağlantı geldiğinde, varsa eskiyi temizle (Smart Reconnect - Zombi temizliği)
            if (m_ClientSocket != INVALID_SOCKET) {
                Core::Logger::LogWarn("Mevcut bağlantı üzerine yeni bağlantı geldi. Eski bağlantı sonlandırılıyor...");
                closesocket(m_ClientSocket);
                m_ClientSocket = INVALID_SOCKET;
                if (m_ClientThread.joinable()) m_ClientThread.join();
            }

            char ipStr[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, &clientAddr.sin6_addr, ipStr, INET6_ADDRSTRLEN);
            Core::Logger::LogInfo(std::format("Yeni istemci bağlandı: {}", ipStr));
            GUI::Dashboard::GetInstance().UpdateClient(std::format("{} 🟢", ipStr));

            // --- TCP_NODELAY OPTİMİZASYONU ---
            BOOL nodelay = TRUE;
            if (setsockopt(clientSocket, IPPROTO_TCP, TCP_NODELAY, (const char*)&nodelay, sizeof(nodelay)) == SOCKET_ERROR) {
                Core::Logger::LogWarn("TCP_NODELAY aktif edilemedi.");
            } else {
                Core::Logger::LogDebug("Ağ Optimizasyonu: TCP_NODELAY aktif. ✅");
            }

            m_ClientSocket = clientSocket;
            m_ClientThread = std::thread(&TCPServer::ClientLoop, this);
        }
    }

    void ClientLoop() {
        char buffer[4096];
        while (m_Running && m_ClientSocket != INVALID_SOCKET) {
            int bytesReceived = recv(m_ClientSocket, buffer, 4096, 0);
            if (bytesReceived > 0) {
                m_BytesReceived += bytesReceived;
                std::string msg(buffer, bytesReceived);
                if (m_OnMessage) m_OnMessage(msg);
            } else if (bytesReceived == 0) {
                Core::Logger::LogInfo("İstemci bağlantısı normal şekilde kapandı.");
                break;
            } else {
                int error = WSAGetLastError();
                if (error != WSAEINTR && m_Running) {
                    Core::Logger::LogWarn(std::format("İstemci bağlantısı hata ile koptu (Hata: {})", error));
                }
                break;
            }
        }
        
        // Döngü bittiğinde soketi temizle
        if (m_ClientSocket != INVALID_SOCKET) {
            closesocket(m_ClientSocket);
            m_ClientSocket = INVALID_SOCKET;
            GUI::Dashboard::GetInstance().UpdateClient("BAĞLI DEĞİL 🔴");
            GUI::Dashboard::GetInstance().UpdateLatency(0, 0);
        }
    }

    int m_Port;
    SOCKET m_ListenSocket;
    SOCKET m_ClientSocket;
    std::atomic<bool> m_Running;
    std::atomic<uint64_t> m_BytesSent{0};
    std::atomic<uint64_t> m_BytesReceived{0};
    std::thread m_AcceptThread;
    std::thread m_ClientThread;
    MessageCallback m_OnMessage;
};

} // namespace Network
