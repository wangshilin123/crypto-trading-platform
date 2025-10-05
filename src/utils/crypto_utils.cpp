#include "utils/crypto_utils.h"
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <openssl/md5.h>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace crypto_trading {
namespace utils {

// ========== CryptoUtils ==========

std::string CryptoUtils::hmacSha256(const std::string& data, const std::string& key)
{
    auto raw = hmacSha256Raw(data, key);
    return toHex(raw);
}

std::vector<uint8_t> CryptoUtils::hmacSha256Raw(const std::string& data, const std::string& key)
{
    std::vector<uint8_t> result(EVP_MAX_MD_SIZE);
    unsigned int len = 0;

    HMAC(
        EVP_sha256(),
        key.c_str(),
        static_cast<int>(key.length()),
        reinterpret_cast<const unsigned char*>(data.c_str()),
        data.length(),
        result.data(),
        &len
    );

    result.resize(len);
    return result;
}

std::string CryptoUtils::sha256(const std::string& data)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, data.c_str(), data.length());
    SHA256_Final(hash, &sha256);

    std::vector<uint8_t> result(hash, hash + SHA256_DIGEST_LENGTH);
    return toHex(result);
}

std::string CryptoUtils::md5(const std::string& data)
{
    unsigned char hash[MD5_DIGEST_LENGTH];
    MD5_CTX md5;
    MD5_Init(&md5);
    MD5_Update(&md5, data.c_str(), data.length());
    MD5_Final(hash, &md5);

    std::vector<uint8_t> result(hash, hash + MD5_DIGEST_LENGTH);
    return toHex(result);
}

std::string CryptoUtils::base64Encode(const std::vector<uint8_t>& data)
{
    static const char base64_chars[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    std::string result;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    for (auto byte : data)
    {
        char_array_3[i++] = byte;
        if (i == 3)
        {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; i < 4; i++)
                result += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i)
    {
        for (j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

        for (j = 0; j < i + 1; j++)
            result += base64_chars[char_array_4[j]];

        while (i++ < 3)
            result += '=';
    }

    return result;
}

std::string CryptoUtils::base64Encode(const std::string& data)
{
    std::vector<uint8_t> bytes(data.begin(), data.end());
    return base64Encode(bytes);
}

std::vector<uint8_t> CryptoUtils::base64Decode(const std::string& encoded)
{
    static const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    std::vector<uint8_t> result;
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];

    for (char c : encoded)
    {
        if (c == '=') break;
        if (!isalnum(c) && c != '+' && c != '/') continue;

        char_array_4[i++] = c;
        if (i == 4)
        {
            for (i = 0; i < 4; i++)
                char_array_4[i] = base64_chars.find(char_array_4[i]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; i < 3; i++)
                result.push_back(char_array_3[i]);
            i = 0;
        }
    }

    if (i)
    {
        for (j = i; j < 4; j++)
            char_array_4[j] = 0;

        for (j = 0; j < 4; j++)
            char_array_4[j] = base64_chars.find(char_array_4[j]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);

        for (j = 0; j < i - 1; j++)
            result.push_back(char_array_3[j]);
    }

    return result;
}

std::string CryptoUtils::urlEncode(const std::string& str)
{
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (char c : str)
    {
        // 保留字符不编码
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
        {
            escaped << c;
        }
        else
        {
            escaped << std::uppercase;
            escaped << '%' << std::setw(2) << int(static_cast<unsigned char>(c));
            escaped << std::nouppercase;
        }
    }

    return escaped.str();
}

std::string CryptoUtils::urlDecode(const std::string& str)
{
    std::string result;
    result.reserve(str.length());

    for (size_t i = 0; i < str.length(); ++i)
    {
        if (str[i] == '%')
        {
            if (i + 2 < str.length())
            {
                int value;
                std::istringstream is(str.substr(i + 1, 2));
                if (is >> std::hex >> value)
                {
                    result += static_cast<char>(value);
                    i += 2;
                }
                else
                {
                    result += str[i];
                }
            }
            else
            {
                result += str[i];
            }
        }
        else if (str[i] == '+')
        {
            result += ' ';
        }
        else
        {
            result += str[i];
        }
    }

    return result;
}

std::string CryptoUtils::toHex(const std::vector<uint8_t>& data)
{
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (uint8_t byte : data)
    {
        oss << std::setw(2) << static_cast<int>(byte);
    }
    return oss.str();
}

std::vector<uint8_t> CryptoUtils::fromHex(const std::string& hex)
{
    std::vector<uint8_t> result;
    result.reserve(hex.length() / 2);

    for (size_t i = 0; i < hex.length(); i += 2)
    {
        std::string byte_str = hex.substr(i, 2);
        uint8_t byte = static_cast<uint8_t>(std::strtol(byte_str.c_str(), nullptr, 16));
        result.push_back(byte);
    }

    return result;
}

// ========== StringUtils ==========

std::string StringUtils::trim(const std::string& str)
{
    auto start = str.begin();
    while (start != str.end() && std::isspace(*start))
    {
        start++;
    }

    auto end = str.end();
    do
    {
        end--;
    } while (std::distance(start, end) > 0 && std::isspace(*end));

    return std::string(start, end + 1);
}

std::vector<std::string> StringUtils::split(const std::string& str, char delimiter)
{
    std::vector<std::string> result;
    std::istringstream stream(str);
    std::string token;

    while (std::getline(stream, token, delimiter))
    {
        result.push_back(token);
    }

    return result;
}

std::string StringUtils::join(const std::vector<std::string>& parts, const std::string& delimiter)
{
    if (parts.empty())
    {
        return "";
    }

    std::ostringstream oss;
    oss << parts[0];
    for (size_t i = 1; i < parts.size(); ++i)
    {
        oss << delimiter << parts[i];
    }

    return oss.str();
}

std::string StringUtils::toUpper(const std::string& str)
{
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

std::string StringUtils::toLower(const std::string& str)
{
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

std::string StringUtils::replace(const std::string& str, const std::string& from, const std::string& to)
{
    std::string result = str;
    size_t start_pos = 0;

    while ((start_pos = result.find(from, start_pos)) != std::string::npos)
    {
        result.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }

    return result;
}

bool StringUtils::startsWith(const std::string& str, const std::string& prefix)
{
    return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
}

bool StringUtils::endsWith(const std::string& str, const std::string& suffix)
{
    return str.size() >= suffix.size() &&
           str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

} // namespace utils
} // namespace crypto_trading
