#include "stringutils.h"
#include <regex>

std::string HexEncode(const void* bytes, size_t size) {
    static const char kHexChars[] = "0123456789ABCDEF";

    // Each input byte creates two output hex characters.
    std::string ret(size * 2, '\0');

    for (size_t i = 0; i < size; ++i) {
        char b = reinterpret_cast<const char*>(bytes)[i];
        ret[(i * 2)] = kHexChars[(b >> 4) & 0xf];
        ret[(i * 2) + 1] = kHexChars[b & 0xf];
    }
    return ret;
}

std::string GetMd5(unsigned char* buffer, unsigned int buf_size)
{
    if (!buffer || buf_size == 0)
        return {};

    MD5_CTX md5_ctx;
    MD5Init(&md5_ctx);
    MD5Update(&md5_ctx, buffer, buf_size);
    MD5Final(&md5_ctx);

    return HexEncode(md5_ctx.digest, 16);
}

std::string GetSha256(std::wstring buffer_string)
{
    if (!buffer_string.length())
        return {};
    SHA256_CTX sha256_ctx;
    SHA256_Init(&sha256_ctx);
    SHA256_Update(&sha256_ctx, buffer_string.data(), buffer_string.length() * sizeof(wchar_t));
    unsigned char digest[SHA256_DIGEST_LENGTH];
    SHA256_Final(digest, &sha256_ctx);
    return HexEncode(digest, SHA256_DIGEST_LENGTH);
}

bool endsWith(const std::string& str, const std::string& withWhat) {
    std::string_view tail = str;

    if (tail.length() < withWhat.length()) {
        return false;
    }

    std::string strLowerCase(tail);
    std::transform(strLowerCase.begin(), strLowerCase.end(), strLowerCase.begin(),
        [](unsigned char c) { return std::tolower(c); });

    std::string whatLowerCase(withWhat);
    std::transform(whatLowerCase.begin(), whatLowerCase.end(), whatLowerCase.begin(),
        [](unsigned char c) { return std::tolower(c); });

    return strLowerCase.compare(strLowerCase.length() - whatLowerCase.length(), whatLowerCase.length(), whatLowerCase) == 0;
}

bool startsWith(const std::string& str, const std::string& withWhat) {
    std::string_view first = str;

    if (first.length() < withWhat.length()) {
        return false;
    }

    std::string strLowerCase(first);
    std::transform(strLowerCase.begin(), strLowerCase.end(), strLowerCase.begin(),
        [](unsigned char c) { return std::tolower(c); });

    std::string whatLowerCase(withWhat);
    std::transform(whatLowerCase.begin(), whatLowerCase.end(), whatLowerCase.begin(),
        [](unsigned char c) { return std::tolower(c); });

    return strLowerCase.find(whatLowerCase, 0) == 0;
}

std::string toLower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(),
        [](const unsigned char c) { return std::tolower(c); });
    return str;
}

std::string toUpper(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(),
        [](const unsigned char c) { return std::toupper(c); });
    return str;
}

bool areEqualCaseInsensitive(const std::string& str1, const std::string& str2) {
    if (str1.size() != str2.size()) {
        return false;
    }

    for (std::string::size_type i = 0, e = str1.size(); i < e; ++i) {
        const unsigned char lc = str1[i];
        const unsigned char rc = str2[i];
        if (std::tolower(lc) != std::tolower(rc)) {
            return false;
        }
    }
    return true;
}

bool contains(const std::string& str, const std::string& sub) {
    return str.find(sub) != std::string::npos;
}

template<typename T>
bool areEqualFPWithEpsilon(const T& x, const T& y, const T& epsilon) {
    // Implementation notes:
    // - Inspiration was taken from
    //   http://www.parashift.com/c++-faq-lite/newbie.html#faq-29.17. See also
    //   Section 4.2 in [D. Knuth, The Art of Computer Programming, Volume II].
    // - std::{abs,isnan,isinf}() in cmath are overloaded for floats, doubles, and
    //   long doubles.
    if (std::isnan(x)) {
        return std::isnan(y);
    }
    else if (std::isnan(y)) {
        return false;
    }
    else if (std::isinf(x)) {
        return std::isinf(x) == std::isinf(y);
    }
    else if (std::isinf(y)) {
        return false;
    }
    return std::abs(x - y) <= epsilon * std::abs(x);
}

template<typename T>
bool areEqual(const T& x, const T& y) {
    return x == y;
}

// Specialization for floats.
template<>
bool areEqual<float>(const float& x, const float& y) {
    return areEqualFPWithEpsilon(x, y, 1e-5f);
}

// Specialization for doubles.
template<>
bool areEqual<double>(const double& x, const double& y) {
    return areEqualFPWithEpsilon(x, y, 1e-10);
}

// Specialization for long doubles.
template<>
bool areEqual<long double>(const long double& x, const long double& y) {
    return areEqualFPWithEpsilon(x, y, 1e-15L);
}

//template<typename I>
//std::string intToHexString(I w, bool addBase, unsigned fillToN)
//{
//    static const char* digits = "0123456789abcdef";
//
//    size_t hex_len = sizeof(I) << 1;
//
//    std::string rc(hex_len, '0');
//    for (size_t i = 0, j = (hex_len - 1) * 4; i < hex_len; ++i, j -= 4)
//    {
//        rc[i] = digits[(w >> j) & 0x0f];
//    }
//
//    bool started = false;
//    std::string res;
//    size_t j = 0;
//    if (addBase)
//    {
//        res.resize(rc.size() + 2);
//        res[0] = '0';
//        res[1] = 'x';
//        j = 2;
//    }
//    else
//    {
//        res.resize(rc.size());
//    }
//    for (size_t i = 0; i < rc.size(); ++i)
//    {
//        if (started)
//        {
//            res[j++] = rc[i];
//        }
//        else if (rc[i] != '0' || (rc.size() - i <= fillToN) || (i == rc.size() - 1))
//        {
//            res[j++] = rc[i];
//            started = true;
//        }
//    }
//    res.resize(j);
//
//    return res;
//}
//
//template<typename N>
//bool strToNum(const std::string& str, N& number,
//    std::ios_base& (*format)(std::ios_base&)) {
//    std::istringstream strStream(str);
//    N convNumber = 0;
//    strStream >> format >> convNumber;
//    if (strStream.fail() || !strStream.eof()) {
//        return false;
//    }
//
//    // The above checks do not detect conversion of a negative number into an
//    // unsigned integer. We have to perform an additional check here.
//    if (std::is_unsigned<N>::value && str[0] == '-') {
//        return false;
//    }
//
//    number = convNumber;
//    return true;
//}

bool isShorterPrefixOfCaseInsensitive(const std::string& str1,
    const std::string& str2) {
    const auto minLen = std::min(str1.length(), str2.length());
    return areEqualCaseInsensitive(str1.substr(0, minLen), str2.substr(0, minLen));
}

bool hasSubstringOnPosition(const std::string& str,
    const std::string& withWhat, std::string::size_type position) {
    return (position < str.length()) && (str.length() - position >= withWhat.length()) &&
        (str.compare(position, withWhat.length(), withWhat) == 0);
}

bool hasSubstringInArea(const std::string& str, const std::string& withWhat,
    std::string::size_type start, std::string::size_type stop) {
    if (start > stop) {
        return false;
    }

    const auto stopIndex = stop + 1;
    const auto stopIterator = stopIndex < str.size() ?
        str.begin() + stopIndex : str.end();
    return std::search(str.begin() + start, stopIterator, withWhat.begin(),
        withWhat.end()) != stopIterator;
}

bool isComposedOnlyOfChars(const std::string& str, const std::string& chars) {
    return str.find_first_not_of(chars) == std::string::npos;
}

bool containsAnyOfChars(const std::string& str, const std::string& chars) {
    return str.find_first_of(chars) != std::string::npos;
}

bool isValidSignaturePattern(const std::string& pattern)
{
    const std::string bodyChars = "0123456789ABCDEF-\?/";
    const std::string allChars = bodyChars + ';';
    return isComposedOnlyOfChars(pattern, allChars)
        && containsAnyOfChars(pattern, bodyChars)
        && pattern.find(';') >= pattern.length() - 1;
}

std::string extractVersion(const std::string& input)
{
    static std::regex e("([0-9]+\\.)+[0-9]+");
    std::smatch match;
    if (regex_search(input, match, e))
    {
        return match.str();
    }

    return std::string();
}

void separateStrings(std::string& str, std::vector<std::string>& words)
{
    str += '\0';
    words.clear();
    std::string tmp;
    const auto strLen = str.length();

    for (std::size_t i = 0, lastPos = 0; i < strLen; ++i)
    {
        if (str[i] == '\0')
        {
            tmp = str.substr(lastPos, i + 1 - lastPos);
            lastPos = i;
            if (tmp != std::string(tmp.length(), '\0'))
            {
                if (tmp[0] == '\0')
                {
                    tmp.erase(0, 1);
                }
                if (tmp[tmp.length() - 1] == '\0')
                {
                    tmp.erase(tmp.length() - 1, 1);
                }
                words.push_back(tmp);
            }
        }
    }

    str.erase(strLen - 1, 1);
}

bool findAutoIt(const std::string& content)
{
    const std::string prefix = "AU3!EA";
    const std::regex regExp(prefix + "[0-9]{2}");
    const auto offset = content.find(prefix);
    return offset != std::string::npos
        && regex_match(content.substr(offset, 8), regExp);
}

std::string toWide(const std::string& str, std::string::size_type length) {
    if (!length) {
        return "";
    }

    const std::string padding(length - 1, '\0');
    std::string result;
    result.reserve(str.length() * length);
    for (auto c : str) {
        result += c + padding;
    }
    return result;
}

const double g_log_2 = 1.44269504088896340736; //instead of using M_LOG2E

double getEntropy(std::string_view bytes)
{
    if (!bytes.size()) return 0;
    double entropy = 0;
    uint32_t byte_count[256] = { 0 }; //Byte count for each of 255 bytes
    for (auto byte: bytes)
        ++byte_count[((unsigned char)byte)];

    for (uint32_t i = 0; i < 256; ++i)
    {
        double temp = static_cast<double>(byte_count[i]) / bytes.size();
        if (temp > 0.)
            entropy += std::abs(temp * (std::log(temp) * g_log_2));
    }

    return entropy;
}

bool isPrintableChar(unsigned char c) {
    return std::isprint(c) && !std::iscntrl(c);
}

bool isNonprintableChar(unsigned char c) {
    return !isPrintableChar(c);
}

char* byteToHexString(uint8_t b, bool uppercase)
{
    static char result[3] = { '\0', '\0', '\0' };
    static const char digits[513] =
        "000102030405060708090A0B0C0D0E0F"
        "101112131415161718191A1B1C1D1E1F"
        "202122232425262728292A2B2C2D2E2F"
        "303132333435363738393A3B3C3D3E3F"
        "404142434445464748494A4B4C4D4E4F"
        "505152535455565758595A5B5C5D5E5F"
        "606162636465666768696A6B6C6D6E6F"
        "707172737475767778797A7B7C7D7E7F"
        "808182838485868788898A8B8C8D8E8F"
        "909192939495969798999A9B9C9D9E9F"
        "A0A1A2A3A4A5A6A7A8A9AAABACADAEAF"
        "B0B1B2B3B4B5B6B7B8B9BABBBCBDBEBF"
        "C0C1C2C3C4C5C6C7C8C9CACBCCCDCECF"
        "D0D1D2D3D4D5D6D7D8D9DADBDCDDDEDF"
        "E0E1E2E3E4E5E6E7E8E9EAEBECEDEEEF"
        "F0F1F2F3F4F5F6F7F8F9FAFBFCFDFEFF";
    static const char digitsLowerAlpha[513] =
        "000102030405060708090a0b0c0d0e0f"
        "101112131415161718191a1b1c1d1e1f"
        "202122232425262728292a2b2c2d2e2f"
        "303132333435363738393a3b3c3d3e3f"
        "404142434445464748494a4b4c4d4e4f"
        "505152535455565758595a5b5c5d5e5f"
        "606162636465666768696a6b6c6d6e6f"
        "707172737475767778797a7b7c7d7e7f"
        "808182838485868788898a8b8c8d8e8f"
        "909192939495969798999a9b9c9d9e9f"
        "a0a1a2a3a4a5a6a7a8a9aaabacadaeaf"
        "b0b1b2b3b4b5b6b7b8b9babbbcbdbebf"
        "c0c1c2c3c4c5c6c7c8c9cacbcccdcecf"
        "d0d1d2d3d4d5d6d7d8d9dadbdcdddedf"
        "e0e1e2e3e4e5e6e7e8e9eaebecedeeef"
        "f0f1f2f3f4f5f6f7f8f9fafbfcfdfeff";

    const char* lut = uppercase ? digits : digitsLowerAlpha;
    std::size_t pos = b * 2;
    result[0] = lut[pos];
    result[1] = lut[pos + 1];

    return &(result[0]);
}

std::string replaceChars(const std::string& str, bool (*predicate)(unsigned char)) {
    std::string prefix("\\x");
    std::string result;
    result.reserve(str.size() * 4);
    for (const auto c : str) {
        if (predicate(c)) {
            result += prefix + byteToHexString(c, false);
        }
        else {
            result += c;
        }
    }
    return result;
}

std::string replaceNonprintableChars(const std::string& str) {
    return replaceChars(str, isNonprintableChar);
}

std::string readNullTerminatedAscii(const std::uint8_t* bytes, std::size_t bytesLen, std::size_t offset,
    std::size_t maxBytes, bool failOnExceed)
{
    std::string result;
    if (!bytes)
    {
        return {};
    }

    if (maxBytes == 0)
    {
        maxBytes = bytesLen;
    }
    else if (offset + maxBytes > bytesLen)
    {
        maxBytes = bytesLen;
    }
    else
    {
        maxBytes += offset;
    }

    std::size_t i;
    for (i = offset; i < maxBytes; i++)
    {
        if (bytes[i] == '\0')
        {
            break;
        }
        result.push_back(bytes[i]);
    }

    if (i == maxBytes && failOnExceed)
    {
        return {};
    }

    return replaceNonprintableChars(result);
}

unsigned int byteSwap32(unsigned int val) {
    return (0xFF000000 & val) >> 24 |
        (0xFF0000 & val) >> 8 |
        (0xFF00 & val) << 8 |
        (0xFF & val) << 24;
}
