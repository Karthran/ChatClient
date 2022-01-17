#pragma once
#include <string>
#include <memory>

const int DEFAULT_BUFLEN = 16384;

class Client
{
public:
    auto run() -> void;
    auto getOutMessageReady() const -> bool;
    auto setOutMessageReady(bool flag) -> void;
    auto getInMessageReady() const -> bool;
    auto setInMessageReady(bool flag) -> void;
    auto getMessage() -> char*;
    auto setMessage(const char* msg, size_t msg_length) -> void;
    auto getServerError() const -> bool;
    auto setBufferSize(size_t size) -> void;
    auto isError() -> bool { return _connect_error; }

private:
    volatile bool _out_message_ready{false};
    volatile bool _in_message_ready{false};
    volatile bool _server_error{false};
    std::shared_ptr<char[]> _exchange_buffer{nullptr};
    volatile size_t _exchange_buffer_size{DEFAULT_BUFLEN};
    volatile bool _need_exchange_buffer_resize{true};
    size_t _message_length{0};
    bool _connect_error{false};
    auto client_thread() -> int;
};
