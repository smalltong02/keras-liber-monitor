#include <Windows.h>
#include <string>
#include <cctype>
#include <iomanip>
#include "PeFormat.h"

namespace cchips {

    const std::map<std::size_t, std::string> PeFormat::resourceTypeMap
    {
        {PeLib::PELIB_RT_CURSOR, "Cursor"},
        {PeLib::PELIB_RT_BITMAP, "Bitmap"},
        {PeLib::PELIB_RT_ICON, "Icon"},
        {PeLib::PELIB_RT_MENU, "Menu"},
        {PeLib::PELIB_RT_DIALOG, "Dialog box"},
        {PeLib::PELIB_RT_STRING, "String-table entry"},
        {PeLib::PELIB_RT_FONTDIR, "Font directory"},
        {PeLib::PELIB_RT_FONT, "Font"},
        {PeLib::PELIB_RT_ACCELERATOR, "Accelerator table"},
        {PeLib::PELIB_RT_RCDATA, "Raw data"},
        {PeLib::PELIB_RT_MESSAGETABLE, "Message-table entry"},
        {PeLib::PELIB_RT_GROUP_CURSOR, "Cursor Group"},
        {PeLib::PELIB_RT_GROUP_ICON, "Icon Group"},
        {PeLib::PELIB_RT_VERSION, "Version"},
        {PeLib::PELIB_RT_DLGINCLUDE, "Include"},
        {PeLib::PELIB_RT_PLUGPLAY, "Plug and Play"},
        {PeLib::PELIB_RT_VXD, "VXD"},
        {PeLib::PELIB_RT_ANICURSOR, "Animated cursor"},
        {PeLib::PELIB_RT_ANIICON, "Animated icon"},
        {PeLib::PELIB_RT_HTML, "HTML"},
        {PeLib::PELIB_RT_MANIFEST, "Manifest"},
        {PeLib::PELIB_RT_DLGINIT, "Dialog box init"},
        {PeLib::PELIB_RT_TOOLBAR, "Toolbar"}
    };

    const std::map<std::size_t, std::string> PeFormat::resourceLanguageMap
    {
        {PeLib::PELIB_LANG_NEUTRAL, "Neutral"},
        {PeLib::PELIB_LANG_ARABIC, "Arabic"},
        {PeLib::PELIB_LANG_BULGARIAN, "Bulgarian"},
        {PeLib::PELIB_LANG_CATALAN, "Catalan"},
        {PeLib::PELIB_LANG_CHINESE, "Chinese"},
        {PeLib::PELIB_LANG_CZECH, "Czech"},
        {PeLib::PELIB_LANG_DANISH, "Danish"},
        {PeLib::PELIB_LANG_GERMAN, "German"},
        {PeLib::PELIB_LANG_GREEK, "Greek"},
        {PeLib::PELIB_LANG_ENGLISH, "English"},
        {PeLib::PELIB_LANG_SPANISH, "Spanish"},
        {PeLib::PELIB_LANG_FINNISH, "Finnish"},
        {PeLib::PELIB_LANG_FRENCH, "French"},
        {PeLib::PELIB_LANG_HEBREW, "Hebrew"},
        {PeLib::PELIB_LANG_HUNGARIAN, "Hungarian"},
        {PeLib::PELIB_LANG_ICELANDIC, "Icelandic"},
        {PeLib::PELIB_LANG_ITALIAN, "Italian"},
        {PeLib::PELIB_LANG_JAPANESE, "Japanese"},
        {PeLib::PELIB_LANG_KOREAN, "Korean"},
        {PeLib::PELIB_LANG_DUTCH, "Dutch"},
        {PeLib::PELIB_LANG_NORWEGIAN, "Norwegian"},
        {PeLib::PELIB_LANG_POLISH, "Polish"},
        {PeLib::PELIB_LANG_PORTUGUESE, "Portuguese"},
        {PeLib::PELIB_LANG_ROMANIAN, "Romanian"},
        {PeLib::PELIB_LANG_RUSSIAN, "Russian"},
        {PeLib::PELIB_LANG_CROATIAN, "Croatian"},
        {PeLib::PELIB_LANG_SERBIAN_NEUTRAL, "Serbian"},
        {PeLib::PELIB_LANG_BOSNIAN_NEUTRAL, "Bosnian"},
        {PeLib::PELIB_LANG_SLOVAK, "Slovak"},
        {PeLib::PELIB_LANG_ALBANIAN, "Albanian"},
        {PeLib::PELIB_LANG_SWEDISH, "Swedish"},
        {PeLib::PELIB_LANG_THAI, "Thai"},
        {PeLib::PELIB_LANG_TURKISH, "Turkish"},
        {PeLib::PELIB_LANG_URDU, "Urdu"},
        {PeLib::PELIB_LANG_INDONESIAN, "Indonesian"},
        {PeLib::PELIB_LANG_UKRAINIAN, "Ukrainian"},
        {PeLib::PELIB_LANG_BELARUSIAN, "Belarusian"},
        {PeLib::PELIB_LANG_SLOVENIAN, "Slovenian"},
        {PeLib::PELIB_LANG_ESTONIAN, "Estonian"},
        {PeLib::PELIB_LANG_LATVIAN, "Latvian"},
        {PeLib::PELIB_LANG_LITHUANIAN, "Lithuanian"},
        {PeLib::PELIB_LANG_PERSIAN, "Persian"},
        {PeLib::PELIB_LANG_VIETNAMESE, "Vietnamese"},
        {PeLib::PELIB_LANG_ARMENIAN, "Armenian"},
        {PeLib::PELIB_LANG_AZERI, "Azeri"},
        {PeLib::PELIB_LANG_BASQUE, "Basque"},
        {PeLib::PELIB_LANG_SORBIAN, "Sorbian"},
        {PeLib::PELIB_LANG_MACEDONIAN, "Macedonian"},
        {PeLib::PELIB_LANG_TSWANA, "Tswana"},
        {PeLib::PELIB_LANG_XHOSA, "Xhosa"},
        {PeLib::PELIB_LANG_ZULU, "Zulu"},
        {PeLib::PELIB_LANG_AFRIKAANS, "Afrikaans"},
        {PeLib::PELIB_LANG_GEORGIAN, "Georgian"},
        {PeLib::PELIB_LANG_FAEROESE, "Faeroese"},
        {PeLib::PELIB_LANG_HINDI, "Hindi"},
        {PeLib::PELIB_LANG_MALTESE, "Maltese"},
        {PeLib::PELIB_LANG_SAMI, "Sami"},
        {PeLib::PELIB_LANG_IRISH, "Irish"},
        {PeLib::PELIB_LANG_MALAY, "Malay"},
        {PeLib::PELIB_LANG_KAZAK, "Kazak"},
        {PeLib::PELIB_LANG_KYRGYZ, "Kyrgyz"},
        {PeLib::PELIB_LANG_SWAHILI, "Swahili"},
        {PeLib::PELIB_LANG_UZBEK, "Uzbek"},
        {PeLib::PELIB_LANG_TATAR, "Tatar"},
        {PeLib::PELIB_LANG_BENGALI, "Bengali"},
        {PeLib::PELIB_LANG_PUNJABI, "Punjabi"},
        {PeLib::PELIB_LANG_GUJARATI, "Gujarati"},
        {PeLib::PELIB_LANG_ORIYA, "Oriya"},
        {PeLib::PELIB_LANG_TAMIL, "Tamil"},
        {PeLib::PELIB_LANG_TELUGU, "Telugu"},
        {PeLib::PELIB_LANG_KANNADA, "Kannada"},
        {PeLib::PELIB_LANG_MALAYALAM, "Malayalam"},
        {PeLib::PELIB_LANG_ASSAMESE, "Assamese"},
        {PeLib::PELIB_LANG_MARATHI, "Marathi"},
        {PeLib::PELIB_LANG_SANSKRIT, "Sanskrit"},
        {PeLib::PELIB_LANG_MONGOLIAN, "Mongolian"},
        {PeLib::PELIB_LANG_TIBETAN, "Tibetan"},
        {PeLib::PELIB_LANG_WELSH, "Welsh"},
        {PeLib::PELIB_LANG_KHMER, "Khmer"},
        {PeLib::PELIB_LANG_LAO, "Lao"},
        {PeLib::PELIB_LANG_GALICIAN, "Galician"},
        {PeLib::PELIB_LANG_KONKANI, "Konkani"},
        {PeLib::PELIB_LANG_MANIPURI, "Manipuri"},
        {PeLib::PELIB_LANG_SINDHI, "Sindhi"},
        {PeLib::PELIB_LANG_SYRIAC, "Syriac"},
        {PeLib::PELIB_LANG_SINHALESE, "Sinhalese"},
        {PeLib::PELIB_LANG_INUKTITUT, "Inuktitut"},
        {PeLib::PELIB_LANG_AMHARIC, "Amharic"},
        {PeLib::PELIB_LANG_TAMAZIGHT, "Tamazight"},
        {PeLib::PELIB_LANG_KASHMIRI, "Kashmiri"},
        {PeLib::PELIB_LANG_NEPALI, "Nepali"},
        {PeLib::PELIB_LANG_FRISIAN, "Frisian"},
        {PeLib::PELIB_LANG_PASHTO, "Pashto"},
        {PeLib::PELIB_LANG_FILIPINO, "Filipino"},
        {PeLib::PELIB_LANG_DIVEHI, "Divehi"},
        {PeLib::PELIB_LANG_HAUSA, "Hausa"},
        {PeLib::PELIB_LANG_YORUBA, "Yoruba"},
        {PeLib::PELIB_LANG_QUECHUA, "Quechua"},
        {PeLib::PELIB_LANG_SOTHO, "Sotho"},
        {PeLib::PELIB_LANG_BASHKIR, "Bashkir"},
        {PeLib::PELIB_LANG_LUXEMBOURGISH, "Luxembourgish"},
        {PeLib::PELIB_LANG_GREENLANDIC, "Greenlandic"},
        {PeLib::PELIB_LANG_IGBO, "Igbo"},
        {PeLib::PELIB_LANG_TIGRIGNA, "Tigrigna"},
        {PeLib::PELIB_LANG_YI, "Yi"},
        {PeLib::PELIB_LANG_MAPUDUNGUN, "Mapudungun"},
        {PeLib::PELIB_LANG_MOHAWK, "Mohawk"},
        {PeLib::PELIB_LANG_BRETON, "Breton"},
        {PeLib::PELIB_LANG_INVARIANT, "Invariant"},
        {PeLib::PELIB_LANG_UIGHUR, "Uighur"},
        {PeLib::PELIB_LANG_MAORI, "Maori"},
        {PeLib::PELIB_LANG_OCCITAN, "Occitan"},
        {PeLib::PELIB_LANG_CORSICAN, "Corsican"},
        {PeLib::PELIB_LANG_ALSATIAN, "Alsatian"},
        {PeLib::PELIB_LANG_YAKUT, "Yakut"},
        {PeLib::PELIB_LANG_KICHE, "Kiche"},
        {PeLib::PELIB_LANG_KINYARWANDA, "Kinyarwanda"},
        {PeLib::PELIB_LANG_WOLOF, "Wolof"},
        {PeLib::PELIB_LANG_DARI, "Dari"},
        {PeLib::PELIB_LANG_MALAGASY, "Malagasy"}
    };

    const std::unordered_set<std::string> PeFormat::usualSectionNames
    {
        ".00cfg",
        ".BSS",
        ".CLR_UEF",
        ".CRT",
        ".DATA",
        ".apiset",
        ".arch",
        ".autoload_text",
        ".bindat",
        ".bootdat",
        ".bss",
        ".buildid",
        ".code",
        ".complua",
        ".cormeta",
        ".cygwin_dll_common",
        ".data",
        ".data1",
        ".data2",
        ".data3",
        ".debug  $F",
        ".debug  $P",
        ".debug  $S",
        ".debug  $T",
        ".debug",
        ".didat",
        ".didata",
        ".drectve ",
        ".edata",
        ".eh_fram",
        ".export",
        ".fasm",
        ".flat",
        ".gfids",
        ".giats",
        ".gljmp",
        ".glue_7",
        ".glue_7t",
        ".idata",
        ".idlsym",
        ".impdata",
        ".import",
        ".itext",
        ".ndata",
        ".orpc",
        ".pdata",
        ".rdata",
        ".reloc",
        ".rodata",
        ".rsrc",
        ".sbss",
        ".script",
        ".sdata",
        ".shared",
        ".srdata",
        ".stab",
        ".stabstr",
        ".sxdata",
        ".text",
        ".text0",
        ".text1",
        ".text2",
        ".text3",
        ".textbss",
        ".tls",
        ".tls$",
        ".udata",
        ".vsdata",
        ".wixburn",
        ".wpp_sf",
        ".xdata",
        "BSS",
        "CODE",
        "DATA",
        "DGROUP",
        "INIT",
        "PAGE",
        "Shared",
        "edata",
        "idata",
        "minATL",
        "rdata",
        "sdata",
        "shared",
        "testdata",
        "text"
    };

    const std::unordered_set<std::string> PeFormat::usualPackerSections
    {
        "!EPack",
        ".ASPack",
        ".ByDwing",
        ".MPRESS1",
        ".MPRESS2",
        ".MaskPE",
        ".RLPack",
        ".RPCrypt",
        ".Themida",
        ".UPX0",
        ".UPX1",
        ".UPX2",
        ".Upack",
        ".WWP32",
        ".WWPACK",
        ".adata",
        ".aspack",
        ".boom",
        ".ccg",
        ".charmve",
        ".ecode",
        ".edata",
        ".enigma1",
        ".enigma2",
        ".gentee",
        ".mackt",
        ".mnbvcx1",
        ".mnbvcx2",
        ".neolit",
        ".neolite",
        ".nsp0",
        ".nsp1",
        ".nsp2",
        ".packed",
        ".perplex",
        ".petite",
        ".pinclie",
        ".rmnet",
        ".seau",
        ".sforce3",
        ".shrink1",
        ".shrink2",
        ".shrink3",
        ".spack",
        ".svkp",
        ".taz",
        ".tsuarch",
        ".tsustub",
        ".vmp0",
        ".vmp1",
        ".vmp2",
        ".winapi",
        ".y0da"
        ".yP",
        "ASPack",
        "BitArts",
        "DAStub",
        "FSG!",
        "MEW",
        "PEBundle",
        "PEC2",
        "PEC2MO",
        "PEC2TO",
        "PECompact2",
        "PELOCKnt",
        "PEPACK!!",
        "PESHiELD",
        "ProCrypt",
        "RCryptor",
        "Themida",
        "UPX!",
        "UPX0",
        "UPX1",
        "UPX2",
        "UPX3",
        "VProtect",
        "WinLicen",
        "_winzip_",
        "kkrunchy",
        "lz32.dll",
        "nsp0",
        "nsp1",
        "nsp2",
        "pebundle",
        "pec",
        "pec1",
        "pec2",
        "pec3",
        "pec4",
        "pec5",
        "pec6",
    };

    const std::map<std::string, std::size_t> PeFormat::usualSectionCharacteristics
    {
        {".bss", (PeLib::PELIB_IMAGE_SCN_CNT_UNINITIALIZED_DATA | PeLib::PELIB_IMAGE_SCN_MEM_READ | PeLib::PELIB_IMAGE_SCN_MEM_WRITE)},
        {".cormeta", PeLib::PELIB_IMAGE_SCN_LNK_INFO},
        {".data", (PeLib::PELIB_IMAGE_SCN_CNT_INITIALIZED_DATA | PeLib::PELIB_IMAGE_SCN_MEM_READ | PeLib::PELIB_IMAGE_SCN_MEM_WRITE)},
        {".debug", (PeLib::PELIB_IMAGE_SCN_CNT_INITIALIZED_DATA | PeLib::PELIB_IMAGE_SCN_MEM_READ | PeLib::PELIB_IMAGE_SCN_MEM_DISCARDABLE)},
        {".drective", PeLib::PELIB_IMAGE_SCN_LNK_INFO},
        {".edata", (PeLib::PELIB_IMAGE_SCN_CNT_INITIALIZED_DATA | PeLib::PELIB_IMAGE_SCN_MEM_READ)},
        {".idata", (PeLib::PELIB_IMAGE_SCN_CNT_INITIALIZED_DATA | PeLib::PELIB_IMAGE_SCN_MEM_READ | PeLib::PELIB_IMAGE_SCN_MEM_WRITE)},
        {".idlsym", PeLib::PELIB_IMAGE_SCN_LNK_INFO},
        {".pdata", (PeLib::PELIB_IMAGE_SCN_CNT_INITIALIZED_DATA | PeLib::PELIB_IMAGE_SCN_MEM_READ)},
        {".rdata", (PeLib::PELIB_IMAGE_SCN_CNT_INITIALIZED_DATA | PeLib::PELIB_IMAGE_SCN_MEM_READ)},
        {".reloc", (PeLib::PELIB_IMAGE_SCN_CNT_INITIALIZED_DATA | PeLib::PELIB_IMAGE_SCN_MEM_READ | PeLib::PELIB_IMAGE_SCN_MEM_DISCARDABLE)},
        {".rsrc", (PeLib::PELIB_IMAGE_SCN_CNT_INITIALIZED_DATA | PeLib::PELIB_IMAGE_SCN_MEM_READ)},
        {".sbss", (PeLib::PELIB_IMAGE_SCN_CNT_UNINITIALIZED_DATA | PeLib::PELIB_IMAGE_SCN_MEM_READ | PeLib::PELIB_IMAGE_SCN_MEM_WRITE)},
        {".sdata", (PeLib::PELIB_IMAGE_SCN_CNT_INITIALIZED_DATA | PeLib::PELIB_IMAGE_SCN_MEM_READ | PeLib::PELIB_IMAGE_SCN_MEM_WRITE)},
        {".srdata", (PeLib::PELIB_IMAGE_SCN_CNT_INITIALIZED_DATA | PeLib::PELIB_IMAGE_SCN_MEM_READ)},
        {".sxdata", PeLib::PELIB_IMAGE_SCN_LNK_INFO},
        {".text", (PeLib::PELIB_IMAGE_SCN_CNT_CODE | PeLib::PELIB_IMAGE_SCN_MEM_EXECUTE | PeLib::PELIB_IMAGE_SCN_MEM_READ)},
        {".tls", (PeLib::PELIB_IMAGE_SCN_CNT_INITIALIZED_DATA | PeLib::PELIB_IMAGE_SCN_MEM_READ | PeLib::PELIB_IMAGE_SCN_MEM_WRITE)},
        {".tls$", (PeLib::PELIB_IMAGE_SCN_CNT_INITIALIZED_DATA | PeLib::PELIB_IMAGE_SCN_MEM_READ | PeLib::PELIB_IMAGE_SCN_MEM_WRITE)},
        {".vsdata", (PeLib::PELIB_IMAGE_SCN_CNT_INITIALIZED_DATA | PeLib::PELIB_IMAGE_SCN_MEM_READ | PeLib::PELIB_IMAGE_SCN_MEM_WRITE)},
        {".xdata", (PeLib::PELIB_IMAGE_SCN_CNT_INITIALIZED_DATA | PeLib::PELIB_IMAGE_SCN_MEM_READ)}
    };

    std::size_t PeFormat::getBytesPerWord() const
    {
        WORD machine = 0;
        if (!IsValid()) return 0;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::PeHeaderT<32>& pe_header32 = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->peHeader();
            machine = pe_header32.getMachine();
        }
        break;
        case 64:
        {
            PeLib::PeHeaderT<64>& pe_header64 = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->peHeader();
            machine = pe_header64.getMachine();
        }
        break;
        default:
            ;
        }

        switch (machine)
        {
            // Architecture::X86
        case PeLib::PELIB_IMAGE_FILE_MACHINE_I386:
        case PeLib::PELIB_IMAGE_FILE_MACHINE_I486:
        case PeLib::PELIB_IMAGE_FILE_MACHINE_PENTIUM:
            return 4;
            // Architecture::X86_64
        case PeLib::PELIB_IMAGE_FILE_MACHINE_AMD64:
            return 8;
        default:
            ;
        }
        return 0;
    }

    bool PeFormat::isDll() const
    {
        WORD characteristics = getFileFlags();
        return characteristics & PeLib::PELIB_IMAGE_FILE_DLL;
    }

    bool PeFormat::isExecutable() const
    {
        return !isDll();
    }

    bool PeFormat::getMachineCode(unsigned long long &result) const
    {
        if (!IsValid()) return false;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::PeHeaderT<32>& pe_header32 = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->peHeader();
            result = pe_header32.getMachine();
            return true;
        }
        break;
        case 64:
        {
            PeLib::PeHeaderT<64>& pe_header64 = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->peHeader();
            result = pe_header64.getMachine();
            return true;
        }
        break;
        default:
            ;
        }
        return false;
    }

    bool PeFormat::getAbiVersion(unsigned long long &result) const
    {
        // not in PE files
        static_cast<void>(result);
        return false;
    }

    bool PeFormat::getImageBaseAddress(unsigned long long &imageBase) const
    {
        if (!IsValid()) return false;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::PeHeaderT<32>& pe_header32 = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->peHeader();
            imageBase = pe_header32.getImageBase();
            return true;
        }
        break;
        case 64:
        {
            PeLib::PeHeaderT<64>& pe_header64 = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->peHeader();
            imageBase = pe_header64.getImageBase();
            return true;
        }
        break;
        default:
            ;
        }
        return false;
    }

    bool PeFormat::getEpAddress(unsigned long long &result) const
    {
        if (!IsValid()) return false;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::PeHeaderT<32>& pe_header32 = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->peHeader();
            result = pe_header32.getAddressOfEntryPoint();
            return true;
        }
        break;
        case 64:
        {
            PeLib::PeHeaderT<64>& pe_header64 = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->peHeader();
            result = pe_header64.getAddressOfEntryPoint();
            return true;
        }
        break;
        default:
            ;
        }
        return false;
    }

    PeFormat::Architecture PeFormat::getTargetArchitecture() const
    {
        unsigned long long machine = 0;
        getMachineCode(machine);

        switch(machine)
        {
        case PeLib::PELIB_IMAGE_FILE_MACHINE_I386:
        case PeLib::PELIB_IMAGE_FILE_MACHINE_I486:
        case PeLib::PELIB_IMAGE_FILE_MACHINE_PENTIUM:
            return Architecture::X86;
        case PeLib::PELIB_IMAGE_FILE_MACHINE_AMD64:
            return Architecture::X86_64;
        case PeLib::PELIB_IMAGE_FILE_MACHINE_R3000_BIG:
        case PeLib::PELIB_IMAGE_FILE_MACHINE_R3000_LITTLE:
        case PeLib::PELIB_IMAGE_FILE_MACHINE_R4000:
        case PeLib::PELIB_IMAGE_FILE_MACHINE_R10000:
        case PeLib::PELIB_IMAGE_FILE_MACHINE_WCEMIPSV2:
        case PeLib::PELIB_IMAGE_FILE_MACHINE_MIPS16:
        case PeLib::PELIB_IMAGE_FILE_MACHINE_MIPSFPU:
        case PeLib::PELIB_IMAGE_FILE_MACHINE_MIPSFPU16:
            return Architecture::MIPS;
        case PeLib::PELIB_IMAGE_FILE_MACHINE_ARM:
        case PeLib::PELIB_IMAGE_FILE_MACHINE_THUMB:
        case PeLib::PELIB_IMAGE_FILE_MACHINE_ARMNT:
        case PeLib::PELIB_IMAGE_FILE_MACHINE_ARM64:
            return Architecture::ARM;
        case PeLib::PELIB_IMAGE_FILE_MACHINE_POWERPC:
        case PeLib::PELIB_IMAGE_FILE_MACHINE_POWERPCFP:
            return Architecture::POWERPC;
        default:
            return Architecture::UNKNOWN;
        }
    }

    std::size_t PeFormat::getDeclaredNumberOfSections() const
    {
        if (!IsValid()) return 0;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::PeHeaderT<32>& pe_header32 = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->peHeader();
            return pe_header32.getNumberOfSections();
        }
        break;
        case 64:
        {
            PeLib::PeHeaderT<64>& pe_header64 = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->peHeader();
            return pe_header64.getAddressOfEntryPoint();
        }
        break;
        default:
            ;
        }
        return 0;
    }

    std::size_t PeFormat::getDeclaredNumberOfSegments() const
    {
        return 0;
    }

    std::size_t PeFormat::getSectionTableOffset() const
    {
        //std::size_t res = getPeHeaderOffset() + formatParser->getSizeOfPeSignature() + PELIB_IMAGE_FILE_HEADER::size() + getOptionalHeaderSize();
        //if (res >= getFileLength())
        //{
        //    res = getPeHeaderOffset() + formatParser->getLoadedSizeOfNtHeaders();
        //}

        //return res;
        return 0;
    }

    std::size_t PeFormat::getSectionTableEntrySize() const
    {
        return PeLib::PELIB_IMAGE_SECTION_HEADER::size();
    }

    std::size_t PeFormat::getSegmentTableOffset() const
    {
        return 0;
    }

    std::size_t PeFormat::getSegmentTableEntrySize() const
    {
        return 0;
    }

    const PeLib::MzHeader & PeFormat::getMzHeader() const
    {
        return mzHeader;
    }

    std::size_t PeFormat::getMzHeaderSize() const
    {
        return mzHeader.size();
    }

    std::size_t PeFormat::getOptionalHeaderSize() const
    {
        if (!IsValid()) return 0;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::PeHeaderT<32>& pe_header32 = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->peHeader();
            return pe_header32.getSizeOfOptionalHeader();
        }
        break;
        case 64:
        {
            PeLib::PeHeaderT<64>& pe_header64 = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->peHeader();
            return pe_header64.getSizeOfOptionalHeader();
        }
        break;
        default:
            ;
        }
        return 0;
    }

    std::size_t PeFormat::getPeHeaderOffset() const
    {
        return mzHeader.getAddressOfPeHeader();
    }

    std::size_t PeFormat::getCoffSymbolTableOffset() const
    {
        if (!IsValid()) return 0;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::PeHeaderT<32>& pe_header32 = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->peHeader();
            return pe_header32.getPointerToSymbolTable();
        }
        break;
        case 64:
        {
            PeLib::PeHeaderT<64>& pe_header64 = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->peHeader();
            return pe_header64.getPointerToSymbolTable();
        }
        break;
        default:
            ;
        }
        return 0;
    }

    std::size_t PeFormat::getNumberOfCoffSymbols() const
    {
        if (!IsValid()) return 0;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::PeHeaderT<32>& pe_header32 = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->peHeader();
            return pe_header32.getNumberOfSymbols();
        }
        break;
        case 64:
        {
            PeLib::PeHeaderT<64>& pe_header64 = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->peHeader();
            return pe_header64.getNumberOfSymbols();
        }
        break;
        default:
            ;
        }
        return 0;
    }

    std::size_t PeFormat::getSizeOfStringTable() const
    {
        if (!IsValid()) return 0;
        return pe_file->coffSymTab().getSizeOfStringTable();
    }

    std::size_t PeFormat::getMajorLinkerVersion() const
    {
        if (!IsValid()) return 0;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::PeHeaderT<32>& pe_header32 = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->peHeader();
            return pe_header32.getMajorLinkerVersion();
        }
        break;
        case 64:
        {
            PeLib::PeHeaderT<64>& pe_header64 = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->peHeader();
            return pe_header64.getMajorLinkerVersion();
        }
        break;
        default:
            ;
        }
        return 0;
    }

    std::size_t PeFormat::getMinorLinkerVersion() const
    {
        if (!IsValid()) return 0;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::PeHeaderT<32>& pe_header32 = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->peHeader();
            return pe_header32.getMinorImageVersion();
        }
        break;
        case 64:
        {
            PeLib::PeHeaderT<64>& pe_header64 = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->peHeader();
            return pe_header64.getMinorImageVersion();
        }
        break;
        default:
            ;
        }
        return 0;
    }

    std::size_t PeFormat::getFileFlags() const
    {
        if (!IsValid()) return 0;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::PeHeaderT<32>& pe_header32 = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->peHeader();
            return pe_header32.getCharacteristics();
        }
        break;
        case 64:
        {
            PeLib::PeHeaderT<64>& pe_header64 = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->peHeader();
            return pe_header64.getCharacteristics();
        }
        break;
        default:
            ;
        }
        return 0;
    }

    std::size_t PeFormat::getTimeStamp() const
    {
        if (!IsValid()) return 0;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::PeHeaderT<32>& pe_header32 = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->peHeader();
            return pe_header32.getTimeDateStamp();
        }
        break;
        case 64:
        {
            PeLib::PeHeaderT<64>& pe_header64 = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->peHeader();
            return pe_header64.getTimeDateStamp();
        }
        break;
        default:
            ;
        }
        return 0;
    }

    std::size_t PeFormat::getChecksum() const
    {
        if (!IsValid()) return 0;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::PeHeaderT<32>& pe_header32 = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->peHeader();
            return pe_header32.getCheckSum();
        }
        break;
        case 64:
        {
            PeLib::PeHeaderT<64>& pe_header64 = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->peHeader();
            return pe_header64.getCheckSum();
        }
        break;
        default:
            ;
        }
        return 0;
    }

    std::size_t PeFormat::getFileAlignment() const
    {
        if (!IsValid()) return 0;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::PeHeaderT<32>& pe_header32 = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->peHeader();
            return pe_header32.getFileAlignment();
        }
        break;
        case 64:
        {
            PeLib::PeHeaderT<64>& pe_header64 = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->peHeader();
            return pe_header64.getFileAlignment();
        }
        break;
        default:
            ;
        }
        return 0;
    }

    std::size_t PeFormat::getSectionAlignment() const
    {
        if (!IsValid()) return 0;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::PeHeaderT<32>& pe_header32 = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->peHeader();
            return pe_header32.getSectionAlignment();
        }
        break;
        case 64:
        {
            PeLib::PeHeaderT<64>& pe_header64 = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->peHeader();
            return pe_header64.getSectionAlignment();
        }
        break;
        default:
            ;
        }
        return 0;
    }

    std::size_t PeFormat::getSizeOfHeaders() const
    {
        if (!IsValid()) return 0;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::PeHeaderT<32>& pe_header32 = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->peHeader();
            return pe_header32.getSizeOfHeaders();
        }
        break;
        case 64:
        {
            PeLib::PeHeaderT<64>& pe_header64 = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->peHeader();
            return pe_header64.getSizeOfHeaders();
        }
        break;
        default:
            ;
        }
        return 0;
    }

    std::size_t PeFormat::getSizeOfImage() const
    {
        if (!IsValid()) return 0;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::PeHeaderT<32>& pe_header32 = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->peHeader();
            return pe_header32.getSizeOfImage();
        }
        break;
        case 64:
        {
            PeLib::PeHeaderT<64>& pe_header64 = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->peHeader();
            return pe_header64.getSizeOfImage();
        }
        break;
        default:
            ;
        }
        return 0;
    }

    std::size_t PeFormat::getSizeOfStackReserve() const
    {
        if (!IsValid()) return 0;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::PeHeaderT<32>& pe_header32 = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->peHeader();
            return pe_header32.getSizeOfStackReserve();
        }
        break;
        case 64:
        {
            PeLib::PeHeaderT<64>& pe_header64 = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->peHeader();
            return pe_header64.getSizeOfStackReserve();
        }
        break;
        default:
            ;
        }
        return 0;
    }

    std::size_t PeFormat::getSizeOfStackCommit() const
    {
        if (!IsValid()) return 0;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::PeHeaderT<32>& pe_header32 = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->peHeader();
            return pe_header32.getSizeOfStackCommit();
        }
        break;
        case 64:
        {
            PeLib::PeHeaderT<64>& pe_header64 = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->peHeader();
            return pe_header64.getSizeOfStackCommit();
        }
        break;
        default:
            ;
        }
        return 0;
    }

    std::size_t PeFormat::getSizeOfHeapReserve() const
    {
        if (!IsValid()) return 0;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::PeHeaderT<32>& pe_header32 = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->peHeader();
            return pe_header32.getSizeOfHeapReserve();
        }
        break;
        case 64:
        {
            PeLib::PeHeaderT<64>& pe_header64 = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->peHeader();
            return pe_header64.getSizeOfHeapReserve();
        }
        break;
        default:
            ;
        }
        return 0;
    }

    std::size_t PeFormat::getSizeOfHeapCommit() const
    {
        if (!IsValid()) return 0;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::PeHeaderT<32>& pe_header32 = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->peHeader();
            return pe_header32.getSizeOfHeapCommit();
        }
        break;
        case 64:
        {
            PeLib::PeHeaderT<64>& pe_header64 = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->peHeader();
            return pe_header64.getSizeOfHeapCommit();
        }
        break;
        default:
            ;
        }
        return 0;
    }

    std::size_t PeFormat::getNumberOfDataDirectories() const
    {
        if (!IsValid()) return 0;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::PeHeaderT<32>& pe_header32 = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->peHeader();
            return pe_header32.calcNumberOfRvaAndSizes();
        }
        break;
        case 64:
        {
            PeLib::PeHeaderT<64>& pe_header64 = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->peHeader();
            return pe_header64.calcNumberOfRvaAndSizes();
        }
        break;
        default:
            ;
        }
        return 0;
    }

    std::size_t PeFormat::getDeclaredNumberOfDataDirectories() const
    {
        if (!IsValid()) return 0;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::PeHeaderT<32>& pe_header32 = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->peHeader();
            return pe_header32.getNumberOfRvaAndSizes();
        }
        break;
        case 64:
        {
            PeLib::PeHeaderT<64>& pe_header64 = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->peHeader();
            return pe_header64.getNumberOfRvaAndSizes();
        }
        break;
        default:
            ;
        }
        return 0;
    }

    bool PeFormat::isDotNet() const
    {
        return false;
    }

    bool PeFormat::isPackedDotNet() const
    {
        //if (isDotNet())
        //{
        //    return false;
        //}

        //return importTable
        //    && importTable->getNumberOfLibraries() == 1
        //    && importTable->getNumberOfImportsInLibraryCaseInsensitive("mscoree.dll");
        return false;
    }

    bool PeFormat::isVisualBasic(unsigned long long &version) const
    {
        //version = 0;
        //return importTable && std::any_of(visualBasicLibrariesMap.begin(), visualBasicLibrariesMap.end(),
        //    [&](const auto &item)
        //    {
        //        if (this->importTable->getNumberOfImportsInLibraryCaseInsensitive(item.first))
        //        {
        //            version = item.second;
        //            return true;
        //        }

        //        return false;
        //    }
        //);
        return false;
    }

    bool PeFormat::getDllFlags(unsigned long long &dllFlags) const
    {
        if (!IsValid()) return 0;
        WORD characteristics = getFileFlags();
        if (characteristics & PeLib::PELIB_IMAGE_FILE_DLL)
        {
            dllFlags = characteristics;
            return true;
        }
        return false;
    }

    bool PeFormat::getNumberOfBaseRelocationBlocks(unsigned long long &relocs) const
    {
        if (!IsValid()) return false;
        unsigned long long addr, size;

        if (PeLib::PELIB_IMAGE_DIRECTORY_ENTRY_BASERELOC >= getNumberOfDataDirectories())
            return false;

        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::PeHeaderT<32>& pe_header32 = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->peHeader();
            addr = pe_header32.getImageDataDirectoryRva(PeLib::PELIB_IMAGE_DIRECTORY_ENTRY_BASERELOC);
            size = pe_header32.getImageDataDirectorySize(PeLib::PELIB_IMAGE_DIRECTORY_ENTRY_BASERELOC);
            if (!addr || !size)
                return 0;
            PeLib::RelocationsDirectoryT<32>& reloc = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->relocDir();
            relocs = reloc.calcNumberOfRelocationData(PeLib::PELIB_IMAGE_DIRECTORY_ENTRY_BASERELOC);
            return true;
        }
        break;
        case 64:
        {
            PeLib::PeHeaderT<64>& pe_header64 = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->peHeader();
            addr = pe_header64.getImageDataDirectoryRva(PeLib::PELIB_IMAGE_DIRECTORY_ENTRY_BASERELOC);
            size = pe_header64.getImageDataDirectorySize(PeLib::PELIB_IMAGE_DIRECTORY_ENTRY_BASERELOC);
            if (!addr || !size)
                return 0;
            PeLib::RelocationsDirectoryT<64>& reloc = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->relocDir();
            relocs = reloc.calcNumberOfRelocationData(PeLib::PELIB_IMAGE_DIRECTORY_ENTRY_BASERELOC);
            return true;
        }
        break;
        default:
            ;
        }
        return false;
    }

    bool PeFormat::getNumberOfRelocations(unsigned long long &relocs) const
    {
        unsigned long long blocks = 0;
        if (!getNumberOfBaseRelocationBlocks(blocks))
        {
            return false;
        }
        relocs = 0;
        for (unsigned long long i = 0; i < blocks; ++i)
        {
            switch (pe_file->getBits()) {
            case 32:
            {
                PeLib::RelocationsDirectoryT<32>& reloc = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->relocDir();
                relocs += reloc.calcNumberOfRelocationData(i);
            }
            break;
            case 64:
            {
                PeLib::RelocationsDirectoryT<64>& reloc = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->relocDir();
                relocs += reloc.calcNumberOfRelocationData(i);
            }
            break;
            default:
                ;
            }
        }
        return true;
    }

    bool PeFormat::getDataDirectoryRelative(unsigned long long index, unsigned long long &relAddr, unsigned long long &size) const
    {
        if (!IsValid()) return false;
        if (index >= getNumberOfDataDirectories())
            return false;

        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::PeHeaderT<32>& pe_header32 = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->peHeader();
            relAddr = pe_header32.getImageDataDirectoryRva(PeLib::PELIB_IMAGE_DIRECTORY_ENTRY_BASERELOC);
            size = pe_header32.getImageDataDirectorySize(PeLib::PELIB_IMAGE_DIRECTORY_ENTRY_BASERELOC);
            return true;
        }
        break;
        case 64:
        {
            PeLib::PeHeaderT<64>& pe_header64 = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->peHeader();
            relAddr = pe_header64.getImageDataDirectoryRva(PeLib::PELIB_IMAGE_DIRECTORY_ENTRY_BASERELOC);
            size = pe_header64.getImageDataDirectorySize(PeLib::PELIB_IMAGE_DIRECTORY_ENTRY_BASERELOC);
            return true;
        }
        break;
        default:
            ;
        }
        return false;
    }

    bool PeFormat::getDataDirectoryAbsolute(unsigned long long index, unsigned long long &absAddr, unsigned long long &size) const
    {
        if (!IsValid()) return false;
        if (index >= getNumberOfDataDirectories())
            return false;

        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::PeHeaderT<32>& pe_header32 = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->peHeader();
            absAddr = pe_header32.getImageDataDirectoryRva(PeLib::PELIB_IMAGE_DIRECTORY_ENTRY_BASERELOC);
            absAddr += pe_header32.getImageBase();
            size = pe_header32.getImageDataDirectorySize(PeLib::PELIB_IMAGE_DIRECTORY_ENTRY_BASERELOC);
            return true;
        }
        break;
        case 64:
        {
            PeLib::PeHeaderT<64>& pe_header64 = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->peHeader();
            absAddr = pe_header64.getImageDataDirectoryRva(PeLib::PELIB_IMAGE_DIRECTORY_ENTRY_BASERELOC);
            absAddr += pe_header64.getImageBase();
            size = pe_header64.getImageDataDirectorySize(PeLib::PELIB_IMAGE_DIRECTORY_ENTRY_BASERELOC);
            return true;
        }
        break;
        default:
            ;
        }
        return false;
    }

    const std::shared_ptr<PeCoffSection> PeFormat::getPeSection(const std::string &secName) const
    {
        for (const auto& item : sections)
        {
            if (item && item->getName() == secName)
            {
                return std::static_pointer_cast<PeCoffSection>(item);
            }
        }

        return nullptr;
    }

    const std::shared_ptr<PeCoffSection> PeFormat::getPeSection(unsigned long long secIndex) const
    {
        if (secIndex >= sections.size())
        {
            return nullptr;
        }

        const auto& iSec = sections[secIndex];
        if (iSec && iSec->getIndex() == secIndex)
        {
            return std::static_pointer_cast<PeCoffSection>(iSec);
        }

        for (const auto& sec : sections)
        {
            if (sec && sec->getIndex() == secIndex)
            {
                return std::static_pointer_cast<PeCoffSection>(sec);
            }
        }

        return nullptr;
    }

    const std::shared_ptr<PeCoffSection> PeFormat::getPeSection(std::uint8_t* address) const
    {
        unsigned long long addr = reinterpret_cast<unsigned long long>(address);
        for (const auto& sec : sections)
        {
            if (sec && sec->getAddress() <= addr && sec->getEndAddress() >= addr)
            {
                return std::static_pointer_cast<PeCoffSection>(sec);
            }
        }

        return nullptr;
    }

    unsigned long long PeFormat::getDecodableStartAddr(std::uint8_t* address) const
    {
        return reinterpret_cast<unsigned long long>(address);
    }

    unsigned long long PeFormat::getStoredNumberOfSections() const
    {
        if (!IsValid()) return 0;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::PeHeaderT<32>& pe_header32 = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->peHeader();
            return pe_header32.calcNumberOfSections();
        }
        break;
        case 64:
        {
            PeLib::PeHeaderT<64>& pe_header64 = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->peHeader();
            return pe_header64.calcNumberOfSections();
        }
        break;
        default:
            ;
        }
        return 0;
    }

    bool PeFormat::peSectionName(std::string &name, unsigned long long sectionIndex) const
    {
        if (!IsValid()) return false;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::PeHeaderT<32>& pe_header32 = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->peHeader();
            if (sectionIndex >= pe_header32.getNumberOfSections())
            {
                return false;
            }

            name = pe_header32.getSectionNameFromStringTable(sectionIndex);
            if (name.empty())
            {
                name = pe_header32.getSectionName(sectionIndex);
            }

            return true;
        }
        break;
        case 64:
        {
            PeLib::PeHeaderT<64>& pe_header64 = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->peHeader();
            if (sectionIndex >= pe_header64.getNumberOfSections())
            {
                return false;
            }

            name = pe_header64.getSectionNameFromStringTable(sectionIndex);
            if (name.empty())
            {
                name = pe_header64.getSectionName(sectionIndex);
            }

            return true;
        }
        break;
        default:
            ;
        }
        return false;
    }

    bool PeFormat::peSectionType(PeCoffSection::Type &secType, unsigned long long sectionIndex) const
    {
        if (!IsValid()) return false;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::PeHeaderT<32>& pe_header32 = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->peHeader();
            if (sectionIndex >= pe_header32.getNumberOfSections())
            {
                return false;
            }

            std::string name;
            const unsigned long long flags = pe_header32.getCharacteristics(sectionIndex);
            if (flags & PeLib::PELIB_IMAGE_SCN_CNT_CODE || flags & PeLib::PELIB_IMAGE_SCN_MEM_EXECUTE)
            {
                secType = PeCoffSection::Type::CODE_SEG;
            }
            else if (flags & PeLib::PELIB_IMAGE_SCN_CNT_UNINITIALIZED_DATA)
            {
                secType = PeCoffSection::Type::BSS_SEG;
            }
            else if (flags & PeLib::PELIB_IMAGE_SCN_MEM_DISCARDABLE && peSectionName(name, sectionIndex)/* && StartsWith(name, ".debug_", false)*/)
            {
                secType = PeCoffSection::Type::DEBUG_SEG;
            }
            else if (flags & PeLib::PELIB_IMAGE_SCN_CNT_INITIALIZED_DATA)
            {
                secType = (!(flags & PeLib::PELIB_IMAGE_SCN_MEM_WRITE)) ? PeCoffSection::Type::CONST_DATA_SEG : PeCoffSection::Type::DATA_SEG;
            }
            else if (flags & PeLib::PELIB_IMAGE_SCN_LNK_INFO)
            {
                secType = PeCoffSection::Type::INFO_SEG;
            }
            else
            {
                secType = PeCoffSection::Type::UNDEFINED_SEC_SEG;
            }

            return true;
        }
        break;
        case 64:
        {
            PeLib::PeHeaderT<64>& pe_header64 = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->peHeader();
            if (sectionIndex >= pe_header64.getNumberOfSections())
            {
                return false;
            }

            std::string name;
            const unsigned long long flags = pe_header64.getCharacteristics(sectionIndex);
            if (flags & PeLib::PELIB_IMAGE_SCN_CNT_CODE || flags & PeLib::PELIB_IMAGE_SCN_MEM_EXECUTE)
            {
                secType = PeCoffSection::Type::CODE_SEG;
            }
            else if (flags & PeLib::PELIB_IMAGE_SCN_CNT_UNINITIALIZED_DATA)
            {
                secType = PeCoffSection::Type::BSS_SEG;
            }
            else if (flags & PeLib::PELIB_IMAGE_SCN_MEM_DISCARDABLE && peSectionName(name, sectionIndex)/* && StartsWith(name, ".debug_", false)*/)
            {
                secType = PeCoffSection::Type::DEBUG_SEG;
            }
            else if (flags & PeLib::PELIB_IMAGE_SCN_CNT_INITIALIZED_DATA)
            {
                secType = (!(flags & PeLib::PELIB_IMAGE_SCN_MEM_WRITE)) ? PeCoffSection::Type::CONST_DATA_SEG : PeCoffSection::Type::DATA_SEG;
            }
            else if (flags & PeLib::PELIB_IMAGE_SCN_LNK_INFO)
            {
                secType = PeCoffSection::Type::INFO_SEG;
            }
            else
            {
                secType = PeCoffSection::Type::UNDEFINED_SEC_SEG;
            }

            return true;
        }
        break;
        default:
            ;
        }
        return false;
    }

    bool PeFormat::getSection(unsigned long long secIndex, PeCoffSection &section) const
    {
        if (!IsValid()) return 0;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::PeHeaderT<32>& pe_header32 = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->peHeader();
            std::string sectionName;
            PeCoffSection::Type sectionType;
            if (!peSectionName(sectionName, secIndex) || !peSectionType(sectionType, secIndex))
            {
                return false;
            }

            section.setName(sectionName);
            section.setType(sectionType);
            section.setIndex(secIndex);
            section.setOffset(pe_header32.getPointerToRawData(secIndex));
            section.setSizeInFile(pe_header32.getSizeOfRawData(secIndex));
            section.setSizeInMemory(pe_header32.getVirtualSize(secIndex));
            section.setAddress(pe_header32.getVirtualAddress(secIndex) ? pe_header32.getVirtualAddress(secIndex) + pe_header32.getImageBase() : 0);
            section.setMemory(section.getAddress());
            section.setPeCoffFlags(pe_header32.getCharacteristics(secIndex));
            section.load(this);
            return true;
        }
        break;
        case 64:
        {
            PeLib::PeHeaderT<64>& pe_header64 = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->peHeader();
            std::string sectionName;
            PeCoffSection::Type sectionType;
            if (!peSectionName(sectionName, secIndex) || !peSectionType(sectionType, secIndex))
            {
                return false;
            }

            section.setName(sectionName);
            section.setType(sectionType);
            section.setIndex(secIndex);
            section.setOffset(pe_header64.getPointerToRawData(secIndex));
            section.setSizeInFile(pe_header64.getSizeOfRawData(secIndex));
            section.setSizeInMemory(pe_header64.getVirtualSize(secIndex));
            section.setAddress(pe_header64.getVirtualAddress(secIndex) ? pe_header64.getVirtualAddress(secIndex) + pe_header64.getImageBase() : 0);
            section.setMemory(section.getAddress());
            section.setPeCoffFlags(pe_header64.getCharacteristics(secIndex));
            section.load(this);
            return true;
        }
        break;
        default:
            ;
        }
        return 0;
    }

    void PeFormat::loadSections()
    {
        for (std::size_t i = 0, e = getStoredNumberOfSections(); i < e; ++i)
        {
            std::shared_ptr<PeCoffSection> section = std::make_shared<PeCoffSection>();
            if (!getSection(i, *section))
            {
                continue;
            }
            sections.push_back(std::move(section));
        }
    }

    bool PeFormat::getImportedLibraryFileName(unsigned long long index, std::string &fileName) const
    {
        if (!IsValid()) return false;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::ImportDirectory<32>& imports = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->impDir();
            if (index >= imports.getNumberOfFiles(PeLib::OLDDIR)) {
                return false;
            }
            fileName = imports.getFileName(index, PeLib::OLDDIR);
            return true;
        }
        break;
        case 64:
        {
            PeLib::ImportDirectory<64>& imports = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->impDir();
            if (index >= imports.getNumberOfFiles(PeLib::OLDDIR)) {
                return false;
            }
            fileName = imports.getFileName(index, PeLib::OLDDIR);
            return true;
        }
        break;
        default:
            ;
        }
        return false;
    }

    std::unique_ptr<Import> PeFormat::getImport(unsigned long long fileIndex, unsigned long long importIndex) const
    {
        if (!IsValid()) return nullptr;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::ImportDirectory<32>& imports = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->impDir();
            if (fileIndex >= imports.getNumberOfFiles(PeLib::OLDDIR) ||
                importIndex >= imports.getNumberOfFunctions(fileIndex, PeLib::OLDDIR))
            {
                return nullptr;
            }

            auto onlyordinal = false;
            auto isOrdinalNumberValid = true;
            unsigned long long ordinalNumber = imports.getFunctionHint(fileIndex, importIndex, PeLib::OLDDIR);
            if (!ordinalNumber)
            {
                const auto firstThunk = imports.getFirstThunk(fileIndex, importIndex, PeLib::OLDDIR);
                const auto originalFirstThunk = imports.getOriginalFirstThunk(fileIndex, importIndex, PeLib::OLDDIR);
                if (firstThunk & PeLib::PELIB_IMAGE_ORDINAL_FLAGS<32>::PELIB_IMAGE_ORDINAL_FLAG)
                {
                    onlyordinal = true;
                    ordinalNumber = firstThunk - PeLib::PELIB_IMAGE_ORDINAL_FLAGS<32>::PELIB_IMAGE_ORDINAL_FLAG;
                }
                else if (originalFirstThunk & PeLib::PELIB_IMAGE_ORDINAL_FLAGS<32>::PELIB_IMAGE_ORDINAL_FLAG)
                {
                    onlyordinal = true;
                    ordinalNumber = originalFirstThunk - PeLib::PELIB_IMAGE_ORDINAL_FLAGS<32>::PELIB_IMAGE_ORDINAL_FLAG;
                }
                else
                {
                    isOrdinalNumberValid = false;
                }
            }

            auto import = std::make_unique<Import>();
            if (isOrdinalNumberValid)
            {
                import->setOrdinalNumber(ordinalNumber);
            }
            else
            {
                import->invalidateOrdinalNumber();
            }
            if (onlyordinal) {
                std::stringstream ss;
                ss << "ordinal_" << ordinalNumber;
                import->setName(ss.str());
            }
            else {
                import->setName(imports.getFunctionName(fileIndex, importIndex, PeLib::OLDDIR));
            }
            import->setAddress(static_cast<PeLib::PeFileT<32>*>(pe_file.get())->peHeader().getImageBase() + imports.getFirstThunk(fileIndex, PeLib::OLDDIR) + importIndex * (32 / 8));
            import->setLibraryIndex(fileIndex);
            return import;
        }
        break;
        case 64:
        {
            PeLib::ImportDirectory<64>& imports = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->impDir();
            if (fileIndex >= imports.getNumberOfFiles(PeLib::OLDDIR) ||
                importIndex >= imports.getNumberOfFunctions(fileIndex, PeLib::OLDDIR))
            {
                return nullptr;
            }

            auto isOrdinalNumberValid = true;
            unsigned long long ordinalNumber = imports.getFunctionHint(fileIndex, importIndex, PeLib::OLDDIR);
            if (!ordinalNumber)
            {
                const auto firstThunk = imports.getFirstThunk(fileIndex, importIndex, PeLib::OLDDIR);
                const auto originalFirstThunk = imports.getOriginalFirstThunk(fileIndex, importIndex, PeLib::OLDDIR);
                if (firstThunk & PeLib::PELIB_IMAGE_ORDINAL_FLAGS<64>::PELIB_IMAGE_ORDINAL_FLAG)
                {
                    ordinalNumber = firstThunk - PeLib::PELIB_IMAGE_ORDINAL_FLAGS<64>::PELIB_IMAGE_ORDINAL_FLAG;
                }
                else if (originalFirstThunk & PeLib::PELIB_IMAGE_ORDINAL_FLAGS<64>::PELIB_IMAGE_ORDINAL_FLAG)
                {
                    ordinalNumber = originalFirstThunk - PeLib::PELIB_IMAGE_ORDINAL_FLAGS<64>::PELIB_IMAGE_ORDINAL_FLAG;
                }
                else
                {
                    isOrdinalNumberValid = false;
                }
            }

            auto import = std::make_unique<Import>();
            if (isOrdinalNumberValid)
            {
                import->setOrdinalNumber(ordinalNumber);
            }
            else
            {
                import->invalidateOrdinalNumber();
            }
            import->setName(imports.getFunctionName(fileIndex, importIndex, PeLib::OLDDIR));
            import->setAddress(static_cast<PeLib::PeFileT<64>*>(pe_file.get())->peHeader().getImageBase() + imports.getFirstThunk(fileIndex, PeLib::OLDDIR) + importIndex * (64 / 8));
            import->setLibraryIndex(fileIndex);
            return import;
        }
        break;
        default:
            ;
        }
        return nullptr;
    }

    bool PeFormat::getDelayImportedLibraryFileName(unsigned long long index, std::string &fileName) const
    {
        if (!IsValid()) return false;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::DelayImportDirectory<32>& delay = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->delayImports();
            const auto *library = delay.getFile(index);
            if (!library)
            {
                return false;
            }

            fileName = library->Name;

            return true;
        }
        break;
        case 64:
        {
            PeLib::DelayImportDirectory<64>& delay = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->delayImports();
            const auto *library = delay.getFile(index);
            if (!library)
            {
                return false;
            }

            fileName = library->Name;

            return true;
        }
        break;
        default:
            ;
        }
        return false;
    }

    std::unique_ptr<Import> PeFormat::getDelayImport(unsigned long long fileIndex, unsigned long long importIndex) const
    {
        if (!IsValid()) return nullptr;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::DelayImportDirectory<32>& delay = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->delayImports();
            const auto *library = delay.getFile(fileIndex);
            if (!library)
            {
                return nullptr;
            }

            const auto *function = library->getFunction(importIndex);
            if (!function)
            {
                return nullptr;
            }

            auto import = std::make_unique<Import>();
            import->setName(function->fname);
            import->setAddress(static_cast<PeLib::PeFileT<32>*>(pe_file.get())->peHeader().getImageBase() + function->address.Value);
            import->setLibraryIndex(fileIndex);
            import->invalidateOrdinalNumber();
            if (library->ordinalNumbersAreValid() && function->hint != 0)
            {
                import->setOrdinalNumber(function->hint);
            }

            return import;
        }
        break;
        case 64:
        {
            PeLib::DelayImportDirectory<64>& delay = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->delayImports();
            const auto *library = delay.getFile(fileIndex);
            if (!library)
            {
                return nullptr;
            }

            const auto *function = library->getFunction(importIndex);
            if (!function)
            {
                return nullptr;
            }

            auto import = std::make_unique<Import>();
            import->setName(function->fname);
            import->setAddress(static_cast<PeLib::PeFileT<64>*>(pe_file.get())->peHeader().getImageBase() + function->address.Value);
            import->setLibraryIndex(fileIndex);
            import->invalidateOrdinalNumber();
            if (library->ordinalNumbersAreValid() && function->hint != 0)
            {
                import->setOrdinalNumber(function->hint);
            }

            return import;
        }
        break;
        default:
            ;
        }
        return nullptr;
    }

    RangeContainer<std::uint64_t> PeFormat::getImportDirectoryOccupiedAddresses() const
    {
        RangeContainer<std::uint64_t> result;
        if (!IsValid()) return result;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::ImportDirectory<32>& imports = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->impDir();
            for (const auto& addresses : imports.getOccupiedAddresses())
            {
                try
                {
                    result.insert(addresses.first, addresses.second);
                }
                catch (const InvalidRangeException&)
                {
                    continue;
                }
            }
            return result;
        }
        break;
        case 64:
        {
            PeLib::ImportDirectory<64>& imports = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->impDir();
            for (const auto& addresses : imports.getOccupiedAddresses())
            {
                try
                {
                    result.insert(addresses.first, addresses.second);
                }
                catch (const InvalidRangeException&)
                {
                    continue;
                }
            }
            return result;
        }
        break;
        default:
            ;
        }
        return result;
    }

    void PeFormat::loadImports()
    {
        std::string libname;

        // Make sure we have import table initialized on the beginning
        if (importTable == nullptr)
            importTable = std::make_unique<ImportTable>();

        for (std::size_t i = 0; getImportedLibraryFileName(i, libname); ++i)
        {
            importTable->addLibrary(libname);

            std::size_t index = 0;
            while (auto import = getImport(i, index))
            {
                importTable->addImport(std::move(import));
                index++;
            }
        }

        for (std::size_t i = 0; getDelayImportedLibraryFileName(i, libname); ++i)
        {
            importTable->addLibrary(libname);

            std::size_t index = 0;
            while (auto import = getDelayImport(i, index))
            {
                import->setLibraryIndex(importTable->getNumberOfLibraries() - 1);
                importTable->addImport(std::move(import));
                index++;
            }
        }

        for (auto&& addressRange : getImportDirectoryOccupiedAddresses())
        {
            nonDecodableRanges.insert(std::move(addressRange));
        }
    }

    unsigned long long PeFormat::getNumberOfExportedFunctions() const
    {
        if (!IsValid()) return 0;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::ExportDirectoryT<32>& exports = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->expDir();
            return exports.calcNumberOfFunctions();
        }
        break;
        case 64:
        {
            PeLib::ExportDirectoryT<64>& exports = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->expDir();
            return exports.calcNumberOfFunctions();
        }
        break;
        default:
            ;
        }
        return 0;
    }

    std::string PeFormat::getNameOfExportTable() const
    {
        if (!IsValid()) return {};
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::ExportDirectoryT<32>& exports = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->expDir();
            return exports.getNameString();
        }
        break;
        case 64:
        {
            PeLib::ExportDirectoryT<64>& exports = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->expDir();
            return exports.getNameString();
        }
        break;
        default:
            ;
        }
        return {};
    }

    bool PeFormat::getExportedFunction(unsigned long long index, Export& exportedFunction) const
    {
        if (!IsValid()) return false;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::ExportDirectoryT<32>& exports = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->expDir();
            if (index >= exports.calcNumberOfFunctions())
            {
                return false;
            }
            exportedFunction.setAddress(exports.getAddressOfFunction(index) + static_cast<PeLib::PeFileT<32>*>(pe_file.get())->peHeader().getImageBase());
            exportedFunction.setOrdinalNumber(exports.getFunctionOrdinal(index));
            exportedFunction.setName(exports.getFunctionName(index));
            return true;
        }
        break;
        case 64:
        {
            PeLib::ExportDirectoryT<64>& exports = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->expDir();
            if (index >= exports.calcNumberOfFunctions())
            {
                return false;
            }
            exportedFunction.setAddress(exports.getAddressOfFunction(index) + static_cast<PeLib::PeFileT<64>*>(pe_file.get())->peHeader().getImageBase());
            exportedFunction.setOrdinalNumber(exports.getFunctionOrdinal(index));
            exportedFunction.setName(exports.getFunctionName(index));
            return true;
        }
        break;
        default:
            ;
        }
        return false;
    }

    RangeContainer<std::uint64_t> PeFormat::getExportDirectoryOccupiedAddresses() const
    {
        RangeContainer<std::uint64_t> result;
        if (!IsValid()) return result;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::ExportDirectoryT<32>& exports = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->expDir();
            for (const auto& addresses : exports.getOccupiedAddresses())
            {
                try
                {
                    result.insert(addresses.first, addresses.second);
                }
                catch (const InvalidRangeException&)
                {
                    continue;
                }
            }
            return result;
        }
        break;
        case 64:
        {
            PeLib::ExportDirectoryT<64>& exports = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->expDir();
            for (const auto& addresses : exports.getOccupiedAddresses())
            {
                try
                {
                    result.insert(addresses.first, addresses.second);
                }
                catch (const InvalidRangeException&)
                {
                    continue;
                }
            }
            return result;
        }
        break;
        default:
            ;
        }
        return result;
    }

    void PeFormat::loadExports()
    {
        Export newExport;
        exportTable = std::make_unique<ExportTable>();
        exportTable->setName(getNameOfExportTable());

        for (std::size_t i = 0, e = getNumberOfExportedFunctions(); i < e; ++i)
        {
            if (!getExportedFunction(i, newExport))
                break;

            if (hasNonprintableChars(newExport.getName()))
            {
                std::stringstream ss;
                ss << "exported_function_" << std::hex << newExport.getAddress();
                newExport.setName(ss.str());
            }
            exportTable->addExport(newExport);
        }

        for (auto&& addressRange : getExportDirectoryOccupiedAddresses())
        {
            nonDecodableRanges.insert(std::move(addressRange));
        }
    }

    const PeLib::ResourceNode* PeFormat::getResourceTreeRoot() const
    {
        if (!IsValid()) return nullptr;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::ResourceDirectoryT<32>& resources = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->resDir();
            return resources.getRoot();
        }
        break;
        case 64:
        {
            PeLib::ResourceDirectoryT<64>& resources = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->resDir();
            return resources.getRoot();
        }
        break;
        default:
            ;
        }
        return nullptr;
    }

    unsigned long long PeFormat::getResourceDirectoryOffset() const
    {
        if (!IsValid()) return 0;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::ResourceDirectoryT<32>& resources = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->resDir();
            return resources.getOffset();
        }
        break;
        case 64:
        {
            PeLib::ResourceDirectoryT<64>& resources = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->resDir();
            return resources.getOffset();
        }
        break;
        default:
            ;
        }
        return 0;
    }

    RangeContainer<std::uint64_t> PeFormat::getResourceDirectoryOccupiedAddresses() const
    {
        RangeContainer<std::uint64_t> result;
        if (!IsValid()) return result;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::ResourceDirectoryT<32>& resources = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->resDir();
            for (const auto& addresses : resources.getOccupiedAddresses())
            {
                try
                {
                    result.insert(addresses.first, addresses.second);
                }
                catch (const InvalidRangeException&)
                {
                    continue;
                }
            }

            return result;
        }
        break;
        case 64:
        {
            PeLib::ResourceDirectoryT<64>& resources = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->resDir();
            for (const auto& addresses : resources.getOccupiedAddresses())
            {
                try
                {
                    result.insert(addresses.first, addresses.second);
                }
                catch (const InvalidRangeException&)
                {
                    continue;
                }
            }

            return result;
        }
        break;
        default:
            ;
        }
        return result;
    }

    bool PeFormat::getResourceNodes(std::vector<const PeLib::ResourceChild*> &nodes, std::vector<std::size_t> &levels)
    {
        nodes.clear();
        levels.clear();
        auto root = getResourceTreeRoot();
        if (!root || !root->getNumberOfChildren())
        {
            return false;
        }
        resourceTree = std::make_unique<ResourceTree>();
        resourceTree->addNode(0, root->getNumberOfChildren());
        levels.push_back(root->getNumberOfChildren());

        for (std::size_t i = 0, e = root->getNumberOfChildren(); i < e; ++i)
        {
            nodes.push_back(root->getChild(i));
        }

        for (std::size_t i = 0, e = nodes.size(); i < e; ++i)
        {
            auto *actual = nodes[i];
            if (actual)
            {
                resourceTree->addNode(levels.size(), actual->getNumberOfChildren());

                for (std::size_t j = 0, f = actual->getNumberOfChildren(); j < f; ++j)
                {
                    nodes.push_back(actual->getChildOfThisChild(j));
                }
            }

            // end of actual level
            if (i + 1 == e && nodes.size() > e)
            {
                levels.push_back(nodes.size() - e);
                e = nodes.size();
            }
        }

        if (!resourceTree->isValidTree())
        {
            assert(false && "Incorrect structure of resources");
        }

        return true;
    }

    void PeFormat::loadResourceNodes(std::vector<const PeLib::ResourceChild*> &nodes, const std::vector<std::size_t> &levels)
    {
        unsigned long long rva = 0, size = 0;
        if (levels.empty() || !getDataDirectoryRelative(PeLib::PELIB_IMAGE_DIRECTORY_ENTRY_RESOURCE, rva, size))
        {
            return;
        }

        resourceTable = std::make_unique<ResourceTable>();
        std::size_t firstLeafIndex = 0;

        for (std::size_t i = 0, e = levels.size() - 1; i < e; ++i)
        {
            firstLeafIndex += levels[i];
        }

        for (std::size_t i = 0, e = resourceTree->getNumberOfLeafs(); i < e; ++i)
        {
            auto *leafChild = nodes[firstLeafIndex + i];
            if (!leafChild)
            {
                continue;
            }
            auto *leafChildNode = leafChild->getNode();
            auto *leaf = dynamic_cast<const PeLib::ResourceLeaf*>(leafChildNode);
            if (!leafChildNode || !leafChildNode->isLeaf() || !leaf)
            {
                continue;
            }
            auto resource = std::make_unique<Resource>();
            resource->setOffset(leaf->getOffsetToData() - rva + getResourceDirectoryOffset());
            resource->setSizeInFile(leaf->getSize());
            resource->load(this);
            resourceTable->addResource(std::move(resource));
        }
    }

    void PeFormat::loadResources()
    {
        size_t iconGroupIDcounter = 0;
        unsigned long long rva = 0, size = 0, imageBase = 0;
        if (!getDataDirectoryRelative(PeLib::PELIB_IMAGE_DIRECTORY_ENTRY_RESOURCE, rva, size))
        {
            return;
        }

        if (!getImageBaseAddress(imageBase))
        {
            return;
        }

        std::vector<const PeLib::ResourceChild*> nodes;
        std::vector<std::size_t> levels;
        if (!getResourceNodes(nodes, levels))
        {
            return;
        }
        else if (resourceTree->getNumberOfLevelsWithoutRoot() != 3)
        {
            loadResourceNodes(nodes, levels);
            return;
        }

        std::unique_ptr<Resource> resource;
        resourceTable = std::make_unique<ResourceTable>();

        for (std::size_t i = 0, e = levels[0], nSft = 0, lSft = 0; i < e; ++i)
        {
            auto *typeChild = nodes[i];
            if (!typeChild)
            {
                continue;
            }

            bool emptyType = false;
            auto type = typeChild->getName();
            if (type.empty())
            {
                type = mapGetValueOrDefault(resourceTypeMap, typeChild->getOffsetToName(), "");
                emptyType = true;
            }

            nSft += typeChild->getNumberOfChildren();

            for (std::size_t j = 0, f = typeChild->getNumberOfChildren(); j < f; ++j)
            {
                auto *nameChild = nodes[e + j + nSft - f];
                if (!nameChild)
                {
                    continue;
                }

                auto name = nameChild->getName();
                lSft += nameChild->getNumberOfChildren();

                for (std::size_t k = 0, g = nameChild->getNumberOfChildren(); k < g; ++k)
                {
                    auto *lanChild = nodes[e + levels[1] + k + lSft - g];
                    if (!lanChild)
                    {
                        continue;
                    }
                    auto *lanChildNode = lanChild->getNode();
                    auto *lanLeaf = dynamic_cast<const PeLib::ResourceLeaf*>(lanChildNode);
                    if (!lanChildNode || !lanChildNode->isLeaf() || !lanLeaf)
                    {
                        continue;
                    }

                    if (type == "Icon")
                    {
                        resource = std::make_unique<ResourceIcon>();
                        resourceTable->addResourceIcon(static_cast<ResourceIcon *>(resource.get()));
                    }
                    else if (type == "Icon Group")
                    {
                        auto iGroup = std::make_unique<ResourceIconGroup>();
                        iGroup->setIconGroupID(iconGroupIDcounter);
                        resource = std::move(iGroup);
                        resourceTable->addResourceIconGroup(static_cast<ResourceIconGroup *>(resource.get()));
                        iconGroupIDcounter++;
                    }
                    else if (type == "Version")
                    {
                        resource = std::make_unique<Resource>();
                        resourceTable->addResourceVersion(resource.get());
                    }
                    else
                    {
                        resource = std::make_unique<Resource>();
                    }
                    resource->setType(type);
                    resource->invalidateTypeId();
                    if (emptyType)
                    {
                        resource->setTypeId(typeChild->getOffsetToName());
                    }

                    resource->setName(name);
                    resource->invalidateNameId();
                    if (resource->hasEmptyName())
                    {
                        resource->setNameId(nameChild->getOffsetToName());
                    }

                    resource->setOffset(lanLeaf->getOffsetToData());
                    resource->setSizeInFile(lanLeaf->getSize());
                    resource->setLanguage(lanChild->getName());
                    resource->invalidateLanguageId();
                    resource->invalidateSublanguageId();
                    if (resource->hasEmptyLanguage())
                    {
                        const auto lIdAll = lanChild->getOffsetToName();
                        const auto lId = lIdAll & 0x3FF;
                        resource->setLanguageId(lId);
                        resource->setSublanguageId((lIdAll & 0xFC00) >> 10);
                        resource->setLanguage(mapGetValueOrDefault(resourceLanguageMap, lId, ""));
                    }
                    resource->load(this);
                    resourceTable->addResource(std::move(resource));
                }
            }
        }

        resourceTable->linkResourceIconGroups();
        resourceTable->parseVersionInfoResources();

        for (auto&& addressRange : getResourceDirectoryOccupiedAddresses())
        {
            nonDecodableRanges.insert(std::move(addressRange));
        }
    }

    unsigned long long PeFormat::getTlsStartAddressOfRawData() const
    {
        if (!IsValid()) return 0;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::TlsDirectory<32>& tlsdir = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->tlsDir();
            return tlsdir.getStartAddressOfRawData();
        }
        break;
        case 64:
        {
            PeLib::TlsDirectory<64>& tlsdir = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->tlsDir();
            return tlsdir.getStartAddressOfRawData();
        }
        break;
        default:
            ;
        }
        return 0;
    }

    unsigned long long PeFormat::getTlsEndAddressOfRawData() const
    {
        if (!IsValid()) return 0;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::TlsDirectory<32>& tlsdir = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->tlsDir();
            return tlsdir.getEndAddressOfRawData();
        }
        break;
        case 64:
        {
            PeLib::TlsDirectory<64>& tlsdir = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->tlsDir();
            return tlsdir.getEndAddressOfRawData();
        }
        break;
        default:
            ;
        }
        return 0;
    }

    unsigned long long PeFormat::getTlsAddressOfIndex() const
    {
        if (!IsValid()) return 0;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::TlsDirectory<32>& tlsdir = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->tlsDir();
            return tlsdir.getAddressOfIndex();
        }
        break;
        case 64:
        {
            PeLib::TlsDirectory<64>& tlsdir = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->tlsDir();
            return tlsdir.getAddressOfIndex();
        }
        break;
        default:
            ;
        }
        return 0;
    }

    unsigned long long PeFormat::getTlsSizeOfZeroFill() const
    {
        if (!IsValid()) return 0;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::TlsDirectory<32>& tlsdir = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->tlsDir();
            return tlsdir.getSizeOfZeroFill();
        }
        break;
        case 64:
        {
            PeLib::TlsDirectory<64>& tlsdir = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->tlsDir();
            return tlsdir.getSizeOfZeroFill();
        }
        break;
        default:
            ;
        }
        return 0;
    }

    unsigned long long PeFormat::getTlsCharacteristics() const
    {
        if (!IsValid()) return 0;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::TlsDirectory<32>& tlsdir = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->tlsDir();
            return tlsdir.getCharacteristics();
        }
        break;
        case 64:
        {
            PeLib::TlsDirectory<64>& tlsdir = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->tlsDir();
            return tlsdir.getCharacteristics();
        }
        break;
        default:
            ;
        }
        return 0;
    }

    unsigned long long PeFormat::getTlsAddressOfCallBacks() const
    {
        if (!IsValid()) return 0;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::TlsDirectory<32>& tlsdir = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->tlsDir();
            return tlsdir.getAddressOfCallBacks();
        }
        break;
        case 64:
        {
            PeLib::TlsDirectory<64>& tlsdir = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->tlsDir();
            return tlsdir.getAddressOfCallBacks();
        }
        break;
        default:
            ;
        }
        return 0;
    }

    void PeFormat::loadTlsInformation()
    {
        unsigned long long rva = 0, size = 0;
        if (!getDataDirectoryRelative(PeLib::PELIB_IMAGE_DIRECTORY_ENTRY_TLS, rva, size) || size == 0)
        {
            return;
        }

        auto callBacksAddr = getTlsAddressOfCallBacks();
        if (!callBacksAddr) return;
        tlsInfo = std::make_unique<TlsInfo>();
        tlsInfo->setRawDataStartAddr(getTlsStartAddressOfRawData());
        tlsInfo->setRawDataEndAddr(getTlsEndAddressOfRawData());
        tlsInfo->setIndexAddr(getTlsAddressOfIndex());
        tlsInfo->setZeroFillSize(getTlsSizeOfZeroFill());
        tlsInfo->setCharacteristics(getTlsCharacteristics());
        tlsInfo->setCallBacksAddr(callBacksAddr);

        while (1)
        {
            uint64_t cbAddr = *reinterpret_cast<std::uint32_t*>(callBacksAddr);
            callBacksAddr += sizeof(std::uint32_t);

            if (cbAddr == 0)
            {
                break;
            }

            tlsInfo->addCallBack(cbAddr);
        }
    }

    const std::shared_ptr<Section> PeFormat::getEpSection()
    {
        unsigned long long ep;
        if (!getEpAddress(ep))
        {
            return nullptr;
        }
        if (!ep) return nullptr;

        unsigned long long image_base = 0;
        if (getImageBaseAddress(image_base) && image_base) {
            for (const auto& item : sections)
            {
                unsigned long long address = item->getAddress();
                unsigned long long size = 0;
                item->getSizeInMemory(size);
                if (address <= (ep + image_base) && (address + size) >= (ep + image_base))
                    return item;
            }
        }
        return nullptr;
    }

    void PeFormat::scanForSectionAnomalies()
    {
        std::size_t nSecs = getDeclaredNumberOfSections();

        unsigned long long epAddr;
        if (getEpAddress(epAddr))
        {
            std::shared_ptr<PeCoffSection> epSec = std::static_pointer_cast<PeCoffSection>(getEpSection());
            if (epSec)
            {
                // scan EP in last section
                const std::shared_ptr<PeCoffSection> lastSec = (nSecs) ? getPeSection(nSecs - 1) : nullptr;
                if (epSec.get() == lastSec.get())
                {
                    anomalies.emplace_back("EpInLastSection", "Entry point in the last section");
                }

                // scan EP in writable section
                if (epSec->getPeCoffFlags() & PeLib::PELIB_IMAGE_SCN_MEM_WRITE)
                {
                    anomalies.emplace_back("EpInWritableSection", "Entry point in writable section");
                }
            }
            else
            {
                // scan EP outside mapped sections
                anomalies.emplace_back("EpOutsideSections", "Entry point is outside of mapped sections");
            }
        }

        std::vector<std::string> duplSections;
        for (std::size_t i = 0; i < nSecs; i++)
        {
            auto sec = getPeSection(i);
            if (!sec)
            {
                continue;
            }

            const auto name = sec->getName();
            const std::string msgName = (name.empty()) ? numToStr(sec->getIndex()) : name;
            const auto flags = sec->getPeCoffFlags();
            if (!name.empty())
            {
                // scan for packer section names
                if (usualPackerSections.find(name) != usualPackerSections.end())
                {
                    if (std::find(duplSections.begin(), duplSections.end(), name) == duplSections.end())
                    {
                        anomalies.emplace_back("PackerSectionName", "Packer section name: " + replaceNonprintableChars(name));
                    }
                }
                // scan for unusual section names
                else if (usualSectionNames.find(name) == usualSectionNames.end())
                {
                    if (std::find(duplSections.begin(), duplSections.end(), name) == duplSections.end())
                    {
                        anomalies.emplace_back("UnusualSectionName", "Unusual section name: " + replaceNonprintableChars(name));
                    }
                }

                // scan for unexpected characteristics
                auto characIt = usualSectionCharacteristics.find(name);
                if (characIt != usualSectionCharacteristics.end() && characIt->second != flags)
                {
                    anomalies.emplace_back("UnusualSectionFlags", "Section " + replaceNonprintableChars(name) + " has unusual characteristics");
                }
            }

            // scan size over 100MB
            if (sec->getSizeInFile() >= 100000000UL)
            {
                anomalies.emplace_back("LargeSection", "Section " + replaceNonprintableChars(msgName) + " has size over 100MB");
            }

            // scan section marked uninitialized but contains data
            if ((flags & PeLib::PELIB_IMAGE_SCN_CNT_UNINITIALIZED_DATA) && (sec->getOffset() != 0 || sec->getSizeInFile() != 0))
            {
                anomalies.emplace_back("UninitSectionHasData", "Section " + replaceNonprintableChars(msgName) + " is marked uninitialized but contains data");
            }

            for (std::size_t j = i + 1; j < nSecs; j++)
            {
                auto cmpSec = getPeSection(j);
                if (!cmpSec)
                {
                    continue;
                }

                // scan for duplicit section names
                const auto cmpName = cmpSec->getName();
                if (!name.empty() && name == cmpName)
                {
                    if (std::find(duplSections.begin(), duplSections.end(), name) == duplSections.end())
                    {
                        anomalies.emplace_back("DuplicitSectionNames", "Multiple sections with name " + replaceNonprintableChars(name));
                        duplSections.push_back(name);
                    }
                }

                // scan for overlapping sections
                auto secStart = sec->getOffset();
                auto secEnd = secStart + sec->getSizeInFile();
                auto cmpSecStart = cmpSec->getOffset();
                auto cmpSecEnd = cmpSecStart + cmpSec->getSizeInFile();
                if ((secStart <= cmpSecStart && cmpSecStart < secEnd) ||
                    (cmpSecStart <= secStart && secStart < cmpSecEnd))
                {
                    const std::string cmpMsgName = (cmpName.empty()) ? numToStr(cmpSec->getIndex()) : cmpName;
                    anomalies.emplace_back("OverlappingSections", "Sections " + replaceNonprintableChars(msgName) + " and " + replaceNonprintableChars(cmpMsgName) + " overlap");
                }
            }
        }
    }

    void PeFormat::scanForResourceAnomalies()
    {
        if (!resourceTable)
        {
            return;
        }

        for (std::size_t i = 0; i < resourceTable->getNumberOfResources(); i++)
        {
            auto res = resourceTable->getResource(i);
            if (!res)
            {
                continue;
            }

            std::size_t nameId;
            std::string msgName = (res->getNameId(nameId)) ? numToStr(nameId) : "<unknown>";

            // scan for resource size over 100MB
            if (res->getSizeInFile() >= 100000000UL)
            {
                anomalies.emplace_back("LargeResource", "Resource " + replaceNonprintableChars(msgName) + " has size over 100MB");
            }

            // scan for resource stretched over multiple sections
            //unsigned long long resAddr;
            unsigned long long image_base = 0;
            if (!getImageBaseAddress(image_base)) return;
            unsigned long long resAddr = image_base + res->getOffset();
            if (resAddr &&
                [&](unsigned long long addr, std::size_t size) ->bool {
                    for (const auto sec : sections)
                    {
                        if (!sec)
                        {
                            continue;
                        }

                        std::size_t addrStart = sec->getAddress();
                        std::size_t addrEnd = sec->getEndAddress();
                        if (addrStart <= addr && addr < addrEnd)
                        {
                            return ((addr + size) > addrEnd);
                        }
                    }

                    return false;
                }(resAddr, res->getSizeInFile()))
            {
                anomalies.emplace_back("StretchedResource", "Resource " + replaceNonprintableChars(msgName) + " is stretched over multiple sections");
            }
        }
    }

    const Import* PeFormat::getImport(unsigned long long address) const
    {
        return importTable ? importTable->getImportOnAddress(address) : nullptr;
    }

    void PeFormat::scanForImportAnomalies()
    {
        // scan for import stretched over multiple sections
        for (const auto &impRange : getImportDirectoryOccupiedAddresses())
        {
            unsigned long long image_base = 0;
            if (!getImageBaseAddress(image_base)) return;
            unsigned long long impAddr = image_base + impRange.getStart();
            if (impAddr &&
                [&](unsigned long long addr, std::size_t size) ->bool {
                    for (const auto sec : sections)
                    {
                        if (!sec)
                        {
                            continue;
                        }

                        std::size_t addrStart = sec->getAddress();
                            std::size_t addrEnd = sec->getEndAddress();
                            if (addrStart <= addr && addr < addrEnd)
                            {
                                return ((addr + size) > addrEnd);
                            }
                    }

                    return false;
                }(impAddr, impRange.getSize()))
            {
                std::string msgName;
                auto imp = getImport(impAddr);
                if (!imp)
                {
                    msgName = "<unknown>";
                }
                else
                {
                    if (imp->hasEmptyName())
                    {
                        unsigned long long ordNum;
                        if (!imp->getOrdinalNumber(ordNum))
                        {
                            msgName = "<unknown>";
                        }
                        else
                        {
                            msgName = numToStr(ordNum);
                        }

                    }
                    else
                    {
                        msgName = imp->getName();
                    }
                }

                anomalies.emplace_back("StretchedImportTable", "Import " + replaceNonprintableChars(msgName) + " is stretched over multiple sections");
            }
        }
    }

    const Export* PeFormat::getExport(unsigned long long address) const
    {
        return exportTable ? exportTable->getExportOnAddress(address) : nullptr;
    }

    void PeFormat::scanForExportAnomalies()
    {
        // scan for export stretched over multiple sections
        for (const auto &expRange : getExportDirectoryOccupiedAddresses())
        {
            unsigned long long image_base = 0;
            if (!getImageBaseAddress(image_base)) return;
            unsigned long long expAddr = image_base + expRange.getStart();
            if (expAddr &&
                [&](unsigned long long addr, std::size_t size) ->bool {
                    for (const auto sec : sections)
                    {
                        if (!sec)
                        {
                            continue;
                        }

                        std::size_t addrStart = sec->getAddress();
                            std::size_t addrEnd = sec->getEndAddress();
                            if (addrStart <= addr && addr < addrEnd)
                            {
                                return !((addr + size) > addrEnd);
                            }
                    }

                    return false;
                }(expAddr, expRange.getSize()))
            {
                std::string msgName;
                auto exp = getExport(expAddr);
                if (!exp)
                {
                    msgName = "<unknown>";
                }
                else
                {
                    if (exp->hasEmptyName())
                    {
                        unsigned long long ordNum;
                        if (!exp->getOrdinalNumber(ordNum))
                        {
                            msgName = "<unknown>";
                        }
                        else
                        {
                            msgName = numToStr(ordNum);
                        }

                    }
                    else
                    {
                        msgName = exp->getName();
                    }
                }

                anomalies.emplace_back("StretchedExportTable", "Export " + replaceNonprintableChars(msgName) + " is stretched over multiple sections");
            }
        }
    }

    bool PeFormat::isSizeOfHeaderMultipleOfFileAlignment() const
    {
        if (!IsValid()) return 0;
        auto isAligned_func = [](std::uint64_t value, std::uint64_t alignment, std::uint64_t& remainder) {
            return (remainder = (value & (alignment - 1))) == 0;
        };
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::PeHeaderT<32>& pe_header32 = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->peHeader();
            std::uint64_t remainder;
            return isAligned_func(pe_header32.getSizeOfHeaders(), pe_header32.getFileAlignment(), remainder);
        }
        break;
        case 64:
        {
            PeLib::PeHeaderT<64>& pe_header64 = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->peHeader();
            std::uint64_t remainder;
            return isAligned_func(pe_header64.getSizeOfHeaders(), pe_header64.getFileAlignment(), remainder);
        }
        break;
        default:
            ;
        }
        return 0;
    }

    void PeFormat::scanForOptHeaderAnomalies()
    {
        // scan for missalignment
        if (!isSizeOfHeaderMultipleOfFileAlignment())
        {
            anomalies.emplace_back("SizeOfHeaderNotAligned", "SizeOfHeader is not aligned to multiple of FileAlignment");
        }
    }

    void PeFormat::InitializePeCoffSections()
    {
        if (!nonDecodableRanges.size()) return;
        std::sort(nonDecodableRanges.begin(), nonDecodableRanges.end());
        unsigned long long image_base = 0;
        if (getImageBaseAddress(image_base) && image_base) {
            for (auto& range : nonDecodableRanges) {
                for (auto& sec : sections) {
                    std::shared_ptr<PeCoffSection> pe_sec = std::static_pointer_cast<PeCoffSection>(sec);
                    if (!pe_sec) continue;
                    if (image_base + range.getStart() >= pe_sec->getAddress() && image_base + range.getStart() <= pe_sec->getEndAddress()) {
                        Range<std::uint64_t> new_range;
                        if (image_base + range.getEnd() <= pe_sec->getEndAddress()) {
                            new_range.setStartEnd(image_base + range.getStart(), image_base + range.getEnd());
                        }
                        else {
                            new_range.setStartEnd(image_base + range.getStart(), pe_sec->getEndAddress());
                        }
                        pe_sec->AddDecodableRange(new_range);
                        break;
                    }
                }
            }
        }
        nonDecodableRanges.clear();
    }

} // namespace cchips
