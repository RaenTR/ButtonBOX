#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <windows.h>
#include <string>
#include <vector>
#include <mutex>
#include <format>
#include "../resource.h"

namespace GUI {

class Dashboard {
public:
    static Dashboard& GetInstance() {
        static Dashboard instance;
        return instance;
    }

    bool Create(HINSTANCE hInstance) {
        m_hInstance = hInstance;
        
        // --- MODERN TASARIM PALETİ ---
        m_hHeaderFont = CreateFontW(14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
            CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");
        m_hFont = CreateFontW(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
            CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");
        m_hLogFont = CreateFontW(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
            CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Consolas");
            
        m_hBgBrush = CreateSolidBrush(RGB(10, 10, 10));      // Abyss Black
        m_hLabelBrush = CreateSolidBrush(RGB(20, 20, 20));   // Dark Card
        m_hLogBrush = CreateSolidBrush(RGB(5, 5, 5));        // Deep Black

        WNDCLASSEXW wc = {0};
        wc.cbSize = sizeof(WNDCLASSEXW);
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = L"ETS2_Pro_Dashboard_V2";
        wc.hbrBackground = m_hBgBrush; 
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(101)); // IDI_ICON1 (101)
        wc.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(101));

        if (!RegisterClassExW(&wc)) return false;

        m_hWnd = CreateWindowExW(0, L"ETS2_Pro_Dashboard_V2", L"ETS2 Button Box Server - Pro v2.1",
            WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 600, 500,
            NULL, NULL, hInstance, NULL);

        if (!m_hWnd) return false;

        // --- UI BİLEŞENLERİ ---
        // 1. Satır: Sunucu Bilgileri (ÜST)
        m_ServerInfoLabel = CreateWindowExW(0, L"STATIC", L"SUNUCU: BEKLEN\u0130YOR...",
            WS_CHILD | WS_VISIBLE | SS_CENTER, 10, 5, 580, 25, m_hWnd, NULL, hInstance, NULL);
        SendMessageW(m_ServerInfoLabel, WM_SETFONT, (WPARAM)m_hHeaderFont, TRUE);

        // 2. Satır: Sistem & İstemci
        m_StatusLabel = CreateWindowExW(0, L"STATIC", L"S\u0130STEM: HAZIR",
            WS_CHILD | WS_VISIBLE | SS_CENTER, 10, 35, 285, 30, m_hWnd, NULL, hInstance, NULL);
        SendMessageW(m_StatusLabel, WM_SETFONT, (WPARAM)m_hFont, TRUE);
        
        m_ClientLabel = CreateWindowExW(0, L"STATIC", L"\u0130STEMC\u0130: YOK",
            WS_CHILD | WS_VISIBLE | SS_CENTER, 305, 35, 285, 30, m_hWnd, NULL, hInstance, NULL);
        SendMessageW(m_ClientLabel, WM_SETFONT, (WPARAM)m_hFont, TRUE);

        // 3. Satır: Oyun, Ping, Döngü
        m_GameLabel = CreateWindowExW(0, L"STATIC", L"OYUN: BEKLEMEDE",
            WS_CHILD | WS_VISIBLE | SS_CENTER, 10, 70, 190, 25, m_hWnd, NULL, hInstance, NULL);
        SendMessageW(m_GameLabel, WM_SETFONT, (WPARAM)m_hFont, TRUE);

        m_LatencyLabel = CreateWindowExW(0, L"STATIC", L"P\u0130NG: --",
            WS_CHILD | WS_VISIBLE | SS_CENTER, 205, 70, 190, 25, m_hWnd, NULL, hInstance, NULL);
        SendMessageW(m_LatencyLabel, WM_SETFONT, (WPARAM)m_hFont, TRUE);

        m_SystemLabel = CreateWindowExW(0, L"STATIC", L"D\u00d6NG\u00dc: --",
            WS_CHILD | WS_VISIBLE | SS_CENTER, 400, 70, 190, 25, m_hWnd, NULL, hInstance, NULL);
        SendMessageW(m_SystemLabel, WM_SETFONT, (WPARAM)m_hFont, TRUE);

        // Log Alanı
        m_LogBox = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
            10, 105, 580, 350, m_hWnd, NULL, hInstance, NULL);
        SendMessageW(m_LogBox, WM_SETFONT, (WPARAM)m_hLogFont, TRUE);

        return true;
    }

    void UpdateServerInfo(const std::string& info) {
        if (m_ServerInfoLabel) {
            std::wstring wide = ToWide(info);
            std::wstring text = L"BA\u011eLANTI: " + wide;
            SetWindowTextW(m_ServerInfoLabel, text.c_str());
        }
    }

    void UpdateStatus(const std::string& statusUTF8) {
        if (m_StatusLabel) {
            std::wstring wide = ToWide(statusUTF8);
            std::wstring newText = L"S\u0130STEM: " + wide;
            SetWindowTextW(m_StatusLabel, newText.c_str());
        }
    }

    void UpdateClient(const std::string& clientUTF8) {
        if (m_ClientLabel) {
            std::wstring wide = ToWide(clientUTF8);
            std::wstring newText = L"\u0130STEMC\u0130: " + wide;
            SetWindowTextW(m_ClientLabel, newText.c_str());
        }
    }

    void UpdateGame(const std::string& gameUTF8) {
        if (m_GameLabel) {
            std::wstring wide = ToWide(gameUTF8);
            std::wstring newText = L"OYUN: " + wide;
            SetWindowTextW(m_GameLabel, newText.c_str());
        }
    }

    void UpdateLatency(long long pingUs, long long systemUs) {
        if (m_LatencyLabel && m_SystemLabel) {
            float pingMs = pingUs / 1000.0f;
            float systemMs = systemUs / 1000.0f;
            
            std::wstring pingText = L"P\u0130NG: " + ((pingUs > 0) ? ToWide(std::format("{:.1f} ms", pingMs)) : L"-- ms");
            std::wstring systemText = L"D\u00d6NG\u00dc: " + ToWide(std::format("{:.2f} ms", systemMs));
            
            SetWindowTextW(m_LatencyLabel, pingText.c_str());
            SetWindowTextW(m_SystemLabel, systemText.c_str());
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
                  m_ServerInfoLabel(NULL), m_LatencyLabel(NULL), m_SystemLabel(NULL),
                  m_hFont(NULL), m_hLogFont(NULL), m_hHeaderFont(NULL), 
                  m_hBgBrush(NULL), m_hLabelBrush(NULL), m_hLogBrush(NULL) {}

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
                int p = 10; // padding

                if (ds.m_ServerInfoLabel) MoveWindow(ds.m_ServerInfoLabel, p, 5, width - (2*p), 25, TRUE);
                
                int halfW = (width - (3*p)) / 2;
                if (ds.m_StatusLabel) MoveWindow(ds.m_StatusLabel, p, 35, halfW, 30, TRUE);
                if (ds.m_ClientLabel) MoveWindow(ds.m_ClientLabel, (2*p) + halfW, 35, halfW, 30, TRUE);

                int thirdW = (width - (4*p)) / 3;
                if (ds.m_GameLabel) MoveWindow(ds.m_GameLabel, p, 70, thirdW, 25, TRUE);
                if (ds.m_LatencyLabel) MoveWindow(ds.m_LatencyLabel, (2*p) + thirdW, 70, thirdW, 25, TRUE);
                if (ds.m_SystemLabel) MoveWindow(ds.m_SystemLabel, (3*p) + (2*thirdW), 70, thirdW, 25, TRUE);
                
                if (ds.m_LogBox) MoveWindow(ds.m_LogBox, p, 105, width - (2*p), height - 115, TRUE);
                
                InvalidateRect(hWnd, NULL, TRUE);
                return 0;
            }
            case WM_CTLCOLORSTATIC: {
                HDC hdc = (HDC)wParam;
                HWND hCtrl = (HWND)lParam;
                SetBkMode(hdc, TRANSPARENT);
                
                wchar_t text[512];
                GetWindowTextW(hCtrl, text, 512);
                std::wstring wText(text);

                if (wText.find(L"\u2705") != std::wstring::npos || wText.find(L"\U0001F7E2") != std::wstring::npos || 
                    wText.find(L"\u00c7ALI\u015eIYOR") != std::wstring::npos || wText.find(L"A\u00c7IK") != std::wstring::npos) {
                    SetTextColor(hdc, RGB(46, 204, 113));
                }
                else if (wText.find(L"ms") != std::wstring::npos) {
                    try {
                        size_t pos = wText.find(L": ");
                        if (pos != std::wstring::npos) {
                            float val = std::stof(wText.substr(pos + 2));
                            if (val < 50.0f) SetTextColor(hdc, RGB(46, 204, 113));
                            else if (val < 150.0f) SetTextColor(hdc, RGB(241, 196, 15));
                            else SetTextColor(hdc, RGB(231, 76, 60));
                        }
                    } catch (...) { SetTextColor(hdc, RGB(200, 200, 200)); }
                }
                else if (wText.find(L"\u274C") != std::wstring::npos || wText.find(L"\U0001F534") != std::wstring::npos || 
                         wText.find(L"YOK") != std::wstring::npos || wText.find(L"KAPALI") != std::wstring::npos) {
                    SetTextColor(hdc, RGB(231, 76, 60));
                }
                else { SetTextColor(hdc, RGB(200, 200, 200)); }
                
                return (LRESULT)ds.m_hLabelBrush;
            }
            case WM_CTLCOLOREDIT: {
                HDC hdc = (HDC)wParam;
                SetTextColor(hdc, RGB(180, 180, 180));
                SetBkColor(hdc, RGB(5, 5, 5));
                return (LRESULT)ds.m_hLogBrush;
            }
            case WM_DESTROY: PostQuitMessage(0); return 0;
        }
        return DefWindowProcW(hWnd, uMsg, wParam, lParam);
    }

    std::mutex m_LogMutex;
    std::vector<std::string> m_PendingLogs;
    HINSTANCE m_hInstance;
    HWND m_hWnd, m_StatusLabel, m_ClientLabel, m_GameLabel, m_LatencyLabel, m_SystemLabel, m_LogBox, m_ServerInfoLabel;
    HFONT m_hFont, m_hLogFont, m_hHeaderFont;
    HBRUSH m_hBgBrush, m_hLabelBrush, m_hLogBrush;
};

} // namespace GUI
