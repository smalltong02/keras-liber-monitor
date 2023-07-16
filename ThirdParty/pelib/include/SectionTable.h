#pragma once
#include "PeLib.h"
#include "Range.h"

namespace cchips {

    class PeFormat;

    class Section
    {
    public:
        enum class Type
        {
            UNDEFINED_SEC_SEG, ///< undefined
            CODE_SEG,              ///< code
            DATA_SEG,              ///< data
            CODE_DATA_SEG,         ///< code and (or) data
            CONST_DATA_SEG,        ///< constant data
            BSS_SEG,               ///< uninitialized data
            DEBUG_SEG,             ///< debug information
            INFO_SEG               ///< auxiliary information
        };
        Section() = default;
        ~Section() = default;

        /// @name Query methods
        /// @{
        bool isUndefined() const { return getType() == Type::UNDEFINED_SEC_SEG; }
        bool isCode() const { return getType() == Type::CODE_SEG; }
        bool isData() const { return getType() == Type::DATA_SEG; }
        bool isCodeAndData() const { return getType() == Type::CODE_DATA_SEG; }
        bool isConstData() const { return getType() == Type::CONST_DATA_SEG; }
        bool isBss() const { return getType() == Type::BSS_SEG; }
        bool isDebug() const { return getType() == Type::DEBUG_SEG; }
        bool isInfo() const { return getType() == Type::INFO_SEG; }
        bool isSomeData() const { return isData() || isCodeAndData() || isConstData(); }
        bool isSomeCode() const { return isCode() || isCodeAndData(); }
        bool isDataOnly() const { return isData() || isConstData() || isDebug() || isBss() || isInfo(); }
        bool isReadOnly() const { return isCode() || isConstData(); }
        /// @}

        /// @name Virtual query methods
        /// @{
        bool isValid(const PeFormat *sOwner) const;
        /// @}

        /// @name Getters
        /// @{
        std::string getName() const { return name; }
        const char* getNameAsCStr() const { return name.c_str(); }
        const std::string_view getBytes(unsigned long long sOffset = 0, unsigned long long sSize = 0) const;
        Section::Type getType() const { return type; }
        unsigned long long getIndex() const { return index; }
        unsigned long long getOffset() const { return offset; }
        unsigned long long getEndOffset() const {
            const auto size = getSizeInFile();
            return size ? getOffset() + size : getOffset() + 1;
        }
        unsigned long long getSizeInFile() const { return fileSize; }
        unsigned long long getLoadedSize() const { return bytes.size(); }
        unsigned long long getAddress() const { return address; }
        unsigned long long getVirtualAddress() const { return virtual_address; }
        unsigned long long getEndAddress() const {
            unsigned long long size = 0;
            if (!(size = getSizeInFile()))
            {
                getSizeInMemory(size);
            }

            return size ? getAddress() + size : getAddress() + 1;
        }
        bool getSizeInMemory(unsigned long long &sMemorySize) const {
            if (memorySizeIsValid)
            {
                sMemorySize = memorySize;
            }

            return memorySizeIsValid;
        }
        bool getSizeOfOneEntry(unsigned long long &sEntrySize) const {
            if (entrySizeIsValid)
            {
                sEntrySize = entrySize;
            }

            return entrySizeIsValid;
        }
        bool getMemory() const { return isInMemory; }

        template<typename NumberType>
        NumberType getBytesAtOffsetAsNumber(unsigned long long sOffset) const
        {
            if (bytes.size() < sizeof(NumberType) || sOffset > bytes.size() - sizeof(NumberType))
            {
                return {};
            }

            auto rawBytes = bytes.data();
            return *reinterpret_cast<const NumberType*>(&rawBytes[sOffset]);
        }
        /// @}

        /// @name Getters of section or segment content
        /// @{
        bool getBits(std::string &sResult) const;
        bool getBytes(std::vector<unsigned char> &sResult, unsigned long long sOffset = 0, unsigned long long sSize = 0) const;
        bool getString(std::string &sResult, unsigned long long sOffset = 0, unsigned long long sSize = 0) const;
        bool getHexBytes(std::string &sResult) const;
        /// @}

        /// @name Setters
        /// @{
        void setName(std::string sName) { name = sName; }
        void setType(Section::Type sType) { type = sType; }
        void setIndex(unsigned long long sIndex) { index = sIndex; }
        void setOffset(unsigned long long sOffset) { offset = sOffset; }
        void setSizeInFile(unsigned long long sFileSize) { fileSize = sFileSize; }
        void setAddress(unsigned long long sAddress) { address = sAddress; }
        void setAddressInMemory(unsigned long long sAddress) { virtual_address = sAddress; }
        void setSizeInMemory(unsigned long long sMemorySize) {
            memorySize = sMemorySize;
            memorySizeIsValid = true;
        }
        void setSizeOfOneEntry(unsigned long long sEntrySize) {
            entrySize = sEntrySize;
            entrySizeIsValid = true;
        }
        void setMemory(bool sMemory) { isInMemory = sMemory; }
        /// @}

        /// @name Other methods
        /// @{
        void invalidateMemorySize() { memorySizeIsValid = false; }
        void invalidateEntrySize() { entrySizeIsValid = false; }
        void load(const PeFormat *sOwner);
        void dump(std::string &sDump) const { return; }
        bool hasEmptyName() const { return name.empty(); }
        bool belong(unsigned long long sAddress) const { return sAddress >= getAddress() && sAddress < getEndAddress(); }
        bool operator<(const Section &sOther) const { return getAddress() < sOther.getAddress(); }
        /// @}
    private:
        std::string name;                     ///< name of section or segment
        std::string_view bytes;                ///< reference to content of section or segment
        Section::Type type = Type::UNDEFINED_SEC_SEG;  ///< type
        unsigned long long index = 0;         ///< index
        unsigned long long offset = 0;        ///< start offset in file
        unsigned long long fileSize = 0;      ///< size in file
        unsigned long long address = 0;       ///< start address in memory
        unsigned long long virtual_address = 0; ///< virtual address
        unsigned long long memorySize = 0;    ///< size in memory
        unsigned long long entrySize = 0;     ///< size of one entry in file
        bool memorySizeIsValid = false;       ///< @c true if size in memory is valid
        bool entrySizeIsValid = false;        ///< size of one entry in section or segment
        bool isInMemory = false;              ///< @c true if the section or segment will appear in the memory image of a process
        bool loaded = false;                  ///< @c true if content of section or segment was successfully loaded from input file
    };

    class PeCoffSection : public Section
    {
    private:
        unsigned long long peCoffFlags; ///< section flags
        RangeContainer<std::uint64_t> nonDecodableRanges;
    public:
        PeCoffSection() : Section(), peCoffFlags(0) {}
        void AddDecodableRange(Range<std::uint64_t>& range) { nonDecodableRanges.insert(range); }
        unsigned long long getPredictAddress(std::uint8_t* addr) const {
            if (!nonDecodableRanges.size()) return getEndAddress();
            for (auto& range : nonDecodableRanges) {
                if (range.getStart() >= reinterpret_cast<unsigned long long>(addr))
                    return range.getStart();
            }
            return getEndAddress();
        }
        /// @name Getters
        /// @{
        unsigned long long getPeCoffFlags() const { return peCoffFlags; }
        /// @}

        /// @name Setters
        /// @{
        void setPeCoffFlags(unsigned long long sPeCoffFlags) { peCoffFlags = sPeCoffFlags; }
        /// @}
    };
} // namespace cchips
