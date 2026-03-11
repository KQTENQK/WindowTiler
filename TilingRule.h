#ifndef __TILINGRULE__
#define __TILINGRULE__

#include <vector>
#include <string>
#include <windows.h>

namespace Tiler
{
    struct TilingRule
    {
        enum Mode { GRID, EXPLICIT } Mode = GRID;
        int Cols = 0;
        int Rows = 0;
        std::vector<RECT> Rects;
        std::wstring ProcessName;
        bool BringToTop = false;
    };
}

#endif
