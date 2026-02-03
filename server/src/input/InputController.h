#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <windows.h>
#include <string>
#include <map>
#include <queue>
#include <thread>
#include <atomic>
#include <condition_variable>
#include "../core/Logger.h"

namespace Input {

class InputController {
public:
    static InputController& GetInstance() {
        static InputController instance;
        return instance;
    }

    ~InputController() { Shutdown(); }

    void Initialize() {
        if (m_Running) return;
        m_Running = true;
        m_WorkerThread = std::thread(&InputController::WorkerLoop, this);
        Core::Logger::LogInfo("Input Worker Thread baslatildi.");
    }

    void Shutdown() {
        m_Running = false;
        m_CV.notify_all();
        if (m_WorkerThread.joinable()) m_WorkerThread.join();
        Core::Logger::LogInfo("Input Worker Thread durduruldu.");
    }

    // Belirli bir tuşu kuyruğa ekle (Asenkron)
    void SendKeyPress(uint16_t scanCode) {
        {
            std::lock_guard<std::mutex> lock(m_QueueMutex);
            m_Queue.push(scanCode);
        }
        m_CV.notify_one();
    }

    // Doğrudan tuş durumu ayarla (Senkron/Immediate)
    void SetKeyState(uint16_t scanCode, bool down) {
        std::lock_guard<std::mutex> lock(m_InputMutex);
        SendInputInternal(scanCode, down);
    }

private:
    InputController() : m_Running(false) { Initialize(); }

    void WorkerLoop() {
        while (m_Running) {
            uint16_t scanCode = 0;
            {
                std::unique_lock<std::mutex> lock(m_QueueMutex);
                m_CV.wait(lock, [this] { return !m_Queue.empty() || !m_Running; });
                
                if (!m_Running && m_Queue.empty()) break;
                
                scanCode = m_Queue.front();
                m_Queue.pop();
            }

            // Simülasyon yap
            {
                std::lock_guard<std::mutex> lock(m_InputMutex);
                SendInputInternal(scanCode, true);  // Down
                Sleep(40);                          // ETS2 için bekleme (Queue sayesinde ana thread'i bloklamaz)
                SendInputInternal(scanCode, false); // Up
            }
            Sleep(10); // Tuşlar arası min. boşluk
        }
    }

    void SendInputInternal(uint16_t scanCode, bool down) {
        INPUT input = { 0 };
        input.type = INPUT_KEYBOARD;
        input.ki.wScan = scanCode;
        input.ki.dwFlags = KEYEVENTF_SCANCODE | (down ? 0 : KEYEVENTF_KEYUP);

        if (SendInput(1, &input, sizeof(INPUT)) == 0) {
            Core::Logger::LogError(std::format("SendInput basarisiz! Hata: {}", GetLastError()));
        }
    }

    std::atomic<bool> m_Running;
    std::thread m_WorkerThread;
    std::queue<uint16_t> m_Queue;
    std::mutex m_QueueMutex;
    std::mutex m_InputMutex;
    std::condition_variable m_CV;
};

} // namespace Input
