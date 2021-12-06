#include <iostream>
#include <ctime>
#include <iomanip>
#include <exception>
#include <map>

#include "Chat.h"
#include "Message.h"
#include "User.h"
#include "Utils.h"
#include "FileUtils.h"

Chat::Chat()
{
    /* std::cout << "Chat constr " << this << std::endl;*/
}

Chat::~Chat()
{
    /*std::cout << "Chat destr " << this << std::endl;*/
}

auto Chat::printMessages(int first_index, int number) const -> void
{

    for (auto i{first_index}; i < number; ++i)
    {
        printMessage(i);
        if (!((i + 1) % MESSAGES_ON_PAGE))
        {
            std::cout << std::endl << RESET << YELLOW << "Press Enter for continue...";
            std::cin.get();  // Suspend via MESSAGES_ON_PAGE messages
        }
    }
}

auto Chat::printMessage(int message_index) const -> void
{

    auto message{_message_array[message_index]};

    const tm& timeinfo{message->getMessageCreationTime()};

    std::cout << BOLDCYAN << std::setw(120) << std::setfill('-') << "-" << std::endl;
    std::cout << BOLDGREEN << std::setw(5) << std::setfill(' ') << std::right << message_index + 1 << "."
              << RESET;  // array's indices begin from 0, Output indices begin from 1
    std::cout << YELLOW << "  Created: ";
    std::cout << BOLDYELLOW << std::setw(MAX_INPUT_SIZE) << std::setfill(' ') << std::left << message->getUser()->getUserName();
    std::cout << std::setw(20) << std::setfill(' ') << RESET << YELLOW;

    Utils::printTimeAndData(timeinfo);

    std::cout << CYAN << std::setw(120) << std::setfill('-') << "-" << std::endl;
    std::cout << BOLDYELLOW << message->getMessage() << RESET << std::endl;

    if (message->isEdited())
    {
        const tm& edit_timeinfo{message->getMessageEditingTime()};
        std::cout << CYAN << std::setw(120) << std::setfill('-') << "-" << std::endl;
        std::cout << YELLOW << "Edited: ";
        Utils::printTimeAndData(edit_timeinfo);
    }
    std::cout << BOLDCYAN << std::setw(120) << std::setfill('-') << "-" << RESET << std::endl;
}

auto Chat::addMessage(const std::shared_ptr<User>& user) -> const std::shared_ptr<Message>
{
    try
    {
        std::string new_message{};

        std::cout << std::endl << YELLOW << "Input message: " << BOLDGREEN;
        std::getline(std::cin, new_message);
        std::cout << RESET;
        std::cout << BOLDYELLOW << "Send message?(Y/N):" << BOLDGREEN;
        if (!Utils::isOKSelect()) return std::make_shared<Message>();
        std::cout << RESET;

        time_t seconds{time(NULL)};
        tm timeinfo;
#if defined(_WIN32)
        localtime_s(&timeinfo, &seconds);
#elif defined(__linux__)
        localtime_r(&seconds, &timeinfo);
#endif

        _message_array.push_back(std::make_shared<Message>(new_message, user, timeinfo));
        return _message_array[_message_array.size() - 1];
    }
    catch (std::exception& e)
    {
        std::cout << BOLDRED << "Exception: " << e.what() << RESET << std::endl;
    }
    return std::make_shared<Message>();
}

auto Chat::deleteMessage(const std::shared_ptr<User>& user, int message_index) -> const std::shared_ptr<Message>
{
    if (message_index < 0 || message_index >= static_cast<int>(_message_array.size())) return std::make_shared<Message>();
    try
    {
        if (user != _message_array.at(message_index)->getUser()) return std::make_shared<Message>();

        printMessage(message_index);

        std::cout << BOLDYELLOW << "Delete message?(Y/N):" << BOLDGREEN;
        if (!Utils::isOKSelect()) return std::make_shared<Message>();
        std::cout << RESET;

        auto it = _message_array.begin();
        auto message{_message_array[message_index]};
        _message_array.erase(it + message_index);

        return message;
    }
    catch (std::exception& e)
    {
        std::cout << BOLDRED << "Exception: " << e.what() << RESET << std::endl;
    }
    return std::make_shared<Message>();
}

auto Chat::editMessage(const std::shared_ptr<User>& user, int message_index) -> const std::shared_ptr<Message>
{
    if (message_index < 0 || message_index >= static_cast<int>(_message_array.size())) return std::make_shared<Message>();
    try
    {
        if (user != _message_array.at(message_index)->getUser()) return std::make_shared<Message>();

        printMessage(message_index);

        std::string new_message{};

        std::cout << YELLOW << "Input new message: " << BOLDGREEN;
        std::getline(std::cin, new_message);
        std::cout << RESET;

        std::cout << BOLDYELLOW << "Save changes?(Y/N):" << BOLDGREEN;
        if (!Utils::isOKSelect()) return std::make_shared<Message>();
        std::cout << RESET;

        time_t seconds{time(NULL)};
        tm timeinfo;
#if defined(_WIN32)
        localtime_s(&timeinfo, &seconds);  // TODO
#elif defined(__linux__)
        localtime_r(&seconds, &timeinfo);
#endif
        _message_array[message_index]->editedMessage(new_message, timeinfo);

        return _message_array[message_index];
    }
    catch (std::exception& e)
    {
        std::cout << BOLDRED << "Exception: " << e.what() << RESET << std::endl;
    }
    return std::make_shared<Message>();
}

auto Chat::getMessageIndex(const std::shared_ptr<Message>& message) -> int
{
    for (auto i{0u}; i < _message_array.size(); ++i)
    {
        if (_message_array[i].get() == message.get()) return i;
    }
    return -1;  // bad index
}

auto Chat::save(File& file) -> void
{
    int first_userID{-1};
    if (_first_user) first_userID = _first_user.get()->getUserID();
    file.write(first_userID);

    int second_userID{-1};
    if (_first_user) second_userID = _second_user.get()->getUserID();
    file.write(second_userID);

    auto message_number{_message_array.size()};

    file.write(message_number);

    for (auto i{0u}; i < message_number; ++i)
    {
        auto message{_message_array[i]->getMessage()};
        auto msg_size{message.size()};
        file.write(msg_size);
        message += '\n';
        file.write(message);
        file.write(_message_array[i]->getUser()->getUserID());
        file.write(_message_array[i]->getMessageCreationTime());
        auto flag = _message_array[i]->isEdited() ? 1 : 0;
        file.write(flag);
        if (flag) file.write(_message_array[i]->getMessageEditingTime());
    }
}

auto Chat::load(File& file, const std::vector<std::shared_ptr<User>>& user) -> void
{

    size_t message_number{0};
    file.read(message_number);
    for (auto i{0u}; i < message_number; ++i)
    {
        auto msg_size{0};
        file.read(msg_size);
        char* str = new char[msg_size + 2]{};               //+1 for '\0' , +1 for separator
        file.getStream().getline(str, msg_size + 2, '\n');  //
        std::string message(str + 1);                       // skip the separator

        delete[] str;

        auto userID{0};
        file.read(userID);

        tm creation_time{};
        file.read(creation_time);

        _message_array.push_back(std::make_shared<Message>(message, user[userID], creation_time));

        int flag_edit{0};
        file.read(flag_edit);

        if (!flag_edit) continue;

        _message_array[i]->setEdited(true);

        tm edited_time{};
        file.read(edited_time);

        _message_array[i]->setMessageEditingTime(edited_time);
    }
}
