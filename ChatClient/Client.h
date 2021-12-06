#pragma once
#include <string>
class Client
{
public:
	auto run() -> void;
    auto getOutMessageReady()const -> bool;
    auto setOutMessageReady(bool flag)const -> void;
    auto getInMessageReady()const -> bool;
    auto setInMessageReady(bool flag)const -> void;
    auto getMessage() -> const std::string&;
    auto setMessage(const std::string& msg)const -> void;
    auto getServerError()const -> bool;
    auto setBufferSize(size_t size) const -> void;
};

