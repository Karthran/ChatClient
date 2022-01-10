#include <iostream>
#include <cassert>
#include <iomanip>
#include <exception>
#include <fstream>
#include <sstream>
#include <string.h>
#include "Application.h"
#include "Client.h"
#include "core.h"

#include "Utils.h"

//#include "Chat.h"
//#include "Message.h"
//#include "User.h"
//#include "SHA1.h"
//#include "PasswordHash.h"
//#include "FileUtils.h"
//#include "NewMessages.h"

#ifdef _WIN32
#include <cstdio>
#include <windows.h>
#pragma execution_character_set("utf-8")
#endif

Application::Application()
{
    _msg_buffer = new char[DEFAULT_BUFLEN];
    Utils::getSelfPath(_self_path);
}

Application::~Application()
{
    delete[] _msg_buffer;
}

auto Application::run() -> void
{
    Utils::printOSVersion();

    _client = new Client();
    _client->run();

    std::cout << std::endl << BOLDYELLOW << UNDER_LINE << "Wellcome to Console Chat!" << RESET << std::endl;

    _user_id.push_back('\0');

    auto isContinue{true};
    while (isContinue)
    {
        std::string menu_arr[]{"Main menu:", "Sign In", "Create account", "Quit"};

        auto menu_item{menu(menu_arr, 4)};

        switch (menu_item)
        {
            case 1: signIn(); break;
            case 2: createAccount(); break;
            default:
                char stop[64];
                size_t size{0};
                addToBuffer(stop, size, static_cast<int>(OperationCode::STOP));
                addToBuffer(stop, size, _user_id.c_str(), _user_id.size());
                talkToServer(stop, size);
                isContinue = false;
                break;
        }
    }
    delete _client;
}

auto Application::createAccount() -> void
{
    std::vector<std::string> reg_data(5);
    createAccount_inputName(reg_data[0]);

    createAccount_inputSurname(reg_data[1]);

    createAccount_inputLogin(reg_data[2]);

    createAccount_inputEmail(reg_data[3]);

    createAccount_inputPassword(reg_data[4]);

    std::cout << BOLDYELLOW << std::endl << "Create account?(Y/N): " << BOLDGREEN;
    if (!Utils::isOKSelect()) return;

    std::string query{};
    query.reserve(1024);
    for (auto i{0}; i < 5; ++i)
    {
        query += reg_data[i];
        query.push_back('\0');
    }

    auto result{sendToServer(query.c_str(), query.size(), OperationCode::REGISTRATION)};

    if (strcmp(result, RETURN_OK.c_str()))
    {
        if (!strcmp(result, "EMAIL"))
        {
            std::cout << std::endl << RED << "Please change email!" << RESET << std::endl;
            return;
        }
        else if (!strcmp(result, "LOGIN"))
        {
            std::cout << std::endl << RED << "Please change login!" << RESET << std::endl;
            return;
        }
        else
        {
            std::cout << std::endl << RED << "Invalid user information!" << RESET << std::endl;
            return;
        }
    }
}

auto Application::createAccount_inputName(std::string& user_name) -> void
{
    std::cout << std::endl;
    std::cout << BOLDYELLOW << UNDER_LINE << "Create account:" << RESET << std::endl;
    auto isOK{false};
    std::cout << "Name(max " << MAX_INPUT_SIZE << " letters): ";
    std::cout << BOLDGREEN;
    Utils::getString(user_name);
    std::cout << RESET;
}

auto Application::createAccount_inputSurname(std::string& surname) -> void
{
    std::cout << "Surname(max " << MAX_INPUT_SIZE << " letters): ";
    std::cout << BOLDGREEN;
    Utils::getString(surname);
    std::cout << RESET;
}

auto Application::createAccount_inputEmail(std::string& user_email) -> void
{
    auto isOK{false};

    while (!isOK)
    {
        std::cout << "Email(max " << MAX_INPUT_SIZE << " letters): ";
        std::cout << BOLDGREEN;
        Utils::getString(user_email);
        std::cout << RESET;

        auto result{sendToServer(user_email.c_str(), user_email.size(), OperationCode::CHECK_EMAIL)};  // in result now OK or ERROR

        if (strcmp(result, RETURN_OK.c_str()))
        {
            std::cout << std::endl << RED << "Please change email!" << RESET << std::endl;
        }
        else
        {
            isOK = true;
        }
    }
}

auto Application::createAccount_inputLogin(std::string& user_login) -> void
{
    auto isOK{false};

    while (!isOK)
    {
        std::cout << "Login(max " << MAX_INPUT_SIZE << " letters): ";
        std::cout << BOLDGREEN;
        Utils::getString(user_login);
        std::cout << RESET;

        auto result{sendToServer(user_login.c_str(), user_login.size(), OperationCode::CHECK_LOGIN)};  // in result now OK or ERROR

        if (strcmp(result, RETURN_OK.c_str()))
        {
            std::cout << std::endl << RED << "Please change login!" << RESET << std::endl;
        }
        else
        {
            isOK = true;
        }
    }
}

auto Application::createAccount_inputPassword(std::string& user_password) const -> void
{
    auto isOK{false};
    while (!isOK)
    {
        Utils::getPassword(user_password, "Password(max " + std::to_string(MAX_INPUT_SIZE) + " letters): ");

        if (user_password.empty()) continue;

        std::string check_user_password;

        Utils::getPassword(check_user_password, "Re-enter your password: ");

        if (user_password != check_user_password)
        {
            std::cout << std::endl << RED << "Password don't match!" << RESET;
        }
        else
        {
            isOK = true;
        }
    }
}

auto Application::signIn() -> void
{
    std::cout << std::endl;
    std::cout << BOLDYELLOW << UNDER_LINE << "Sign In:" << RESET << std::endl;

    std::string user_login{};
    std::string user_password{};
    while (true)
    {
        signIn_inputLogin(user_login);
        signIn_inputPassword(user_password);

        std::string query{};
        query.reserve(1024);
        query += user_login;
        query.push_back('\0');
        query += user_password;
        query.push_back('\0');

        auto result{sendToServer(query.c_str(), query.size(), OperationCode::SIGN_IN)};  // in result now ID or ERROR

        if (strcmp(result, RETURN_ERROR.c_str()))
        {
            auto res_length{strlen(result)};
            _user_id = result;
            _user_id.push_back('\0');
            selectCommonOrPrivate();
            return;
        }

        std::cout << std::endl << RED << "Login or Password don't match!" << std::endl;
        std::cout << BOLDYELLOW << std::endl << "Try again?(Y/N):" << BOLDGREEN;
        if (!Utils::isOKSelect()) return;
    }
}

auto Application::signIn_inputLogin(std::string& user_login) const -> void
{
    std::cout << RESET << "Login:";
    std::cout << BOLDGREEN;
    Utils::getString(user_login);
    std::cout << RESET;
}
auto Application::signIn_inputPassword(std::string& user_password) const -> void
{
    Utils::getPassword(user_password, "Password: ");
}

auto Application::selectCommonOrPrivate() -> void
{
    auto isContinue{true};
    while (isContinue)
    {
        std::string result = _user_id;
        // sendToServer(result, OperationCode::NEW_MESSAGES);  // in result now number of new messages

        std::string menu_arr[] = {"Select chat type:", "Common chat", "Private chat", "Sign Out"};

        // auto user_number{std::stoi(result)};
        // if (user_number)  // if exist new message for this user
        //{
        //    menu_arr[2] = BOLDYELLOW + menu_arr[2] + RESET + GREEN + "(New message(s) from " + std::to_string(user_number) + " user(s))" +
        //                  RESET;  // menu_arr[2] = "Private chat"
        //}

        auto menu_item{menu(menu_arr, 4)};

        switch (menu_item)
        {
            case 1: commonChat(); break;
            case 2: /* privateMenu();*/ break;
            default: isContinue = false; break;
        }
    }

    return;
}

auto Application::commonChat() -> int
{
    auto isContinue{true};
    while (isContinue)
    {
        std::string menu_arr[]{"Common Chat:", "View chat", "Add message", "Edit message", "Delete message", "Exit"};
        auto menu_item{menu(menu_arr, 6)};

        switch (menu_item)
        {
            case 1:
            {
                auto result{sendToServer(" ", 1, OperationCode::COMMON_CHAT_GET_MESSAGES)};  // in result now ID or ERROR

                auto messages_num{-1};
                getFromBuffer(result, 0, messages_num);
                std::cout << "Messages number: " << messages_num << std::endl;

                // std::cout << std::endl;
                // std::string result = "-1" + DELIMITER + "-1";                      // -1 common chat users id
                // sendToServer(result, OperationCode::GET_NUMBER_MESSAGES_IN_CHAT);  // in result now number of new messages
                // if (result == RETURN_ERROR) break;
                // auto message_num{std::stoi(result)};
                // for (auto i{0}; i < message_num; ++i)
                //{
                //    result = std::to_string(i);
                //    sendToServer(result, OperationCode::COMMON_CHAT_GET_MESSAGE);  // in result now OK or ERROR
                //    std::cout << result;
                //}
                break;
            }
            case 2: commonChat_addMessage(); break;
            case 3: /*commonChat_editMessage(user);*/ break;
            case 4: /*commonChat_deleteMessage(user);*/ break;
            default: isContinue = false; break;
        }
    }
    return SUCCESSFUL;
}

auto Application::commonChat_addMessage() -> void
{
    std::string new_message{};

    std::cout << std::endl << YELLOW << "Input message: " << BOLDGREEN;
    Utils::getString(new_message);
    std::cout << RESET;
    std::cout << BOLDYELLOW << "Send message?(Y/N):" << BOLDGREEN;
    std::cout << RESET;

    if (!Utils::isOKSelect()) return;

    new_message.push_back('\0');
    sendToServer(new_message.c_str(), new_message.size(), OperationCode::COMMON_CHAT_ADD_MESSAGE);  // in result now OK or ERROR
}

// auto Application::commonChat_editMessage(const std::shared_ptr<User>& user) const -> void
//{
//    //std::cout << std::endl << YELLOW << "Select message number for editing: " << BOLDGREEN;
//    //int message_number{Utils::inputIntegerValue()};
//    //std::cout << RESET;
//    //_common_chat->editMessage(user, message_number - 1);  // array's indices begin from 0, Output indices begin from 1
//}

// auto Application::commonChat_deleteMessage(const std::shared_ptr<User>& user) const -> void
//{
//    //std::cout << std::endl << YELLOW << "Select message number for deleting: " << BOLDGREEN;
//    //int message_number{Utils::inputIntegerValue()};
//    //std::cout << RESET;
//    //_common_chat->deleteMessage(user, message_number - 1);  // array's indices begin from 0, Output indices begin from 1
//}

// auto Application::privateMenu(const std::shared_ptr<User>& user) -> int
//{
//    //auto isContinue{true};
//    //while (isContinue)
//    //{
//    //    printNewMessagesUsers(user);
//
//    //    std::string menu_arr[]{"Private Chat:", "View chat users names", "Select target user by name", "Select target user by ID",
//    "Exit"};
//
//    //    auto menu_item{menu(menu_arr, 5)};
//
//    //    switch (menu_item)
//    //    {
//    //        case 1: privateMenu_viewUsersNames(); break;
//    //        case 2:
//    //        {
//    //            auto index{0};
//    //            if ((index = privateMenu_selectByName(user)) != UNSUCCESSFUL) privateChat(user, _user_array[index]);
//    //        }
//    //        break;
//    //        case 3: privateMenu_selectByID(user); break;
//    //        default: isContinue = false; break;
//    //    }
//    //}
//    //return 0;
//}

// auto Application::privateMenu_viewUsersNames() const -> void
//{
//    //std::cout << std::endl;
//    //std::cout << BOLDGREEN << std::setw(5) << std::setfill(' ') << std::right << "ID"
//    //          << "." << BOLDYELLOW << std::setw(MAX_INPUT_SIZE) << std::setfill(' ') << std::left << "User Name" << std::endl;
//
//    //for (auto i{0}; i < _current_user_number; ++i)
//    //{
//    //    std::cout << BOLDGREEN << std::setw(5) << std::setfill(' ') << std::right << i + 1 << "." << BOLDYELLOW <<
//    std::setw(MAX_INPUT_SIZE)
//    //              << std::setfill(' ') << std::left << _user_array[i]->getUserName()
//    //              << std::endl;  // array's indices begin from 0, Output indices begin from 1
//    //    if (!((i + 1) % LINE_TO_PAGE))
//    //    {
//    //        std::cout << std::endl << RESET << YELLOW << "Press Enter for continue...";
//    //        std::cin.get();  //  Suspend via LINE_TO_PAGE lines
//    //    }
//    //}
//    //std::cout << RESET;
//}
//
// auto Application::privateMenu_selectByName(const std::shared_ptr<User>& user) const -> int
//{
//    //auto index{UNSUCCESSFUL};
//    //auto isOK{false};
//    //while (!isOK)
//    //{
//    //    std::cout << std::endl << RESET << YELLOW << "Input target user name: " << BOLDYELLOW;
//    //    std::string user_name;
//    //    std::cin >> user_name;
//    //    std::cout << RESET;
//    //    const std::string& (User::*get_name)() const = &User::getUserName;
//    //    if ((index = checkingForStringExistence(user_name, get_name)) == UNSUCCESSFUL)
//    //    {
//    //        std::cout << RED << "User don't exist!" << std::endl;
//    //        std::cout << std::endl << BOLDYELLOW << "Try again?(Y/N):" << BOLDGREEN;
//    //        if (!Utils::isOKSelect()) return UNSUCCESSFUL;
//    //        continue;
//    //    }
//    //    isOK = true;
//    //}
//    //return index;
//}
//
// auto Application::privateMenu_selectByID(const std::shared_ptr<User>& user) -> void
//{
//    //std::cout << std::endl << RESET << YELLOW << "Input target user ID: " << BOLDGREEN;
//    //auto index{Utils::inputIntegerValue()};
//    //std::cout << RESET;
//    //try
//    //{
//    //    privateChat(user, _user_array.at(index - 1));  // array's indices begin from 0, Output indices begin from 1
//    //}
//    //catch (std::exception& e)
//    //{
//    //    std::cout << BOLDRED << "Exception: " << e.what() << RESET << std::endl;
//    //}
//}
//
// auto Application::printNewMessagesUsers(const std::shared_ptr<User>& user) -> void
//{
//    //auto new_message{_new_messages_array[user->getUserID()]};
//    //auto user_number{new_message->usersNumber()};
//    //if (user_number)
//    //{
//    //    std::cout << std::endl;
//    //    std::cout << BOLDYELLOW << UNDER_LINE << "User sended new message(s):" << RESET << std::endl;
//    //    std::cout << std::endl;
//    //    std::cout << BOLDGREEN << std::setw(5) << std::setfill(' ') << std::right << "ID"
//    //              << "." << BOLDYELLOW << std::setw(MAX_INPUT_SIZE) << std::setfill(' ') << std::left << "User Name" << std::endl;
//
//    //    for (auto i{0u}; i < user_number; ++i)
//    //    {
//    //        auto userID{new_message->getUserID(i)};
//    //        auto msg_vector{new_message->getMessages(userID)};
//    //        auto msg_number{msg_vector.size()};
//    //        std::cout << BOLDGREEN << std::setw(5) << std::setfill(' ') << std::right << userID + 1 << "." << BOLDYELLOW
//    //                  << std::setw(MAX_INPUT_SIZE) << std::setfill(' ') << std::left << _user_array[userID]->getUserName() << RESET <<
//    GREEN
//    //                  << "(" << msg_number << " new message(s))" << std::endl;  // array's indices begin from 0, Output indices begin
//    from 1
//    //    }
//    //}
//}
//
// auto Application::privateChat(const std::shared_ptr<User>& source_user, const std::shared_ptr<User>& target_user) -> int
//{
//    //auto isContinue{true};
//
//    //auto currentChat{getPrivateChat(source_user, target_user)};
//
//    //while (isContinue)
//    //{
//    //    std::string menu_arr[]{"Private Chat:", "View chat", "Add message", "Edit message", "Delete message", "Exit"};
//
//    //    auto menu_item{menu(menu_arr, 6)};
//
//    //    switch (menu_item)
//    //    {
//    //        case 1:
//    //            if (currentChat.get()->isInitialized())
//    //            {
//    //                std::cout << std::endl;
//    //                currentChat->printMessages(0, currentChat->getCurrentMessageNum());
//
//    //                auto new_message{_new_messages_array[source_user->getUserID()]};
//    //                auto msg_vector{new_message->getMessages(target_user->getUserID())};
//    //                auto msg_number{msg_vector.size()};
//    //                if (msg_number)
//    //                {
//    //                    new_message->removeAllMessages(target_user->getUserID());
//    //                }
//    //            }
//    //            break;
//    //        case 2: privateChat_addMessage(source_user, target_user, currentChat); break;
//    //        case 3: privateChat_editMessage(source_user, target_user, currentChat); break;
//    //        case 4: privateChat_deleteMessage(source_user, target_user, currentChat); break;
//    //        default: isContinue = false; break;
//    //    }
//    //}
//    //return 0;
//}

// auto Application::privateChat_addMessage(
//    const std::shared_ptr<User>& source_user, const std::shared_ptr<User>& target_user, std::shared_ptr<Chat>& chat) -> void
//{
//    //if (!chat->isInitialized())
//    //{
//    //    chat = std::make_shared<Chat>();
//    //    long long first_userID{source_user->getUserID()};
//    //    long long second_userID{target_user->getUserID()};
//    //    auto isSwap(Utils::minToMaxOrder(first_userID, second_userID));
//
//    //    long long mapKey{(static_cast<long long>(first_userID) << 32) + second_userID};  // Create value for key value
//
//    //    if (isSwap)
//    //    {
//    //        chat->setFirstUser(target_user);
//    //        chat->setSecondUser(source_user);
//    //    }
//    //    else
//    //    {
//    //        chat->setFirstUser(source_user);
//    //        chat->setSecondUser(target_user);
//    //    }
//    //    _private_chat_array[mapKey] = chat;
//    //    ++_current_chat_number;
//    //    chat->setInitialized(true);
//    //}
//    //auto message{chat->addMessage(source_user)};
//    //if (!message->isInitialized()) return;
//    //auto index{target_user->getUserID()};
//    //_new_messages_array[index]->addNewMessage(message);
//}

// auto Application::privateChat_editMessage(
//    const std::shared_ptr<User>& source_user, const std::shared_ptr<User>& target_user, const std::shared_ptr<Chat>& chat) const -> void
//{
//    //std::cout << std::endl << RESET << YELLOW << "Select message number for editing: " << BOLDGREEN;
//    //int message_number{Utils::inputIntegerValue()};
//    //std::cout << RESET;
//    //if (chat->isInitialized())
//    //{
//    //    auto message{chat->editMessage(source_user, message_number - 1)};  // array's indices begin from 0, Output indices begin from 1
//    //    if (!message->isInitialized()) return;
//
//    //    auto index{target_user->getUserID()};
//    //    _new_messages_array[index]->addNewMessage(message);
//    //}
//}

// auto Application::privateChat_deleteMessage(
//    const std::shared_ptr<User>& source_user, const std::shared_ptr<User>& target_user, const std::shared_ptr<Chat>& chat) const -> void
//{
//    //std::cout << std::endl << RESET << YELLOW << "Select message number for deleting: " << BOLDGREEN;
//    //int message_number{Utils::inputIntegerValue()};
//    //std::cout << RESET;
//    //if (chat->isInitialized())
//    //{
//    //    auto message{chat->deleteMessage(source_user, message_number - 1)};  // array's indices begin from 0, Output indices begin from
//    1
//    //    if (!message->isInitialized()) return;
//
//    //    auto index{target_user->getUserID()};
//    //    _new_messages_array[index]->removeNewMessage(message);
//    //}
//}

// auto Application::getPrivateChat(const std::shared_ptr<User>& source_user, const std::shared_ptr<User>& target_user) const
//    -> const std::shared_ptr<Chat>
//{
//    //long long first_userID{source_user->getUserID()};
//    //long long second_userID{target_user->getUserID()};
//
//    //Utils::minToMaxOrder(first_userID, second_userID);
//
//    //long long searchID{(static_cast<long long>(first_userID) << 32) + second_userID};  // Create value for search
//
//    //auto it = _private_chat_array.begin();
//
//    //for (; it != _private_chat_array.end(); ++it)
//    //{
//    //    if (it->first == searchID) return it->second;
//    //}
//
//    //return std::make_shared<Chat>();
//}

// auto Application::checkingForStringExistence(const std::string& string, const std::string& (User::*get)() const) const -> int
//{
//    //for (auto i{0}; i < _current_user_number; ++i)
//    //{
//    //    if (string == (_user_array[i].get()->*get)()) return i;
//    //}
//    //return UNSUCCESSFUL;
//}

auto Application::menu(std::string* string_arr, int size) const -> int
{
    if (size <= 0) return UNSUCCESSFUL;

    std::cout << std::endl;
    std::cout << BOLDYELLOW << UNDER_LINE << string_arr[0] << RESET << std::endl;  // index 0 is Menu Name

    for (auto i{1}; i < size; ++i)
    {
        std::cout << BOLDGREEN << i << "." << RESET << string_arr[i] << std::endl;
    }
    std::cout << YELLOW << "Your choice?: " << BOLDGREEN;
    int menu_item{Utils::inputIntegerValue()};
    std::cout << RESET;

    return menu_item;
}

auto Application::sendToServer(const char* message, size_t message_length, OperationCode operation_code) -> const char*
{
    if (_msg_buffer_size < message_length + HEADER_SIZE)
    {
        _msg_buffer_size = message_length + HEADER_SIZE;
        _client->setBufferSize(_msg_buffer_size);
        delete[] _msg_buffer;
        _msg_buffer = new char[_msg_buffer_size];
    }

    _client->setBufferSize(message_length + HEADER_SIZE);

    _current_msg_length = 0;

    addToBuffer(_msg_buffer, _current_msg_length, static_cast<int>(OperationCode::CHECK_SIZE));
    addToBuffer(_msg_buffer, _current_msg_length, message_length);

    auto receive_buf{talkToServer(_msg_buffer, _current_msg_length)};

    _current_msg_length = 0;

    addToBuffer(_msg_buffer, _current_msg_length, static_cast<int>(operation_code));
    addToBuffer(_msg_buffer, _current_msg_length, static_cast<int>(OperationCode::CHECK_SIZE));
    addToBuffer(_msg_buffer, _current_msg_length, message, message_length);

    receive_buf = talkToServer(_msg_buffer, _current_msg_length);

    auto message_size{-1};
    getFromBuffer(receive_buf, sizeof(int), message_size);
    //   std::cout << "Message Size: " << message_size << std::endl;

    //    getFromBuffer(receive_buf, sizeof(int), message_size);
    //    std::cout << "First: " << message_size << std::endl;

    _client->setBufferSize(message_size + HEADER_SIZE);

    _current_msg_length = 0;

    addToBuffer(_msg_buffer, _current_msg_length, static_cast<int>(operation_code));
    addToBuffer(_msg_buffer, _current_msg_length, static_cast<int>(OperationCode::READY));

    receive_buf = talkToServer(_msg_buffer, _current_msg_length);

    // std::cout << "READY" << std::endl;

    receive_buf[message_size] = '\0';

    return receive_buf;
}

auto Application::talkToServer(const char* message, size_t msg_length) const -> char*
{
    while (_client->getOutMessageReady())
    {
    }
    // std::cout << message << std::endl;
    _client->setMessage(message, msg_length);
    _client->setOutMessageReady(true);
    while (!_client->getInMessageReady())
    {
        if (_client->getServerError())
        {
            break;
        }
    }
    _client->setInMessageReady(false);
    return _client->getMessage();
}
auto Application::addToBuffer(char* buffer, size_t& cur_msg_len, int value) const -> void
{
    auto length{sizeof(value)};
    auto char_ptr{reinterpret_cast<char*>(&value)};

    for (auto i{0}; i < length; ++i)
    {
        buffer[i + cur_msg_len] = char_ptr[i];
    }
    cur_msg_len += length;
}

auto Application::addToBuffer(char* buffer, size_t& cur_msg_len, const char* string, size_t str_len) const -> void
{
    for (auto i{0}; i < str_len; ++i)
    {
        buffer[i + cur_msg_len] = string[i];
    }
    cur_msg_len += str_len;
}

auto Application::getFromBuffer(const char* buffer, size_t shift, int& value) const -> void
{
    char val_buff[sizeof(value)];
    auto length{sizeof(value)};

    for (auto i{0}; i < length; ++i)
    {
        val_buff[i] = buffer[shift + i];
    }
    value = *(reinterpret_cast<int*>(val_buff));
}

auto Application::getFromBuffer(const char* buffer, size_t shift, char* string, size_t str_len) const -> void
{
    for (auto i{0}; i < str_len; ++i)
    {
        string[i] = buffer[shift + i];
    }
}
