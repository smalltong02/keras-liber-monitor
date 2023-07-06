#pragma once
#include "SharedFunc.h"
#include "SectionTable.h"
#include "PeFormat.h"

namespace cchips {

    bool Section::isValid(const PeFormat *sOwner) const
    {
        if (!sOwner || getOffset() >= sOwner->getSizeOfImage())
        {
            return false;
        }
        if (!isBss() && !getSizeInFile())
        {
            return false;
        }
        if (!isBss() && entrySizeIsValid && entrySize > getSizeInFile())
        {
            return false;
        }

        return true;
    }

    bool Section::getBits(std::string &sResult) const {
        sResult = bytesToBits(bytes.data(), bytes.size());
        return loaded;
    }

    const std::string_view Section::getBytes(unsigned long long sOffset, unsigned long long sSize) const {
        if (sOffset >= bytes.size())
        {
            return std::string_view("");
        }

        return std::string_view(bytes.data() + sOffset, getRealSizeInRegion(sOffset, sSize, bytes.size()));
    }

    bool Section::getBytes(std::vector<unsigned char> &sResult, unsigned long long sOffset, unsigned long long sSize) const
    {
        if (sOffset >= bytes.size())
        {
            return false;
        }

        sSize = getRealSizeInRegion(sOffset, sSize, bytes.size());
        sResult.reserve(sSize);
        sResult.assign(bytes.begin() + sOffset, bytes.begin() + sOffset + sSize);
        return loaded;
    }

    bool Section::getString(std::string &sResult, unsigned long long sOffset, unsigned long long sSize) const
    {
        if (sOffset >= bytes.size())
        {
            return false;
        }

        bytesToString(bytes.data(), bytes.size(), sResult, sOffset, sSize);
        return loaded;
    }

    bool Section::getHexBytes(std::string &sResult) const
    {
        bytesToHexString(bytes.data(), bytes.size(), sResult);
        return loaded;
    }

    void Section::load(const PeFormat *sOwner)
    {
        size_t loadsize = isInMemory ? memorySize : fileSize;
        if (!loadsize || !address)
        {
            bytes = "";
            loaded = sOwner && offset < sOwner->getSizeOfImage();
            return;
        }

        bytes = std::string_view(reinterpret_cast<const char*>(address), (std::min)(loadsize, sOwner->getSizeOfImage() - offset));
        loaded = true;
    }
} // namespace cchips
