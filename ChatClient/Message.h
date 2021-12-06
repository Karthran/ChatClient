#pragma once
#include <iostream>
#include <string>

class User;

class Message
{
public:
    Message() { /*std::cout << "Message constr " << this << std::endl; */};
    Message(const std::string& message, const std::shared_ptr<User>& user, const tm& message_creation_time)
        : _message(message), _user(user), _message_creation_time(message_creation_time), _is_message_initialized(true)
    {
        /*std::cout << "Message constr " << this << std::endl; */
    }
    ~Message() { /*std::cout << "Message destr " << this << std::endl;*/ }
    auto setMessage(const std::string& message) -> void { _message = message; }
    auto setUser(const std::shared_ptr<User>& user) -> void { _user = user; }
    auto setMessageCreationTime(const tm& time) -> void { _message_creation_time = time; }
    auto setMessageEditingTime(const tm& time) -> void { _message_editing_time = time; }
    auto setEdited(bool edit) -> void { _is_edited = edit; }

    auto getMessage() const -> const std::string& { return _message; }
    auto getUser() const -> const std::shared_ptr<User>& { return _user; }
    auto getMessageCreationTime() const -> const tm& { return _message_creation_time; }
    auto getMessageEditingTime() const -> const tm& { return _message_editing_time; }
    auto isEdited() const -> bool { return _is_edited; }

    auto editedMessage(const std::string& new_message, const tm& message_editing_time) -> void
    {
        _message = new_message;
        _message_editing_time = message_editing_time;
        _is_edited = true;
    }

    auto isInitialized() -> bool { return _is_message_initialized; }

private:
    std::string _message{};
    std::shared_ptr<User> _user{nullptr};
    tm _message_creation_time{};
    bool _is_edited{false};
    tm _message_editing_time{};
    bool _is_message_initialized{false};
};


