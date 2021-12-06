#include <string.h>
#include <memory>
#include <cstdlib>
#include <ctime>
#include "SHA1.h"
#include "PasswordHash.h"

auto cycle_shift_left(uint val, int bit_count) -> uint
{
    return (val << bit_count | val >> (32 - bit_count));
}

auto bring_to_human_view(uint val)->uint
{
    return ((val & 0x000000FF) << 24) | ((val & 0x0000FF00) << 8) | ((val & 0x00FF0000) >> 8) | ((val & 0xFF000000) >> 24);
}

auto sha1(const std::string& message, const std::string& salt) -> std::shared_ptr<PasswordHash>
{
    std::string password_salt = message + salt;

    uint msize_bytes{static_cast<uint>(password_salt.size())};

    //инициализация
    uint A = H[0];
    uint B = H[1];
    uint C = H[2];
    uint D = H[3];
    uint E = H[4];

    // подсчет целого числа блоков
    uint totalBlockCount = msize_bytes / one_block_size_bytes;

    // подсчет, сколько байт нужно, чтобы дополнить последний блок
    uint needAdditionalBytes = one_block_size_bytes - (msize_bytes - totalBlockCount * one_block_size_bytes);

    if (needAdditionalBytes < 8)
    {
        totalBlockCount += 2;
        needAdditionalBytes += one_block_size_bytes;
    }
    else
    {
        totalBlockCount += 1;
    }

    // размер дополненного по всем правилам сообщения
    uint extendedMessageSize = msize_bytes + needAdditionalBytes;

    // выделяем новый буфер и копируем в него исходный
    unsigned char* newMessage = new unsigned char[extendedMessageSize];
    memcpy(newMessage, password_salt.c_str(), msize_bytes);

    // первый бит ставим '1', остальные обнуляем
    newMessage[msize_bytes] = 0x80;
    memset(newMessage + msize_bytes + 1, 0, needAdditionalBytes - 1);

    // задаем длину исходного сообщения в битах
    uint* ptr_to_size = (uint*)(newMessage + extendedMessageSize - 4);
    *ptr_to_size = bring_to_human_view(msize_bytes * 8);

    ExpendBlock exp_block;
    //раунды поехали
    for (uint i = 0; i < totalBlockCount; i++)
    {

        // берем текущий блок и дополняем его
        unsigned char* cur_p = newMessage + one_block_size_bytes * i;
        Block block = (Block)cur_p;

        // первые 16 4байтовых чисел
        for (int j = 0; j < one_block_size_uints; j++)
        {
            exp_block[j] = bring_to_human_view(block[j]);
        }
        // следующие 64...
        for (int j = one_block_size_uints; j < block_expend_size_uints; j++)
        {
            exp_block[j] = exp_block[j - 3] ^ exp_block[j - 8] ^ exp_block[j - 14] ^ exp_block[j - 16];
            exp_block[j] = cycle_shift_left(exp_block[j], 1);
        }

        // инициализация
        uint a = H[0];
        uint b = H[1];
        uint c = H[2];
        uint d = H[3];
        uint e = H[4];

        // пересчитываем
        for (int j = 0; j < block_expend_size_uints; j++)
        {
            uint f;
            uint k;
            // в зависимости от раунда считаем по-разному
            if (j < 20)
            {
                f = (b & c) | ((~b) & d);
                k = 0x5A827999;
            }
            else if (j < 40)
            {
                f = b ^ c ^ d;
                k = 0x6ED9EBA1;
            }
            else if (j < 60)
            {
                f = (b & c) | (b & d) | (c & d);
                k = 0x8F1BBCDC;
            }
            else
            {
                f = b ^ c ^ d;
                k = 0xCA62C1D6;
            }

            // перемешивание
            uint temp = cycle_shift_left(a, 5) + f + e + k + exp_block[j];
            e = d;
            d = c;
            c = cycle_shift_left(b, 30);
            b = a;
            a = temp;
        }
        // пересчитываем
        A = A + a;
        B = B + b;
        C = C + c;
        D = D + d;
        E = E + e;
    }

    // A,B,C,D,E являются выходными 32-х битными составляющими посчитанного хэша
    std::shared_ptr<PasswordHash> ph = std::make_shared<PasswordHash>();
    Hash hash; 
    hash._A = A;
    hash._B = B;
    hash._C = C;
    hash._D = D;
    hash._E = E;

    ph->setHash(hash);
    ph->setSalt(salt);

    // чистим за собой
    delete[] newMessage;
    return ph;
}

auto getSalt() -> const std::string
{
    std::string salt{};
    srand(static_cast<uint>(time(0)));
    auto SaltArrayLength{sizeof(alphanum) - 1}; // -1 for last '\0'
    for (auto i{0}; i < SALTLENGTH; ++i)
    {
        salt.push_back(alphanum[rand() % SaltArrayLength]);
    }
    return salt;
}
