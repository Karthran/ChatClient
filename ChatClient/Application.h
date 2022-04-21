#pragma once
#include <string>
#include <memory>
#include <vector>
#include <condition_variable>
#include <mutex>

class Client;
enum class OperationCode;

class Application
{
public:
    Application();

    void run();
    auto sendToServer(const char* message, size_t message_length, OperationCode operation_code) -> const char*;

private:
    std::shared_ptr <char[]> _msg_buffer{};
    size_t _msg_buffer_size{0};
    size_t _current_msg_length{0};

    std::string _self_path{};
    std::string _user_id{"0"};
    std::string _private_chat_id{"0"};
    mutable std::mutex in_mutex;
    mutable std::mutex out_mutex;

    auto talkToServer(const char* message, size_t msg_length) const -> char*;

    auto addToBuffer(char* buffer, size_t& cur_msg_len, int value) const -> void;
    auto addToBuffer(char* buffer, size_t& cur_msg_len, const char* string, size_t str_len) const -> void;

    auto getFromBuffer(const char* buffer, size_t shift, int& value) const -> void;
    auto getFromBuffer(const char* buffer, size_t shift, char* string, size_t str_len) const -> void;

    auto createAccount() -> void;
    auto createAccount_inputName(std::string& name) -> void;
    auto createAccount_inputSurname(std::string& surname) -> void;
    auto createAccount_inputEmail(std::string& email) -> void;
    auto createAccount_inputLogin(std::string& login) -> void;
    auto createAccount_inputPassword(std::string& password) const -> void;

    auto signIn() -> void;
    auto signIn_inputLogin(std::string& user_login) const -> void;
    auto signIn_inputPassword(std::string& user_password) const -> void;

    auto selectCommonOrPrivate() -> void;
    auto addNewMsgInCommonChatToMenuItem(std::string& menu_item) -> void;
    auto addNewMsgInPrivateChatToMenuItem(std::string& menu_item) -> void;

    auto commonChat() -> int;
    auto commonChat_viewMessage() -> void;
    auto commonChat_addMessage() -> void;
    auto commonChat_editMessage() -> void;
    auto commonChat_deleteMessage() -> void;

    auto privateMenu() -> void;
    auto privateMenu_viewUsersNames() -> void;
    auto privateMenu_viewUsersExistsChat() -> void;
    auto privateMenu_selectByID() -> void;
    auto printUserIDNameSurnameWithNewMessages() -> void;

    auto privateChat() -> void;
    auto privateChat_viewMessage() -> void;
    auto privateChat_addMessage() -> void;
    auto privateChat_editMessage() -> void;
    auto privateChat_deleteMessage() -> void;

    /* string_arr{0] is Menu Name , printed with underline and without number*/
    auto menu(std::string* string_arr, int size) const -> int;
    auto printMessages(const char*& data_ptr, int messages_num, int columns_num, bool is_new = false, bool use_status = false) const
        -> void;
    auto printMessage(const std::vector<std::string>& message, bool is_new, bool use_status) const -> void;
    auto editMessage(std::string& edited_message) -> void;
};
