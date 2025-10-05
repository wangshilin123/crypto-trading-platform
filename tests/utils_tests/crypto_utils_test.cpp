#include <gtest/gtest.h>
#include "utils/crypto_utils.h"

using namespace crypto_trading::utils;

TEST(CryptoUtilsTest, HmacSha256)
{
    std::string data = "Hello World";
    std::string key = "secret_key";

    std::string signature = CryptoUtils::hmacSha256(data, key);

    // HMAC-SHA256应该返回64个十六进制字符（32字节）
    EXPECT_EQ(signature.length(), 64);

    // 相同输入应该产生相同签名
    std::string signature2 = CryptoUtils::hmacSha256(data, key);
    EXPECT_EQ(signature, signature2);

    // 不同数据应该产生不同签名
    std::string signature3 = CryptoUtils::hmacSha256("Different data", key);
    EXPECT_NE(signature, signature3);
}

TEST(CryptoUtilsTest, HmacSha256Binance)
{
    // 币安API签名测试用例
    std::string query = "symbol=BTCUSDT&side=BUY&type=LIMIT&quantity=1&price=9000&timestamp=1578963600000";
    std::string api_secret = "test_secret";

    std::string signature = CryptoUtils::hmacSha256(query, api_secret);
    EXPECT_FALSE(signature.empty());
    EXPECT_EQ(signature.length(), 64);
}

TEST(CryptoUtilsTest, Sha256)
{
    std::string data = "test data";
    std::string hash = CryptoUtils::sha256(data);

    EXPECT_EQ(hash.length(), 64);  // SHA256产生64个十六进制字符

    // 相同输入产生相同哈希
    EXPECT_EQ(hash, CryptoUtils::sha256(data));

    // 不同输入产生不同哈希
    EXPECT_NE(hash, CryptoUtils::sha256("different data"));
}

TEST(CryptoUtilsTest, MD5)
{
    std::string data = "test data";
    std::string hash = CryptoUtils::md5(data);

    EXPECT_EQ(hash.length(), 32);  // MD5产生32个十六进制字符
}

TEST(CryptoUtilsTest, Base64Encode)
{
    std::string data = "Hello World!";
    std::string encoded = CryptoUtils::base64Encode(data);

    EXPECT_EQ(encoded, "SGVsbG8gV29ybGQh");
}

TEST(CryptoUtilsTest, Base64Decode)
{
    std::string encoded = "SGVsbG8gV29ybGQh";
    auto decoded = CryptoUtils::base64Decode(encoded);

    std::string result(decoded.begin(), decoded.end());
    EXPECT_EQ(result, "Hello World!");
}

TEST(CryptoUtilsTest, Base64RoundTrip)
{
    std::string original = "The quick brown fox jumps over the lazy dog";
    std::string encoded = CryptoUtils::base64Encode(original);
    auto decoded = CryptoUtils::base64Decode(encoded);

    std::string result(decoded.begin(), decoded.end());
    EXPECT_EQ(result, original);
}

TEST(CryptoUtilsTest, UrlEncode)
{
    EXPECT_EQ(CryptoUtils::urlEncode("hello world"), "hello%20world");
    EXPECT_EQ(CryptoUtils::urlEncode("test@example.com"), "test%40example.com");
    EXPECT_EQ(CryptoUtils::urlEncode("a=b&c=d"), "a%3Db%26c%3Dd");
    EXPECT_EQ(CryptoUtils::urlEncode("simple"), "simple");  // 不需要编码
}

TEST(CryptoUtilsTest, UrlDecode)
{
    EXPECT_EQ(CryptoUtils::urlDecode("hello%20world"), "hello world");
    EXPECT_EQ(CryptoUtils::urlDecode("test%40example.com"), "test@example.com");
    EXPECT_EQ(CryptoUtils::urlDecode("a%3Db%26c%3Dd"), "a=b&c=d");
}

TEST(CryptoUtilsTest, UrlEncodeDecodeRoundTrip)
{
    std::string original = "symbol=BTCUSDT&type=LIMIT&price=50000.5";
    std::string encoded = CryptoUtils::urlEncode(original);
    std::string decoded = CryptoUtils::urlDecode(encoded);

    EXPECT_EQ(decoded, original);
}

TEST(CryptoUtilsTest, ToHex)
{
    std::vector<uint8_t> data = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
    std::string hex = CryptoUtils::toHex(data);

    EXPECT_EQ(hex, "0123456789abcdef");
}

TEST(CryptoUtilsTest, FromHex)
{
    std::string hex = "0123456789abcdef";
    auto data = CryptoUtils::fromHex(hex);

    std::vector<uint8_t> expected = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
    EXPECT_EQ(data, expected);
}

TEST(CryptoUtilsTest, HexRoundTrip)
{
    std::vector<uint8_t> original = {0xFF, 0x00, 0xAA, 0x55};
    std::string hex = CryptoUtils::toHex(original);
    auto decoded = CryptoUtils::fromHex(hex);

    EXPECT_EQ(decoded, original);
}

TEST(StringUtilsTest, Trim)
{
    EXPECT_EQ(StringUtils::trim("  hello  "), "hello");
    EXPECT_EQ(StringUtils::trim("\t\ntest\n\t"), "test");
    EXPECT_EQ(StringUtils::trim("no_spaces"), "no_spaces");
    EXPECT_EQ(StringUtils::trim("   "), "");
}

TEST(StringUtilsTest, Split)
{
    auto parts = StringUtils::split("a,b,c,d", ',');
    ASSERT_EQ(parts.size(), 4);
    EXPECT_EQ(parts[0], "a");
    EXPECT_EQ(parts[1], "b");
    EXPECT_EQ(parts[2], "c");
    EXPECT_EQ(parts[3], "d");

    auto parts2 = StringUtils::split("one|two|three", '|');
    ASSERT_EQ(parts2.size(), 3);
}

TEST(StringUtilsTest, Join)
{
    std::vector<std::string> parts = {"a", "b", "c"};
    EXPECT_EQ(StringUtils::join(parts, ","), "a,b,c");
    EXPECT_EQ(StringUtils::join(parts, " | "), "a | b | c");

    std::vector<std::string> empty;
    EXPECT_EQ(StringUtils::join(empty, ","), "");
}

TEST(StringUtilsTest, ToUpper)
{
    EXPECT_EQ(StringUtils::toUpper("hello"), "HELLO");
    EXPECT_EQ(StringUtils::toUpper("Hello World"), "HELLO WORLD");
    EXPECT_EQ(StringUtils::toUpper("ALREADY_UPPER"), "ALREADY_UPPER");
}

TEST(StringUtilsTest, ToLower)
{
    EXPECT_EQ(StringUtils::toLower("HELLO"), "hello");
    EXPECT_EQ(StringUtils::toLower("Hello World"), "hello world");
    EXPECT_EQ(StringUtils::toLower("already_lower"), "already_lower");
}

TEST(StringUtilsTest, Replace)
{
    EXPECT_EQ(StringUtils::replace("hello world", "world", "universe"), "hello universe");
    EXPECT_EQ(StringUtils::replace("aaa", "a", "b"), "bbb");
    EXPECT_EQ(StringUtils::replace("test", "xyz", "abc"), "test");  // 不存在
}

TEST(StringUtilsTest, StartsWith)
{
    EXPECT_TRUE(StringUtils::startsWith("hello world", "hello"));
    EXPECT_TRUE(StringUtils::startsWith("test", "test"));
    EXPECT_FALSE(StringUtils::startsWith("hello", "world"));
    EXPECT_FALSE(StringUtils::startsWith("hi", "hello"));
}

TEST(StringUtilsTest, EndsWith)
{
    EXPECT_TRUE(StringUtils::endsWith("hello world", "world"));
    EXPECT_TRUE(StringUtils::endsWith("test", "test"));
    EXPECT_FALSE(StringUtils::endsWith("hello", "world"));
    EXPECT_FALSE(StringUtils::endsWith("hi", "hello"));
}

TEST(StringUtilsTest, Format)
{
    EXPECT_EQ(StringUtils::format("Hello %s", "World"), "Hello World");
    EXPECT_EQ(StringUtils::format("Number: %d", 42), "Number: 42");
    EXPECT_EQ(StringUtils::format("%.2f", 3.14159), "3.14");
}
