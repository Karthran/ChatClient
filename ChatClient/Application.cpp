#include <iostream>
#include <memory>
#include <iomanip>
#include <exception>
#include <string.h>
#include <chrono>
#include "Application.h"
#include "Client.h"
#include "core.h"

#include "Utils.h"

#ifdef _WIN32
#include <cstdio>
#include <windows.h>
#pragma execution_character_set("utf-8")
#endif

std::shared_ptr<Client> _client{};
std::condition_variable _in_ready_event_holder;
std::condition_variable _out_ready_event_holder; 

Application::Application()
{
    _msg_buffer = std::shared_ptr<char[]>(new char[DEFAULT_BUFLEN]);
    Utils::getSelfPath(_self_path);
}

auto Application::run() -> void
{
    Utils::printOSVersion();

    _client = std::make_shared<Client>();
    _client->run(_in_ready_event_holder, _out_ready_event_holder);

    std::cout << std::endl << BOLDYELLOW << UNDER_LINE << "Wellcome to Console Chat!" << RESET << std::endl;

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
                if (_client->isError())
                {
                    break;
                }
                else
                {
                    char stop[64];
                    size_t size{0};
                    addToBuffer(stop, size, static_cast<int>(OperationCode::STOP));
                    addToBuffer(stop, size, _user_id.c_str(), _user_id.size());
                    talkToServer(stop, size);
                    isContinue = false;
                    break;
                }
        }
        if (_client->isError()) break;
    }
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

        auto result{sendToServer(query.c_str(), query.size(), OperationCode::SIGN_IN)};

        if (strcmp(result, RETURN_ERROR.c_str()))
        {
            _user_id = result;
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
        std::string menu_arr[] = {"Select chat type:", "Common chat", "Private chat", "Sign Out"};

        addNewMsgInCommonChatToMenuItem(menu_arr[1]);
        addNewMsgInPrivateChatToMenuItem(menu_arr[2]);

        auto menu_item{menu(menu_arr, 4)};

        switch (menu_item)
        {
            case 1: commonChat(); break;
            case 2: privateMenu(); break;
            default: isContinue = false; break;
        }
    }

    return;
}

auto Application::addNewMsgInCommonChatToMenuItem(std::string& menu_item) -> void
{
    auto result{sendToServer(" ", 1, OperationCode::NEW_MESSAGES_IN_COMMON_CHAT)};

    auto row_num{-1};
    auto column_num{-1};
    getFromBuffer(result, sizeof(int), row_num);
    getFromBuffer(result, 2 * sizeof(int), column_num);
    auto data_ptr{result + 3 * sizeof(int)};
    if (row_num)
    {
        auto sum_msg{0};
        for (auto i{0}; i < row_num; ++i)
        {
            auto length{strlen(data_ptr)};
            data_ptr += length + 1;
            sum_msg += std::stoi(data_ptr);
            length = strlen(data_ptr);
            data_ptr += length + 1;
        }
        menu_item = BOLDYELLOW + menu_item + RESET + GREEN + "(" + std::to_string(sum_msg) + " new message(s) from " +
                    std::to_string(row_num) + " user(s))" + RESET;
    }
}

auto Application::addNewMsgInPrivateChatToMenuItem(std::string& menu_item) -> void
{
    auto result{sendToServer(" ", 1, OperationCode::NEW_MESSAGES_IN_PRIVATE_CHAT)};
    auto row_num{-1};
    auto column_num{-1};
    getFromBuffer(result, sizeof(int), row_num);
    getFromBuffer(result, 2 * sizeof(int), column_num);
    auto data_ptr{result + 3 * sizeof(int)};
    if (row_num)
    {
        auto sum_msg{0};
        for (auto i{0}; i < row_num; ++i)
        {
            auto length{strlen(data_ptr)};
            data_ptr += length + 1;
            sum_msg += std::stoi(data_ptr);
            length = strlen(data_ptr);
            data_ptr += length + 1;
        }
        menu_item = BOLDYELLOW + menu_item + RESET + GREEN + "(" + std::to_string(sum_msg) + " new message(s) from " +
                    std::to_string(row_num) + " user(s))" + RESET;
    }
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
            case 1: commonChat_viewMessage(); break;
            case 2: commonChat_addMessage(); break;
            case 3: commonChat_editMessage(); break;
            case 4: commonChat_deleteMessage(); break;
            default: isContinue = false; break;
        }
    }
    return SUCCESSFUL;
}

auto Application::commonChat_viewMessage() -> void
{
    auto result{sendToServer(" ", 1, OperationCode::COMMON_CHAT_GET_MESSAGES)};

    auto old_messages_num{-1};
    auto old_column_num{-1};
    auto new_messages_num{-1};
    auto new_column_num{-1};
    getFromBuffer(result, sizeof(int), old_messages_num);  // first int OperationCode::COMMON_CHAT_GET_MESSAGES
    getFromBuffer(result, 2 * sizeof(int), old_column_num);
    getFromBuffer(result, 3 * sizeof(int), new_messages_num);
    getFromBuffer(result, 4 * sizeof(int), new_column_num);

    auto data_ptr{result + 5 * sizeof(int)};
    printMessages(data_ptr, old_messages_num, old_column_num);
    printMessages(data_ptr, new_messages_num, new_column_num, true);
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

auto Application::commonChat_editMessage() -> void
{
    std::cout << std::endl << YELLOW << "Select message number for editing: " << BOLDGREEN;
    int message_number{Utils::inputIntegerValue()};
    std::cout << RESET;
    auto str_number{std::to_string(message_number)};
    str_number.push_back('\0');

    auto result{sendToServer(str_number.c_str(), str_number.size(), OperationCode::COMMON_CHAT_CHECK_MESSAGE)};
    auto messages_num{-1};
    auto column_num{-1};
    getFromBuffer(result, sizeof(int), messages_num);  // first int OperationCode::COMMON_CHAT_CHECK_MESSAGE
    getFromBuffer(result, 2 * sizeof(int), column_num);
    auto data_ptr{result + 3 * sizeof(int)};
    if (!messages_num) return;
    printMessages(data_ptr, messages_num, column_num);

    std::string edited_message{};
    editMessage(edited_message);
    if (!edited_message.size()) return;
    edited_message = str_number + edited_message;
    edited_message.push_back('\0');
    sendToServer(edited_message.c_str(), edited_message.size(), OperationCode::COMMON_CHAT_EDIT_MESSAGE);
}

auto Application::commonChat_deleteMessage() -> void
{
    std::cout << std::endl << YELLOW << "Select message number for deleting: " << BOLDGREEN;
    int message_number{Utils::inputIntegerValue()};
    std::cout << RESET;
    auto str_number{std::to_string(message_number)};
    str_number.push_back('\0');

    auto result{sendToServer(str_number.c_str(), str_number.size(), OperationCode::COMMON_CHAT_CHECK_MESSAGE)};
    auto messages_num{-1};
    auto column_num{-1};
    getFromBuffer(result, sizeof(int), messages_num);  // first int OperationCode::COMMON_CHAT_CHECK_MESSAGE
    getFromBuffer(result, 2 * sizeof(int), column_num);
    auto data_ptr{result + 3 * sizeof(int)};
    if (!messages_num) return;
    printMessages(data_ptr, messages_num, column_num);
    std::cout << BOLDYELLOW << "Delete message?(Y/N):" << BOLDGREEN;
    if (!Utils::isOKSelect()) return;
    std::cout << RESET;
    sendToServer(str_number.c_str(), str_number.size(), OperationCode::COMMON_CHAT_DELETE_MESSAGE);
}

auto Application::privateMenu() -> void
{
    auto isContinue{true};
    while (isContinue)
    {
        printUserIDNameSurnameWithNewMessages();

        std::string menu_arr[]{
            "Private Chat:", "View chat users names", "View users with private chat", "Select target user by ID", "Exit"};

        auto menu_item{menu(menu_arr, 5)};

        switch (menu_item)
        {
            case 1: privateMenu_viewUsersNames(); break;
            case 2: privateMenu_viewUsersExistsChat(); break;
            case 3: privateMenu_selectByID(); break;
            default: isContinue = false; break;
        }
    }
}

auto Application::privateMenu_viewUsersNames() -> void
{
    auto result{sendToServer(" ", 1, OperationCode::VIEW_USERS_ID_NAME_SURNAME)};
    auto messages_num{-1};
    auto columns_num{-1};
    getFromBuffer(result, sizeof(int), messages_num);  // first int OperationCode::VIEW_USERS_ID_NAME_SURNAME
    getFromBuffer(result, 2 * sizeof(int), columns_num);
    auto data_ptr{result + 3 * sizeof(int)};
    if (!messages_num) return;

    std::vector<std::string> message{};

    std::cout << std::endl;
    std::cout << BOLDGREEN << std::setw(5) << std::setfill(' ') << std::right << "ID"
              << "." << BOLDYELLOW << "User Name" << std::endl;

    for (auto msg_index{0}; msg_index < messages_num; ++msg_index)
    {
        message.clear();
        for (auto str_index{0}; str_index < columns_num; ++str_index)
        {
            auto length{strlen(data_ptr)};
            message.push_back(data_ptr);
            data_ptr += length + 1;
        }
        std::cout << BOLDGREEN << std::setw(5) << std::setfill(' ') << std::right << message[0] << "." << BOLDYELLOW << message[1] << " "
                  << message[2] << std::endl;

        if (!((msg_index + 1) % LINE_TO_PAGE))
        {
            std::cout << std::endl << RESET << YELLOW << "Press Enter for continue...";
            std::cin.get();  //  Suspend via LINE_TO_PAGE lines
        }
    }
    std::cout << RESET;
}

auto Application::privateMenu_viewUsersExistsChat() -> void
{
    auto result{sendToServer(" ", 1, OperationCode::VIEW_USERS_WITH_PRIVATE_CHAT)};
    auto messages_num{-1};
    auto columns_num{-1};
    getFromBuffer(result, sizeof(int), messages_num);  // first int OperationCode::VIEW_USERS_WITH_PRIVATE_CHAT
    getFromBuffer(result, 2 * sizeof(int), columns_num);
    auto data_ptr{result + 3 * sizeof(int)};
    if (!messages_num) return;

    std::vector<std::string> message{};

    std::cout << std::endl;
    std::cout << BOLDGREEN << std::setw(5) << std::setfill(' ') << std::right << "ID"
              << "." << BOLDYELLOW << "User Name" << std::endl;

    for (auto msg_index{0}; msg_index < messages_num; ++msg_index)
    {
        message.clear();
        for (auto str_index{0}; str_index < columns_num; ++str_index)
        {
            auto length{strlen(data_ptr)};
            message.push_back(data_ptr);
            data_ptr += length + 1;
        }
        std::cout << BOLDGREEN << std::setw(5) << std::setfill(' ') << std::right << message[0] << "." << BOLDYELLOW << message[1] << " "
                  << message[2] << std::endl;

        if (!((msg_index + 1) % LINE_TO_PAGE))
        {
            std::cout << std::endl << RESET << YELLOW << "Press Enter for continue...";
            std::cin.get();  //  Suspend via LINE_TO_PAGE lines
        }
    }
    std::cout << RESET;
}

auto Application::privateMenu_selectByID() -> void
{
    std::cout << std::endl << RESET << YELLOW << "Input target user ID: " << BOLDGREEN;
    auto message_number{Utils::inputIntegerValue()};
    std::cout << RESET;

    auto str_number{std::to_string(message_number)};
    str_number.push_back('\0');

    auto result{sendToServer(str_number.c_str(), str_number.size(), OperationCode::GET_PRIVATE_CHAT_ID)};
    auto messages_num{-1};
    auto column_num{-1};
    getFromBuffer(result, sizeof(int), messages_num);  // first int OperationCode::GET_PRIVATE_CHAT_ID
    getFromBuffer(result, 2 * sizeof(int), column_num);
    auto data_ptr{result + 3 * sizeof(int)};
    if (!messages_num) return;
    _private_chat_id = data_ptr;
    privateChat();
}

auto Application::printUserIDNameSurnameWithNewMessages() -> void
{
    auto result{sendToServer(" ", 1, OperationCode::VIEW_USERS_WITH_NEW_MESSAGES)};
    auto messages_num{-1};
    auto columns_num{-1};
    getFromBuffer(result, sizeof(int), messages_num);  // first int OperationCode::VIEW_USERS_WITH_NEW_MESSAGES
    getFromBuffer(result, 2 * sizeof(int), columns_num);
    auto data_ptr{result + 3 * sizeof(int)};
    if (!messages_num) return;

    std::vector<std::string> message{};

    std::cout << std::endl;
    std::cout << BOLDYELLOW << UNDER_LINE << "User sended new message(s):" << RESET << std::endl;
    std::cout << std::endl;
    std::cout << BOLDGREEN << std::setw(5) << std::setfill(' ') << std::right << "ID"
              << "." << BOLDYELLOW << "User Name" << std::endl;

    for (auto msg_index{0}; msg_index < messages_num; ++msg_index)
    {
        message.clear();
        for (auto str_index{0}; str_index < columns_num; ++str_index)
        {
            auto length{strlen(data_ptr)};
            message.push_back(data_ptr);
            data_ptr += length + 1;
        }

        std::cout << BOLDGREEN << std::setw(5) << std::setfill(' ') << std::right << message[0] << "." << BOLDYELLOW << message[2] << " "
                  << message[3] << RESET << GREEN << "(" << message[1] << " new message(s))" << std::endl;
    }
}

auto Application::privateChat() -> void
{
    auto isContinue{true};

    while (isContinue)
    {
        std::string menu_arr[]{"Private Chat:", "View chat", "Add message", "Edit message", "Delete message", "Exit"};

        auto menu_item{menu(menu_arr, 6)};

        switch (menu_item)
        {
            case 1: privateChat_viewMessage(); break;
            case 2: privateChat_addMessage(); break;
            case 3: privateChat_editMessage(); break;
            case 4: privateChat_deleteMessage(); break;
            default: isContinue = false; break;
        }
    }
}

auto Application::privateChat_viewMessage() -> void
{
    std::string chat_id = _private_chat_id;
    chat_id.push_back('\0');
    auto result{sendToServer(chat_id.c_str(), chat_id.size(), OperationCode::PRIVATE_CHAT_GET_MESSAGES)};

    auto old_messages_num{-1};
    auto old_column_num{-1};
    auto new_messages_num{-1};
    auto new_column_num{-1};
    getFromBuffer(result, sizeof(int), old_messages_num);  // first int OperationCode::PRIVATE_CHAT_GET_MESSAGES
    getFromBuffer(result, 2 * sizeof(int), old_column_num);
    getFromBuffer(result, 3 * sizeof(int), new_messages_num);
    getFromBuffer(result, 4 * sizeof(int), new_column_num);

    auto data_ptr{result + 5 * sizeof(int)};
    printMessages(data_ptr, old_messages_num, old_column_num, false, true);
    printMessages(data_ptr, new_messages_num, new_column_num, true, true);
}

auto Application::privateChat_addMessage() -> void
{
    std::string new_message{};

    std::cout << std::endl << YELLOW << "Input message: " << BOLDGREEN;
    Utils::getString(new_message);
    std::cout << RESET;
    std::cout << BOLDYELLOW << "Send message?(Y/N):" << BOLDGREEN;
    std::cout << RESET;

    if (!Utils::isOKSelect()) return;
    new_message.push_back('\0');
    new_message += _private_chat_id;
    new_message.push_back('\0');
    sendToServer(new_message.c_str(), new_message.size(), OperationCode::PRIVATE_CHAT_ADD_MESSAGE);
}

auto Application::privateChat_editMessage() -> void
{
    std::cout << std::endl << YELLOW << "Select message number for editing: " << BOLDGREEN;
    int message_number{Utils::inputIntegerValue()};
    std::cout << RESET;
    auto query_data{std::to_string(message_number)};
    query_data.push_back('\0');
    query_data += _private_chat_id;
    query_data.push_back('\0');

    auto result{sendToServer(query_data.c_str(), query_data.size(), OperationCode::PRIVATE_CHAT_CHECK_MESSAGE)};
    auto messages_num{-1};
    auto column_num{-1};
    getFromBuffer(result, sizeof(int), messages_num);  // first int OperationCode::PRIVATE_CHAT_CHECK_MESSAGE
    getFromBuffer(result, 2 * sizeof(int), column_num);
    auto data_ptr{result + 3 * sizeof(int)};
    if (!messages_num) return;
    printMessages(data_ptr, messages_num, column_num, false, true);

    std::string edited_message{};
    editMessage(edited_message);
    if (!edited_message.size()) return;
    edited_message = query_data + edited_message;
    edited_message.push_back('\0');
    sendToServer(edited_message.c_str(), edited_message.size(), OperationCode::PRIVATE_CHAT_EDIT_MESSAGE);
}

auto Application::privateChat_deleteMessage() -> void
{
    std::cout << std::endl << RESET << YELLOW << "Select message number for deleting: " << BOLDGREEN;
    int message_number{Utils::inputIntegerValue()};
    std::cout << RESET;
    auto query_data{std::to_string(message_number)};
    query_data.push_back('\0');
    query_data += _private_chat_id;
    query_data.push_back('\0');

    auto result{sendToServer(query_data.c_str(), query_data.size(), OperationCode::PRIVATE_CHAT_CHECK_MESSAGE)};
    auto messages_num{-1};
    auto column_num{-1};
    getFromBuffer(result, sizeof(int), messages_num);  // first int OperationCode::PRIVATE_CHAT_CHECK_MESSAGE
    getFromBuffer(result, 2 * sizeof(int), column_num);
    auto data_ptr{result + 3 * sizeof(int)};
    if (!messages_num) return;

    printMessages(data_ptr, messages_num, column_num, false, true);
    std::cout << BOLDYELLOW << "Delete message?(Y/N):" << BOLDGREEN;
    if (!Utils::isOKSelect()) return;
    std::cout << RESET;
    sendToServer(query_data.c_str(), query_data.size(), OperationCode::PRIVATE_CHAT_DELETE_MESSAGE);
}

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

auto Application::printMessages(const char*& data_ptr, int messages_num, int columns_num, bool is_new, bool use_status) const -> void
{
    std::vector<std::string> message{};

    for (auto msg_index{0}; msg_index < messages_num; ++msg_index)
    {
        message.clear();
        for (auto str_index{0}; str_index < columns_num; ++str_index)
        {
            auto length{strlen(data_ptr)};
            message.push_back(data_ptr);
            data_ptr += length + 1;
        }

        printMessage(message, is_new, use_status);
        if (!((msg_index + 1) % MESSAGES_ON_PAGE))
        {
            std::cout << std::endl << RESET << YELLOW << "Press Enter for continue...";
            std::cin.get();  // Suspend via MESSAGES_ON_PAGE messages
        }
    }
}

auto Application::printMessage(const std::vector<std::string>& message, bool is_new, bool use_status) const -> void
{
    std::cout << BOLDCYAN << std::setw(120) << std::setfill('-') << "-" << std::endl;
    std::cout << BOLDGREEN << std::setw(5) << std::setfill(' ') << std::right << message[0] << "." << RESET;
    std::cout << YELLOW << "  Created: ";
    std::cout << BOLDYELLOW << message[1] << " " << message[2] << " "
              << "(ID: " << message[3] << ")";
    std::cout << std::setw(20) << std::setfill(' ') << RESET << YELLOW;

    std::cout << message[5];

    if (use_status && message[3] == _user_id) std::cout << "       " << BOLDGREEN << message[8] << RESET;
    if (is_new && message[3] != _user_id) std::cout << "       " << BOLDRED << "New" << RESET;
    std::cout << std::endl;

    std::cout << CYAN << std::setw(120) << std::setfill('-') << "-" << std::endl;
    std::cout << BOLDYELLOW << message[4] << RESET << std::endl;

    if (message[6] == "1")
    {
        std::cout << CYAN << std::setw(120) << std::setfill('-') << "-" << std::endl;
        std::cout << YELLOW << "    Edited: ";
        std::cout << message[7] << std::endl;
    }
    std::cout << BOLDCYAN << std::setw(120) << std::setfill('-') << "-" << RESET << std::endl;
}

auto Application::editMessage(std::string& edited_message) -> void
{
    std::cout << YELLOW << "Input new message: " << BOLDGREEN;
    Utils::getString(edited_message);
    std::cout << RESET;

    std::cout << BOLDYELLOW << "Save changes?(Y/N):" << BOLDGREEN;
    if (!Utils::isOKSelect()) edited_message.clear();
    std::cout << RESET;
}

auto Application::sendToServer(const char* message, size_t message_length, OperationCode operation_code) -> const char*
{
    if (_msg_buffer_size < message_length + HEADER_SIZE)
    {
        _msg_buffer_size = message_length + HEADER_SIZE;
        _client->setBufferSize(_msg_buffer_size);
        _msg_buffer = std::shared_ptr<char[]>(new char[_msg_buffer_size]);
    }

    //_client->setBufferSize(message_length + HEADER_SIZE);
    _current_msg_length = 0;
    addToBuffer(_msg_buffer.get(), _current_msg_length, static_cast<int>(OperationCode::CHECK_SIZE));
    addToBuffer(_msg_buffer.get(), _current_msg_length, message_length);
    auto receive_buf{talkToServer(_msg_buffer.get(), _current_msg_length)};
    if (!receive_buf) return nullptr;

    _current_msg_length = 0;
    addToBuffer(_msg_buffer.get(), _current_msg_length, static_cast<int>(operation_code));
    addToBuffer(_msg_buffer.get(), _current_msg_length, static_cast<int>(OperationCode::CHECK_SIZE));
    addToBuffer(_msg_buffer.get(), _current_msg_length, message, message_length);
    receive_buf = talkToServer(_msg_buffer.get(), _current_msg_length);
    if (!receive_buf) return nullptr;

    auto message_size{-1};
    getFromBuffer(receive_buf, sizeof(int), message_size);
    _client->setBufferSize(message_size + HEADER_SIZE);
    _current_msg_length = 0;
    addToBuffer(_msg_buffer.get(), _current_msg_length, static_cast<int>(operation_code));
    addToBuffer(_msg_buffer.get(), _current_msg_length, static_cast<int>(OperationCode::READY));
    receive_buf = talkToServer(_msg_buffer.get(), _current_msg_length);
    if (!receive_buf) return nullptr;

    receive_buf[message_size] = '\0';

    return receive_buf;
}

auto Application::talkToServer(const char* message, size_t msg_length) const -> char*
{
    std::unique_lock<std::mutex> out_ready_lock(out_mutex);
    _out_ready_event_holder.wait_for(out_ready_lock, std::chrono::milliseconds(500), []() { return !_client->getOutMessageReady(); });
    if (_client->getOutMessageReady()) return nullptr;

    _client->setMessage(message, msg_length);
    _client->setOutMessageReady(true);
    _out_ready_event_holder.notify_one();

    std::unique_lock<std::mutex> in_ready_lock(in_mutex);
    _in_ready_event_holder.wait_for(in_ready_lock, std::chrono::milliseconds(500), []() { return _client->getInMessageReady(); });
    if (!_client->getInMessageReady()) return nullptr;

    _client->setInMessageReady(false);
    return _client->getMessage();
}
auto Application::addToBuffer(char* buffer, size_t& cur_msg_len, int value) const -> void
{
    size_t length = sizeof(value);
    auto char_ptr{reinterpret_cast<char*>(&value)};

    for (size_t i = 0; i < length; ++i)
    {
        buffer[i + cur_msg_len] = char_ptr[i];
    }
    cur_msg_len += length;
}

auto Application::addToBuffer(char* buffer, size_t& cur_msg_len, const char* string, size_t str_len) const -> void
{
    for (size_t i = 0; i < str_len; ++i)
    {
        buffer[i + cur_msg_len] = string[i];
    }
    cur_msg_len += str_len;
}

auto Application::getFromBuffer(const char* buffer, size_t shift, int& value) const -> void
{
    char val_buff[sizeof(value)];
    size_t length = sizeof(value);

    for (size_t i = 0; i < length; ++i)
    {
        val_buff[i] = buffer[shift + i];
    }
    value = *(reinterpret_cast<int*>(val_buff));
}

auto Application::getFromBuffer(const char* buffer, size_t shift, char* string, size_t str_len) const -> void
{
    for (size_t i =0; i < str_len; ++i)
    {
        string[i] = buffer[shift + i];
    }
}
