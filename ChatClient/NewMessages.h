#pragma once
#include <iostream>
#include <map>
#include <vector>

class Message;

const int BAD_INDEX = -1;
class NewMessages
{
public:
    NewMessages() { /*std::cout << "NewMessages constr " << this << std::endl;*/ }
    ~NewMessages() { /*std::cout << "NewMessages destr " << this << std::endl;*/ }

    auto isNewMessagesExists() const -> bool { return isNewMsgExists; }
    auto addNewMessage(const std::shared_ptr<Message>& message) -> void;
    auto removeNewMessage(const std::shared_ptr<Message>& message) -> void;
    auto removeAllMessages(int userID) -> void;
    auto usersNumber() const -> size_t { return _userIDarray.size(); }
    auto getUserID(int index) -> int { return _userIDarray[index]; }
    auto newMessageNumber(int userID) -> size_t { return _newMessagesMap[userID].size(); }
    auto getMessages(int userID) -> const std::vector<std::shared_ptr<Message>>& { return _newMessagesMap[userID]; }
    auto isUserIDExists(int userID) const -> int;
    auto setLastVisitTime(const tm& tm) -> void;
    auto getLastVisitTime() -> const tm& { return _lastVisitTime; }

private:
    //std::map<int, int> _userIDarray{}; // key - index ; value userID
    std::vector<int> _userIDarray{};
    std::map<int, std::vector<std::shared_ptr<Message>>> _newMessagesMap{}; // key userID; value Message
    bool isNewMsgExists{false};
    tm _lastVisitTime{};

    auto eraseUserID(int userID) -> void;
};

