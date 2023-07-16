#pragma once
#include "PeLib.h"

namespace cchips {

    class Export
    {
    private:
        std::string name;
        unsigned long long address = 0;
        unsigned long long ordinalNumber = 0;
        bool ordinalNumberIsValid = false;
        bool blinkage = false;
    public:
        virtual ~Export() = default;

        /// @name Getters
        /// @{
        std::string getName() const { return name; }
        unsigned long long getAddress() const { return address; }
        bool getOrdinalNumber(unsigned long long &exportOrdinalNumber) const;
        void Clear() {
            name.clear();
            address = 0;
            ordinalNumber = 0;
            ordinalNumberIsValid = false;
            blinkage = false;
        }
        /// @}

        /// @name Setters
        /// @{
        void setName(std::string exportName) { name = exportName; }
        void setAddress(unsigned long long exportAddress) { address = exportAddress; }
        void setLinkage() { blinkage = true; }
        void setOrdinalNumber(unsigned long long exportOrdinalNumber) {
            ordinalNumber = exportOrdinalNumber;
            ordinalNumberIsValid = true;
        }
        /// @}

        /// @name Other methods
        /// @{
        virtual bool isUsedForExphash() const { return false; }
        void invalidateOrdinalNumber() { ordinalNumberIsValid = false; }
        bool hasEmptyName() const { return name.empty(); }
        bool isLinkage() const { return blinkage; }
        /// @}
    };

    class ExportTable
    {
    private:
        using exportsIterator = std::vector<Export>::const_iterator;
        std::vector<Export> exports;                ///< stored exports
        std::string name;
    public:
        /// @name Getters
        /// @{
        const std::string& getName() const { return name; }
        std::size_t getNumberOfExports() const { return exports.size(); }
        const Export* getExport(std::size_t exportIndex) const { return (exportIndex < getNumberOfExports()) ? &exports[exportIndex] : nullptr; }
        const Export* getExport(const std::string &name) const;
        const Export* getExportOnAddress(unsigned long long address) const;
        /// @}

        /// @name Iterators
        /// @{
        exportsIterator begin() const { return exports.begin(); }
        exportsIterator end() const { return exports.end(); }
        /// @}

        /// @name Other methods
        /// @{
        void clear() { exports.clear(); }
        void setName(std::string& name_string) { name = name_string; }
        void addExport(Export &newExport) { exports.push_back(newExport); }
        bool hasExports() const { return !exports.empty(); }
        bool hasExport(const std::string &name) const { return getExport(name); }
        bool hasExport(unsigned long long address) const { return getExportOnAddress(address); }
        bool empty() const { return exports.empty(); }
        void dump(std::string &dumpTable) const { return; }
        /// @}
    };
} // namespace cchips
