#include <iostream>
#include <cassert>
#include <iomanip>
#include <exception>
#include <fstream>
#include <sstream>

#include "Application.h"
#include "Client.h"
#include "core.h"

#include "Chat.h"
#include "Message.h"
#include "Utils.h"
#include "User.h"
#include "SHA1.h"
#include "PasswordHash.h"
#include "FileUtils.h"
#include "NewMessages.h"

Application::Application()
{
    _common_chat = std::make_shared<Chat>();
    Utils::getSelfPath(_self_path);
}

auto Application::run() -> void
{
    Utils::printOSVersion();

    _client = new Client();
    _client->run();
    //    loop();

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
                talkToServer(std::to_string(static_cast<int>(OperationCode::STOP)));
                isContinue = false;
                break;
        }
    }
}

auto Application::createAccount() -> void
{
    std::string user_name{};
    createAccount_inputName(user_name);

    std::string user_login;
    createAccount_inputLogin(user_login);

    std::string user_password;
    createAccount_inputPassword(user_password);

    std::cout << BOLDYELLOW << std::endl << "Create account?(Y/N): " << BOLDGREEN;
    if (!Utils::isOKSelect()) return;

    std::string result = user_name + " " + user_login + " " + user_password;

    sendToServer(result, OperationCode::REGISTRATION);  // in result now OK or ERROR with LOGIN or NAME

    std::stringstream stream(result);

    stream >> result;

    if (result == RETURN_ERROR)
    {
        stream >> result;
        if (result == "NAME")
        {
            std::cout << std::endl << RED << "Please change name!" << RESET << std::endl;
            return;
        }
        else if (result == "LOGIN")
        {
            std::cout << std::endl << RED << "Please change login." << RESET << std::endl;
            return;
        }
    }
    ++_current_user_number;
}

auto Application::createAccount_inputName(std::string& user_name) const -> void
{
    std::cout << std::endl;
    std::cout << BOLDYELLOW << UNDER_LINE << "Create account:" << RESET << std::endl;
    auto isOK{false};
    while (!isOK)
    {
        std::cout << "Name(max " << MAX_INPUT_SIZE << " letters): ";
        std::cout << BOLDGREEN;
        Utils::getString(user_name, MAX_INPUT_SIZE);
        std::cout << RESET;

        std::string result = user_name;
        sendToServer(result, OperationCode::CHECK_NAME);  // in result now OK or ERROR

        if (result == RETURN_ERROR)
        {
            std::cout << std::endl << RED << "Please change name!" << RESET << std::endl;
        }
        else
        {
            isOK = true;
        }
    }
}

auto Application::createAccount_inputLogin(std::string& user_login) const -> void
{
    auto isOK{false};

    while (!isOK)
    {
        std::cout << std::endl << "Login(max " << MAX_INPUT_SIZE << " letters): ";
        std::cout << BOLDGREEN;
        Utils::getString(user_login, MAX_INPUT_SIZE);
        std::cout << RESET;

        std::string result = user_login;
        sendToServer(result, OperationCode::CHECK_LOGIN);  // in result now OK or ERROR

        if (result == RETURN_ERROR)
        {
            std::cout << std::endl << RED << "Please change login." << RESET;
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

        std::string result = user_login + " " + user_password;
        sendToServer(result, OperationCode::SIGN_IN);  // in result now OK or ERROR
        std::stringstream stream(result);
        stream >> result;

        if (result == RETURN_OK)
        {
            stream >> result;
            _user_index = std::stoi(result);
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
        std::string result = std::to_string(_user_index);
        sendToServer(result, OperationCode::NEW_MESSAGES);  // in result now number of new messages

        std::string menu_arr[] = {"Select chat type:", "Common chat", "Private chat", "Sign Out"};

        auto user_number{std::stoi(result)};

        if (user_number)  // if exist new message for this user
        {
            menu_arr[2] = BOLDYELLOW + menu_arr[2] + RESET + GREEN + "(New message(s) from " + std::to_string(user_number) + " user(s))" +
                          RESET;  // menu_arr[2] = "Private chat"
        }

        auto menu_item{menu(menu_arr, 4)};

        const std::shared_ptr<User> user;

        switch (menu_item)
        {
            case 1: commonChat(user); break;
            case 2: privateMenu(user); break;
            default: isContinue = false; break;
        }
    }

    return;
}

auto Application::commonChat(const std::shared_ptr<User>& user) const -> int
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
                std::cout << std::endl;
                std::string result = "-1" + DELIMITER + "-1";                      // -1 common chat users id
                sendToServer(result, OperationCode::GET_NUMBER_MESSAGES_IN_CHAT);  // in result now number of new messages
                if (result == RETURN_ERROR) break;
                auto message_num{std::stoi(result)};
                for (auto i{0}; i < message_num; ++i)
                {
                    result = std::to_string(i);
                    sendToServer(result, OperationCode::COMMON_CHAT_GET_MESSAGES);  // in result now OK or ERROR
                    std::cout << result;
                }
                break;
            }
            case 2: commonChat_addMessage(user); break;
            case 3: commonChat_editMessage(user); break;
            case 4: commonChat_deleteMessage(user); break;
            default: isContinue = false; break;
        }
    }
    return SUCCESSFUL;
}

auto Application::commonChat_addMessage(const std::shared_ptr<User>& user) const -> void
{
    _common_chat->addMessage(user);
}

auto Application::commonChat_editMessage(const std::shared_ptr<User>& user) const -> void
{
    std::cout << std::endl << YELLOW << "Select message number for editing: " << BOLDGREEN;
    int message_number{Utils::inputIntegerValue()};
    std::cout << RESET;
    _common_chat->editMessage(user, message_number - 1);  // array's indices begin from 0, Output indices begin from 1
}

auto Application::commonChat_deleteMessage(const std::shared_ptr<User>& user) const -> void
{
    std::cout << std::endl << YELLOW << "Select message number for deleting: " << BOLDGREEN;
    int message_number{Utils::inputIntegerValue()};
    std::cout << RESET;
    _common_chat->deleteMessage(user, message_number - 1);  // array's indices begin from 0, Output indices begin from 1
}

auto Application::privateMenu(const std::shared_ptr<User>& user) -> int
{
    auto isContinue{true};
    while (isContinue)
    {
        printNewMessagesUsers(user);

        std::string menu_arr[]{"Private Chat:", "View chat users names", "Select target user by name", "Select target user by ID", "Exit"};

        auto menu_item{menu(menu_arr, 5)};

        switch (menu_item)
        {
            case 1: privateMenu_viewUsersNames(); break;
            case 2:
            {
                auto index{0};
                if ((index = privateMenu_selectByName(user)) != UNSUCCESSFUL) privateChat(user, _user_array[index]);
            }
            break;
            case 3: privateMenu_selectByID(user); break;
            default: isContinue = false; break;
        }
    }
    return 0;
}

auto Application::privateMenu_viewUsersNames() const -> void
{
    std::cout << std::endl;
    std::cout << BOLDGREEN << std::setw(5) << std::setfill(' ') << std::right << "ID"
              << "." << BOLDYELLOW << std::setw(MAX_INPUT_SIZE) << std::setfill(' ') << std::left << "User Name" << std::endl;

    for (auto i{0}; i < _current_user_number; ++i)
    {
        std::cout << BOLDGREEN << std::setw(5) << std::setfill(' ') << std::right << i + 1 << "." << BOLDYELLOW << std::setw(MAX_INPUT_SIZE)
                  << std::setfill(' ') << std::left << _user_array[i]->getUserName()
                  << std::endl;  // array's indices begin from 0, Output indices begin from 1
        if (!((i + 1) % LINE_TO_PAGE))
        {
            std::cout << std::endl << RESET << YELLOW << "Press Enter for continue...";
            std::cin.get();  //  Suspend via LINE_TO_PAGE lines
        }
    }
    std::cout << RESET;
}
auto Application::privateMenu_selectByName(const std::shared_ptr<User>& user) const -> int
{
    auto index{UNSUCCESSFUL};
    auto isOK{false};
    while (!isOK)
    {
        std::cout << std::endl << RESET << YELLOW << "Input target user name: " << BOLDYELLOW;
        std::string user_name;
        std::cin >> user_name;
        std::cout << RESET;
        const std::string& (User::*get_name)() const = &User::getUserName;
        if ((index = checkingForStringExistence(user_name, get_name)) == UNSUCCESSFUL)
        {
            std::cout << RED << "User don't exist!" << std::endl;
            std::cout << std::endl << BOLDYELLOW << "Try again?(Y/N):" << BOLDGREEN;
            if (!Utils::isOKSelect()) return UNSUCCESSFUL;
            continue;
        }
        isOK = true;
    }
    return index;
}
auto Application::privateMenu_selectByID(const std::shared_ptr<User>& user) -> void
{
    std::cout << std::endl << RESET << YELLOW << "Input target user ID: " << BOLDGREEN;
    auto index{Utils::inputIntegerValue()};
    std::cout << RESET;
    try
    {
        privateChat(user, _user_array.at(index - 1));  // array's indices begin from 0, Output indices begin from 1
    }
    catch (std::exception& e)
    {
        std::cout << BOLDRED << "Exception: " << e.what() << RESET << std::endl;
    }
}

auto Application::printNewMessagesUsers(const std::shared_ptr<User>& user) -> void
{
    auto new_message{_new_messages_array[user->getUserID()]};
    auto user_number{new_message->usersNumber()};
    if (user_number)
    {
        std::cout << std::endl;
        std::cout << BOLDYELLOW << UNDER_LINE << "User sended new message(s):" << RESET << std::endl;
        std::cout << std::endl;
        std::cout << BOLDGREEN << std::setw(5) << std::setfill(' ') << std::right << "ID"
                  << "." << BOLDYELLOW << std::setw(MAX_INPUT_SIZE) << std::setfill(' ') << std::left << "User Name" << std::endl;

        for (auto i{0u}; i < user_number; ++i)
        {
            auto userID{new_message->getUserID(i)};
            auto msg_vector{new_message->getMessages(userID)};
            auto msg_number{msg_vector.size()};
            std::cout << BOLDGREEN << std::setw(5) << std::setfill(' ') << std::right << userID + 1 << "." << BOLDYELLOW
                      << std::setw(MAX_INPUT_SIZE) << std::setfill(' ') << std::left << _user_array[userID]->getUserName() << RESET << GREEN
                      << "(" << msg_number << " new message(s))" << std::endl;  // array's indices begin from 0, Output indices begin from 1
        }
    }
}

auto Application::privateChat(const std::shared_ptr<User>& source_user, const std::shared_ptr<User>& target_user) -> int
{
    auto isContinue{true};

    auto currentChat{getPrivateChat(source_user, target_user)};

    while (isContinue)
    {
        std::string menu_arr[]{"Private Chat:", "View chat", "Add message", "Edit message", "Delete message", "Exit"};

        auto menu_item{menu(menu_arr, 6)};

        switch (menu_item)
        {
            case 1:
                if (currentChat.get()->isInitialized())
                {
                    std::cout << std::endl;
                    currentChat->printMessages(0, currentChat->getCurrentMessageNum());

                    auto new_message{_new_messages_array[source_user->getUserID()]};
                    auto msg_vector{new_message->getMessages(target_user->getUserID())};
                    auto msg_number{msg_vector.size()};
                    if (msg_number)
                    {
                        new_message->removeAllMessages(target_user->getUserID());
                    }
                }
                break;
            case 2: privateChat_addMessage(source_user, target_user, currentChat); break;
            case 3: privateChat_editMessage(source_user, target_user, currentChat); break;
            case 4: privateChat_deleteMessage(source_user, target_user, currentChat); break;
            default: isContinue = false; break;
        }
    }
    return 0;
}

auto Application::privateChat_addMessage(
    const std::shared_ptr<User>& source_user, const std::shared_ptr<User>& target_user, std::shared_ptr<Chat>& chat) -> void
{
    if (!chat->isInitialized())
    {
        chat = std::make_shared<Chat>();
        long long first_userID{source_user->getUserID()};
        long long second_userID{target_user->getUserID()};
        auto isSwap(Utils::minToMaxOrder(first_userID, second_userID));

        long long mapKey{(static_cast<long long>(first_userID) << 32) + second_userID};  // Create value for key value

        if (isSwap)
        {
            chat->setFirstUser(target_user);
            chat->setSecondUser(source_user);
        }
        else
        {
            chat->setFirstUser(source_user);
            chat->setSecondUser(target_user);
        }
        _private_chat_array[mapKey] = chat;
        ++_current_chat_number;
        chat->setInitialized(true);
    }
    auto message{chat->addMessage(source_user)};
    if (!message->isInitialized()) return;
    auto index{target_user->getUserID()};
    _new_messages_array[index]->addNewMessage(message);
}
auto Application::privateChat_editMessage(
    const std::shared_ptr<User>& source_user, const std::shared_ptr<User>& target_user, const std::shared_ptr<Chat>& chat) const -> void
{
    std::cout << std::endl << RESET << YELLOW << "Select message number for editing: " << BOLDGREEN;
    int message_number{Utils::inputIntegerValue()};
    std::cout << RESET;
    if (chat->isInitialized())
    {
        auto message{chat->editMessage(source_user, message_number - 1)};  // array's indices begin from 0, Output indices begin from 1
        if (!message->isInitialized()) return;

        auto index{target_user->getUserID()};
        _new_messages_array[index]->addNewMessage(message);
    }
}

auto Application::privateChat_deleteMessage(
    const std::shared_ptr<User>& source_user, const std::shared_ptr<User>& target_user, const std::shared_ptr<Chat>& chat) const -> void
{
    std::cout << std::endl << RESET << YELLOW << "Select message number for deleting: " << BOLDGREEN;
    int message_number{Utils::inputIntegerValue()};
    std::cout << RESET;
    if (chat->isInitialized())
    {
        auto message{chat->deleteMessage(source_user, message_number - 1)};  // array's indices begin from 0, Output indices begin from 1
        if (!message->isInitialized()) return;

        auto index{target_user->getUserID()};
        _new_messages_array[index]->removeNewMessage(message);
    }
}

auto Application::getPrivateChat(const std::shared_ptr<User>& source_user, const std::shared_ptr<User>& target_user) const
    -> const std::shared_ptr<Chat>
{
    long long first_userID{source_user->getUserID()};
    long long second_userID{target_user->getUserID()};

    Utils::minToMaxOrder(first_userID, second_userID);

    long long searchID{(static_cast<long long>(first_userID) << 32) + second_userID};  // Create value for search

    auto it = _private_chat_array.begin();

    for (; it != _private_chat_array.end(); ++it)
    {
        if (it->first == searchID) return it->second;
    }

    return std::make_shared<Chat>();
}

auto Application::checkingForStringExistence(const std::string& string, const std::string& (User::*get)() const) const -> int
{
    for (auto i{0}; i < _current_user_number; ++i)
    {
        if (string == (_user_array[i].get()->*get)()) return i;
    }
    return UNSUCCESSFUL;
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

auto Application::save() -> void
{
    if (!saveUserArray()) return;

    savePasswordHash();

    saveChats();

    saveNewMessages();
}

auto Application::saveUserArray() const -> bool
{
    // Save vector<User>
    File file_user(_self_path + std::string("User.txt"), std::fstream::out);
    if (file_user.getError()) return false;

    file_user.write(_user_array.size());

    for (auto i{0}; i < _user_array.size(); ++i)
    {
        file_user.write(_user_array[i]->getUserName());
        file_user.write(_user_array[i]->getUserLogin());
        file_user.write(_user_array[i]->getUserID());
    }
    return true;
}

auto Application::savePasswordHash() -> void
{
    // Save _password_hash
    File file_hash(_self_path + std::string("UserHash.txt"), std::fstream::out);
    for (auto i{0}; i < _user_array.size(); ++i)
    {
        file_hash.write(_password_hash[_user_array[i]->getUserLogin()]->getSalt());
        Hash hash = _password_hash[_user_array[i]->getUserLogin()]->getHash();
        file_hash.write(hash._A);
        file_hash.write(hash._B);
        file_hash.write(hash._C);
        file_hash.write(hash._D);
        file_hash.write(hash._E);
    }
}

auto Application::saveChats() const -> void
{
    // Save Chats (Common and Privats)
    File file_chat(_self_path + std::string("Chat.txt"), std::fstream::out);

    _common_chat->save(file_chat);

    file_chat.write(_private_chat_array.size());

    for (auto ch : _private_chat_array)
    {
        ch.second->save(file_chat);
    }
}

auto Application::saveNewMessages() -> void
{
    // Save New Messages (Common and Privats)
    File file_newmsg(_self_path + std::string("NewMessages.txt"), std::fstream::out);

    auto target_users_number{_new_messages_array.size()};
    file_newmsg.write(target_users_number);

    for (auto i{0u}; i < target_users_number; ++i)
    {
        auto newMessage{_new_messages_array[i]};
        auto source_users_number{newMessage->usersNumber()};
        if (!source_users_number) continue;      // if  user has no new messages
        file_newmsg.write(i);                    // save target userID
        file_newmsg.write(source_users_number);  // save initiator numbers

        for (auto j{0u}; j < source_users_number; ++j)
        {

            auto userID{newMessage->getUserID(j)};
            long long first_userID{i};
            long long second_userID{userID};
            Utils::minToMaxOrder(first_userID, second_userID);
            long long mapKey{(static_cast<long long>(first_userID) << 32) + second_userID};  // Create value for key value
            file_newmsg.write(userID);                                                       // save initiator ID
            auto message{newMessage->getMessages(userID)};                                   //
            auto msg_number{message.size()};                                                 //
            file_newmsg.write(msg_number);                                                   // msg number from initiator

            for (auto k{0}; k < msg_number; ++k)
            {
                file_newmsg.write(_private_chat_array[mapKey]->getMessageIndex(message[k]));  // msg index in chat
            }
        }
    }
}

auto Application::load() -> void
{
    if (!loadUserArray()) return;

    loadPasswordHash();

    loadChats();

    loadNewMessages();
}

// Load vector<User>
auto Application::loadUserArray() -> bool
{
    File file_user(_self_path + std::string("User.txt"), std::fstream::in);

    if (file_user.getError()) return false;

    size_t user_count{0};
    file_user.read(user_count);
    _current_user_number = static_cast<int>(user_count);

    for (auto i{0}; i < _current_user_number; ++i)
    {
        std::string name{};
        file_user.read(name);
        std::string login{};
        file_user.read(login);
        int userID{-1};
        file_user.read(userID);

        std::shared_ptr<User> user = std::make_shared<User>(name, login, userID);

        _user_array.push_back(user);

        _new_messages_array.push_back(std::make_shared<NewMessages>());  // TODO need loading!!!
    }
    return true;
}

// Load Password Hash
auto Application::loadPasswordHash() -> void
{
    File file_hash(_self_path + std::string("UserHash.txt"), std::fstream::in);
    for (auto i{0}; i < _user_array.size(); ++i)
    {
        std::string salt{};
        file_hash.read(salt);
        Hash hash;
        file_hash.read(hash._A);
        file_hash.read(hash._B);
        file_hash.read(hash._C);
        file_hash.read(hash._D);
        file_hash.read(hash._E);
        _password_hash[_user_array[i]->getUserLogin()] = std::make_shared<PasswordHash>(hash, salt);
    }
}

// Load Chats (Common and Privats)
auto Application::loadChats() -> void
{
    File file_chat(_self_path + std::string("Chat.txt"), std::fstream::in);

    int user1{0}, user2{0};
    file_chat.read(user1);
    file_chat.read(user2);
    if (user1 > 0 || user2 > 0) return;  // Chat.txt begin from -1 -1 (Common chat don't have users)

    _common_chat.get()->load(file_chat, _user_array);

    size_t private_chats_number{0};
    file_chat.read(private_chats_number);

    for (auto i{0}; i < private_chats_number; ++i)
    {
        int first_userID{0}, second_userID{0};
        file_chat.read(first_userID);
        file_chat.read(second_userID);

        long long keyID{(static_cast<long long>(first_userID) << 32) + second_userID};

        _private_chat_array[keyID] = std::make_shared<Chat>();

        _private_chat_array[keyID]->setFirstUser(_user_array[first_userID]);
        _private_chat_array[keyID]->setSecondUser(_user_array[second_userID]);

        _private_chat_array[keyID]->load(file_chat, _user_array);
    }
}

auto Application::loadNewMessages() -> void
{
    // Load New Messages (Common and Privats)
    File file_newmsg(_self_path + std::string("NewMessages.txt"), std::fstream::in);
    if (file_newmsg.getError()) return;
    auto user_number{0};
    file_newmsg.read(user_number);
    while (!file_newmsg.getStream().eof()) /* for (auto i{0}; i < user_number; ++i)*/
    {
        auto target_userID{0};
        file_newmsg.read(target_userID);
        auto users_with_new_msg_number{0};
        file_newmsg.read(users_with_new_msg_number);
        for (auto j{0}; j < users_with_new_msg_number; ++j)
        {
            auto init_userID{0};
            file_newmsg.read(init_userID);

            long long first_userID{target_userID};
            long long second_userID{init_userID};

            Utils::minToMaxOrder(first_userID, second_userID);
            long long mapKey{(static_cast<long long>(first_userID) << 32) + second_userID};  // Create value for key value
            auto chat{_private_chat_array[mapKey]};
            auto msg_number{0};
            file_newmsg.read(msg_number);
            for (auto k{0}; k < msg_number; ++k)
            {
                auto msg_index{0};
                file_newmsg.read(msg_index);
                auto message{chat->getMessageByIndex(msg_index)};
                _new_messages_array[target_userID]->addNewMessage(message);
            }
        }
    }
    return;
}

auto Application::sendToServer(std::string& message, OperationCode operation_code) const -> void
{
    std::string msg_to_srv =
        std::to_string(static_cast<int>(OperationCode::CHECK_SIZE)) + " " + std::to_string(message.size() + HEADER_SIZE);
    std::string msg_from_srv = talkToServer(msg_to_srv);

    msg_to_srv = std::to_string(static_cast<int>(operation_code)) + " " + std::to_string(static_cast<int>(OperationCode::CHECK_SIZE)) +
                 " " + message;
    msg_from_srv = talkToServer(msg_to_srv);

    auto msg_size{std::stoi(std::string(msg_from_srv, 2))};

    _client->setBufferSize(msg_size + HEADER_SIZE);

    msg_to_srv =
        std::to_string(static_cast<int>(operation_code)) + " " + std::to_string(static_cast<int>(OperationCode::READY)) /*+ " " + message*/;

    message = talkToServer(msg_to_srv);
    // return
}

auto Application::talkToServer(const std::string& message) const -> const std::string&
{
    while (_client->getOutMessageReady())
    {
    }
    // std::cout << message << std::endl;
    _client->setMessage(message);
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

auto Application::loop() -> void
{
    std::string message;
    bool loop_flag = true;
    while (loop_flag)
    {
        std::getline(std::cin, message);
        if (message == "end")
        {
            talkToServer(std::to_string(static_cast<int>(OperationCode::STOP)));
            loop_flag = false;
            break;
        }
        sendToServer(message, OperationCode::CHECK_NAME);

        std::cout << message << std::endl;
    }
    std::cout << "Client stop!" << std::endl;
    return;
}
