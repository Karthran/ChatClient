#pragma once
#include <iostream>
#include <fstream>
#include <string>

class File
{
public:
#if defined(_WIN32)
    File(const std::string& file_name, std::fstream::_Openmode flags = std::fstream::out)
#elif defined(__linux__)
    File(const std::string& file_name, std::ios_base::openmode flags = std::fstream::out)
#endif
    {
        _fs.open(file_name, flags);
        if (_fs.is_open()) _error = false;
    }
    ~File() { _fs.close(); }

    auto write(const std::string& out_string) -> void;
    auto read(std::string& in_string) -> void;

    auto write(const int& out_int) -> void;
    auto read(int& in_int) -> void;

    auto write(const unsigned int& out_uint) -> void;
    auto read(unsigned int& in_uint) -> void;

    auto write(const size_t& out_size_t) -> void;
    auto read(size_t& in_size_t) -> void;

    auto write(const tm& out_tm) -> void;
    auto read(tm& in_tm) -> void;

    auto getError() const -> bool { return _error; }

    auto getStream() -> std::fstream& { return _fs; }

private:
    std::fstream _fs;
    bool _error{true};
};

inline auto File::write(const std::string& out_string) -> void
{
    if (!_fs.is_open()) return;

    _fs << out_string << " ";
}

inline auto File::read(std::string& in_string) -> void
{
    if (!_fs.is_open()) return;

    _fs >> in_string;
}

inline auto File::write(const int& out_int) -> void
{
    if (!_fs.is_open()) return;

    _fs << out_int << " ";
}

inline auto File::read(int& in_int) -> void
{
    if (!_fs.is_open()) return;

    _fs >> in_int;
}

inline auto File::write(const unsigned int& out_uint) -> void
{
    if (!_fs.is_open()) return;

    _fs << out_uint << " ";
}
inline auto File::read(unsigned int& in_uint) -> void
{
    if (!_fs.is_open()) return;

    _fs >> in_uint;
}

inline auto File::write(const size_t& out_size_t) -> void
{
    if (!_fs.is_open()) return;

    _fs << out_size_t << " ";
}
inline auto File::read(size_t& in_size_t) -> void
{
    if (!_fs.is_open()) return;

    _fs >> in_size_t;
}

inline auto File::write(const tm& out_tm) -> void
{
    if (!_fs.is_open()) return;

    _fs << out_tm.tm_sec << " ";
    _fs << out_tm.tm_min << " ";
    _fs << out_tm.tm_hour << " ";
    _fs << out_tm.tm_mday << " ";
    _fs << out_tm.tm_mon << " ";
    _fs << out_tm.tm_year << " ";
    _fs << out_tm.tm_wday << " ";
    _fs << out_tm.tm_yday << " ";
    _fs << out_tm.tm_isdst << " ";
}
inline auto File::read(tm& in_tm) -> void
{
    if (!_fs.is_open()) return;

    _fs >> in_tm.tm_sec;
    _fs >> in_tm.tm_min;
    _fs >> in_tm.tm_hour;
    _fs >> in_tm.tm_mday;
    _fs >> in_tm.tm_mon;
    _fs >> in_tm.tm_year;
    _fs >> in_tm.tm_wday;
    _fs >> in_tm.tm_yday;
    _fs >> in_tm.tm_isdst;
}
