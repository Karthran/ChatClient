#include "Application.h"
#include <iostream>
#include <string>

#define _DEBUG

#ifdef _WIN32
#include <cstdio>
#include <windows.h>
#pragma execution_character_set("utf-8")
#endif

auto main() -> int
{
    //  setlocale(LC_ALL, "");

#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);  // UTF8
#endif

    Application app;
    app.run();

    return 0;
}
