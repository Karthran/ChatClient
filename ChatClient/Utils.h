#pragma once
#include <string>

const std::string RESET = "\033[0m";
const std::string UNDER_LINE = "\033[4m";
const std::string BLACK = "\033[30m";              /* Black */
const std::string RED = "\033[31m";                /* Red */
const std::string GREEN = "\033[32m";              /* Green */
const std::string YELLOW = "\033[33m";             /* Yellow */
const std::string BLUE = "\033[34m";               /* Blue */
const std::string MAGENTA = "\033[35m";            /* Magenta */
const std::string CYAN = "\033[36m";               /* Cyan */
const std::string WHITE = "\033[37m";              /* White */
const std::string BOLDBLACK = "\033[1m\033[30m";   /* Bold Black */
const std::string BOLDRED = "\033[1m\033[31m";     /* Bold Red */
const std::string BOLDGREEN = "\033[1m\033[32m";   /* Bold Green */
const std::string BOLDYELLOW = "\033[1m\033[33m";  /* Bold Yellow */
const std::string BOLDBLUE = "\033[1m\033[34m";    /* Bold Blue */
const std::string BOLDMAGENTA = "\033[1m\033[35m"; /* Bold Magenta */
const std::string BOLDCYAN = "\033[1m\033[36m";    /* Bold Cyan */
const std::string BOLDWHITE = "\033[1m\033[37m";   /* Bold White */

const int MAX_INPUT_SIZE = 30;
const int UNSUCCESSFUL = -1;
const int SUCCESSFUL = 1;
const int MAX_MESSAGES_IN_CHAT = 10000;
const int MESSAGES_ON_PAGE = 5;
const int LINE_TO_PAGE = 15;
const int IGNORED_NUM = 0xffff;

class Utils
{
public:
    static auto isOKSelect() -> bool;
    static auto inputIntegerValue() -> int;
    static auto printTimeAndData(const tm& timeinfo) -> void;
    //    static auto getBoundedString(std::string& string, int size, bool hidden = false) -> void;
    static auto getString(std::string& string, size_t size = 0) -> void;
    static auto getPassword(std::string& string, const std::string& text) -> void;
    static auto getSelfPath(std::string& path) -> void;
    static auto printOSVersion() -> void;

    template <typename T>
    static auto minToMaxOrder(T& min, T& max) -> bool;

    template <typename T>
    static auto sign(T val) -> int;

#if defined(_WIN32)
    static auto getWindowsVersionName() -> const char*;
#endif
};

template <typename T>
auto Utils::minToMaxOrder(T& min, T& max) -> bool
{
    auto isSwap{false};
    if (min > max)
    {
        std::swap(min, max);
        isSwap = true;
    }
    return isSwap;
}

template <typename T>
auto Utils::sign(T val) -> int
{
    return (T(0) < val) - (val < T(0));
}
