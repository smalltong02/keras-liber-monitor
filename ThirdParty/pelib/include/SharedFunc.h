#pragma once
#include <sstream>
#include <unordered_set>
#include <cctype>
#include <iomanip>
#include "PeLib.h"

namespace cchips {

    bool areEqualCaseInsensitive(const std::string &str1, const std::string &str2);
    bool isPrintableChar(unsigned char c);
    bool isNonprintableChar(unsigned char c);
    bool hasNonprintableChars(const std::string &str);
    std::size_t getRealSizeInRegion(std::size_t offset, std::size_t requestedSize, std::size_t regionSize);
    std::string unicodeToAscii(const std::uint8_t *bytes, std::size_t nBytes);
    std::string unicodeToAscii(const std::uint8_t *bytes, std::size_t nBytes, std::size_t &nRead);
    std::uint64_t alignDown(std::uint64_t value, std::uint64_t alignment);
    std::uint64_t alignUp(std::uint64_t value, std::uint64_t alignment);
    std::string lcidToStr(std::size_t lcid);
    std::string codePageToStr(std::size_t cpage);
    std::string replaceChars(const std::string &str, bool(*predicate)(unsigned char));
    std::string replaceNonprintableChars(const std::string &str);

    template<typename N>
    std::string bytesToBits(const N *data, std::size_t dataSize) {
#define BITS_IN_BYTE 8
        if (!data) {
            dataSize = 0;
        }

        std::string result;
        result.reserve(dataSize * BITS_IN_BYTE);

        for (std::size_t i = 0; i < dataSize; ++i) {
            auto& item = data[i];

            for (std::size_t j = 0; j < BITS_IN_BYTE; ++j) {
                // 0x80 = 0b10000000
                result += ((item << j) & 0x80) ? '1' : '0';
            }
        }

        return result;
    }

    template<typename N> void bytesToString(const N *data, std::size_t dataSize, std::string &result, std::size_t offset = 0, std::size_t size = 0)
    {
        if (!data)
        {
            dataSize = 0;
        }

        if (offset >= dataSize)
        {
            size = 0;
        }
        else
        {
            size = (size == 0 || offset + size > dataSize) ? dataSize - offset : size;
        }

        result.clear();
        result.reserve(size);
        result = std::string(reinterpret_cast<const char*>(data + offset), size);
    }

    template<typename N> void bytesToString(
        const std::vector<N>& bytes,
        std::string& result,
        std::size_t offset = 0,
        std::size_t size = 0)
    {
        bytesToString(bytes.data(), bytes.size(), result, offset, size);
    }

    template<typename N> void bytesToHexString(const N *data, std::size_t dataSize, std::string &result, std::size_t offset = 0, std::size_t size = 0, bool uppercase = true)
    {
        if (!data)
        {
            dataSize = 0;
        }

        if (offset >= dataSize)
        {
            size = 0;
        }
        else
        {
            size = (size == 0 || offset + size > dataSize) ? dataSize - offset : size;
        }

        // Sample: 4A2A008CF1AEE9BA49D8D1DAA22D8E868365ACE633823D464478239F27ED4F18
        // Tool: redec-fileinfo.exe, Debug, x64, data = image, dataSize = 0xE1BC00
        // Optimized: This code now takes 0.106 seconds to convert (measured in VS 2015 IDE)
        // (down from about 40 seconds)
        const char * intToHex = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";
        std::size_t hexIndex = 0;

        // Reserve the necessary space for the hexa string
        result.resize(size * 2);

        // Convert to hexa byte-by-byte. No reallocations
        for (std::size_t i = 0; i < size; ++i, hexIndex += 2)
        {
            std::uint8_t oneByte = data[offset + i];

            result[hexIndex + 0] = intToHex[(oneByte >> 0x04) & 0x0F];
            result[hexIndex + 1] = intToHex[(oneByte >> 0x00) & 0x0F];
        }
    }

    template<typename Map>
    typename Map::mapped_type mapGetValueOrDefault(
        const Map &m,
        const typename Map::key_type &key,
        typename Map::mapped_type defaultValue = typename Map::mapped_type()) {
        auto i = m.find(key);
        return i != m.end() ? i->second : defaultValue;
    }

    template<typename N>
    inline std::string numToStr(const N number,
        std::ios_base &(*format)(std::ios_base &) = std::dec) {
        std::ostringstream strStream;
        strStream << format << number;
        return strStream.str();
    }
} // namespace cchips
