#pragma once
#include "PeLibAux.h"
#include "PeLib.h"
#include "Range.h"
#include "SharedFunc.h"
#include "DynamicBuffer.h"
#include "ImportTable.h"
#include "ExportTable.h"
#include "ResourceTable.h"
#include "SectionTable.h"
#include "TlsInfo.h"

namespace cchips {

    class PeFormat
    {
    public:
        enum class Architecture
        {
            UNKNOWN,
            X86,
            X86_64,
            ARM,
            POWERPC,
            MIPS
        };

        PeFormat() = delete;
        PeFormat(std::unique_ptr<PeLib::PeFile> file) : pe_file(std::move(file)) {
            if(pe_file)
                mzHeader = pe_file->mzHeader();
            loadSections();
            loadSymbols();
            loadImports();
            loadExports();
            loadPdbInfo();
            loadResources();
            loadTlsInformation();
            loadStrings();
            scanForAnomalies();
            InitializePeCoffSections();
        }
        ~PeFormat() = default;

        bool IsValid() const { if (pe_file) return true; else return false; }
        std::size_t getBytesPerWord() const;

        bool isDll() const;
        bool isExecutable() const;
        bool getMachineCode(unsigned long long &result) const;
        bool getAbiVersion(unsigned long long &result) const;
        bool getImageBaseAddress(unsigned long long &imageBase) const;
        bool getEpAddress(unsigned long long &result) const;
        unsigned int getBits() { if (IsValid()) return pe_file->getBits(); return 0; }
        Architecture getTargetArchitecture() const;
        std::size_t getDeclaredNumberOfSections() const;
        std::size_t getDeclaredNumberOfSegments() const;
        std::size_t getSectionTableOffset() const;
        std::size_t getSectionTableEntrySize() const;
        std::size_t getSegmentTableOffset() const;
        std::size_t getSegmentTableEntrySize() const;

        const PeLib::MzHeader & getMzHeader() const;
        std::size_t getMzHeaderSize() const;
        std::size_t getOptionalHeaderSize() const;
        std::size_t getPeHeaderOffset() const;
        std::size_t getCoffSymbolTableOffset() const;
        std::size_t getNumberOfCoffSymbols() const;
        std::size_t getSizeOfStringTable() const;
        std::size_t getMajorLinkerVersion() const;
        std::size_t getMinorLinkerVersion() const;
        std::size_t getFileFlags() const;
        std::size_t getTimeStamp() const;
        std::size_t getChecksum() const;
        std::size_t getFileAlignment() const;
        std::size_t getSectionAlignment() const;
        std::size_t getSizeOfHeaders() const;
        std::size_t getSizeOfImage() const;
        std::size_t getSizeOfStackReserve() const;
        std::size_t getSizeOfStackCommit() const;
        std::size_t getSizeOfHeapReserve() const;
        std::size_t getSizeOfHeapCommit() const;
        std::size_t getNumberOfDataDirectories() const;
        std::size_t getDeclaredNumberOfDataDirectories() const;

        const std::shared_ptr<Section> getEpSection();
        const std::unique_ptr<ImportTable>& getImportTable() { return importTable; }
        const std::unique_ptr<ExportTable>& getExportTable() { return exportTable; }

        bool isDotNet() const;
        bool isPackedDotNet() const;
        bool isVisualBasic(unsigned long long &version) const;
        bool getDllFlags(unsigned long long &dllFlags) const;
        bool getNumberOfBaseRelocationBlocks(unsigned long long &relocs) const;
        bool getNumberOfRelocations(unsigned long long &relocs) const;
        bool getDataDirectoryRelative(unsigned long long index, unsigned long long &relAddr, unsigned long long &size) const;
        bool getDataDirectoryAbsolute(unsigned long long index, unsigned long long &absAddr, unsigned long long &size) const;
        const std::shared_ptr<PeCoffSection> getPeSection(const std::string &secName) const;
        const std::shared_ptr<PeCoffSection> getPeSection(unsigned long long secIndex) const;
        const std::shared_ptr<PeCoffSection> getPeSection(std::uint8_t* address) const;
        unsigned long long getDecodableStartAddr(std::uint8_t* address) const;
        
        void scanForAnomalies() {
            scanForSectionAnomalies();
            scanForResourceAnomalies();
            scanForImportAnomalies();
            scanForExportAnomalies();
            scanForOptHeaderAnomalies();
        }

    private:
        static const std::map<std::size_t, std::string> resourceTypeMap;
        static const std::map<std::size_t, std::string> resourceLanguageMap;
        static const std::unordered_set<std::string> usualSectionNames;
        static const std::unordered_set<std::string> usualPackerSections;
        static const std::map<std::string, std::size_t> usualSectionCharacteristics;
        void loadSections();
        void loadSymbols() { return; }
        void loadImports();
        void loadExports();
        void loadPdbInfo() { return; }
        void loadResourceNodes(std::vector<const PeLib::ResourceChild*> &nodes, const std::vector<std::size_t> &levels);
        void loadResources();
        void loadCertificates() { return; }
        void loadTlsInformation();
        void loadStrings() { return; }
        void InitializePeCoffSections();

        void scanForSectionAnomalies();
        void scanForResourceAnomalies();
        void scanForImportAnomalies();
        void scanForExportAnomalies();
        void scanForOptHeaderAnomalies();
        std::unique_ptr<PeLib::PeFile> pe_file = nullptr;              ///< PeLib representation of PE file*/
        PeLib::MzHeader mzHeader;
        std::unique_ptr<ImportTable> importTable;                                         ///< table of imports
        std::unique_ptr<ExportTable> exportTable;                                         ///< table of exports
        std::unique_ptr<ResourceTable> resourceTable;                                     ///< table of resources
        std::unique_ptr<ResourceTree> resourceTree;                                       ///< structure of resource tree
        std::vector<std::shared_ptr<Section>> sections;                                   ///< file sections
        std::unique_ptr<TlsInfo> tlsInfo;                                 ///< thread-local information
        std::vector<std::pair<std::string, std::string>> anomalies;       ///< file format anomalies
        RangeContainer<std::uint64_t> nonDecodableRanges;                   ///< Address ranges which should not be decoded for instructions.

        unsigned long long getStoredNumberOfSections() const;
        bool peSectionName(std::string &name, unsigned long long sectionIndex) const;
        bool peSectionType(PeCoffSection::Type &secType, unsigned long long sectionIndex) const;
        bool getSection(unsigned long long secIndex, PeCoffSection &section) const;

        bool getImportedLibraryFileName(unsigned long long index, std::string &fileName) const;
        std::unique_ptr<Import> getImport(unsigned long long fileIndex, unsigned long long importIndex) const;
        bool getDelayImportedLibraryFileName(unsigned long long index, std::string &fileName) const;
        std::unique_ptr<Import> getDelayImport(unsigned long long fileIndex, unsigned long long importIndex) const;
        RangeContainer<std::uint64_t> getImportDirectoryOccupiedAddresses() const;

        unsigned long long getNumberOfExportedFunctions() const;
        std::string getNameOfExportTable() const;
        bool getExportedFunction(unsigned long long index, Export& exportedFunction) const;
        RangeContainer<std::uint64_t> getExportDirectoryOccupiedAddresses() const;

        const PeLib::ResourceNode* getResourceTreeRoot() const;
        bool getResourceNodes(std::vector<const PeLib::ResourceChild*> &nodes, std::vector<std::size_t> &levels);
        unsigned long long getResourceDirectoryOffset() const;
        RangeContainer<std::uint64_t> getResourceDirectoryOccupiedAddresses() const;

        unsigned long long getTlsStartAddressOfRawData() const;
        unsigned long long getTlsEndAddressOfRawData() const;
        unsigned long long getTlsAddressOfIndex() const;
        unsigned long long getTlsSizeOfZeroFill() const;
        unsigned long long getTlsCharacteristics() const;
        unsigned long long getTlsAddressOfCallBacks() const;

        const Import* getImport(unsigned long long address) const;
        const Export* getExport(unsigned long long address) const;

        bool isSizeOfHeaderMultipleOfFileAlignment() const;
    };
} // namespace cchips
