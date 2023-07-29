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
#include "RichHeader.h"
#include "VBHeader.h"
#include "CLRHeader.h"
#include "MetadataHeader.h"
#include "MetadataStream.h"
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

        enum class Endianness
        {
            UNKNOWN,
            LITTLE,
            BIG
        };

        using ExportDirectoryInfo = struct {
            uint32_t characteristics;
            uint32_t timestamp;
            uint16_t major_version;
            uint16_t minor_version;
            std::string name;
            uint32_t ordinal_base;
            uint32_t number_of_functions;
            uint32_t number_of_names;
            uint32_t address_of_functions;
            uint32_t address_of_names;
            uint32_t address_of_name_ordinals;
        };

        PeFormat() = delete;
        PeFormat(std::unique_ptr<PeLib::PeFile> file) : pe_file(std::move(file)) {
            if (!pe_file)
                return;
            pe_file->readMzHeader();
            pe_file->readPeHeader();
            pe_file->readImportDirectory();
            pe_file->readExportDirectory();
            pe_file->readIatDirectory();
            pe_file->readBoundImportDirectory();
            pe_file->readDelayImportDirectory();
            pe_file->readCoffSymbolTable();
            pe_file->readDebugDirectory();
            pe_file->readTlsDirectory();
            pe_file->readResourceDirectory();
            pe_file->readSecurityDirectory();
            pe_file->readComHeaderDirectory();
            mzHeader = pe_file->mzHeader();
            loadRichHeader();
            loadSections();
            loadSymbols();
            loadImports();
            loadExports();
            loadPdbInfo();
            loadResources();
            loadTlsInformation();
            loadDotnetHeaders();
            loadVisualBasicHeader();
            loadStrings();
            scanForAnomalies();
            InitializePeCoffSections();
        }
        ~PeFormat() = default;

        bool IsValid() const { if (pe_file) return true; else return false; }
        std::size_t getBytesPerWord() const;
        std::string getFileName() const {
            if (!IsValid()) return {};
            return pe_file->getFileName();
        }

        bool isDll() const;
        bool isExecutable() const;
        bool isSys() const;
        bool getMachineCode(unsigned long long &result) const;
        bool getAbiVersion(unsigned long long &result) const;
        bool getImageBaseAddress(unsigned long long &imageBase) const;
        bool getEpAddress(unsigned long long &result) const;
        bool isLoadingFile() const { if (pe_file) return pe_file->isLoadingFile(); else return false; }
        bool isLoadingMemory() const { if (pe_file) return pe_file->isLoadingMemory(); else return false; }
        unsigned int getBits() const { if (IsValid()) return pe_file->getBits(); return 0; }
        Architecture getTargetArchitecture() const;
        std::size_t getDeclaredNumberOfSections() const;
        std::size_t getDeclaredNumberOfSegments() const;
        std::size_t getDeclaredFileLength() const;
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
        const std::shared_ptr<PeCoffSection> getResSection();
        const std::unique_ptr<ImportTable>& getImportTable() { return importTable; }
        const std::unique_ptr<ExportTable>& getExportTable() { return exportTable; }
        const std::unique_ptr<ResourceTable>& getResourceTable() { return resourceTable; }

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
        const std::shared_ptr<PeCoffSection> getSectionByRva(unsigned long long rva) const;
        unsigned long long getDecodableStartAddr(std::uint8_t* address) const;
        std::size_t getLoadedFileLength() const;
        const unsigned char* getLoadedBytesData() const;
        bool getBytes(std::vector<unsigned char>& result, unsigned long long offset, unsigned long long numberOfBytes) const;
        bool getEpBytes(std::vector<unsigned char>& result, unsigned long long numberOfBytes) const;
        bool getHexBytes(std::string& result, unsigned long long offset, unsigned long long numberOfBytes) const;
        bool getHexEpBytes(std::string& result, unsigned long long numberOfBytes) const;
        bool getStringBytes(std::string& result, unsigned long long offset, unsigned long long numberOfBytes) const;
        bool getStringEpBytes(std::string& result, unsigned long long numberOfBytes) const;
        Endianness getEndianness() const;
        bool swapEndianness(std::string& str, std::size_t items, std::size_t length = 1) const;
        bool hexToLittle(std::string& str) const;
        bool getXByteOffset(std::uint64_t offset, std::uint64_t x, std::uint64_t& res, Endianness e);
        bool getXByte(std::uint64_t rva, std::uint64_t x, std::uint64_t& res, Endianness e) const;
        void checkOverlay();
        const std::vector<std::unique_ptr<PeLib::PdbInfo>>& getPdbInfo() const {
            return pdbInfolist;
        }
        
        void scanForAnomalies() {
            scanForSectionAnomalies();
            scanForResourceAnomalies();
            scanForImportAnomalies();
            scanForExportAnomalies();
            scanForOptHeaderAnomalies();
        }

        bool hasImportsTable() const;
        bool hasExportsTable() const;
        bool hasResources() const;
        bool hasSecurity() const;
        bool hasReloc() const;
        bool hasTls() const;
        bool hasBoundImportsTable() const;
        bool hasDelayImportsTable() const;
        bool hasDebug() const;
        std::string getSubsystemDesc() const;
        uint16_t getSubsystem() const;
        uint16_t getMinorOsVersion() const;
        uint16_t getMajorOsVersion() const;
        uint16_t getMinorSubsystemVersion() const;
        uint16_t getMajorSubsystemVersion() const;
        int32_t getPeHeaderStart() const;
        uint16_t getCharacteristics() const;
        uint16_t getDllCharacteristics() const;
        uint32_t getPeSignature() const;
        uint32_t getMagic() const;
        bool verifyChecksum() const;
        unsigned int getChecksumFileOffset() const;
        uint32_t getSizeofNtHeader() const;
        uint32_t getBaseOfCode() const;
        ExportDirectoryInfo get_export_directory_info() const;
        bool hasOverlay() const;
        double caculateEntropy();
        uint32_t getOverlaySize() const { return overlay_size; }
        uint64_t getOverlayOffset() const { return overlay_offset; }
        std::size_t getByteLength() const;
        std::size_t getNibbleLength() const;
        std::size_t getNumberOfNibblesInByte() const;
        std::size_t bytesFromNibblesRounded(std::size_t nibbles) const;
        std::size_t nibblesFromBytes(std::size_t bytes) const;
        std::size_t bytesFromNibbles(std::size_t nibbles) const;
        bool getStringFromEnd(std::string& result, unsigned long long numberOfBytes) const;
        const Resource* getVersionResource() const;
        const Resource* getManifestResource() const;
        std::uint64_t getValidRvaAddressFromFileAddress(std::uint64_t address) const;
        std::uint64_t getValidOffsetFromMemRva(std::uint64_t memaddr) const;
        std::uint32_t getValidOffsetFromRva(std::uint32_t rva) const;

        const Import* getImport(unsigned long long address) const;
        const Export* getExport(unsigned long long address) const;
        const Import* getImport(const std::string& func_name) const;
        const Export* getExport(const std::string& func_name) const;
        const PeLib::RichHeader& getRichHeader() const;

    private:
        static const std::map<std::size_t, std::string> resourceTypeMap;
        static const std::map<std::size_t, std::string> resourceLanguageMap;
        static const std::unordered_set<std::string> usualSectionNames;
        static const std::unordered_set<std::string> usualPackerSections;
        static const std::map<std::string, std::size_t> usualSectionCharacteristics;
        static const std::map<std::string, std::size_t> visualBasicLibrariesMap;
        static const std::vector<std::string> PeFormat::stubDatabase;
        static const std::size_t STANDARD_RICH_HEADER_OFFSET = 0x80;
        void loadRichHeader();
        void loadSections();
        void loadSymbols() { return; }
        void loadImports();
        void loadExports();
        void loadPdbInfo();
        void loadResourceNodes(std::vector<const PeLib::ResourceChild*> &nodes, const std::vector<std::size_t> &levels);
        void loadResources();
        void loadCertificates() { return; }
        void loadTlsInformation();
        void loadDotnetHeaders();
        void loadVisualBasicHeader();
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
        std::vector<std::unique_ptr<PeLib::PdbInfo>> pdbInfolist;                          ///< information about related PDB debug file
        VisualBasicInfo visualBasicInfo;                           ///< visual basic header information
        std::unique_ptr<CLRHeader> clrHeader;                      ///< .NET CLR header
        std::unique_ptr<MetadataHeader> metadataHeader;            ///< .NET metadata header
        std::unique_ptr<MetadataStream> metadataStream;            ///< .NET metadata stream
        std::unique_ptr<int> blobStream;                            ///< .NET blob stream
        std::unique_ptr<int> guidStream;                            ///< .NET GUID stream
        std::unique_ptr<int> stringStream;                          ///< .NET string stream
        std::unique_ptr<int> userStringStream;                      ///< .NET user string stream
        std::vector<std::pair<std::string, std::string>> anomalies;       ///< file format anomalies
        RangeContainer<std::uint64_t> nonDecodableRanges;                   ///< Address ranges which should not be decoded for instructions.
        bool has_overlay_data = false;
        uint64_t overlay_offset = 0;
        uint32_t overlay_size = 0;
        double entropy = 0;

        unsigned long long getStoredNumberOfSections() const;
        bool peSectionName(std::string &name, unsigned long long sectionIndex) const;
        bool peSectionType(PeCoffSection::Type &secType, unsigned long long sectionIndex) const;
        bool getSection(unsigned long long secIndex, PeCoffSection &section) const;
        std::size_t findDosStub(const std::string& plainFile);
        std::size_t getRichHeaderOffset(const std::string& plainFile);
        bool getImportedLibraryFileName(unsigned long long index, std::string &fileName) const;
        std::unique_ptr<Import> getImport(unsigned long long fileIndex, unsigned long long importIndex) const;
        bool getDelayImportedLibraryFileName(unsigned long long index, std::string &fileName) const;
        std::unique_ptr<Import> getDelayImport(unsigned long long fileIndex, unsigned long long importIndex) const;
        bool getBoundImportedLibraryFileName(unsigned long long index, std::string& fileName) const;
        std::unique_ptr<Import> getBoundImport(unsigned long long fileIndex, unsigned long long importIndex) const;
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
        bool getOffsetFromAddress(std::uint64_t& result, std::uint64_t address) const;
        bool parseVisualBasicProjectInfo(std::size_t structureOffset);
        bool parseVisualBasicComRegistrationData(std::size_t structureOffset);
        bool parseVisualBasicComRegistrationInfo(std::size_t structureOffset,
            std::size_t comRegDataOffset);
        bool parseVisualBasicExternTable(std::size_t structureOffset, std::size_t nEntries);
        bool parseVisualBasicObjectTable(std::size_t structureOffset);
        bool parseVisualBasicObjects(std::size_t structureOffset, std::size_t nObjects);
        bool getComDirectoryRelative(std::uint64_t& relAddr, std::uint64_t& size) const;
        std::unique_ptr<CLRHeader> getClrHeader() const;
        void parseMetadataStream(std::uint64_t baseAddress, std::uint64_t offset, std::uint64_t size) {}
        void parseBlobStream(std::uint64_t baseAddress, std::uint64_t offset, std::uint64_t size) {}
        void parseGuidStream(std::uint64_t baseAddress, std::uint64_t offset, std::uint64_t size) {}
        void parseStringStream(std::uint64_t baseAddress, std::uint64_t offset, std::uint64_t size) {}
        void parseUserStringStream(std::uint64_t baseAddress, std::uint64_t offset, std::uint64_t size) {}
        void detectModuleVersionId();
        void detectTypeLibId();
        void detectDotnetTypes();

        uint32_t calculateChecksum() const;

        using GetNByteFn = std::function<bool(
            std::uint64_t,
            std::uint64_t,
            std::uint64_t&,
            Endianness)>;
        bool getNTBSImpl(const GetNByteFn& get1ByteFn, std::uint64_t address, std::string& res, std::size_t size);
        bool getNTBSOffset(std::uint64_t offset, std::string& res, std::size_t size = 0);
        bool getNTBS(std::uint64_t address, std::string& res, std::size_t size = 0);

        bool isSizeOfHeaderMultipleOfFileAlignment() const;
        bool createValueFromBytes(
            const std::vector<std::uint8_t>& data,
            std::uint64_t& value,
            Endianness endian,
            std::uint64_t offset,
            std::uint64_t size) const;

        //Helper function to align number down
        template<typename T>
        static inline T align_down(T x, uint32_t align)
        {
            return x & ~(static_cast<T>(align) - 1);
        }

        //Helper function to align number up
        template<typename T>
        static inline T align_up(T x, uint32_t align)
        {
            return (x & static_cast<T>(align - 1)) ? align_down(x, align) + static_cast<T>(align) : x;
        }
    };
} // namespace cchips
