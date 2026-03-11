#include <windows.h>
#include "TilingRoutine.h"
#include "Config.h"

const wchar_t* CONFIG_PATH = L"%AppData%\\tiling";
const DWORD POLL_INTERVAL = 150;

DWORD WINAPI StartTilingRoutine(LPVOID)
{
    Tiler::TilingRoutine::Start(CONFIG_PATH, POLL_INTERVAL);

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            InitializeCriticalSection(&Tiler::Config::gCsConfig);
            CreateThread(NULL, 0, StartTilingRoutine, NULL, 0, NULL);
            break;
        
        case DLL_PROCESS_DETACH:
            DeleteCriticalSection(&Tiler::Config::gCsConfig);
            break;
    }
    
    return TRUE;
}