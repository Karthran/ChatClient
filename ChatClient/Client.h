#pragma once
#include <string>
#include <memory>
#include <mutex>
#include <condition_variable>
const int DEFAULT_BUFLEN = 16384;

class Client
{
public:
    auto run(std::condition_variable& in_holder, std::condition_variable& out_holder) -> void;
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
    volatile bool _server_error{false};
    std::shared_ptr<char[]> _exchange_buffer{nullptr};
    volatile size_t _exchange_buffer_size{DEFAULT_BUFLEN};
    volatile bool _need_exchange_buffer_resize{true};
    size_t _message_length{0};
    bool _connect_error{false};
    std::mutex in_mutex;
    std::mutex out_mutex;

    auto client_thread(std::condition_variable& in_holder, std::condition_variable& out_holder) -> int;
};
