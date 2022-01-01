#pragma once
#include <string>
#include <memory>
#include <vector>
#include <map>
#include <unordered_map>

class Client;
enum class OperationCode;

//class Chat;
//class User;
//class PasswordHash;
//class NewMessages;

class Application
{
public:
    Application();
    ~Application();

    void run();
    auto sendToServer(const char* message, size_t message_length, OperationCode operation_code) -> const char*;

private:
    Client* _client{nullptr};
    char* _msg_buffer{nullptr};
    size_t _msg_buffer_size{0};
    size_t _current_msg_length{0};

    std::string _self_path{};
    int _user_id{-1};

    auto talkToServer(const char* message, size_t msg_length) const -> char*;

    auto addToBuffer(char* buffer, size_t& cur_msg_len, int value) const -> void;
    auto addToBuffer(char* buffer, size_t& cur_msg_len, const char* string, size_t str_len) const -> void;

    auto getFromBuffer(const char* buffer, size_t shift, int& value) const -> void;
    auto getFromBuffer(const char* buffer, size_t shift, char* string, size_t str_len) const -> void;

    auto createAccount() -> void;
    auto createAccount_inputName(std::string& name) -> void;
    auto createAccount_inputLogin(std::string& login) const -> void;
    auto createAccount_inputPassword(std::string& password) const -> void;

    auto signIn() -> void;
    auto signIn_inputLogin(std::string& user_login) const -> void;
    auto signIn_inputPassword(std::string& user_password) const -> void;

    auto selectCommonOrPrivate() -> void;

    //auto commonChat(const std::shared_ptr<User>& user) const -> int;
    //auto commonChat_addMessage() const -> void;
    //auto commonChat_editMessage(const std::shared_ptr<User>& user) const -> void;
    //auto commonChat_deleteMessage(const std::shared_ptr<User>& user) const -> void;

    //auto privateMenu(const std::shared_ptr<User>& user) -> int;
    //auto privateMenu_viewUsersNames() const -> void;
    //auto privateMenu_selectByName(const std::shared_ptr<User>& user) const -> int;
    //auto privateMenu_selectByID(const std::shared_ptr<User>& user) -> void;
    //auto printNewMessagesUsers(const std::shared_ptr<User>& user) -> void;

    //auto privateChat(const std::shared_ptr<User>& source_user, const std::shared_ptr<User>& target_user) -> int;

    //auto privateChat_addMessage(
    //    const std::shared_ptr<User>& source_user, const std::shared_ptr<User>& target_user, std::shared_ptr<Chat>& chat) -> void;

    //auto privateChat_editMessage(const std::shared_ptr<User>& source_user, const std::shared_ptr<User>& target_user,
    //    const std::shared_ptr<Chat>& chat) const -> void;

    //auto privateChat_deleteMessage(const std::shared_ptr<User>& source_user, const std::shared_ptr<User>& target_user,
    //    const std::shared_ptr<Chat>& chat) const -> void;

    /*Finds chat in array, return empty shared_ptr if chat don't exist */
    //auto getPrivateChat(const std::shared_ptr<User>& source_user, const std::shared_ptr<User>& target_user) const
    //    -> const std::shared_ptr<Chat>;

    /*Searches for matching line*/
    //auto checkingForStringExistence(const std::string& string, const std::string& (User::*get)() const) const -> int;

    /* string_arr{0] is Menu Name , printed with underline and without number*/
    auto menu(std::string* string_arr, int size) const -> int;
};
