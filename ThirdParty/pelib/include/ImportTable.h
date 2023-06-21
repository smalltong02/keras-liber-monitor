#pragma once
#include "PeLib.h"

namespace cchips {

    class Import
    {
    public:
        enum class UsageType
        {
            UNKNOWN,
            FUNCTION,
            OBJECT,
            FILE
        };

    private:
        std::string name;
        unsigned long long libraryIndex = 0;
        unsigned long long address = 0;
        unsigned long long ordinalNumber = 0;
        bool ordinalNumberIsValid = false;
        UsageType usageType = UsageType::UNKNOWN;
    public:
        virtual ~Import() = default;

        /// @name Getters
        /// @{
        std::string getName() const { return name; }
        unsigned long long getLibraryIndex() const { return libraryIndex; }
        unsigned long long getAddress() const { return address; }
        bool getOrdinalNumber(unsigned long long &importOrdinalNumber) const {
            if (ordinalNumberIsValid)
            {
                importOrdinalNumber = ordinalNumber;
            }
            return ordinalNumberIsValid;
        }
        Import::UsageType getUsageType() const { return usageType; }
        /// @}

        /// @name Usage type queries
        /// @{
        bool isUnknown() const { return getUsageType() == UsageType::UNKNOWN; }
        bool isFunction() const { return getUsageType() == UsageType::FUNCTION; }
        bool isObject() const { return getUsageType() == UsageType::OBJECT; }
        bool isFile() const { return getUsageType() == UsageType::FILE; }
        /// @}

        /// @name Setters
        /// @{
        void setName(const std::string& importName) { name = importName; }
        void setLibraryIndex(unsigned long long importLibraryIndex) { libraryIndex = importLibraryIndex; }
        void setAddress(unsigned long long importAddress) { address = importAddress; }
        void setOrdinalNumber(unsigned long long importOrdinalNumber) {
            ordinalNumber = importOrdinalNumber;
            ordinalNumberIsValid = true;
        }
        void setUsageType(Import::UsageType importUsageType) { usageType = importUsageType; }
        /// @}

        /// @name Other methods
        /// @{
        virtual bool isUsedForImphash() const { return false; }
        void invalidateOrdinalNumber() { ordinalNumberIsValid = false; }
        bool hasEmptyName() const { return name.empty(); }
        /// @}
    };

    class ImportTable
    {
    private:
        using importsIterator = std::vector<std::unique_ptr<Import>>::const_iterator;
        std::vector<std::string> libraries;           ///< name of libraries
        std::vector<std::unique_ptr<Import>> imports; ///< stored imports
    public:
        /// @name Getters
        /// @{
        ImportTable() = default;
        ~ImportTable() = default;
        std::size_t getNumberOfLibraries() const { return libraries.size(); }
        std::size_t getNumberOfImports() const { return imports.size(); }
        std::size_t getNumberOfImportsInLibrary(std::size_t libraryIndex) const;
        std::size_t getNumberOfImportsInLibrary(const std::string &name) const;
        std::size_t getNumberOfImportsInLibraryCaseInsensitive(const std::string &name) const;

        std::string getLibrary(std::size_t libraryIndex) const { return (libraryIndex < getNumberOfLibraries()) ? libraries[libraryIndex] : ""; }
        const Import* getImport(std::size_t importIndex) const { return (importIndex < getNumberOfImports()) ? imports[importIndex].get() : nullptr; }
        const Import* getImport(const std::string &name) const;
        const Import* getImportOnAddress(unsigned long long address) const;
        /// @}

        /// @name Iterators
        /// @{
        importsIterator begin() const { return imports.begin(); }
        importsIterator end() const { return imports.end(); }
        /// @}

        /// @name Other methods
        /// @{
        void clear();
        void addLibrary(std::string name) { libraries.push_back(name); }
        void addImport(std::unique_ptr<Import>&& import) { imports.push_back(std::move(import)); }
        bool hasLibraries() const { return !libraries.empty(); }
        bool hasLibrary(const std::string &name) const { return std::find(libraries.begin(), libraries.end(), name) != libraries.end(); }
        bool hasLibraryCaseInsensitive(const std::string &name) const;
        bool hasImports() const { return !imports.empty(); }
        bool hasImport(const std::string &name) const { return getImport(name); }
        bool hasImport(unsigned long long address) const { return getImportOnAddress(address); }
        bool invalidImpHash() const { return true; }
        bool empty() const { return !hasLibraries() && !hasImports(); }
        void dump(std::string &dumpTable) const { return; }
        void dumpLibrary(std::size_t libraryIndex, std::string &libraryDump) const { return; }
        /// @}
    };
} // namespace cchips
