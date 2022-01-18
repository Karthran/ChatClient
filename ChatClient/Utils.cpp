#include <iostream>
#include <iomanip>

#if defined(_WIN32)
#include <conio.h>
#include <windows.h>
#include <VersionHelpers.h>

#elif defined(__linux__)
#include <unistd.h>
#include <limits.h>
#include <sys/utsname.h>

#endif

#include "Utils.h"

auto Utils::isOKSelect() -> bool
{
    char select;
    std::cin >> select;
    if (select != 'Y' && select != 'y') return false;
    return true;
}

auto Utils::inputIntegerValue() -> int
{
    auto value{UNSUCCESSFUL};

    while (true)
    {
        std::cin >> value;

        if (std::cin.fail())
        {
            std::cin.clear();
            std::cin.ignore(IGNORED_NUM, '\n');
            std::cout << "Incorrect value entered!" << std::endl;
            std::cout << "Try again: ";
        }
        else
        {
            std::cin.ignore(IGNORED_NUM, '\n');
            return value;
        }
    }
}

auto Utils::printTimeAndData(const tm& timeinfo) -> void
{
    std::cout << std::setw(2) << std::setfill('0') << std::right << timeinfo.tm_hour << ":";
    std::cout << std::setw(2) << std::setfill('0') << std::right << timeinfo.tm_min << ":";
    std::cout << std::setw(2) << std::setfill('0') << std::right << timeinfo.tm_sec << "   ";

    std::cout << std::setw(2) << std::setfill('0') << std::right << timeinfo.tm_mday << "/";
    std::cout << std::setw(2) << std::setfill('0') << std::right << timeinfo.tm_mon + 1 << "/";
    std::cout << timeinfo.tm_year + 1900 << std::endl;
}

auto Utils::getString(std::string& string) -> void
{
#ifdef _WIN32
    wchar_t wstr[MAX_INPUT_SIZE];
    char mb_str[MAX_INPUT_SIZE * 3 + 1];
    unsigned long read;
    void* con = GetStdHandle(STD_INPUT_HANDLE);
    ReadConsole(con, wstr, MAX_INPUT_SIZE, &read, NULL);
    int size1 = WideCharToMultiByte(CP_UTF8, 0, wstr, read, mb_str, sizeof(mb_str), NULL, NULL);
    mb_str[size1 - 2] = '\0';
    string = std::string(mb_str);

#elif defined __linux__
    std::getline(std::cin, string);
#endif  //
}

auto Utils::getPassword(std::string& password, const std::string& text) -> void
{
#if defined(_WIN32)

    std::cout << text;
    std::cout << BOLDGREEN;

    auto c{' '};
    auto i{0};
    password.erase();
    while ((c = _getch()) != '\r')
    {
        password.push_back(c);
        _putch('*');
        if (++i == MAX_INPUT_SIZE)
        {
            while (_getch() != '\r')
            {
            }
            break;
        }
    }

#elif defined(__linux__)
    password = std::string(getpass(text.c_str()));
#endif
    std::cout << RESET << std::endl;
}

auto Utils::getSelfPath(std::string& path) -> void
{
#if defined(_WIN32)
    path.erase();
#elif defined(__linux__)
    char buff[PATH_MAX];
    ssize_t len = ::readlink("/proc/self/exe", buff, sizeof(buff) - 1);
    if (len != -1)
    {
        for (auto i{len - 1}; i >= 0; --i)
        {
            if (buff[i] != '/') continue;
            buff[i + 1] = '\0';
            path = std::string(buff);
            break;
        }
    }
#endif
}

auto Utils::printOSVersion() -> void
{
#if defined(_WIN32)

    std::cout << std::endl;
    std::cout << "OS name: Windows" << std::endl;
    std::cout << "OS version: " << getWindowsVersionName() << std::endl;

#elif defined(__linux__)

    struct utsname utsname;  // объект для структуры типа utsname
    uname(&utsname);         // передаем объект по ссылке

    std::cout << std::endl;
    std::cout << "OS name: " << utsname.sysname << std::endl;
    std::cout << "OS version: " << utsname.version << std::endl;

#endif
}
#if defined(_WIN32)
auto Utils::getWindowsVersionName() -> const char*
{
    if (IsWindows10OrGreater())
    {
        return "10";
    }
    if (IsWindows8Point1OrGreater())
    {
        return "8.1";
    }
    if (IsWindows8OrGreater())
    {
        return "8";
    }
    if (IsWindows7OrGreater())
    {
        return "7";
    }
    if (IsWindowsVistaOrGreater())
    {
        return "Vista";
    }
    if (IsWindowsXPOrGreater())
    {
        return "XP";
    }
    return "Unknown";
}
#endif