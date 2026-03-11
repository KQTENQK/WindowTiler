#ifndef __UTILS__
#define __UTILS__

#include <vector>
#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <fstream>
#include <sstream>

namespace Tiler
{
    class Utils
    {
        public:
            static std::vector<RECT> ParseRects(const std::string& rectString)
            {
                std::vector<RECT> result;
                std::istringstream iss(rectString);
                std::string token;

                while (std::getline(iss, token, ';'))
                {
                    token.erase(0, token.find_first_not_of(" \t"));
                    token.erase(token.find_last_not_of(" \t") + 1);

                    if (token.empty())
                        continue;
                    
                    int x;
                    int y;
                    int w;
                    int h;

                    if (sscanf_s(token.c_str(), "%d,%d,%d,%d", &x, &y, &w, &h) == 4)
                    {
                        RECT rc = {x, y, x + w, y + h};
                        result.push_back(rc);
                    }
                }

                return result;
            }

            static std::wstring GetProcessNameFromPid(DWORD pid)
            {
                std::wstring result;
                HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

                if (snap == INVALID_HANDLE_VALUE)
                    return result;

                PROCESSENTRY32W pe;
                pe.dwSize = sizeof(pe);

                if (Process32FirstW(snap, &pe))
                {
                    do
                    {
                        if (pe.th32ProcessID == pid)
                        {
                            result = pe.szExeFile;

                            break;
                        }
                    } while (Process32NextW(snap, &pe));
                }

                CloseHandle(snap);

                return result;
            }

            static std::vector<HWND> GetAllWindows()
            {
                std::vector<HWND> result;

                EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL
                {
                    auto vec = reinterpret_cast<std::vector<HWND>*>(lParam);

                    if (IsWindowVisible(hwnd) && GetWindowTextLengthW(hwnd) > 0)
                        vec->push_back(hwnd);

                    return TRUE;
                }, reinterpret_cast<LPARAM>(&result));

                return result;
            }
    };
}

#endif
