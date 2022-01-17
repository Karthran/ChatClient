#include "Application.h"
#include <iostream>
#include <string>

#ifdef _WIN32
#include <cstdio>
#include <windows.h>
#pragma execution_character_set("utf-8")
#endif

auto main() -> int
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);  
#endif

    Application app;
    app.run();

    return 0;
}
