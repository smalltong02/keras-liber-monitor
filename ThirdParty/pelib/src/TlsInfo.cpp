#pragma once
#include "TlsInfo.h"

namespace cchips {

    bool TlsInfo::getRawDataStartAddr(std::uint64_t &res) const
    {
        if (!rawDataStartAddrValid)
        {
            return false;
        }
        res = rawDataStartAddr;
        return true;
    }

    bool TlsInfo::getRawDataEndAddr(std::uint64_t &res) const
    {
        if (!rawDataEndAddrValid)
        {
            return false;
        }
        res = rawDataEndAddr;
        return true;
    }

    bool TlsInfo::getIndexAddr(std::uint64_t &res) const
    {
        if (!indexAddrValid)
        {
            return false;
        }
        res = indexAddr;
        return true;
    }

    bool TlsInfo::getCallBacksAddr(std::uint64_t &res) const
    {
        if (!callBacksAddrValid)
        {
            return false;
        }
        res = callBacksAddr;
        return true;
    }

    bool TlsInfo::getZeroFillSize(std::uint32_t &res) const
    {
        if (!zeroFillSizeValid)
        {
            return false;
        }
        res = zeroFillSize;
        return true;
    }

    bool TlsInfo::getCharacteristics(std::uint32_t &res) const
    {
        if (!characteristicsValid)
        {
            return false;
        }
        res = characteristics;
        return true;
    }
} // namespace cchips
