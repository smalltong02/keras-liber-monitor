#pragma once
#include <limits>
#include "Handler.h"


namespace cchips {
    class CCrossRefFeatureBuilder : public CPeInsideHandler
    {
    public:
        CCrossRefFeatureBuilder() = default;
        bool Initialize() override { return true; }
        ~CCrossRefFeatureBuilder() = default;

        bool Scan(std::unique_ptr<cchips::PeFormat>& pe_format, std::unique_ptr<cchips::CRapidJsonWrapper>& json_result) override;
    };

    struct DetectParams
    {
        enum class SearchType
        {
            EXACT_MATCH,   ///< only identical signatures
            MOST_SIMILAR,  ///< the most similar signature
            SIM_LIST       ///< list of similar signatures
        };

        SearchType searchType;  ///< type of search

        bool internal;  ///< use of internal signature database
        bool external;  ///< use of external signature database

        std::size_t epBytesCount;

const static std::size_t EP_BYTES_SIZE = 50;
        DetectParams(
            SearchType searchType_,
            bool internal_,
            bool external_,
            std::size_t epBytesCount_ = EP_BYTES_SIZE)
            : searchType(searchType_)
            , internal(internal_)
            , external(external_)
            , epBytesCount(epBytesCount_)
        {

        }
    };

    struct DetectResult
    {
        enum class ToolType
        {
            UNKNOWN,
            COMPILER,
            LINKER,
            INSTALLER,
            PACKER,
            OTHER
        };

        enum class DetectionMethod
        {
            UNKNOWN,             ///< unknown detection method
            COMBINED,            ///< combination of methods
            SIGNATURE,           ///< yara or slashed signature
            DWARF_DEBUG_H,       ///< DWARF debug information
            SECTION_TABLE_H,     ///< section table
            IMPORT_TABLE_H,      ///< import symbols
            EXPORT_TABLE_H,      ///< export symbols
            SYMBOL_TABLE_H,      ///< symbols
            LINKER_VERSION_H,    ///< linker version
            LINKED_LIBRARIES_H,  ///< specific libraries
            STRING_SEARCH_H,     ///< specific strings
            DYNAMIC_ENTRIES_H,   ///< .dynamic section
            COMMENT_H,           ///< .comment section
            NOTE_H,              ///< .note section
            MANIFEST_H,          ///< manifest resource
            HEADER_H,            ///< MZ header
            YARA_RULE,           ///< Heuristic detection by a YARA rule
            OTHER_H              ///< other heuristic
        };

        enum class DetectionStrength
        {
            LOW,
            MEDIUM,
            HIGH,
            SURE
        };

        ToolType type = ToolType::UNKNOWN;
        std::string name;
        std::string versionInfo;
        std::string additionalInfo;

        /// total number of significant nibbles
        unsigned long long impCount = 0;
        /// matched number of significant nibbles
        unsigned long long agreeCount = 0;

        /// detection type
        DetectionMethod source = DetectionMethod::UNKNOWN;
        /// detection strength
        DetectionStrength strength = DetectionStrength::LOW;

        bool isReliable() const {
            return source != DetectionMethod::UNKNOWN
                && strength > DetectionStrength::MEDIUM;
        }
        bool isCompiler() const {
            return type == ToolType::COMPILER;
        }
        bool isLinker() const {
            return type == ToolType::LINKER;
        }
        bool isInstaller() const {
            return type == ToolType::INSTALLER;
        }
        bool isPacker() const {
            return type == ToolType::PACKER;
        }
        bool isKnownType() const {
            return type != ToolType::UNKNOWN;
        }
        bool isUnknownType() const {
            return type == ToolType::UNKNOWN;
        }
    };

    struct DetectLanguage
    {
        bool bytecode = false;  /// < @c true if bytecode is detected

        std::string name;            ///< name of programming language
        std::string additionalInfo;  ///< some additional information
    };

    struct ToolInformation
    {
        enum class Packed
        {
            NOT_PACKED,
            PROBABLY_NO,
            PROBABLY_YES,
            PACKED
        };

        std::vector<std::string> errorMessages;
        std::vector<DetectResult> detectedTools;
        std::vector<DetectLanguage> detectedLanguages;

        bool entryPointOffset = false;
        std::uint64_t epOffset =
            std::numeric_limits<std::uint64_t>::max();

        bool entryPointAddress = false;
        std::uint64_t epAddress =
            std::numeric_limits<std::uint64_t>::max();
        std::uint64_t imageBase =
            std::numeric_limits<std::uint64_t>::max();

        uint64_t overlayOffset = 0;
        size_t overlaySize = 0;

        bool entryPointSection = false;
        cchips::Section epSection;
        std::string epBytes;

        void addTool(
            DetectResult::DetectionMethod source,
            DetectResult::DetectionStrength strength,
            DetectResult::ToolType toolType,
            const std::string& name,
            const std::string& version = "",
            const std::string& extra = "");
        void addTool(
            std::size_t matchNibbles,
            std::size_t totalNibbles,
            DetectResult::ToolType toolType,
            const std::string& name,
            const std::string& version = "",
            const std::string& extra = "");
        void addLanguage(
            const std::string& name,
            const std::string& extra = "",
            bool bytecode = false);

        bool isReliableResult(std::size_t resultIndex) const;
        bool hasReliableResult() const;
        Packed isPacked() const;
    };

    class Search
    {
    public:
        struct Similarity
        {
            unsigned long long same = 0;   ///< matched number of significant nibbles
            unsigned long long total = 0;  ///< total number of significant nibbles
            double ratio = 0.0;              ///< @a same divided by @a total
        };

        class RelativeJump
        {
        private:
            std::string slash;
            std::size_t bytesAfter;
        public:
            RelativeJump(std::string sSlash, std::size_t sBytesAfter);
            std::string getSlash() const;
            std::size_t getSlashNibbleSize() const;
            std::size_t getBytesAfter() const;
            const static std::map<PeFormat::Architecture, std::vector<Search::RelativeJump>> jumpMap;
        };
    private:
        std::unique_ptr<cchips::PeFormat>& m_pe_format;
        std::string nibbles;
        std::string plain;
        std::vector<RelativeJump> jumps;
        std::size_t averageSlashLen;
        bool fileLoaded;
        bool fileSupported;
        bool haveSlashes() const;
        std::size_t nibblesFromBytes(std::size_t nBytes) const;
        std::size_t bytesFromNibbles(std::size_t nNibbles) const;
        /// @}
    public:
        Search(std::unique_ptr<cchips::PeFormat>& pe_format);
        bool isFileLoaded() const { return fileLoaded; }
        bool isFileSupported() const { return fileSupported; }
        const std::string& getNibbles() const { return nibbles; }
        const std::string&  getPlainString() const { return plain; }
        const RelativeJump* getRelativeJump(
            std::size_t fileOffset,
            std::size_t shift,
            std::int64_t& moveSize) const;
        unsigned long long countImpNibbles(
            const std::string& signPattern) const;
        unsigned long long findUnslashedSignature(
            const std::string& signPattern,
            std::size_t startOffset,
            std::size_t stopOffset) const;
        unsigned long long findSlashedSignature(
            const std::string& signPattern,
            std::size_t startOffset,
            std::size_t stopOffset) const;
        unsigned long long exactComparison(
            const std::string& signPattern,
            std::size_t fileOffset,
            std::size_t shift = 0) const;
        bool countSimilarity(
            const std::string& signPattern,
            Similarity& sim,
            std::size_t fileOffset,
            std::size_t shift = 0) const;
        bool areaSimilarity(
            const std::string& signPattern,
            Similarity& sim,
            std::size_t startOffset,
            std::size_t stopOffset) const;
        bool hasString(const std::string& str) const { return contains(plain, str); }
        bool hasString(const std::string& str, std::size_t fileOffset) const { return hasSubstringOnPosition(plain, str, fileOffset); }
        bool hasString(
            const std::string& str,
            std::size_t startOffset,
            std::size_t stopOffset) const {
            return hasSubstringInArea(plain, str, startOffset, stopOffset);
        }
        bool hasStringInSection(
            const std::string& str,
            const std::shared_ptr<cchips::PeCoffSection> section) const {
            return section && hasString(
                str,
                section->getOffset(),
                section->getOffset() + section->getLoadedSize() - 1);
        }
        bool hasStringInSection(
            const std::string& str,
            std::size_t sectionIndex) const {
            if (auto section = m_pe_format->getPeSection(sectionIndex); section) {
                return hasStringInSection(str, section);
            }
            return false;
        }
        bool hasStringInSection(
            const std::string& str,
            const std::string& sectionName) const {
            if (auto section = m_pe_format->getPeSection(sectionName); section) {
                return hasStringInSection(str, section);
            }
            return false;
        }
        bool createSignature(
            std::string& pattern,
            std::size_t fileOffset,
            std::size_t size) const;
    };

    class Heuristics
    {
    private:
        void getSectionHeuristics();
        bool parseGccComment(const std::string& record);
        bool parseGhcComment(const std::string& record);
        bool parseOpen64Comment(const std::string& record);
        void getCommentSectionsHeuristics();
        bool parseGccProducer(const std::string& producer);
        bool parseClangProducer(const std::string& producer);
        bool parseTmsProducer(const std::string& producer);
        void getDwarfInfo();
        std::string getEmbarcaderoVersion();
        void getEmbarcaderoHeuristics();
        void getSymbolHeuristic();
        void getCommonToolsHeuristics();
        void getCommonLanguageHeuristics() { ; }
        std::string embarcaderoVersionToExtra(const std::string& version);
        std::string commentSectionNameByFormat(unsigned int format);

        static const std::vector<std::pair<std::string, std::size_t>> delphiStrings;
        static const std::map<std::string, std::string> delphiVersionMap;

    protected:
        std::unique_ptr<cchips::PeFormat>& pe_format;
        Search& search;
        bool canSearch;
        ToolInformation& toolInfo;

        std::vector<std::shared_ptr<cchips::PeCoffSection>> sections;
        std::map<std::string, std::size_t> sectionNameMap;
        std::size_t noOfSections;

        bool priorityLanguageIsSet = false;

        std::string getUpxVersion();
        const DetectResult* isDetected(
            const std::string& name,
            const DetectResult::DetectionStrength minStrength = DetectResult::DetectionStrength::LOW);

        virtual void getFormatSpecificCompilerHeuristics() { ; }
        virtual void getFormatSpecificLanguageHeuristics() { ; }

        void addCompiler(
            DetectResult::DetectionMethod source,
            DetectResult::DetectionStrength strength,
            const std::string& name,
            const std::string& version = "",
            const std::string& extra = "") {
            toolInfo.addTool(source, strength, DetectResult::ToolType::COMPILER, name, version, extra);
        }
        void addLinker(
            DetectResult::DetectionMethod source,
            DetectResult::DetectionStrength strength,
            const std::string& name,
            const std::string& version = "",
            const std::string& extra = "") {
            toolInfo.addTool(source, strength, DetectResult::ToolType::LINKER, name, version, extra);
        }
        void addInstaller(
            DetectResult::DetectionMethod source,
            DetectResult::DetectionStrength strength,
            const std::string& name,
            const std::string& version = "",
            const std::string& extra = "") {
            toolInfo.addTool(source, strength, DetectResult::ToolType::INSTALLER, name, version, extra);
        }
        void addPacker(
            DetectResult::DetectionMethod source,
            DetectResult::DetectionStrength strength,
            const std::string& name,
            const std::string& version = "",
            const std::string& extra = "") {
            toolInfo.addTool(source, strength, DetectResult::ToolType::PACKER, name, version, extra);
        }

        void addCompiler(
            std::size_t matchNibbles,
            std::size_t totalNibbles,
            const std::string& name,
            const std::string& version = "",
            const std::string& extra = "") {
            toolInfo.addTool(matchNibbles, totalNibbles, DetectResult::ToolType::COMPILER, name, version, extra);
        }
        void addPacker(
            std::size_t matchNibbles,
            std::size_t totalNibbles,
            const std::string& name,
            const std::string& version = "",
            const std::string& extra = "") {
            toolInfo.addTool(matchNibbles, totalNibbles, DetectResult::ToolType::PACKER, name, version, extra);
        }
        void addLanguage(
            const std::string& name,
            const std::string& extraInfo = "",
            bool isBytecode = false) {
            if (priorityLanguageIsSet)
            {
                return;
            }

            toolInfo.addLanguage(name, extraInfo, isBytecode);
        }
        void addPriorityLanguage(
            const std::string& name,
            const std::string& extraInfo = "",
            bool isBytecode = false) {
            if (priorityLanguageIsSet)
            {
                return;
            }

            priorityLanguageIsSet = true;
            toolInfo.detectedLanguages.clear();
            toolInfo.addLanguage(name, extraInfo, isBytecode);
        }

        std::size_t findSectionName(const std::string& sectionName) const {
            return mapGetValueOrDefault(sectionNameMap, sectionName, 0);
        }
        std::size_t findSectionNameStart(const std::string& sectionName) const {
            std::size_t result = 0;
            for (const auto section : sections)
            {
                std::string name = section->getName();
                if (startsWith(name, sectionName))
                {
                    result++;
                }
            }

            return result;
        }

    public:
        const std::size_t MINIMUM_GHC_RECORD_SIZE = 9;

        Heuristics(
            std::unique_ptr<cchips::PeFormat>& pe_format, Search& searcher,
            ToolInformation& toolInfo);
        virtual ~Heuristics() = default;

        void getAllHeuristics() {
            // Detect languages
            getCommonLanguageHeuristics();
            getFormatSpecificLanguageHeuristics();

            // Detect compilers
            getCommonToolsHeuristics();
            getFormatSpecificCompilerHeuristics();
        }
    };

    class Signature
    {
    public:
        std::string name;       ///< name of used tool
        std::string version;    ///< version of used tool
        std::string pattern;    ///< signature pattern
        std::string additional; ///< additional information about tool
        unsigned startOffset = 0;   ///< start offset of pattern
        unsigned endOffset = 0;     ///< end offset of pattern

        Signature(
            std::string sName,
            std::string sVersion,
            std::string sPattern,
            std::string sAdditional = "",
            unsigned sStart = 0,
            unsigned sEnd = 0
        ) : name(sName), version(sVersion), pattern(sPattern), additional(sAdditional), startOffset(sStart), endOffset(sEnd) {}

        static bool isValidSignaturePattern(const std::string& pattern);
    };

    class PeHeuristics : public Heuristics
    {
    public:
        struct PeHeaderStyle
        {
            // Note: Having "(const) std::string" instead of "const char *" here
            // makes MS Visual Studio 2017 compiler (v 15.9.8) exit with
            // "fatal error C1001: An internal error has occurred in the compiler"
            // const std::string headerStyle;
            const char* headerStyle;
            uint16_t headerWords[0x0D];
        };

        PeHeuristics(
            std::unique_ptr<cchips::PeFormat>& pe_format, Search& searcher,
            ToolInformation& toolInfo)
            : Heuristics(pe_format, searcher, toolInfo)
            , m_pe_format(pe_format)
            , declaredLength(m_pe_format->getDeclaredFileLength())
            , loadedLength(m_pe_format->getLoadedFileLength())
        {}

    private:
        std::unique_ptr<cchips::PeFormat>& m_pe_format; ///< parser of input PE file

        std::size_t declaredLength; ///< declared length of file
        std::size_t loadedLength;   ///< actual loaded length of file

        /// @name Auxiliary methods
        /// @{
        std::string getEnigmaVersion();
        std::string getUpxAdditionalInfo(std::size_t metadataPos);
        /// @}

        /// @name Heuristics for detection of original language
        /// @{
        void getGoHeuristics();
        void getAutoItHeuristics();
        void getDotNetHeuristics();
        void getVisualBasicHeuristics();
        /// @}

        /// @name Heuristics for detection of used compiler or packer
        /// @{
        std::int32_t getInt32Unaligned(const std::uint8_t* codePtr);
        const std::uint8_t* skip_NOP_JMP8_JMP32(
            const std::uint8_t* codeBegin,
            const std::uint8_t* codePtr,
            const std::uint8_t* codeEnd,
            std::size_t maxCount);
        void getHeaderStyleHeuristics();
        void getSlashedSignatures();
        void getMorphineHeuristics();
        void getStarForceHeuristics();
        void getSafeDiscHeuristics();
        bool checkSecuROMSignature(
            const char* fileData,
            const char* fileDataEnd,
            uint32_t FileOffset);
        void getSecuROMHeuristics();
        void getMPRMMGVAHeuristics();
        void getActiveMarkHeuristics();
        void getRLPackHeuristics();
        void getPetiteHeuristics();
        void getPelockHeuristics();
        void getEzirizReactorHeuristics();
        void getUpxHeuristics();
        void getFsgHeuristics();
        void getPeCompactHeuristics();
        void getAndpakkHeuristics();
        void getEnigmaHeuristics();
        void getVBoxHeuristics();
        void getActiveDeliveryHeuristics();
        void getAdeptProtectorHeuristics();
        void getCodeLockHeuristics();
        void getNetHeuristic();
        void getExcelsiorHeuristics();
        void getVmProtectHeuristics();
        void getBorlandDelphiHeuristics();
        void getBeRoHeuristics();
        void getMsvcIntelHeuristics();
        void getArmadilloHeuristic();
        void getStarforceHeuristic();
        void getLinkerVersionHeuristic();
        void getRdataHeuristic();
        void getNullsoftHeuristic();
        void getManifestHeuristic();
        void getSevenZipHeuristics();
        void getMewSectionHeuristics();
        void getNsPackSectionHeuristics();
        void getPeSectionHeuristics();
        /// @}
        std::string getNullsoftManifestVersion(const std::string& manifest);

        static const std::vector<PeHeaderStyle> headerStyles;
        static const std::size_t LIGHTWEIGHT_FILE_SCAN_AREA = 0x80000;
        static const std::vector<Signature> x86SlashedSignatures;
        static const std::map<char, std::string> peCompactMap;
        static const std::vector<std::string> enigmaPatterns;
        static const std::vector<std::string> dotNetShrinkPatterns;
        static const std::string msvcRuntimeString;
        static const std::vector<std::string> msvcRuntimeStrings;
    protected:
        /// @name Virtual methods
        /// @{
        virtual void getFormatSpecificCompilerHeuristics() override;
        virtual void getFormatSpecificLanguageHeuristics() override;
        /// @}
    };

    class CompilerDetector : private NonCopyable
    {
    public:
        enum class ReturnCode
        {
            return_ok,
            return_arg,
            return_file_not_exist,
            return_file_problem,
            return_entry_point_detection,
            return_unknown_format,
            return_format_parser_problem,
            return_macho_ar_detected,
            return_archive_detected,
            return_unknown_cp,
        };

        CompilerDetector(
            std::unique_ptr<cchips::PeFormat>& pe_format,
            DetectParams& params,
            ToolInformation& toolInfo);

        ReturnCode GetAllInformation();

        static const std::set<std::string> _external_database_suffixes;

    protected:
        ToolInformation& m_toolinfo;
        cchips::PeFormat::Architecture m_architecture;
        cchips::Search m_search;
        std::unique_ptr<Heuristics> m_heuristics;
        std::vector<std::string> m_internal_paths;
        fs::path m_path_to_shared;
        std::set<std::string> m_external_suffixes;

    private:
        std::unique_ptr<cchips::PeFormat>& m_pe_format;
        DetectParams& m_detect_params;
        std::vector<std::string> m_external_db;

        static bool compareExtraInfo(const DetectResult& a, const DetectResult& b, bool& result);
        static bool compareForSort(const DetectResult& a, const DetectResult& b);
        bool getExternalDatabases();
        void removeCompilersWithLessSimilarity(double ratio);
        void removeUnusedCompilers();
        void getAllHeuristics();
        ReturnCode getAllSignatures();
        ReturnCode getAllCompilers();
    };
} // namespace cchips
