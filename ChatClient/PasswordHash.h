#pragma once
#include <iostream>
#include <string>

typedef unsigned int uint;
struct Hash
{
    uint _A{0};
    uint _B{0};
    uint _C{0};
    uint _D{0};
    uint _E{0};

    friend auto operator==(const Hash& hash1, const Hash& hash2) -> bool
    {
        if ((hash1._A != hash2._A) || (hash1._B != hash2._B) || (hash1._C != hash2._C) || (hash1._D != hash2._D) || (hash1._E != hash2._E))
            return false;
        return true;
    }
};

class PasswordHash
{
public:
    PasswordHash() { /*std::cout << "PasswordHash constr " << this << std::endl;*/ }
    PasswordHash(Hash hash, std::string salt) : _hash(hash), _salt(salt) { /*std::cout << "PasswordHash constr " << this << std::endl;*/ }
    ~PasswordHash() { /*std::cout << "PasswordHash destr " << this << std::endl;*/ }
    auto setSalt(const std::string& salt) -> void { _salt = salt; }
    auto setHash(const Hash& hash) -> void { _hash = hash; }

    auto getSalt() const -> const std::string& { return _salt; }
    auto getHash() const -> const Hash& { return _hash; }

private:
    Hash _hash;
    std::string _salt{};
};
