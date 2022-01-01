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

const int DEFAULT_PORT = 27777;  // Á
#endif  //  _WIN32

#ifdef _WIN32

auto Client::client_thread() -> int
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
        return 1;
    }

    /*char* recvbuf{nullptr};*/
    size_t current_buffer_size{0};

    while (true)
    {
        if (_need_exchange_buffer_resize)
        {
            if (current_buffer_size < _exchange_buffer_size)
            {
                current_buffer_size = _exchange_buffer_size;
                _exchange_buffer = std::shared_ptr<char[]>(new char[current_buffer_size]);  /////////////////////////////////////

                // std::cout << " New Buffer Size: " << current_buffer_size << std::endl;
            }
            _need_exchange_buffer_resize = false;
        }

        while (!_out_message_ready)
        {
        }

        iResult = send(ConnectSocket, _exchange_buffer.get(), _message_length, 0);  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

        if (iResult == SOCKET_ERROR)
        {
            printf("send failed with error: %d\n", WSAGetLastError());
            closesocket(ConnectSocket);
            WSACleanup();
            _server_error = true;
            break;
        }
        _out_message_ready = false;
        // printf("Bytes Sent: %ld\n", iResult);

        if (*(reinterpret_cast<int*>(_exchange_buffer.get())) ==
            static_cast<int>(OperationCode::STOP))  // TODO Check exit
                                                    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        {
            // shutdown the connection since no more data will be sent
            iResult = shutdown(ConnectSocket, SD_SEND);
            _server_error = true;
            break;
        }

        iResult = recv(ConnectSocket, _exchange_buffer.get(), current_buffer_size, 0);  ///////////////////////////////////////
        if (iResult > 0)
        {
            // printf("Bytes received: %d\n", iResult);
            // message = std::string(recvbuf, iResult);
            _in_message_ready = true;

            // std::cout << "InMessage: " << message << std::endl;
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
    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();

    _exchange_buffer = nullptr;  ////////////////////////////////////////////////////////////////////////////
    return 0;
}

#elif defined __linux__
auto Client::client_thread() -> int
{

    int socket_file_descriptor, connection;
    struct sockaddr_in serveraddress, client;

    socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_file_descriptor == -1)
    {
        std::cout << "Creation of Socket failed!" << std::endl;
        exit(1);
    }

    serveraddress.sin_addr.s_addr = inet_addr("127.0.0.1");
    serveraddress.sin_port = htons(DEFAULT_PORT);
    serveraddress.sin_family = AF_INET;
    connection = connect(socket_file_descriptor, (struct sockaddr*)&serveraddress, sizeof(serveraddress));
    if (connection == -1)
    {
        std::cout << "Connection with the server failed.!" << std::endl;
        exit(1);
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
        while (!_out_message_ready)
        {
        }

        ssize_t bytes = write(socket_file_descriptor, _exchange_buffer.get(), _message_length);

        if (bytes >= 0)
        {
            _out_message_ready = false;
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
        }
    }

    close(connection);
    _exchange_buffer = nullptr;

    return 0;
}

#endif  // _WIN32

auto Client::run() -> void
{
    std::thread tr(&Client::client_thread, this);
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
