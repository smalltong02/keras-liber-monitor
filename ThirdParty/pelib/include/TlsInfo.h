#pragma once
#include "PeLib.h"

namespace cchips {

    class TlsInfo
    {
    private:
        std::vector<std::uint64_t> callBacks;  ///< addresses of callback functions
        std::uint64_t rawDataStartAddr = 0;    ///< start address of raw data
        std::uint64_t rawDataEndAddr = 0;      ///< end address of raw data
        std::uint64_t indexAddr = 0;           ///< address of index
        std::uint64_t callBacksAddr = 0;       ///< address of array of callbacks
        std::uint32_t zeroFillSize = 0;        ///< size of zero fill
        std::uint32_t characteristics = 0;     ///< characteristics
        bool rawDataStartAddrValid = false;    ///< member validity flag
        bool rawDataEndAddrValid = false;      ///< member validity flag
        bool indexAddrValid = false;           ///< member validity flag
        bool callBacksAddrValid = false;       ///< member validity flag
        bool zeroFillSizeValid = false;        ///< member validity flag
        bool characteristicsValid = false;     ///< member validity flag
    public:
        /// @name Getters
        /// @{
        bool getRawDataStartAddr(std::uint64_t &res) const;
        bool getRawDataEndAddr(std::uint64_t &res) const;
        bool getIndexAddr(std::uint64_t &res) const;
        bool getCallBacksAddr(std::uint64_t &res) const;
        bool getZeroFillSize(std::uint32_t &res) const;
        bool getCharacteristics(std::uint32_t &res) const;
        const std::vector<std::uint64_t> &getCallBacks() const { return callBacks; }
        /// @}

        /// @name Setters
        /// @{
        void setRawDataStartAddr(std::uint64_t sAddr) {
            rawDataStartAddr = sAddr;
            rawDataStartAddrValid = true;
        }
        void setRawDataEndAddr(std::uint64_t eAddr) {
            rawDataEndAddr = eAddr;
            rawDataEndAddrValid = true;
        }
        void setIndexAddr(std::uint64_t iAddr) {
            indexAddr = iAddr;
            indexAddrValid = true;
        }
        void setCallBacksAddr(std::uint64_t cbAddr) {
            callBacksAddr = cbAddr;
            callBacksAddrValid = true;
        }
        void setZeroFillSize(std::uint32_t zFill) {
            zeroFillSize = zFill;
            zeroFillSizeValid = true;
        }
        void setCharacteristics(std::uint32_t chars) {
            characteristics = chars;
            characteristicsValid = true;
        }
        /// @}

        /// @name Other methods
        /// @{
        void addCallBack(std::uint64_t cb) { callBacks.push_back(cb); }
        /// @}
    };
} // namespace cchips
