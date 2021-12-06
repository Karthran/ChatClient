#pragma once
#include <string>
#include <memory>

class PasswordHash;

typedef unsigned int uint;

const uint one_block_size_bytes = 64;     // количество байб в блоке
const uint one_block_size_uints = 16;     // количество 4байтовых  в блоке
const uint block_expend_size_uints = 80;  // количество 4байтовых в дополненном блоке

const uint SHA1HASHLENGTHBYTES = 20;
const uint SHA1HASHLENGTHUINTS = 5;
const uint SALTLENGTH = SHA1HASHLENGTHBYTES;

typedef uint* Block;
typedef uint ExpendBlock[block_expend_size_uints];

const char alphanum[] = "0123456789"
                        "!@#$%^&*"
                        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                        "abcdefghijklmnopqrstuvwxyz";

const uint H[5] = {0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0};  // константы, согласно стандарту

auto cycle_shift_left(uint val, int bit_count) -> uint;
auto bring_to_human_view(uint val) -> uint;


auto sha1(const std::string& message, const std::string& salt) -> std::shared_ptr<PasswordHash>;  
auto getSalt() -> const std::string;
