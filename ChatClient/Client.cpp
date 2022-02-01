#include "Client.h"

#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <string.h>

#include "core.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

// Need to link with Ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")
//#pragma comment (lib, "Mswsock.lib")
//#pragma comment(lib, "AdvApi32.lib")

const char* DEFAULT_PORT = "27777";

#elif defined __linux__
#include <unistd.h>
//#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

const int DEFAULT_PORT = 27777; 
#endif  //  _WIN32

volatile bool _out_message_ready{false};
volatile bool _in_message_ready{false};

#ifdef _WIN32
auto Client::client_thread(std::condition_variable& in_holder, std::condition_variable& out_holder) -> int
{
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL, *ptr = NULL, hints;
    int iResult;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        printf("WSAStartup failed with error: %d\n", iResult);
        _connect_error = true;
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo("127.0.0.1", DEFAULT_PORT, &hints, &result);
    if (iResult != 0)
    {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        _connect_error = true;
        return 1;
    }

    // Attempt to connect to an address until one succeeds
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
    {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET)
        {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            _connect_error = true;
            return 1;
        }

        // Connect to server.
        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR)
        {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET)
    {
        printf("Unable to connect to server!\n");
        WSACleanup();
        _connect_error = true;
        return 1;
    }

    size_t current_buffer_size{0};

    while (true)
    {
        if (_need_exchange_buffer_resize)
        {
            if (current_buffer_size < _exchange_buffer_size)
            {
                current_buffer_size = _exchange_buffer_size;
                _exchange_buffer = std::shared_ptr<char[]>(new char[current_buffer_size]);  
            }
            _need_exchange_buffer_resize = false;
        }

        std::unique_lock<std::mutex> out_lock(out_mutex);
        out_holder.wait(out_lock, []() { return _out_message_ready; });

        iResult = send(ConnectSocket, _exchange_buffer.get(), _message_length, 0);  

        if (iResult == SOCKET_ERROR)
        {
            printf("send failed with error: %d\n", WSAGetLastError());
            closesocket(ConnectSocket);
            WSACleanup();
            _server_error = true;
            break;
        }
        _out_message_ready = false;
        out_holder.notify_one();

        if (*(reinterpret_cast<int*>(_exchange_buffer.get())) == static_cast<int>(OperationCode::STOP))
        {
            iResult = shutdown(ConnectSocket, SD_SEND);
            _server_error = true;
            break;
        }

        iResult = recv(ConnectSocket, _exchange_buffer.get(), current_buffer_size, 0);  
        if (iResult > 0)
        {
            _in_message_ready = true;
            in_holder.notify_one();
        }
        else if (iResult == 0)
        {
            printf("Connection closed\n");
            break;
        }
        else
        {
            printf("recv failed with error: %d\n", WSAGetLastError());
            break;
        }
    }
    closesocket(ConnectSocket);
    WSACleanup();

    return 0;
}

#elif defined __linux__
auto Client::client_thread(std::condition_variable& in_holder, std::condition_variable& out_holder) -> int
{

    int socket_file_descriptor, connection;
    struct sockaddr_in serveraddress, client;

    socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_file_descriptor == -1)
    {
        std::cout << "Creation of Socket failed!" << std::endl;
        _connect_error = true;
        return 1;
    }

    serveraddress.sin_addr.s_addr = inet_addr("127.0.0.1");
    serveraddress.sin_port = htons(DEFAULT_PORT);
    serveraddress.sin_family = AF_INET;
    connection = connect(socket_file_descriptor, (struct sockaddr*)&serveraddress, sizeof(serveraddress));
    if (connection == -1)
    {
        std::cout << "Connection with the server failed.!" << std::endl;
        _connect_error = true;
        return 1;
    }

    size_t current_buffer_size{0};

    while (1)
    {
        if (_need_exchange_buffer_resize)
        {
            if (current_buffer_size < _exchange_buffer_size)
            {
                current_buffer_size = _exchange_buffer_size;
                _exchange_buffer = std::shared_ptr<char[]>(new char[current_buffer_size]);
            }
            _need_exchange_buffer_resize = false;
        }

        std::unique_lock<std::mutex> out_lock(out_mutex);
        out_holder.wait(out_lock, []() { return _out_message_ready; });

        ssize_t bytes = write(socket_file_descriptor, _exchange_buffer.get(), _message_length);

        if (bytes >= 0)
        {
            _out_message_ready = false;
            out_holder.notify_one();

        }
        else
        {
            _server_error = true;
            break;
        }

        if (*(reinterpret_cast<int*>(_exchange_buffer.get())) == static_cast<int>(OperationCode::STOP))
        {
            _server_error = true;
            break;
        }

        ssize_t length = read(socket_file_descriptor, _exchange_buffer.get(), current_buffer_size);
        if (length > 0)
        {
            _in_message_ready = true;
            in_holder.notify_one();
        }
    }

    close(connection);

    return 0;
}

#endif  // _WIN32

auto Client::run(std::condition_variable& in_holder, std::condition_variable& out_holder) -> void
{
    _exchange_buffer = std::shared_ptr<char[]>(new char[DEFAULT_BUFLEN]);
    std::thread tr(&Client::client_thread, this, std::ref(in_holder), std::ref(out_holder));
    tr.detach();
}

auto Client::getOutMessageReady() const -> bool
{
    return _out_message_ready;
}

auto Client::setOutMessageReady(bool flag) -> void
{
    _out_message_ready = flag;
}

auto Client::getInMessageReady() const -> bool
{
    return _in_message_ready;
}

auto Client::setInMessageReady(bool flag) -> void
{
    _in_message_ready = flag;
}

auto Client::getMessage() -> char*
{
    return _exchange_buffer.get();
}

auto Client::setMessage(const char* msg, size_t msg_length) -> void
{
    _message_length = msg_length;

    for (auto i{0}; i < _message_length; ++i)
    {
        _exchange_buffer[i] = msg[i];
    }
}

auto Client::getServerError() const -> bool
{
    return _server_error;
}

auto Client::setBufferSize(size_t size) -> void
{
    _exchange_buffer_size = size;
    _need_exchange_buffer_resize = true;
}
