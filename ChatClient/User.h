#pragma once
#include <string>
#include <iostream>
class User
{
public:
    User() { /*std::cout << "User constr " << this << std::endl;*/ }
    User(const std::string& user_name, const std::string& user_login, int user_ID)
        : _user_name(user_name), _user_login(user_login), _user_ID(user_ID)
    {
       /* std::cout << "User constr " << this << std::endl;*/
    }

    ~User() {/* std::cout << "User destr " << this << std::endl;*/ }

    auto setUserName(const std::string& name) -> void { _user_name = name; }
    auto setUserLogin(const std::string& login) -> void { _user_login = login; }
    auto setUserID(int id) -> void { _user_ID = id; }

    auto getUserName() const -> const std::string& { return _user_name; }
    auto getUserLogin() const -> const std::string& { return _user_login; }
    auto getUserID() const -> int { return _user_ID; }

private:
    std::string _user_name{};
    std::string _user_login{};
    int _user_ID{};
};
