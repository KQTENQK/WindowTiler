#ifndef __CONFIG__
#define __CONFIG__

#include <string>
#include <map>
#include <sstream>
#include <fstream>
#include "TilingRule.h"
#include "Utils.h"

namespace Tiler
{
    class Config
    {
        public:
            static inline std::map<std::wstring, TilingRule> gRules;
            static inline CRITICAL_SECTION gCsConfig;

            static void LoadConfig(const wchar_t* filename)
            {
                std::ifstream file(filename);

                if (!file.is_open())
                    return;

                std::map<std::wstring, TilingRule> newRules;
                std::string line;
                std::wstring currentPrefix;
                TilingRule currentRule;

                auto finishSection = [&]()
                {
                    if (!currentPrefix.empty())
                    {
                        newRules[currentPrefix] = currentRule;
                        currentPrefix.clear();
                    }
                };

                while (std::getline(file, line))
                {
                    size_t commentPos = line.find_first_of("#");

                    if (commentPos != std::string::npos)
                        line = line.substr(0, commentPos);

                    line.erase(0, line.find_first_not_of(" \t\r\n"));
                    line.erase(line.find_last_not_of(" \t\r\n") + 1);

                    if (line.empty())
                        continue;

                    if (line.front() == '[' && line.back() == ']')
                    {
                        finishSection();
                        std::string prefixStr = line.substr(1, line.size() - 2);
                        int len = MultiByteToWideChar(CP_UTF8, 0, prefixStr.c_str(), -1, NULL, 0);
                        currentPrefix.resize(len - 1);
                        MultiByteToWideChar(CP_UTF8, 0, prefixStr.c_str(), -1, &currentPrefix[0], len);
                        currentRule = TilingRule();
                        currentRule.BringToTop = false;
                    }
                    else
                    {
                        size_t eqPos = line.find('=');

                        if (eqPos == std::string::npos)
                            continue;

                        std::string key = line.substr(0, eqPos);
                        std::string value = line.substr(eqPos + 1);
                        key.erase(0, key.find_first_not_of(" \t"));
                        key.erase(key.find_last_not_of(" \t") + 1);
                        value.erase(0, value.find_first_not_of(" \t"));
                        value.erase(value.find_last_not_of(" \t") + 1);

                        if (key == "mode")
                        {
                            if (value == "grid")
                                currentRule.Mode = TilingRule::GRID;

                            else if (value == "explicit")
                                currentRule.Mode = TilingRule::EXPLICIT;
                        }
                        
                        else if (key == "cols")
                        {
                            try
                            {
                                currentRule.Cols = std::stoi(value);
                            }
                            catch(...) {  }
                        }
                        
                        else if (key == "rows")
                        {
                            if (value == "auto")
                            {
                                currentRule.Rows = -1;
                            }
                            else
                            {
                                try
                                {
                                    currentRule.Rows = std::stoi(value);
                                }
                                catch(...) {  }
                            }
                        }
                        
                        else if (key == "rects")
                        {
                            currentRule.Rects = Utils::ParseRects(value);
                        }
                        
                        else if (key == "process")
                        {
                            int len = MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, NULL, 0);
                            currentRule.ProcessName.resize(len - 1);
                            MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, &currentRule.ProcessName[0], len);
                        }
                        
                        else if (key == "bring_to_top")
                        {
                            currentRule.BringToTop = (value == "true" || value == "1" || value == "yes");
                        }
                    }
                }

                finishSection();

                EnterCriticalSection(&gCsConfig);
                gRules.swap(newRules);
                LeaveCriticalSection(&gCsConfig);

                file.close();
            }

            static bool GetFileWriteTime(const wchar_t* filename, FILETIME* ft)
            {
                HANDLE hFile = CreateFileW
                (
                    filename,
                    GENERIC_READ,
                    FILE_SHARE_READ,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL
                );

                if (hFile == INVALID_HANDLE_VALUE)
                    return false;

                bool success = GetFileTime(hFile, NULL, NULL, ft);
                CloseHandle(hFile);

                return success;
            }
    };
}

#endif
