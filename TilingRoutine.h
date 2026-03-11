#ifndef __TILINGROUTINE__
#define __TILINGROUTINE__

#include <map>
#include <knownfolders.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <shlobj.h>
#include <string>
#include <vector>
#include <windows.h>
#include "Utils.h"
#include "Config.h"

namespace Tiler
{
    class TilingRoutine
    {
        public:
            static inline FILETIME gLastConfigWrite = {0};
            static inline std::map<std::wstring, std::map<HWND, int>> gWindowAssignments;

            static void Start(const wchar_t* configPathShort, DWORD pollInterval)
            {
                wchar_t configPath[MAX_PATH];
                ExpandEnvironmentStringsW(configPathShort, configPath, MAX_PATH);

                Config::LoadConfig(configPath);
                Config::GetFileWriteTime(configPath, &gLastConfigWrite);

                while (true)
                {
                    FILETIME currentFiletime;

                    if (Config::GetFileWriteTime(configPath, &currentFiletime))
                    {
                        if (CompareFileTime(&currentFiletime, &gLastConfigWrite) != 0)
                        {
                            Config::LoadConfig(configPath);
                            gLastConfigWrite = currentFiletime;
                        }
                    }

                    auto windows = Utils::GetAllWindows();

                    EnterCriticalSection(&Config::gCsConfig);

                    std::map<std::wstring, std::vector<HWND>> windowsByPrefix;

                    for (HWND hwnd : windows)
                    {
                        wchar_t title[256];

                        GetWindowTextW(hwnd, title, 256);

                        DWORD pid = 0;

                        GetWindowThreadProcessId(hwnd, &pid);

                        std::wstring processName = Utils::GetProcessNameFromPid(pid);

                        std::wstring bestPrefix;
                        const TilingRule* bestRule = nullptr;

                        for (const auto& pair : Config::gRules)
                        {
                            const std::wstring& prefix = pair.first;

                            if (wcsncmp(title, prefix.c_str(), prefix.length()) == 0)
                            {
                                if (prefix.length() > bestPrefix.length())
                                {
                                    bestPrefix = prefix;
                                    bestRule = &pair.second;
                                }
                            }
                        }

                        if (!bestRule)
                            continue;

                        if (!bestRule->ProcessName.empty())
                            if (_wcsicmp(processName.c_str(), bestRule->ProcessName.c_str()) != 0)
                                continue;

                        windowsByPrefix[bestPrefix].push_back(hwnd);
                    }

                    for (auto& entry : windowsByPrefix)
                    {
                        std::sort(entry.second.begin(), entry.second.end());

                        const std::wstring& prefix = entry.first;
                        std::vector<HWND>& hwnds = entry.second;
                        auto it = Config::gRules.find(prefix);

                        if (it == Config::gRules.end())
                            continue;

                        const TilingRule& rule = it->second;

                        RECT desktop;
                        SystemParametersInfoW(SPI_GETWORKAREA, 0, &desktop, 0);

                        if (rule.Mode == TilingRule::GRID)
                        {
                            int cols = rule.Cols;
                            int rows = rule.Rows;

                            if (rows == -1)
                                rows = (hwnds.size() + cols - 1) / cols;
                            
                            int totalCells = cols * rows;
                            auto& assignments = gWindowAssignments[prefix];

                            std::map<HWND, int> newAssignments;
                            std::vector<bool> used(totalCells, false);

                            for (HWND hwnd : hwnds)
                            {
                                auto it = assignments.find(hwnd);

                                if (it != assignments.end() && it->second >= 0 && it->second < totalCells)
                                {
                                    newAssignments[hwnd] = it->second;
                                    used[it->second] = true;
                                }
                            }

                            int nextFree = 0;

                            for (HWND hwnd : hwnds)
                            {
                                if (newAssignments.find(hwnd) != newAssignments.end())
                                    continue;
                                
                                while (nextFree < totalCells && used[nextFree])
                                    nextFree++;

                                if (nextFree < totalCells)
                                {
                                    newAssignments[hwnd] = nextFree;
                                    used[nextFree] = true;
                                }
                            }

                            assignments = newAssignments;

                            int cellWidth = (desktop.right - desktop.left) / cols;
                            int cellHeight = (desktop.bottom - desktop.top) / rows;

                            for (const auto& pair : newAssignments)
                            {
                                HWND hwnd = pair.first;
                                int idx = pair.second;
                                int row = idx / cols;
                                int col = idx % cols;

                                if (row >= rows)
                                    continue;

                                int left = desktop.left + col * cellWidth;
                                int top = desktop.top + row * cellHeight;

                                ShowWindow(hwnd, SW_RESTORE);
                                
                                UINT flags = SWP_NOZORDER | SWP_NOACTIVATE;

                                if (rule.BringToTop)
                                    SetWindowPos(hwnd, HWND_TOP, left, top, cellWidth, cellHeight, SWP_SHOWWINDOW);
                                else
                                    SetWindowPos(hwnd, NULL, left, top, cellWidth, cellHeight, flags);
                            }

                        }
                        
                        else if (rule.Mode == TilingRule::EXPLICIT)
                        {
                            auto& assignments = gWindowAssignments[prefix];
                            std::map<HWND, int> newAssignments;
                            std::vector<bool> used(rule.Rects.size(), false);

                            for (HWND hwnd : hwnds)
                            {
                                auto it = assignments.find(hwnd);

                                if (it != assignments.end() && it->second >= 0 && it->second < (int)rule.Rects.size())
                                {
                                    newAssignments[hwnd] = it->second;
                                    used[it->second] = true;
                                }
                            }

                            int nextFree = 0;

                            for (HWND hwnd : hwnds)
                            {
                                if (newAssignments.find(hwnd) != newAssignments.end())
                                    continue;

                                while (nextFree < (int)used.size() && used[nextFree])
                                    nextFree++;

                                if (nextFree < (int)used.size())
                                {
                                    newAssignments[hwnd] = nextFree;
                                    used[nextFree] = true;
                                }
                            }

                            assignments = newAssignments;

                            for (const auto& pair : newAssignments)
                            {
                                HWND hwnd = pair.first;
                                int idx = pair.second;
                                const RECT& rc = rule.Rects[idx];
                                int width = rc.right - rc.left;
                                int height = rc.bottom - rc.top;

                                ShowWindow(hwnd, SW_RESTORE);

                                UINT flags = SWP_NOZORDER | SWP_NOACTIVATE;

                                if (rule.BringToTop)
                                    SetWindowPos(hwnd, HWND_TOP, rc.left, rc.top, width, height, SWP_SHOWWINDOW);
                                else
                                    SetWindowPos(hwnd, NULL, rc.left, rc.top, width, height, flags);
                            }
                        }
                    }

                    windowsByPrefix.clear();
                    LeaveCriticalSection(&Config::gCsConfig);
                    Sleep(pollInterval);
                }
            }
    };
}

#endif
