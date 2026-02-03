#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <windows.h>
#include <string>
#include <vector>
#include <mutex>

namespace GUI {

class Dashboard {
public:
    static Dashboard& GetInstance() {
        static Dashboard instance;
        return instance;
    }

    bool Create(HINSTANCE hInstance) {
        m_hInstance = hInstance;
        
        // --- MODERN TASARIM PALETI ---
        m_hFont = CreateFontW(22, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
            CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");
        m_hLogFont = CreateFontW(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
            CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Consolas");
            
        m_hBgBrush = CreateSolidBrush(RGB(15, 15, 15));      // Ultra Dark
        m_hLabelBrush = CreateSolidBrush(RGB(25, 25, 25));   // Card Background
        m_hLogBrush = CreateSolidBrush(RGB(10, 10, 10));     // Abyss Black

        WNDCLASSEXW wc = {0};
        wc.cbSize = sizeof(WNDCLASSEXW);
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = L"ETS2_Pro_Dashboard_V2";
        wc.hbrBackground = m_hBgBrush; 
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);

        if (!RegisterClassExW(&wc)) return false;

        m_hWnd = CreateWindowExW(0, L"ETS2_Pro_Dashboard_V2", L"ETS2 Button Box - Professional Control Panel",
            WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 700, 600,
            NULL, NULL, hInstance, NULL);

        if (!m_hWnd) return false;

        // --- UI BILESENLERI (Owner Draw Feeling) ---
        // \u0130 = İ, \u015e = Ş
        m_StatusLabel = CreateWindowExW(0, L"STATIC", L"Sistem Durumu: HAZIRLANIYOR...",
            WS_CHILD | WS_VISIBLE | SS_CENTER, 20, 20, 640, 50, m_hWnd, NULL, hInstance, NULL);
        SendMessageW(m_StatusLabel, WM_SETFONT, (WPARAM)m_hFont, TRUE);
        
        m_ClientLabel = CreateWindowExW(0, L"STATIC", L"\u0130stemci: BA\u011eLI DE\u011e\u0130L",
            WS_CHILD | WS_VISIBLE | SS_CENTER, 20, 80, 640, 50, m_hWnd, NULL, hInstance, NULL);
        SendMessageW(m_ClientLabel, WM_SETFONT, (WPARAM)m_hFont, TRUE);

        m_GameLabel = CreateWindowExW(0, L"STATIC", L"Oyun: BEKLEN\u0130YOR...",
            WS_CHILD | WS_VISIBLE | SS_CENTER, 20, 140, 640, 40, m_hWnd, NULL, hInstance, NULL);
        SendMessageW(m_GameLabel, WM_SETFONT, (WPARAM)m_hFont, TRUE);

        m_LatencyLabel = CreateWindowExW(0, L"STATIC", L"Gecikme (Ping): -- ms",
            WS_CHILD | WS_VISIBLE | SS_CENTER, 20, 190, 640, 40, m_hWnd, NULL, hInstance, NULL);
        SendMessageW(m_LatencyLabel, WM_SETFONT, (WPARAM)m_hFont, TRUE);

        m_LogBox = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
            20, 240, 640, 290, m_hWnd, NULL, hInstance, NULL);
        SendMessageW(m_LogBox, WM_SETFONT, (WPARAM)m_hLogFont, TRUE);

        return true;
    }

    void UpdateStatus(const std::string& statusUTF8) {
        if (m_StatusLabel) {
            std::wstring wide = ToWide(statusUTF8);
            std::wstring newText = L"Sistem Durumu: " + wide;
            wchar_t current[256];
            GetWindowTextW(m_StatusLabel, current, 256);
            if (newText != current) {
                SetWindowTextW(m_StatusLabel, newText.c_str());
                InvalidateRect(m_StatusLabel, NULL, TRUE);
            }
        }
    }

    void UpdateClient(const std::string& clientUTF8) {
        if (m_ClientLabel) {
            std::wstring wide = ToWide(clientUTF8);
            std::wstring newText = L"\u0130stemci: " + wide;
            wchar_t current[256];
            GetWindowTextW(m_ClientLabel, current, 256);
            if (newText != current) {
                SetWindowTextW(m_ClientLabel, newText.c_str());
                InvalidateRect(m_ClientLabel, NULL, TRUE);
            }
        }
    }

    void UpdateGame(const std::string& gameUTF8) {
        if (m_GameLabel) {
            std::wstring wide = ToWide(gameUTF8);
            std::wstring newText = L"Oyun: " + wide;
            wchar_t current[256];
            GetWindowTextW(m_GameLabel, current, 256);
            if (newText != current) {
                SetWindowTextW(m_GameLabel, newText.c_str());
                InvalidateRect(m_GameLabel, NULL, TRUE);
            }
        }
    }

    void UpdateLatency(long long pingUs, long long systemMs) {
        if (m_LatencyLabel) {
            float pingMs = pingUs / 1000.0f;
            
            std::wstring pingPart = (pingUs > 0) ? std::to_wstring((int)pingMs) + L" ms" : L"-- ms";
            std::wstring systemPart = std::to_wstring((int)systemMs) + L" ms";
            
            std::wstring newText = L"\u0130leti\u015fim: " + pingPart + L"  |  Sistem D\u00f6ng\u00fcs\u00fc: " + systemPart;
            
            wchar_t current[256];
            GetWindowTextW(m_LatencyLabel, current, 256);
            if (newText != current) {
                SetWindowTextW(m_LatencyLabel, newText.c_str());
                // Force immediate redraw to prevent flicker/delay
                InvalidateRect(m_LatencyLabel, NULL, FALSE); 
                UpdateWindow(m_LatencyLabel);
            }
        }
    }

    void AddLog(const std::string& logUTF8) {
        std::lock_guard<std::mutex> lock(m_LogMutex);
        m_PendingLogs.push_back(logUTF8);
    }

    void ProcessGUI() {
        {
            std::lock_guard<std::mutex> lock(m_LogMutex);
            if (!m_PendingLogs.empty()) {
                std::wstring allLogs;
                for (const auto& log : m_PendingLogs) {
                    allLogs += ToWide(log) + L"\r\n";
                }
                
                int len = GetWindowTextLengthW(m_LogBox);
                SendMessageW(m_LogBox, EM_SETSEL, len, len);
                SendMessageW(m_LogBox, EM_REPLACESEL, 0, (LPARAM)allLogs.c_str());
                m_PendingLogs.clear();
            }
        }

        MSG msg;
        while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) return;
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    void HideConsole() {
        HWND hConsole = GetConsoleWindow();
        if (hConsole) ShowWindow(hConsole, SW_HIDE);
    }

    bool IsOpen() const { return IsWindow(m_hWnd); }

private:
    Dashboard() : m_hWnd(NULL), m_StatusLabel(NULL), m_ClientLabel(NULL), m_GameLabel(NULL), m_LogBox(NULL), 
                  m_hFont(NULL), m_hLogFont(NULL), m_hBgBrush(NULL), m_hLabelBrush(NULL), m_hLogBrush(NULL) {}

    std::wstring ToWide(const std::string& str) {
        if (str.empty()) return L"";
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
        std::wstring wstrTo(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
        return wstrTo;
    }

    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        Dashboard& ds = GetInstance();
        switch (uMsg) {
            case WM_SIZE: {
                int width = LOWORD(lParam);
                int height = HIWORD(lParam);
                
                // Orantili dikey dagilim (Padding: 10, Label Height: 45)
                if (ds.m_StatusLabel) MoveWindow(ds.m_StatusLabel, 20, 10, width - 40, 40, TRUE);
                if (ds.m_ClientLabel) MoveWindow(ds.m_ClientLabel, 20, 55, width - 40, 40, TRUE);
                if (ds.m_GameLabel) MoveWindow(ds.m_GameLabel, 20, 100, width - 40, 40, TRUE);
                if (ds.m_LatencyLabel) MoveWindow(ds.m_LatencyLabel, 20, 145, width - 40, 40, TRUE);
                
                // Log kutusu kalan tum alani kaplasin
                if (ds.m_LogBox) MoveWindow(ds.m_LogBox, 20, 200, width - 40, height - 220, TRUE);
                
                // Eski metinlerin hayalet gibi kalmamasi icin tumu yeniden ciz
                InvalidateRect(hWnd, NULL, TRUE);
                return 0;
            }
            case WM_CTLCOLORSTATIC: {
                HDC hdcStatic = (HDC)wParam;
                HWND hStatic = (HWND)lParam;
                
                SetBkMode(hdcStatic, TRANSPARENT);
                
                // --- RENK DINAMIKLERI ---
                wchar_t text[512];
                GetWindowTextW(hStatic, text, 512);
                std::wstring wText(text);

                if (wText.find(L"\u0020\u2705") != std::wstring::npos || // ✅
                    wText.find(L"\u0020\U0001F7E2") != std::wstring::npos || // 🟢
                    wText.find(L"\u00c7ALIS\u0130YOR") != std::wstring::npos || // ÇALIŞIYOR
                    wText.find(L"\u00c7ALI\u015eIYOR") != std::wstring::npos ||
                    wText.find(L"A\u00c7IK") != std::wstring::npos) // AÇIK
                {
                    SetTextColor(hdcStatic, RGB(46, 204, 113)); // Emerald Green
                }
                else if (wText.find(L"ms") != std::wstring::npos) {
                    // Latency Coloring: < 50ms Green, < 150ms Yellow, > 150ms Red
                    try {
                        size_t pos = wText.find(L": ");
                        if (pos != std::wstring::npos) {
                            std::wstring valStr = wText.substr(pos + 2);
                            int val = std::stoi(valStr);
                            if (val < 50) SetTextColor(hdcStatic, RGB(46, 204, 113));
                            else if (val < 150) SetTextColor(hdcStatic, RGB(241, 196, 15));
                            else SetTextColor(hdcStatic, RGB(231, 76, 60));
                        }
                    } catch (...) {
                        SetTextColor(hdcStatic, RGB(200, 200, 200));
                    }
                }
                else if (wText.find(L"\u0020\u274C") != std::wstring::npos || // ❌
                         wText.find(L"\u0020\U0001F534") != std::wstring::npos || // 🔴
                         wText.find(L"YOK") != std::wstring::npos ||
                         wText.find(L"DE\u011e\u0130L") != std::wstring::npos ||
                         wText.find(L"KAPALI") != std::wstring::npos ||
                         wText.find(L"BEKLEN\u0130YOR") != std::wstring::npos)
                {
                    SetTextColor(hdcStatic, RGB(231, 76, 60));  // Alizarin Red
                }
                else {
                    SetTextColor(hdcStatic, RGB(200, 200, 200)); // Default Light Gray
                }
                
                return (LRESULT)ds.m_hLabelBrush;
            }
            case WM_CTLCOLOREDIT: {
                HDC hdcEdit = (HDC)wParam;
                SetTextColor(hdcEdit, RGB(189, 195, 199)); // Silver Log Text
                SetBkColor(hdcEdit, RGB(10, 10, 10));
                return (LRESULT)ds.m_hLogBrush;
            }
            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;
        }
        return DefWindowProcW(hWnd, uMsg, wParam, lParam);
    }

    std::mutex m_LogMutex;
    std::vector<std::string> m_PendingLogs;
    HINSTANCE m_hInstance;
    HWND m_hWnd, m_StatusLabel, m_ClientLabel, m_GameLabel, m_LatencyLabel, m_LogBox;
    HFONT m_hFont, m_hLogFont;
    HBRUSH m_hBgBrush, m_hLabelBrush, m_hLogBrush;
};

} // namespace GUI
