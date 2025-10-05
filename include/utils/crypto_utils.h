#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace crypto_trading {
namespace utils {

/**
 * @brief 加密工具类
 *
 * @details 提供HMAC签名、Base64编码等功能
 */
class CryptoUtils
{
public:
    /**
     * @brief HMAC-SHA256签名
     * @param data 待签名数据
     * @param key 密钥
     * @return 十六进制签名字符串
     */
    static std::string hmacSha256(const std::string& data, const std::string& key);

    /**
     * @brief HMAC-SHA256签名（返回二进制）
     * @param data 待签名数据
     * @param key 密钥
     * @return 二进制签名数据
     */
    static std::vector<uint8_t> hmacSha256Raw(const std::string& data, const std::string& key);

    /**
     * @brief SHA256哈希
     * @param data 待哈希数据
     * @return 十六进制哈希字符串
     */
    static std::string sha256(const std::string& data);

    /**
     * @brief MD5哈希
     * @param data 待哈希数据
     * @return 十六进制哈希字符串
     */
    static std::string md5(const std::string& data);

    /**
     * @brief Base64编码
     * @param data 原始数据
     * @return Base64字符串
     */
    static std::string base64Encode(const std::vector<uint8_t>& data);

    /**
     * @brief Base64编码
     * @param data 原始字符串
     * @return Base64字符串
     */
    static std::string base64Encode(const std::string& data);

    /**
     * @brief Base64解码
     * @param encoded Base64字符串
     * @return 原始数据
     */
    static std::vector<uint8_t> base64Decode(const std::string& encoded);

    /**
     * @brief URL编码
     * @param str 原始字符串
     * @return URL编码字符串
     */
    static std::string urlEncode(const std::string& str);

    /**
     * @brief URL解码
     * @param str URL编码字符串
     * @return 原始字符串
     */
    static std::string urlDecode(const std::string& str);

    /**
     * @brief 二进制转十六进制字符串
     * @param data 二进制数据
     * @return 十六进制字符串
     */
    static std::string toHex(const std::vector<uint8_t>& data);

    /**
     * @brief 十六进制字符串转二进制
     * @param hex 十六进制字符串
     * @return 二进制数据
     */
    static std::vector<uint8_t> fromHex(const std::string& hex);
};

/**
 * @brief 字符串工具类
 */
class StringUtils
{
public:
    /**
     * @brief 去除字符串两端空白
     * @param str 原始字符串
     * @return 处理后的字符串
     */
    static std::string trim(const std::string& str);

    /**
     * @brief 字符串分割
     * @param str 原始字符串
     * @param delimiter 分隔符
     * @return 分割后的字符串数组
     */
    static std::vector<std::string> split(const std::string& str, char delimiter);

    /**
     * @brief 字符串拼接
     * @param parts 字符串数组
     * @param delimiter 分隔符
     * @return 拼接后的字符串
     */
    static std::string join(const std::vector<std::string>& parts, const std::string& delimiter);

    /**
     * @brief 转大写
     * @param str 原始字符串
     * @return 大写字符串
     */
    static std::string toUpper(const std::string& str);

    /**
     * @brief 转小写
     * @param str 原始字符串
     * @return 小写字符串
     */
    static std::string toLower(const std::string& str);

    /**
     * @brief 字符串替换
     * @param str 原始字符串
     * @param from 被替换的子串
     * @param to 替换成的子串
     * @return 替换后的字符串
     */
    static std::string replace(const std::string& str, const std::string& from, const std::string& to);

    /**
     * @brief 判断是否以指定前缀开头
     * @param str 原始字符串
     * @param prefix 前缀
     * @return 是否匹配
     */
    static bool startsWith(const std::string& str, const std::string& prefix);

    /**
     * @brief 判断是否以指定后缀结尾
     * @param str 原始字符串
     * @param suffix 后缀
     * @return 是否匹配
     */
    static bool endsWith(const std::string& str, const std::string& suffix);

    /**
     * @brief 格式化字符串（类似sprintf）
     * @tparam Args 参数类型
     * @param format 格式字符串
     * @param args 参数
     * @return 格式化后的字符串
     */
    template<typename... Args>
    static std::string format(const std::string& format, Args... args);
};

// 模板实现
template<typename... Args>
std::string StringUtils::format(const std::string& format, Args... args)
{
    size_t size = std::snprintf(nullptr, 0, format.c_str(), args...) + 1;
    std::vector<char> buf(size);
    std::snprintf(buf.data(), size, format.c_str(), args...);
    return std::string(buf.data(), buf.data() + size - 1);
}

} // namespace utils
} // namespace crypto_trading
