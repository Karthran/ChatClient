#include <memory>
#include "NewMessages.h"
#include "Message.h"
#include "User.h"

auto NewMessages::addNewMessage(const std::shared_ptr<Message>& message) -> void
{
    auto userID{message->getUser()->getUserID()};
    auto it = _userIDarray.begin();
    for (; it != _userIDarray.end(); ++it)
    {
        if (*it == userID) break;
    }

    if (it == _userIDarray.end()) _userIDarray.push_back(userID);

    for (auto it = _newMessagesMap[userID].begin(); it != _newMessagesMap[userID].end(); ++it)
    {
        if (it->get() == message.get()) return; // message already exists 
    }

    _newMessagesMap[userID].push_back(message);
    isNewMsgExists = true;
}

auto NewMessages::removeNewMessage(const std::shared_ptr<Message>& message) -> void
{
    auto userID{message->getUser()->getUserID()};

    for (auto it = _newMessagesMap[userID].begin(); it != _newMessagesMap[userID].end();)
    {
        if (it->get() == message.get())
        {
            it = _newMessagesMap[userID].erase(it);
            break;
        }
        else
            ++it;
     }
    auto msg_numbers = _newMessagesMap[userID].size();
    if (!msg_numbers)
    {
        _newMessagesMap.erase(userID);
        eraseUserID(userID);
    }
    if (!_newMessagesMap.size()) isNewMsgExists = false;
}

auto NewMessages::removeAllMessages(int userID) -> void
{
    for (auto it = _newMessagesMap[userID].begin(); it != _newMessagesMap[userID].end();)
    {
            it = _newMessagesMap[userID].erase(it);
    }
    _newMessagesMap.erase(userID);
    eraseUserID(userID);
    if (!_newMessagesMap.size()) isNewMsgExists = false;
}

inline auto NewMessages::eraseUserID(int userID) -> void
{
    auto it = _userIDarray.begin();
    for (auto it = _userIDarray.begin(); it != _userIDarray.end();)
    {
        if (*it == userID)
        {
            _userIDarray.erase(it);
            break;
        }
        else
            ++it;
    }
}

inline auto NewMessages::isUserIDExists(int userID) const -> int
{
    for (auto it = _userIDarray.begin(); it != _userIDarray.end(); ++it)
    {
        if (*it != userID) continue;
        return static_cast<int>(it - _userIDarray.begin());
    }
    return BAD_INDEX;
}

inline auto NewMessages::setLastVisitTime(const tm& tm) -> void
{
    _lastVisitTime = tm;
    //_lastVisitTime.tm_sec = tm.tm_sec;
    //_lastVisitTime.tm_min = tm.tm_min;
    //_lastVisitTime.tm_hour = tm.tm_hour;
    //_lastVisitTime.tm_mday = tm.tm_mday;
    //_lastVisitTime.tm_mon = tm.tm_mon;
    //_lastVisitTime.tm_year = tm.tm_year;
    //_lastVisitTime.tm_wday = tm.tm_wday;
    //_lastVisitTime.tm_yday = tm.tm_yday;
    //_lastVisitTime.tm_isdst = tm.tm_isdst;
}
