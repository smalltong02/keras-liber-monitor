#include <Windows.h>
#include <string>
#include <cctype>
#include <iomanip>
#include "PeLibAux.h"
#include "PeFormat.h"
#include "..\StaticPEManager\InfoBuilder\PeInside\stringutils.h"

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

    const std::map<std::string, std::size_t> PeFormat::visualBasicLibrariesMap =
    {
        {"msvbvm10.dll", 1},
        {"msvbvm20.dll", 2},
        {"msvbvm30.dll", 3},
        {"msvbvm40.dll", 4},
        {"msvbvm50.dll", 5},
        {"msvbvm60.dll", 6},
        {"vb40032.dll", 4}
    };

    const std::vector<std::string> PeFormat::stubDatabase =
    {
        "This program cannot be run in DOS mode",
        "This program must be run under Win32",
        "This program requires Microsoft Windows",
        "Win32 only"
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

    bool PeFormat::isSys() const
    {
        if (isDll()) return false;
        WORD subsystem = getSubsystem();
        return (subsystem == PeLib::PELIB_IMAGE_SUBSYSTEM_NATIVE);
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
            return pe_header64.getNumberOfSections();
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

    std::size_t PeFormat::getDeclaredFileLength() const
    {
        std::size_t declSize = 0;

        for (const auto item : sections)
        {
            if (item && item->getType() != Section::Type::BSS_SEG)
            {
                declSize = std::max(declSize, static_cast<std::size_t>(item->getOffset() + item->getSizeInFile()));
            }
        }

        //for (const auto* item : segments)
        //{
        //    if (item)
        //    {
        //        declSize = std::max(declSize, static_cast<std::size_t>(item->getOffset() + item->getSizeInFile()));
        //    }
        //}

        return declSize;
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
            return pe_header32.getMinorLinkerVersion();
        }
        break;
        case 64:
        {
            PeLib::PeHeaderT<64>& pe_header64 = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->peHeader();
            return pe_header64.getMinorLinkerVersion();
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
        if (!clrHeader || !metadataHeader) {
            return false;
        }

        std::uint32_t correctHdrSize = 72;
        if (clrHeader->getHeaderSize() != correctHdrSize)
        {
            return false;
        }

        std::uint32_t numberOfRvaAndSizes = getDeclaredNumberOfDataDirectories();
        // If the binary is 64bit, check NumberOfRvaAndSizes, otherwise don't
        if (pe_file->getBits() == 64)
        {
            if (numberOfRvaAndSizes < PeLib::PELIB_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR)
            {
                return false;
            }
        }
        else if (!isDll())
        { // If 32 bit check if first 2 bytes at entry point are 0xFF 0x25

            std::uint64_t entryAddr = 0;
            if (!getEpAddress(entryAddr))
            {
                return false;
            }
            std::uint64_t bytes[2];
            if (!getXByte(entryAddr, 1, bytes[0], Endianness::UNKNOWN) || !getXByte(entryAddr + 1, 1, bytes[1], Endianness::UNKNOWN))
            {
                return false;
            }
            if (bytes[0] != 0xFF || bytes[1] != 0x25)
            {
                return false;
            }
        }

        return true;
    }

    bool PeFormat::isPackedDotNet() const
    {
        if (isDotNet())
        {
            return false;
        }

        return importTable
            && importTable->getNumberOfLibraries() == 1
            && importTable->getNumberOfImportsInLibraryCaseInsensitive("mscoree.dll");
        return false;
    }

    bool PeFormat::isVisualBasic(unsigned long long &version) const
    {
        version = 0;
        return importTable && std::any_of(visualBasicLibrariesMap.begin(), visualBasicLibrariesMap.end(),
            [&](const auto &item)
            {
                if (this->importTable->getNumberOfImportsInLibraryCaseInsensitive(item.first))
                {
                    version = item.second;
                    return true;
                }

                return false;
            }
        );
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
        relAddr = 0; size = 0;
        if (!IsValid()) return false;
        if (index >= getNumberOfDataDirectories())
            return false;

        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::PeHeaderT<32>& pe_header32 = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->peHeader();
            relAddr = pe_header32.getImageDataDirectoryRva(index);
            size = pe_header32.getImageDataDirectorySize(index);
            return true;
        }
        break;
        case 64:
        {
            PeLib::PeHeaderT<64>& pe_header64 = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->peHeader();
            relAddr = pe_header64.getImageDataDirectoryRva(index);
            size = pe_header64.getImageDataDirectorySize(index);
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
        absAddr = 0; size = 0;
        if (!IsValid()) return false;
        if (index >= getNumberOfDataDirectories())
            return false;

        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::PeHeaderT<32>& pe_header32 = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->peHeader();
            absAddr = pe_header32.getImageDataDirectoryRva(index);
            absAddr = getValidOffsetFromRva(absAddr);
            if (pe_file->isLoadingMemory()) {
                absAddr += pe_header32.getImageBase();
            }
            else if (pe_file->isLoadingFile()) {
                absAddr += (uint64_t)getLoadedBytesData();
            }
            size = pe_header32.getImageDataDirectorySize(index);
            return true;
        }
        break;
        case 64:
        {
            PeLib::PeHeaderT<64>& pe_header64 = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->peHeader();
            absAddr = pe_header64.getImageDataDirectoryRva(index);
            absAddr = getValidOffsetFromRva(absAddr);
            if (pe_file->isLoadingMemory()) {
                absAddr += pe_header64.getImageBase();
            }
            else if (pe_file->isLoadingFile()) {
                absAddr += (uint64_t)getLoadedBytesData();
            }
            size = pe_header64.getImageDataDirectorySize(index);
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

    const std::shared_ptr<PeCoffSection> PeFormat::getPeSectionByRva(unsigned long long rva) const
    {
        for (const auto& sec : sections)
        {
            auto address = sec->getVirtualAddress();
            unsigned long long size = 0;
            sec->getSizeInMemory(size);
            if (sec && address <= rva && (address + size) >= rva)
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

    std::size_t PeFormat::getLoadedFileLength() const
    {
        if (pe_file)
            return pe_file->getLoadedFileLength();
        return false;
    }

    const unsigned char* PeFormat::getLoadedBytesData() const
    {
        if (pe_file)
            return pe_file->getLoadedBytesData();
        return nullptr;
    }

    bool PeFormat::getBytes(std::vector<unsigned char>& result, unsigned long long offset, unsigned long long numberOfBytes) const
    {
        if (pe_file)
            return pe_file->getBytes(result, offset, numberOfBytes);
        return false;
    }

    bool PeFormat::getEpBytes(std::vector<unsigned char>& result, unsigned long long numberOfBytes) const
    {
        if (pe_file)
            return pe_file->getEpBytes(result, numberOfBytes);
        return false;
    }
    bool PeFormat::getHexBytes(std::string& result, unsigned long long offset, unsigned long long numberOfBytes) const
    {
        if (pe_file)
            return pe_file->getHexBytes(result, offset, numberOfBytes);
        return false;
    }

    bool PeFormat::getHexEpBytes(std::string& result, unsigned long long numberOfBytes) const
    {
        if (pe_file)
            return pe_file->getHexEpBytes(result, numberOfBytes);
        return false;
    }

    bool PeFormat::getStringBytes(std::string& result, unsigned long long offset, unsigned long long numberOfBytes) const
    {
        if (pe_file)
            return pe_file->getStringBytes(result, offset, numberOfBytes);
        return false;
    }

    bool PeFormat::getStringEpBytes(std::string& result, unsigned long long numberOfBytes) const
    {
        if (pe_file)
            return pe_file->getStringEpBytes(result, numberOfBytes);
        return false;
    }

    PeFormat::Endianness PeFormat::getEndianness() const
    {
        unsigned long long code = PeLib::PELIB_IMAGE_FILE_MACHINE_UNKNOWN;
        getMachineCode(code);
        switch (code)
        {
        case PeLib::PELIB_IMAGE_FILE_MACHINE_I386:
        case PeLib::PELIB_IMAGE_FILE_MACHINE_I486:
        case PeLib::PELIB_IMAGE_FILE_MACHINE_PENTIUM:
        case PeLib::PELIB_IMAGE_FILE_MACHINE_AMD64:
        case PeLib::PELIB_IMAGE_FILE_MACHINE_R3000_LITTLE:
        case PeLib::PELIB_IMAGE_FILE_MACHINE_R4000:
        case PeLib::PELIB_IMAGE_FILE_MACHINE_R10000:
        case PeLib::PELIB_IMAGE_FILE_MACHINE_WCEMIPSV2:
        case PeLib::PELIB_IMAGE_FILE_MACHINE_MIPS16:
        case PeLib::PELIB_IMAGE_FILE_MACHINE_MIPSFPU:
        case PeLib::PELIB_IMAGE_FILE_MACHINE_MIPSFPU16:
        case PeLib::PELIB_IMAGE_FILE_MACHINE_ARM:
        case PeLib::PELIB_IMAGE_FILE_MACHINE_THUMB:
        case PeLib::PELIB_IMAGE_FILE_MACHINE_ARMNT:
        case PeLib::PELIB_IMAGE_FILE_MACHINE_ARM64:
        case PeLib::PELIB_IMAGE_FILE_MACHINE_POWERPC:
        case PeLib::PELIB_IMAGE_FILE_MACHINE_POWERPCFP:
            return Endianness::LITTLE;
        case PeLib::PELIB_IMAGE_FILE_MACHINE_R3000_BIG:
            return Endianness::BIG;
        default:
            return Endianness::UNKNOWN;
        }
    }

    bool PeFormat::swapEndianness(std::string& str, std::size_t items, std::size_t length) const
    {
        if (!length || !items || str.size() < items * length)
        {
            return false;
        }

        const auto middleWordIndex = items / 2;
        const auto middleLengthIndex = length / 2;
        const auto wasteLen = str.size() % (items * length);
        str.erase(str.size() - wasteLen, wasteLen);

        for (std::size_t i = 0, e = str.size(); i < e; i += items * length)
        {
            for (std::size_t j = 0; j < middleWordIndex; ++j)
            {
                for (std::size_t k = 0; k < length; ++k)
                {
                    std::swap(
                        str[i + j * length + k],
                        str[i + (items - j) * length - k - 1]
                    );
                }
            }

            if (middleWordIndex && middleLengthIndex)
            {
                for (std::size_t j = 0; j < items; ++j)
                {
                    for (std::size_t k = 0; k < middleLengthIndex; ++k)
                    {
                        std::swap(
                            str[i + j * length + k],
                            str[i + (j + 1) * length - k - 1]
                        );
                    }
                }
            }
        }

        return true;
    }

    bool PeFormat::hexToLittle(std::string& str) const
    {
        if (getEndianness() == Endianness::UNKNOWN)
        {
            return false;
        }

        return (getEndianness() == Endianness::LITTLE)
            ? true
            : swapEndianness(
                str,
                getBytesPerWord(),
                getNumberOfNibblesInByte()
            );
    }

    bool PeFormat::createValueFromBytes(
        const std::vector<std::uint8_t>& data,
        std::uint64_t& value,
        Endianness endian,
        std::uint64_t offset,
        std::uint64_t size) const
    {
        const std::uint64_t realSize = (!size || offset + size > data.size())
            ? data.size() - offset
            : size;
        if (offset >= data.size() || (size && realSize != size))
        {
            return false;
        }

        if (endian == Endianness::UNKNOWN && (getEndianness() == Endianness::LITTLE))
        {
            endian = Endianness::LITTLE;
        }
        else if (endian == Endianness::UNKNOWN && (getEndianness() == Endianness::BIG))
        {
            endian = Endianness::BIG;
        }
        else if (endian == Endianness::UNKNOWN)
        {
            return false;
        }

        value = 0;

        for (std::uint64_t i = 0; i < realSize; ++i)
        {
            value += static_cast<std::uint64_t>(data[offset + i])
                << (getByteLength()
                    * (endian == Endianness::LITTLE ? i : realSize - i - 1));
        }

        return true;
    }

    bool PeFormat::getXByteOffset(std::uint64_t offset, std::uint64_t x, std::uint64_t& res, Endianness e)
    {
        if (offset + x > getLoadedFileLength() || x * getByteLength() > sizeof(res) * CHAR_BIT)
        {
            return false;
        }
        else if (!x)
        {
            res = 0;
            return true;
        }
        std::vector<PeLib::byte> bytes;
        if (getBytes(bytes, offset, x)) {
            return createValueFromBytes(bytes, res, e, 0, x);
        }
        return false;
    }

    bool PeFormat::getXByte(std::uint64_t rva, std::uint64_t x, std::uint64_t& res, Endianness e) const
    {
        const auto secSeg = getPeSectionByRva(rva);
        if (!secSeg || x * getByteLength() > sizeof(res) * CHAR_BIT)
        {
            return false;
        }
        else if (!x)
        {
            res = 0;
            return true;
        }

        const auto secOffset = rva - secSeg->getVirtualAddress();
        const auto offset = secSeg->getOffset() + secOffset;
        std::vector<unsigned char> result;
        getBytes(result, 0, getLoadedFileLength());
        return (secOffset + x > secSeg->getLoadedSize() || offset + x > getLoadedFileLength()) ?
            false : createValueFromBytes(result, res, e, offset, x);
    }

    void PeFormat::checkOverlay()
    {
        if (!sections.size())
            return;
        unsigned long long pointer_to_raw_data = sections.back()->getOffset();
        unsigned long long size_of_raw_data = sections.back()->getSizeInFile();
        unsigned long long virtual_size = 0;
        sections.back()->getSizeInMemory(virtual_size);
        if (!virtual_size) return;
        unsigned long long read_size = align_up(pointer_to_raw_data + size_of_raw_data, getFileAlignment()) - align_down(pointer_to_raw_data, 0x200);
        if (align_up(pointer_to_raw_data, 0x1000) < read_size)
            read_size = align_up(size_of_raw_data, 0x1000);
        if (virtual_size && align_up(virtual_size, 0x1000) < read_size)
            read_size = align_up(virtual_size, 0x1000);
        unsigned long long overlay_start = read_size + align_down(pointer_to_raw_data, 0x200);
        if ((size_t)overlay_start < pe_file->getLoadedFileLength())
        {
            has_overlay_data = true;
            overlay_offset = overlay_start;
            overlay_size = (uint32_t)(pe_file->getLoadedFileLength() - (size_t)overlay_start);
        }
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
            section.setAddressInMemory(pe_header32.getVirtualAddress(secIndex));
            section.setSizeInMemory(pe_header32.getVirtualSize(secIndex));
            if (pe_file->isLoadingFile()) {
                section.setAddress(pe_header32.getPointerToRawData(secIndex) ? pe_header32.getPointerToRawData(secIndex) + (uint64_t)getLoadedBytesData() : 0);
                section.setMemory(false);
            }
            else {
                section.setAddress(pe_header32.getVirtualAddress(secIndex) ? pe_header32.getVirtualAddress(secIndex) + pe_header32.getImageBase() : 0);
                section.setMemory(true);
            }
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
            section.setAddressInMemory(pe_header64.getVirtualAddress(secIndex));
            section.setSizeInMemory(pe_header64.getVirtualSize(secIndex));
            if (pe_file->isLoadingFile()) {
                section.setAddress(pe_header64.getPointerToRawData(secIndex) ? pe_header64.getPointerToRawData(secIndex) + (uint64_t)getLoadedBytesData() : 0);
                section.setMemory(false);
            }
            else {
                section.setAddress(pe_header64.getVirtualAddress(secIndex) ? pe_header64.getVirtualAddress(secIndex) + pe_header64.getImageBase() : 0);
                section.setMemory(true);
            }
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

    std::size_t PeFormat::findDosStub(const std::string& plainFile)
    {
        for (const auto& item : stubDatabase)
        {
            const auto offset = plainFile.find(item);
            if (offset != std::string::npos)
            {
                return offset;
            }
        }

        return std::string::npos;
    }

    std::size_t PeFormat::getRichHeaderOffset(const std::string& plainFile)
    {
        std::size_t richOffset = 0, prev = findDosStub(plainFile);

        if (prev != std::string::npos)
        {
            for (std::size_t i = 0, next = 0; (next = plainFile.find('\0', prev)) != std::string::npos; ++i)
            {
                if (i)
                {
                    if (next != prev)
                    {
                        break;
                    }
                    richOffset = ++prev;
                }
                else
                {
                    richOffset = prev = ++next;
                }
            }
        }

        return richOffset ? richOffset + getMzHeaderSize() : STANDARD_RICH_HEADER_OFFSET;
    }

    const PeLib::RichHeader& PeFormat::getRichHeader() const
    {
        return pe_file->richHeader();
    }

    void PeFormat::loadRichHeader()
    {
        if (getPeHeaderOffset() <= getMzHeaderSize())
        {
            return;
        }
        std::string plainText;
        getStringBytes(plainText, getMzHeaderSize(), getPeHeaderOffset() - getMzHeaderSize());
        auto offset = getRichHeaderOffset(plainText);
        auto standardOffset = (offset == STANDARD_RICH_HEADER_OFFSET);
        if (offset >= getPeHeaderOffset())
        {
            return;
        }

        pe_file->readRichHeader(offset, getPeHeaderOffset() - offset);
        auto& header = pe_file->richHeader();
        std::vector<std::size_t> validStructOffsets;
        if (header.isStructureValid())
        {
            validStructOffsets.push_back(offset);
        }
        // try space immediately after DOS (MZ) header
        if (!header.isHeaderValid() && offset > getMzHeaderSize() && getPeHeaderOffset() > getMzHeaderSize())
        {
            offset = getMzHeaderSize();
            standardOffset |= (offset == STANDARD_RICH_HEADER_OFFSET);
            pe_file->readRichHeader(offset, getPeHeaderOffset() - offset);
            if (header.isStructureValid())
            {
                validStructOffsets.push_back(offset);
            }
        }
        // try standard offset of rich header
        if (!header.isHeaderValid() && !standardOffset && STANDARD_RICH_HEADER_OFFSET < getPeHeaderOffset())
        {
            offset = STANDARD_RICH_HEADER_OFFSET;
            pe_file->readRichHeader(offset, getPeHeaderOffset() - offset);
            if (header.isStructureValid())
            {
                validStructOffsets.push_back(offset);
            }
        }
        if (!header.isHeaderValid() && validStructOffsets.empty())
        {
            return;
        }

        std::string signature;
        if (!header.isHeaderValid())
        {
            const auto nonStandardOffset = std::any_of(validStructOffsets.begin(), validStructOffsets.end(),
                [&](const auto& off)
                {
                    return off != STANDARD_RICH_HEADER_OFFSET && off != this->getMzHeaderSize();
                });
            std::size_t maxOffset = 0;

            for (const auto off : validStructOffsets)
            {
                if (off > maxOffset && (!nonStandardOffset || (off != STANDARD_RICH_HEADER_OFFSET && off != getMzHeaderSize())))
                {
                    maxOffset = off;
                }
            }

            pe_file->readRichHeader(maxOffset, getPeHeaderOffset() - maxOffset, true);
            signature = header.getDecryptedHeaderItemsSignature({ 0, 1, 2, 3 });
        }

        for (const auto& item : header)
        {
            PeLib::LinkerInfo info;
            info.setProductId(item.ProductId);
            info.setProductBuild(item.ProductBuild);
            info.setNumberOfUses(item.Count);
            info.setProductName(item.ProductName);
            info.setVisualStudioName(item.VisualStudioName);
            signature += item.Signature;
            header.addLinkerInfo(info);
        }
        return;
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
            if (pe_file->isLoadingFile()) {
                import->setAddress(reinterpret_cast<unsigned long long>(getLoadedBytesData()) + getValidOffsetFromRva(imports.getFirstThunk(fileIndex, PeLib::OLDDIR)) + importIndex * (32 / 8));
            }
            else {
                import->setAddress(static_cast<PeLib::PeFileT<32>*>(pe_file.get())->peHeader().getImageBase() + imports.getFirstThunk(fileIndex, PeLib::OLDDIR) + importIndex * (32 / 8));
            }
            
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
            if (pe_file->isLoadingFile()) {
                import->setAddress(reinterpret_cast<unsigned long long>(getLoadedBytesData()) + getValidOffsetFromRva(imports.getFirstThunk(fileIndex, PeLib::OLDDIR)) + importIndex * (64 / 8));
            }
            else {
                import->setAddress(static_cast<PeLib::PeFileT<64>*>(pe_file.get())->peHeader().getImageBase() + imports.getFirstThunk(fileIndex, PeLib::OLDDIR) + importIndex * (64 / 8));
            }
            import->setLibraryIndex(fileIndex);
            return import;
        }
        break;
        default:
            ;
        }
        return nullptr;
    }

    bool PeFormat::getBoundImportedLibraryFileName(unsigned long long index, std::string& fileName) const
    {
        if (!IsValid()) return false;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::BoundImportDirectory& bound = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->boundImpDir();
            const auto name = bound.getModuleName(index);
            if (!name.length())
            {
                return false;
            }

            fileName = name;

            return true;
        }
        break;
        case 64:
        {
            PeLib::BoundImportDirectory& bound = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->boundImpDir();
            const auto name = bound.getModuleName(index);
            if (!name.length())
            {
                return false;
            }

            fileName = name;

            return true;
        }
        break;
        default:
            ;
        }
        return false;
    }

    std::unique_ptr<Import> PeFormat::getBoundImport(unsigned long long fileIndex, unsigned long long importIndex) const
    {
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
            if (pe_file->isLoadingFile()) {
                import->setAddress(reinterpret_cast<unsigned long long>(getLoadedBytesData()) + getValidOffsetFromRva(function->address.Value));
            }
            else {
                import->setAddress(static_cast<PeLib::PeFileT<32>*>(pe_file.get())->peHeader().getImageBase() + function->address.Value);
            }
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
            if (pe_file->isLoadingFile()) {
                import->setAddress(reinterpret_cast<unsigned long long>(getLoadedBytesData()) + getValidOffsetFromRva(function->address.Value));
            }
            else {
                import->setAddress(static_cast<PeLib::PeFileT<64>*>(pe_file.get())->peHeader().getImageBase() + function->address.Value);
            }
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

        //for (std::size_t i = 0; getBoundImportedLibraryFileName(i, libname); ++i)
        //{
        //    importTable->addLibrary(libname);

        //    std::size_t index = 0;
        //    while (auto import = getBoundImport(i, index))
        //    {
        //        importTable->addImport(std::move(import));
        //        index++;
        //    }
        //}

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
        exportedFunction.Clear();
        switch (pe_file->getBits()) {
        case 32:
        {
            auto iddrva = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->peHeader().getIddExportRva();
            auto iddsize = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->peHeader().getIddExportSize();
            PeLib::ExportDirectoryT<32>& exports = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->expDir();
            if (index >= exports.calcNumberOfFunctions())
            {
                return false;
            }
            if (pe_file->isLoadingFile()) {
                auto iddoffset = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->peHeader().rvaToOffset(iddrva);
                auto funcoffset = getValidOffsetFromRva(exports.getAddressOfFunction(index));
                if (funcoffset > iddoffset && iddoffset < iddoffset + iddsize) {
                    exportedFunction.setLinkage();
                }
                exportedFunction.setAddress(funcoffset + reinterpret_cast<unsigned long long>(getLoadedBytesData()));
            }
            else {
                auto funcrva = exports.getAddressOfFunction(index);
                if (funcrva > iddrva && funcrva < iddrva + iddsize) {
                    exportedFunction.setLinkage();
                }
                exportedFunction.setAddress(exports.getAddressOfFunction(index) + static_cast<PeLib::PeFileT<32>*>(pe_file.get())->peHeader().getImageBase());
            }
            exportedFunction.setOrdinalNumber(exports.getFunctionOrdinal(index));
            exportedFunction.setName(exports.getFunctionName(index));
            return true;
        }
        break;
        case 64:
        {
            auto iddrva = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->peHeader().getIddExportRva();
            auto iddsize = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->peHeader().getIddExportSize();
            PeLib::ExportDirectoryT<64>& exports = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->expDir();
            if (index >= exports.calcNumberOfFunctions())
            {
                return false;
            }
            if (pe_file->isLoadingFile()) {
                auto iddoffset = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->peHeader().rvaToOffset(iddrva);
                auto funcoffset = getValidOffsetFromRva(exports.getAddressOfFunction(index));
                if (funcoffset > iddoffset && iddoffset < iddoffset + iddsize) {
                    exportedFunction.setLinkage();
                }
                exportedFunction.setAddress(funcoffset + reinterpret_cast<unsigned long long>(getLoadedBytesData()));
            }
            else {
                auto funcrva = exports.getAddressOfFunction(index);
                if (funcrva > iddrva && funcrva < iddrva + iddsize) {
                    exportedFunction.setLinkage();
                }
                exportedFunction.setAddress(exports.getAddressOfFunction(index) + static_cast<PeLib::PeFileT<64>*>(pe_file.get())->peHeader().getImageBase());
            }
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

    std::uint64_t PeFormat::getValidRvaAddressFromFileAddress(std::uint64_t address) const
    {
        unsigned long long imagebase = 0;
        if (!getImageBaseAddress(imagebase))
            return address;
        if (isLoadingMemory()) {
            return address;
        }
        for (auto& sec : sections) {
            if (sec) {
                if (sec->getAddress() <= address && address < (sec->getAddress() + sec->getSizeInFile())) {
                    auto offset = address - (uint64_t)getLoadedBytesData();
                    offset -= sec->getOffset();
                    offset += sec->getVirtualAddress();
                    return offset + imagebase;
                }
            }
        }
        return address;
    }

    std::uint64_t PeFormat::getValidOffsetFromMemRva(std::uint64_t memaddr) const
    {
        if (isLoadingMemory()) {
            return memaddr;
        }
        auto rva = memaddr;
        unsigned long long imagebase = 0;
        if (!getImageBaseAddress(imagebase))
            return rva;
        rva -= imagebase;
        auto offset = getValidOffsetFromRva(rva);
        if (offset == -1) {
            return 0;
        }
        return offset + (uint64_t)getLoadedBytesData();
    }

    std::uint32_t PeFormat::getValidOffsetFromRva(std::uint32_t rva) const
    {
        if (isLoadingMemory()) {
            return rva;
        }
        const std::uint32_t PELIB_PAGE_SIZE = 0x1000;
        const std::uint32_t PELIB_SECTOR_SIZE = 0x200;
        // If we have sections loaded, then we calculate the file offset from section headers
        if (sections.size())
        {
            PeLib::dword pointerToRawData = 0;
            PeLib::dword sizeOfRawData = 0;
            PeLib::dword virtualAddress = 0;
            PeLib::dword virtualSize = 0;
            PeLib::dword sectionAlignment = 0;
            PeLib::dword sizeOfHeaders = 0;
            // Check whether the rva goes into any section
            for (int index = 0; index < sections.size(); index++)
            {
                switch (pe_file->getBits()) {
                case 32:
                {
                    PeLib::PeHeaderT<32>& pe_header32 = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->peHeader();
                    pointerToRawData = pe_header32.getPointerToRawData(index);
                    sizeOfRawData = pe_header32.getSizeOfRawData(index);
                    virtualAddress = pe_header32.getVirtualAddress(index);
                    virtualSize = pe_header32.getVirtualSize(index);
                    sectionAlignment = pe_header32.getSectionAlignment();
                    sizeOfHeaders = pe_header32.getSizeOfHeaders();
                }
                break;
                case 64:
                {
                    PeLib::PeHeaderT<64>& pe_header64 = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->peHeader();
                    pointerToRawData = pe_header64.getPointerToRawData(index);
                    sizeOfRawData = pe_header64.getSizeOfRawData(index);
                    virtualAddress = pe_header64.getVirtualAddress(index);
                    virtualSize = pe_header64.getVirtualSize(index);
                    sectionAlignment = pe_header64.getSectionAlignment();
                    sizeOfHeaders = pe_header64.getSizeOfHeaders();
                }
                break;
                default:
                    ;
                }

                // Only if the pointer to raw data is not zero
                if (pointerToRawData != 0 && sizeOfRawData != 0)
                {
                    std::uint32_t realPointerToRawData = pointerToRawData;
                    std::uint32_t sectionRvaStart = virtualAddress;
                    std::uint32_t virtSize = virtualSize;
                    std::uint32_t rawSize = sizeOfRawData;

                    // if rawSize is larger than what is mapped to memory, use only the mapped part
                    std::uint32_t section_size = virtSize >= rawSize ? virtSize : rawSize;
                    // For multi-section images, real pointer to raw data is aligned down to sector size
                    if (sectionAlignment >= PELIB_PAGE_SIZE)
                        realPointerToRawData = realPointerToRawData & ~(PELIB_SECTOR_SIZE - 1);

                    // Check if the claimed real pointer can actually exist in the file
                    std::uint64_t offset = rva - sectionRvaStart;
                    bool fitsInFile = realPointerToRawData + offset < pe_file->getLoadedFileLength();

                    // Is the RVA inside that part of the section, that is backed by disk data?
                    if (sectionRvaStart <= rva && rva < (sectionRvaStart + section_size) && fitsInFile)
                    {
                        // Make sure we round the pointer to raw data down to PELIB_SECTOR_SIZE.
                        // In case when PointerToRawData is less than 0x200, it maps to the header!
                        return realPointerToRawData + offset;
                    }
                }
            }

            // Check if the rva goes into the header
            return (rva < sizeOfHeaders) ? rva : UINT32_MAX;
        }

        return UINT32_MAX;
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

                    std::uint64_t dataOffset = getValidOffsetFromRva(lanLeaf->getOffsetToData());
                    resource->setOffset(dataOffset);
                    //resource->setOffset(lanLeaf->getOffsetToData());
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

        //while (1)
        //{
        //    uint64_t cbAddr = *reinterpret_cast<std::uint32_t*>(callBacksAddr);
        //    callBacksAddr += sizeof(std::uint32_t);

        //    if (cbAddr == 0)
        //    {
        //        break;
        //    }

        //    tlsInfo->addCallBack(cbAddr);
        //}
    }

    bool PeFormat::getComDirectoryRelative(std::uint64_t& relAddr, std::uint64_t& size) const
    {
        relAddr = 0; size = 0;
        if (!IsValid()) return 0;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::PeHeaderT<32>& pe_header32 = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->peHeader();
            if (pe_header32.getMagic() == PeLib::PELIB_IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
                relAddr = pe_header32.getImageDataDirectoryRva(PeLib::PELIB_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);
                size = pe_header32.getImageDataDirectorySize(PeLib::PELIB_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);
                return true;
            }
            else if (pe_header32.getNumberOfRvaAndSizes() > PeLib::PELIB_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR) {
                relAddr = pe_header32.getImageDataDirectoryRva(PeLib::PELIB_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);
                size = pe_header32.getImageDataDirectorySize(PeLib::PELIB_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);
                return true;
            }
            return false;
        }
        break;
        case 64:
        {
            PeLib::PeHeaderT<64>& pe_header64 = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->peHeader();
            if (pe_header64.getMagic() == PeLib::PELIB_IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
                relAddr = pe_header64.getImageDataDirectoryRva(PeLib::PELIB_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);
                size = pe_header64.getImageDataDirectorySize(PeLib::PELIB_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);
                return true;
            }
            if (pe_header64.getNumberOfRvaAndSizes() > PeLib::PELIB_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR) {
                relAddr = pe_header64.getImageDataDirectoryRva(PeLib::PELIB_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);
                size = pe_header64.getImageDataDirectorySize(PeLib::PELIB_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);
                return true;
            }
            return false;
        }
        break;
        default:
            ;
        }
        return false;
    }

    std::unique_ptr<CLRHeader> PeFormat::getClrHeader() const
    {
        if (!IsValid()) return 0;
        auto isAligned_func = [](std::uint64_t value, std::uint64_t alignment, std::uint64_t& remainder) {
            return (remainder = (value & (alignment - 1))) == 0;
        };
        switch (pe_file->getBits()) {
        case 32:
        {
            const auto pe_file32 = static_cast<PeLib::PeFileT<32>*>(pe_file.get());
            const auto& comHeader = pe_file32->comDir();
            auto clr_header = std::make_unique<CLRHeader>();
            clr_header->setHeaderSize(comHeader.getSizeOfHeader());
            clr_header->setMajorRuntimeVersion(comHeader.getMajorRuntimeVersion());
            clr_header->setMinorRuntimeVersion(comHeader.getMinorRuntimeVersion());
            clr_header->setMetadataDirectoryAddress(comHeader.getMetaDataVa());
            clr_header->setMetadataDirectorySize(comHeader.getMetaDataSize());
            clr_header->setFlags(comHeader.getFlags());
            clr_header->setEntryPointToken(comHeader.getEntryPointToken());
            clr_header->setResourcesAddress(comHeader.getResourcesVa());
            clr_header->setResourcesSize(comHeader.getResourcesSize());
            clr_header->setStrongNameSignatureAddress(comHeader.getStrongNameSignatureVa());
            clr_header->setStrongNameSignatureSize(comHeader.getStrongNameSignatureSize());
            clr_header->setCodeManagerTableAddress(comHeader.getCodeManagerTableVa());
            clr_header->setCodeManagerTableSize(comHeader.getCodeManagerTableSize());
            clr_header->setVTableFixupsDirectoryAddress(comHeader.getVTableFixupsVa());
            clr_header->setVTableFixupsDirectorySize(comHeader.getVTableFixupsSize());
            clr_header->setExportAddressTableAddress(comHeader.getExportAddressTableJumpsVa());
            clr_header->setExportAddressTableSize(comHeader.getExportAddressTableJumpsSize());
            clr_header->setPrecompileHeaderAddress(comHeader.getManagedNativeHeaderVa());
            clr_header->setPrecompileHeaderSize(comHeader.getManagedNativeHeaderSize());
            return clr_header;
        }
        break;
        case 64:
        {
            const auto pe_file64 = static_cast<PeLib::PeFileT<64>*>(pe_file.get());
            const auto& comHeader = pe_file64->comDir();
            auto clr_header = std::make_unique<CLRHeader>();
            clr_header->setHeaderSize(comHeader.getSizeOfHeader());
            clr_header->setMajorRuntimeVersion(comHeader.getMajorRuntimeVersion());
            clr_header->setMinorRuntimeVersion(comHeader.getMinorRuntimeVersion());
            clr_header->setMetadataDirectoryAddress(comHeader.getMetaDataVa());
            clr_header->setMetadataDirectorySize(comHeader.getMetaDataSize());
            clr_header->setFlags(comHeader.getFlags());
            clr_header->setEntryPointToken(comHeader.getEntryPointToken());
            clr_header->setResourcesAddress(comHeader.getResourcesVa());
            clr_header->setResourcesSize(comHeader.getResourcesSize());
            clr_header->setStrongNameSignatureAddress(comHeader.getStrongNameSignatureVa());
            clr_header->setStrongNameSignatureSize(comHeader.getStrongNameSignatureSize());
            clr_header->setCodeManagerTableAddress(comHeader.getCodeManagerTableVa());
            clr_header->setCodeManagerTableSize(comHeader.getCodeManagerTableSize());
            clr_header->setVTableFixupsDirectoryAddress(comHeader.getVTableFixupsVa());
            clr_header->setVTableFixupsDirectorySize(comHeader.getVTableFixupsSize());
            clr_header->setExportAddressTableAddress(comHeader.getExportAddressTableJumpsVa());
            clr_header->setExportAddressTableSize(comHeader.getExportAddressTableJumpsSize());
            clr_header->setPrecompileHeaderAddress(comHeader.getManagedNativeHeaderVa());
            clr_header->setPrecompileHeaderSize(comHeader.getManagedNativeHeaderSize());
            return clr_header;
        }
        break;
        default:
            ;
        }
        return nullptr;
    }

    void PeFormat::loadDotnetHeaders()
    {
        std::uint64_t metadataHeaderRva = 0;

        // If our file contains CLR header, then use it. Note that .NET framework doesn't
        // verify the OPTIONAL_HEADER::NumberOfRvaAndSizes, so we must do it the same way.
        std::uint64_t comHeaderAddress, comHeaderSize;
        if (getComDirectoryRelative(comHeaderAddress, comHeaderSize) && comHeaderSize)
        {
            clrHeader = getClrHeader();
            metadataHeaderRva = clrHeader->getMetadataDirectoryAddress();
        }
        else
        {
            return;
        }

        // If not, then try to guess whether the file could possibly be .NET file based on imports and try to search for metadata header
        // LZ: Don't. This will lead to the situation when totally unrelated .NET metadata will be read from the binary,
        // for example from a binary embedded in resources or in overlay.
        // Sample: 76360c777ac93d7fc7398b5d140c4117eb08501cac30d170f33ab260e1788e74
        /*
        else
        {
            if (importTable && importTable->getImport("mscoree.dll"))
            {
                metadataHeaderAddress = detectPossibleMetadataHeaderAddress();
                if (metadataHeaderAddress == 0)
                    return;
            }
            else
            {
                return;
            }
        }
        */

        // This explicit initialization needs to be here, because clang 4.0 has bug in optimizer and it causes problem in valgrind.
        std::uint64_t signature = 0;
        if (!getXByte(metadataHeaderRva, 4, signature, Endianness::UNKNOWN) || signature != MetadataHeaderSignature)
        {
            return;
        }

        std::uint64_t majorVersion, minorVersion, versionLength;
        if (!getXByte(metadataHeaderRva + 4, 2, majorVersion, Endianness::UNKNOWN)
            || !getXByte(metadataHeaderRva + 6, 2, minorVersion, Endianness::UNKNOWN)
            || !getXByte(metadataHeaderRva + 12, 2, versionLength, Endianness::UNKNOWN))
        {
            return;
        }

        std::string version;
        if (!getNTBS(metadataHeaderRva + 16, version, versionLength))
        {
            return;
        }

        auto metadataHeaderStreamsHeader = metadataHeaderRva + 16 + versionLength;

        std::uint64_t flags, streamCount;
        if (!getXByte(metadataHeaderStreamsHeader, 1, flags, Endianness::UNKNOWN)
            || !getXByte(metadataHeaderStreamsHeader + 2, 2, streamCount, Endianness::UNKNOWN))
        {
            return;
        }

        metadataHeader = std::make_unique<MetadataHeader>();
        unsigned long long imagebase = 0;
        if (pe_file->isLoadingFile()) {
            imagebase = (unsigned long long)getLoadedBytesData();
        }
        else {
            if (!getImageBaseAddress(imagebase))
            {
                return;
            }
        }
        metadataHeader->setAddress(metadataHeaderRva - imagebase);
        metadataHeader->setMajorVersion(majorVersion);
        metadataHeader->setMinorVersion(minorVersion);
        metadataHeader->setVersion(version);
        metadataHeader->setFlags(flags);

        auto currentAddress = metadataHeaderStreamsHeader + 4;
        for (std::uint64_t i = 0; i < streamCount; ++i)
        {
            std::uint64_t streamOffset, streamSize;
            std::string streamName;

            if (!getXByte(currentAddress, 4, streamOffset, Endianness::UNKNOWN)
                || !getXByte(currentAddress + 4, 4, streamSize, Endianness::UNKNOWN)
                || !getNTBS(currentAddress + 8, streamName))
            {
                return;
            }

            if ((streamName == "#~" || streamName == "#-") && !metadataStream)
                parseMetadataStream(metadataHeaderRva, streamOffset, streamSize);
            else if (streamName == "#Blob" && !blobStream)
                parseBlobStream(metadataHeaderRva, streamOffset, streamSize);
            else if (streamName == "#GUID" && !guidStream)
                parseGuidStream(metadataHeaderRva, streamOffset, streamSize);
            else if (streamName == "#Strings" && !stringStream)
                parseStringStream(metadataHeaderRva, streamOffset, streamSize);
            else if (streamName == "#US" && !userStringStream)
                parseUserStringStream(metadataHeaderRva, streamOffset, streamSize);

            // Round-up to the nearest higher multiple of 4
            currentAddress += 8 + ((streamName.length() + 4) & ~3);
        }

        detectModuleVersionId();
        detectTypeLibId();
        detectDotnetTypes();
    }

    bool PeFormat::parseVisualBasicProjectInfo(std::size_t structureOffset)
    {
        std::vector<std::uint8_t> bytes;
        std::uint64_t vbExternTableOffset = 0;
        std::uint64_t vbObjectTableOffset = 0;
        std::string projPath;
        std::size_t offset = 0;
        struct VBProjInfo vbpi;

        if (!getBytes(bytes, structureOffset, vbpi.structureSize()) || bytes.size() != vbpi.structureSize())
        {
            return false;
        }

        DynamicBuffer structContent(bytes);
        vbpi.version = structContent.read<std::uint32_t>(offset); offset += sizeof(vbpi.version);
        vbpi.objectTableAddr = structContent.read<std::uint32_t>(offset); offset += sizeof(vbpi.objectTableAddr);
        vbpi.null = structContent.read<std::uint32_t>(offset); offset += sizeof(vbpi.null);
        vbpi.codeStartAddr = structContent.read<std::uint32_t>(offset); offset += sizeof(vbpi.codeStartAddr);
        vbpi.codeEndAddr = structContent.read<std::uint32_t>(offset); offset += sizeof(vbpi.codeEndAddr);
        vbpi.dataSize = structContent.read<std::uint32_t>(offset); offset += sizeof(vbpi.dataSize);
        vbpi.threadSpaceAddr = structContent.read<std::uint32_t>(offset); offset += sizeof(vbpi.threadSpaceAddr);
        vbpi.exHandlerAddr = structContent.read<std::uint32_t>(offset); offset += sizeof(vbpi.exHandlerAddr);
        vbpi.nativeCodeAddr = structContent.read<std::uint32_t>(offset); offset += sizeof(vbpi.nativeCodeAddr);
        std::memcpy(&vbpi.pathInformation, static_cast<void*>(&bytes.data()[offset]), sizeof(vbpi.pathInformation)); offset += sizeof(vbpi.pathInformation);
        vbpi.externalTableAddr = structContent.read<std::uint32_t>(offset); offset += sizeof(vbpi.externalTableAddr);
        vbpi.nExternals = structContent.read<std::uint32_t>(offset); offset += sizeof(vbpi.nExternals);

        projPath = unicodeToAscii(vbpi.pathInformation, sizeof(vbpi.pathInformation));
        visualBasicInfo.setProjectPath(projPath);
        visualBasicInfo.setPcode(vbpi.nativeCodeAddr == 0);

        if (getOffsetFromAddress(vbExternTableOffset, vbpi.externalTableAddr))
        {
            parseVisualBasicExternTable(vbExternTableOffset, vbpi.nExternals);
        }

        if (getOffsetFromAddress(vbObjectTableOffset, vbpi.objectTableAddr))
        {
            parseVisualBasicObjectTable(vbObjectTableOffset);
        }

        return true;
    }

    bool PeFormat::parseVisualBasicObjects(std::size_t structureOffset, std::size_t nObjects)
    {
        std::vector<unsigned char> allBytes;
        if (!getBytes(allBytes, 0, pe_file->getLoadedFileLength()) && allBytes.size() != pe_file->getLoadedFileLength())
            return false;
        std::vector<std::uint8_t> bytes;
        struct VBPublicObjectDescriptor vbpod;
        std::size_t offset = 0;

        for (std::size_t i = 0; i < nObjects; i++)
        {
            std::unique_ptr<VisualBasicObject> object;
            if (!getBytes(bytes, structureOffset + i * vbpod.structureSize(), vbpod.structureSize())
                || bytes.size() != vbpod.structureSize())
            {
                break;
            }

            offset = 0;
            DynamicBuffer structContent(bytes);
            vbpod.objectInfoAddr = structContent.read<std::uint32_t>(offset); offset += sizeof(vbpod.objectInfoAddr);
            vbpod.reserved = structContent.read<std::uint32_t>(offset); offset += sizeof(vbpod.reserved);
            vbpod.publicBytesAddr = structContent.read<std::uint32_t>(offset); offset += sizeof(vbpod.publicBytesAddr);
            vbpod.staticBytesAddr = structContent.read<std::uint32_t>(offset); offset += sizeof(vbpod.staticBytesAddr);
            vbpod.modulePublicAddr = structContent.read<std::uint32_t>(offset); offset += sizeof(vbpod.modulePublicAddr);
            vbpod.moduleStaticAddr = structContent.read<std::uint32_t>(offset); offset += sizeof(vbpod.moduleStaticAddr);
            vbpod.objectNameAddr = structContent.read<std::uint32_t>(offset); offset += sizeof(vbpod.objectNameAddr);
            vbpod.nMethods = structContent.read<std::uint32_t>(offset); offset += sizeof(vbpod.nMethods);
            vbpod.methodNamesAddr = structContent.read<std::uint32_t>(offset); offset += sizeof(vbpod.methodNamesAddr);
            vbpod.staticVarsCopyAddr = structContent.read<std::uint32_t>(offset); offset += sizeof(vbpod.staticVarsCopyAddr);
            vbpod.objectType = structContent.read<std::uint32_t>(offset); offset += sizeof(vbpod.objectType);
            vbpod.null = structContent.read<std::uint32_t>(offset); offset += sizeof(vbpod.null);

            std::uint64_t objectNameOffset;
            if (!getOffsetFromAddress(objectNameOffset, vbpod.objectNameAddr))
            {
                continue;
            }

            std::string objectName = readNullTerminatedAscii(allBytes.data(), allBytes.size(), objectNameOffset,
                VB_MAX_STRING_LEN, true);
            object = std::make_unique<VisualBasicObject>();
            object->setName(objectName);

            std::uint64_t methodAddrOffset;
            if (getOffsetFromAddress(methodAddrOffset, vbpod.methodNamesAddr))
            {
                for (std::size_t mIdx = 0; mIdx < vbpod.nMethods; mIdx++)
                {
                    if (!getBytes(bytes, methodAddrOffset + mIdx * sizeof(std::uint32_t), sizeof(std::uint32_t))
                        || bytes.size() != sizeof(std::uint32_t))
                    {
                        break;
                    }

                    auto methodNameAddr = *reinterpret_cast<std::uint32_t*>(bytes.data());

                    if (getEndianness() != Endianness::LITTLE)
                    {
                        methodNameAddr = byteSwap32(methodNameAddr);
                    }

                    std::uint64_t methodNameOffset;
                    if (!getOffsetFromAddress(methodNameOffset, methodNameAddr))
                    {
                        continue;
                    }

                    std::string methodName = readNullTerminatedAscii(allBytes.data(), allBytes.size(),
                        methodNameOffset, VB_MAX_STRING_LEN, true);

                    if (!methodName.empty())
                    {
                        object->addMethod(methodName);
                    }
                }
            }

            if (!objectName.empty() || object->getNumberOfMethods() > 0)
            {
                visualBasicInfo.addObject(std::move(object));
            }
        }

        return true;
    }

    bool PeFormat::parseVisualBasicObjectTable(std::size_t structureOffset)
    {
        std::vector<unsigned char> allBytes;
        if (!getBytes(allBytes, 0, pe_file->getLoadedFileLength()) && allBytes.size() != pe_file->getLoadedFileLength())
            return false;
        std::vector<std::uint8_t> bytes;
        std::size_t offset = 0;
        std::uint64_t projectNameOffset = 0;
        std::uint64_t objectDescriptorsOffset = 0;
        struct VBObjectTable vbot;
        std::string projName;

        if (!getBytes(bytes, structureOffset, vbot.structureSize()) || bytes.size() != vbot.structureSize())
        {
            return false;
        }

        DynamicBuffer structContent(bytes);
        vbot.null1 = structContent.read<std::uint32_t>(offset); offset += sizeof(vbot.null1);
        vbot.execCOMAddr = structContent.read<std::uint32_t>(offset); offset += sizeof(vbot.execCOMAddr);
        vbot.projecInfo2Addr = structContent.read<std::uint32_t>(offset); offset += sizeof(vbot.projecInfo2Addr);
        vbot.reserved = structContent.read<std::uint32_t>(offset); offset += sizeof(vbot.reserved);
        vbot.null2 = structContent.read<std::uint32_t>(offset); offset += sizeof(vbot.null2);
        vbot.projectObjectAddr = structContent.read<std::uint32_t>(offset); offset += sizeof(vbot.projectObjectAddr);
        std::memcpy(&vbot.objectGUID, static_cast<void*>(&bytes.data()[offset]), sizeof(vbot.objectGUID)); offset += sizeof(vbot.objectGUID);
        vbot.flagsCompileState = structContent.read<std::uint16_t>(offset); offset += sizeof(vbot.flagsCompileState);
        vbot.nObjects = structContent.read<std::uint16_t>(offset); offset += sizeof(vbot.nObjects);
        vbot.nCompiledObjects = structContent.read<std::uint16_t>(offset); offset += sizeof(vbot.nCompiledObjects);
        vbot.nUsedObjects = structContent.read<std::uint16_t>(offset); offset += sizeof(vbot.nUsedObjects);
        vbot.objectDescriptorsAddr = structContent.read<std::uint32_t>(offset); offset += sizeof(vbot.objectDescriptorsAddr);
        vbot.IDE1 = structContent.read<std::uint32_t>(offset); offset += sizeof(vbot.IDE1);
        vbot.IDE2 = structContent.read<std::uint32_t>(offset); offset += sizeof(vbot.IDE2);
        vbot.IDE3 = structContent.read<std::uint32_t>(offset); offset += sizeof(vbot.IDE3);
        vbot.projectNameAddr = structContent.read<std::uint32_t>(offset); offset += sizeof(vbot.projectNameAddr);
        vbot.LCID1 = structContent.read<std::uint32_t>(offset); offset += sizeof(vbot.LCID1);
        vbot.LCID2 = structContent.read<std::uint32_t>(offset); offset += sizeof(vbot.LCID2);
        vbot.IDE4 = structContent.read<std::uint32_t>(offset); offset += sizeof(vbot.IDE4);
        vbot.templateVesion = structContent.read<std::uint32_t>(offset); offset += sizeof(vbot.templateVesion);

        visualBasicInfo.setProjectPrimaryLCID(vbot.LCID1);
        visualBasicInfo.setProjectSecondaryLCID(vbot.LCID2);
        visualBasicInfo.setObjectTableGUID(vbot.objectGUID);

        if (!visualBasicInfo.hasProjectName() && getOffsetFromAddress(projectNameOffset, vbot.projectNameAddr))
        {
            projName = readNullTerminatedAscii(allBytes.data(), allBytes.size(), projectNameOffset,
                VB_MAX_STRING_LEN, true);
            visualBasicInfo.setProjectName(projName);
        }

        if (getOffsetFromAddress(objectDescriptorsOffset, vbot.objectDescriptorsAddr))
        {
            parseVisualBasicObjects(objectDescriptorsOffset, vbot.nObjects);
        }

        visualBasicInfo.computeObjectTableHashes();
        return true;
    }

    bool PeFormat::parseVisualBasicExternTable(std::size_t structureOffset, std::size_t nEntries)
    {
        std::vector<unsigned char> allBytes;
        if (!getBytes(allBytes, 0, pe_file->getLoadedFileLength()) && allBytes.size() != pe_file->getLoadedFileLength())
            return false;
        std::vector<std::uint8_t> bytes;
        struct VBExternTableEntry entry;
        struct VBExternTableEntryData entryData;
        std::uint64_t vbExternEntryDataOffset = 0;
        std::size_t offset = 0;

        for (std::size_t i = 0; i < nEntries; i++)
        {
            std::string moduleName;
            std::string apiName;

            if (!getBytes(bytes, structureOffset + i * entry.structureSize(), entry.structureSize())
                || bytes.size() != entry.structureSize())
            {
                break;
            }

            offset = 0;
            DynamicBuffer entryContent(bytes);
            entry.type = entryContent.read<std::uint32_t>(offset); offset += sizeof(entry.type);
            entry.importDataAddr = entryContent.read<std::uint32_t>(offset); offset += sizeof(entry.importDataAddr);

            if (entry.type != static_cast<std::uint32_t>(VBExternTableEntryType::external))
            {
                continue;
            }

            if (!getOffsetFromAddress(vbExternEntryDataOffset, entry.importDataAddr))
            {
                continue;
            }

            if (!getBytes(bytes, vbExternEntryDataOffset, entryData.structureSize())
                || bytes.size() != entryData.structureSize())
            {
                continue;
            }

            offset = 0;
            DynamicBuffer entryDataContent(bytes);
            entryData.moduleNameAddr = entryDataContent.read<std::uint32_t>(offset); offset += sizeof(entryData.moduleNameAddr);
            entryData.apiNameAddr = entryDataContent.read<std::uint32_t>(offset); offset += sizeof(entryData.apiNameAddr);

            std::uint64_t moduleNameOffset;
            if (getOffsetFromAddress(moduleNameOffset, entryData.moduleNameAddr))
            {
                moduleName = readNullTerminatedAscii(allBytes.data(), allBytes.size(),
                    moduleNameOffset, VB_MAX_STRING_LEN, true);
            }

            std::uint64_t apiNameOffset;
            if (getOffsetFromAddress(apiNameOffset, entryData.apiNameAddr))
            {
                apiName = readNullTerminatedAscii(allBytes.data(), allBytes.size(),
                    apiNameOffset, VB_MAX_STRING_LEN, true);
            }

            if (!moduleName.empty() || !apiName.empty())
            {
                auto ext = std::make_unique<VisualBasicExtern>();
                ext->setModuleName(moduleName);
                ext->setApiName(apiName);
                visualBasicInfo.addExtern(std::move(ext));
            }
        }

        visualBasicInfo.computeExternTableHashes();

        return true;
    }

    bool PeFormat::parseVisualBasicComRegistrationData(std::size_t structureOffset)
    {
        std::vector<unsigned char> allBytes;
        if (!getBytes(allBytes, 0, pe_file->getLoadedFileLength()) && allBytes.size() != pe_file->getLoadedFileLength())
            return false;
        std::vector<std::uint8_t> bytes;
        std::size_t offset = 0;
        struct VBCOMRData vbcrd;
        std::string projName;
        std::string helpFile;
        std::string projDesc;

        if (!getBytes(bytes, structureOffset, vbcrd.structureSize()) || bytes.size() != vbcrd.structureSize())
        {
            return false;
        }

        DynamicBuffer structContent(bytes);
        vbcrd.regInfoOffset = structContent.read<std::uint32_t>(offset); offset += sizeof(vbcrd.regInfoOffset);
        vbcrd.projNameOffset = structContent.read<std::uint32_t>(offset); offset += sizeof(vbcrd.projNameOffset);
        vbcrd.helpFileOffset = structContent.read<std::uint32_t>(offset); offset += sizeof(vbcrd.helpFileOffset);
        vbcrd.projDescOffset = structContent.read<std::uint32_t>(offset); offset += sizeof(vbcrd.projDescOffset);
        std::memcpy(&vbcrd.projCLSID, static_cast<void*>(&bytes.data()[offset]), sizeof(vbcrd.projCLSID)); offset += sizeof(vbcrd.projCLSID);
        vbcrd.projTlbLCID = structContent.read<std::uint32_t>(offset); offset += sizeof(vbcrd.projTlbLCID);
        vbcrd.unknown = structContent.read<std::uint32_t>(offset); offset += sizeof(vbcrd.unknown);
        vbcrd.tlbVerMajor = structContent.read<std::uint32_t>(offset); offset += sizeof(vbcrd.tlbVerMajor);
        vbcrd.tlbVerMinor = structContent.read<std::uint32_t>(offset); offset += sizeof(vbcrd.tlbVerMinor);

        visualBasicInfo.setTypeLibLCID(vbcrd.projTlbLCID);
        visualBasicInfo.setTypeLibMajorVersion(vbcrd.tlbVerMajor);
        visualBasicInfo.setTypeLibMinorVersion(vbcrd.tlbVerMinor);

        if (!visualBasicInfo.hasProjectName() && vbcrd.projNameOffset != 0)
        {
            projName = readNullTerminatedAscii(allBytes.data(), allBytes.size(),
                structureOffset + vbcrd.projNameOffset, VB_MAX_STRING_LEN, true);
        }
        if (!visualBasicInfo.hasProjectHelpFile() && vbcrd.helpFileOffset != 0)
        {
            helpFile = readNullTerminatedAscii(allBytes.data(), allBytes.size(),
                structureOffset + vbcrd.helpFileOffset, VB_MAX_STRING_LEN, true);
        }
        if (!visualBasicInfo.hasProjectDescription() && vbcrd.projDescOffset != 0)
        {
            projDesc = readNullTerminatedAscii(allBytes.data(), allBytes.size(),
                structureOffset + vbcrd.projDescOffset, VB_MAX_STRING_LEN, true);
        }

        visualBasicInfo.setTypeLibCLSID(vbcrd.projCLSID);

        if (vbcrd.regInfoOffset != 0)
        {
            parseVisualBasicComRegistrationInfo(structureOffset + vbcrd.regInfoOffset, structureOffset);
        }

        return true;
    }

    bool PeFormat::parseVisualBasicComRegistrationInfo(std::size_t structureOffset,
        std::size_t comRegDataOffset)
    {
        std::vector<unsigned char> allBytes;
        if (!getBytes(allBytes, 0, pe_file->getLoadedFileLength()) && allBytes.size() != pe_file->getLoadedFileLength())
            return false;
        std::vector<std::uint8_t> bytes;
        std::size_t offset = 0;
        struct VBCOMRInfo vbcri;
        std::string COMObjectName;
        std::string COMObjectDesc;

        if (!getBytes(bytes, structureOffset, vbcri.structureSize()) || bytes.size() != vbcri.structureSize())
        {
            return false;
        }

        DynamicBuffer structContent(bytes);
        vbcri.ifInfoOffset = structContent.read<std::uint32_t>(offset); offset += sizeof(vbcri.ifInfoOffset);
        vbcri.objNameOffset = structContent.read<std::uint32_t>(offset); offset += sizeof(vbcri.objNameOffset);
        vbcri.objDescOffset = structContent.read<std::uint32_t>(offset); offset += sizeof(vbcri.objDescOffset);
        vbcri.instancing = structContent.read<std::uint32_t>(offset); offset += sizeof(vbcri.instancing);
        vbcri.objID = structContent.read<std::uint32_t>(offset); offset += sizeof(vbcri.objID);
        std::memcpy(&vbcri.objCLSID, static_cast<void*>(&bytes.data()[offset]), sizeof(vbcri.objCLSID)); offset += sizeof(vbcri.objCLSID);
        vbcri.isInterfaceFlag = structContent.read<std::uint32_t>(offset); offset += sizeof(vbcri.isInterfaceFlag);
        vbcri.ifCLSIDOffset = structContent.read<std::uint32_t>(offset); offset += sizeof(vbcri.ifCLSIDOffset);
        vbcri.eventCLSIDOffset = structContent.read<std::uint32_t>(offset); offset += sizeof(vbcri.eventCLSIDOffset);
        vbcri.hasEvents = structContent.read<std::uint32_t>(offset); offset += sizeof(vbcri.hasEvents);
        vbcri.olemicsFlags = structContent.read<std::uint32_t>(offset); offset += sizeof(vbcri.olemicsFlags);
        vbcri.classType = structContent.read<std::uint32_t>(offset); offset += sizeof(vbcri.classType);
        vbcri.objectType = structContent.read<std::uint32_t>(offset); offset += sizeof(vbcri.objectType);
        vbcri.toolboxBitmap32 = structContent.read<std::uint32_t>(offset); offset += sizeof(vbcri.toolboxBitmap32);
        vbcri.defaultIcon = structContent.read<std::uint32_t>(offset); offset += sizeof(vbcri.defaultIcon);
        vbcri.isDesignerFlag = structContent.read<std::uint32_t>(offset); offset += sizeof(vbcri.isDesignerFlag);
        vbcri.designerDataOffset = structContent.read<std::uint32_t>(offset); offset += sizeof(vbcri.designerDataOffset);

        if (vbcri.objNameOffset != 0)
        {
            COMObjectName = readNullTerminatedAscii(allBytes.data(), allBytes.size(),
                comRegDataOffset + vbcri.objNameOffset, VB_MAX_STRING_LEN, true);
            visualBasicInfo.setCOMObjectName(COMObjectName);
        }
        if (vbcri.objDescOffset != 0)
        {
            COMObjectDesc = readNullTerminatedAscii(allBytes.data(), allBytes.size(),
                comRegDataOffset + vbcri.objDescOffset, VB_MAX_STRING_LEN, true);
            visualBasicInfo.setCOMObjectDescription(COMObjectDesc);
        }

        visualBasicInfo.setCOMObjectCLSID(vbcri.objCLSID);
        visualBasicInfo.setCOMObjectType(vbcri.objectType);

        if (vbcri.isInterfaceFlag != 0 && vbcri.ifCLSIDOffset != 0 &&
            getBytes(bytes, comRegDataOffset + vbcri.ifCLSIDOffset, 16) && bytes.size() == 16)
        {
            visualBasicInfo.setCOMObjectInterfaceCLSID(bytes.data());
        }

        if (vbcri.hasEvents != 0 && vbcri.eventCLSIDOffset != 0 &&
            getBytes(bytes, comRegDataOffset + vbcri.eventCLSIDOffset, 16) && bytes.size() == 16)
        {
            visualBasicInfo.setCOMObjectEventsCLSID(bytes.data());
        }

        return true;
    }

    void PeFormat::loadVisualBasicHeader()
    {
        std::vector<unsigned char> allBytes;
        if (!getBytes(allBytes, 0, pe_file->getLoadedFileLength()) && allBytes.size() != pe_file->getLoadedFileLength())
            return;
        std::vector<std::uint8_t> bytes;
        std::uint64_t version = 0;
        std::uint64_t vbHeaderAddress = 0;
        std::uint64_t vbHeaderOffset = 0;
        std::uint64_t vbProjectInfoOffset = 0;
        std::uint64_t vbComDataRegistrationOffset = 0;
        std::string projLanguageDLL;
        std::string projBackupLanguageDLL;
        std::string projExeName;
        std::string projDesc;
        std::string helpFile;
        std::string projName;
        std::size_t offset = 0;
        struct VBHeader vbh;

        if (!isVisualBasic(version))
        {
            return;
        }

        // first instruction is expected to be PUSH <vbHeaderAddress> (0x68 <b0> <b1> <b2> <b3>)
        if (!getEpBytes(bytes, 5) || bytes.size() != 5 || bytes[0] != 0x68)
        {
            return;
        }

        vbHeaderAddress = bytes[4] << 24 | bytes[3] << 16 | bytes[2] << 8 | bytes[1];
        if (!getOffsetFromAddress(vbHeaderOffset, vbHeaderAddress))
        {
            return;
        }

        if (!getBytes(bytes, vbHeaderOffset, vbh.structureSize()) || bytes.size() != vbh.structureSize())
        {
            return;
        }

        DynamicBuffer structContent(bytes);
        vbh.signature = structContent.read<std::uint32_t>(offset); offset += sizeof(vbh.signature);
        vbh.runtimeBuild = structContent.read<std::uint16_t>(offset); offset += sizeof(vbh.runtimeBuild);
        std::memcpy(&vbh.languageDLL, static_cast<void*>(&bytes.data()[offset]), sizeof(vbh.languageDLL)); offset += sizeof(vbh.languageDLL);
        std::memcpy(&vbh.backupLanguageDLL, static_cast<void*>(&bytes.data()[offset]), sizeof(vbh.backupLanguageDLL)); offset += sizeof(vbh.backupLanguageDLL);
        vbh.runtimeDLLVersion = structContent.read<std::uint16_t>(offset); offset += sizeof(vbh.runtimeDLLVersion);
        vbh.LCID1 = structContent.read<std::uint32_t>(offset); offset += sizeof(vbh.LCID1);
        vbh.LCID2 = structContent.read<std::uint32_t>(offset); offset += sizeof(vbh.LCID2);
        vbh.subMainAddr = structContent.read<std::uint32_t>(offset); offset += sizeof(vbh.subMainAddr);
        vbh.projectInfoAddr = structContent.read<std::uint32_t>(offset); offset += sizeof(vbh.projectInfoAddr);
        vbh.MDLIntObjsFlags = structContent.read<std::uint32_t>(offset); offset += sizeof(vbh.MDLIntObjsFlags);
        vbh.MDLIntObjsFlags2 = structContent.read<std::uint32_t>(offset); offset += sizeof(vbh.MDLIntObjsFlags2);
        vbh.threadFlags = structContent.read<std::uint32_t>(offset); offset += sizeof(vbh.threadFlags);
        vbh.nThreads = structContent.read<std::uint32_t>(offset); offset += sizeof(vbh.nThreads);
        vbh.nForms = structContent.read<std::uint16_t>(offset); offset += sizeof(vbh.nForms);
        vbh.nExternals = structContent.read<std::uint16_t>(offset); offset += sizeof(vbh.nExternals);
        vbh.nThunks = structContent.read<std::uint32_t>(offset); offset += sizeof(vbh.nThunks);
        vbh.GUITableAddr = structContent.read<std::uint32_t>(offset); offset += sizeof(vbh.GUITableAddr);
        vbh.externalTableAddr = structContent.read<std::uint32_t>(offset); offset += sizeof(vbh.externalTableAddr);
        vbh.COMRegisterDataAddr = structContent.read<std::uint32_t>(offset); offset += sizeof(vbh.COMRegisterDataAddr);
        vbh.projExeNameOffset = structContent.read<std::uint32_t>(offset); offset += sizeof(vbh.projExeNameOffset);
        vbh.projDescOffset = structContent.read<std::uint32_t>(offset); offset += sizeof(vbh.projDescOffset);
        vbh.helpFileOffset = structContent.read<std::uint32_t>(offset); offset += sizeof(vbh.helpFileOffset);
        vbh.projNameOffset = structContent.read<std::uint32_t>(offset); offset += sizeof(vbh.projNameOffset);

        if (vbh.signature != VBHEADER_SIGNATURE)
        {
            return;
        }

        if (vbh.projExeNameOffset != 0)
        {
            projExeName = readNullTerminatedAscii(allBytes.data(), allBytes.size(),
                vbHeaderOffset + vbh.projExeNameOffset, VB_MAX_STRING_LEN, true);
            visualBasicInfo.setProjectExeName(projExeName);
        }
        if (vbh.projDescOffset != 0)
        {
            projDesc = readNullTerminatedAscii(allBytes.data(), allBytes.size(),
                vbHeaderOffset + vbh.projDescOffset, VB_MAX_STRING_LEN, true);
            visualBasicInfo.setProjectDescription(projDesc);
        }
        if (vbh.helpFileOffset != 0)
        {
            helpFile = readNullTerminatedAscii(allBytes.data(), allBytes.size(),
                vbHeaderOffset + vbh.helpFileOffset, VB_MAX_STRING_LEN, true);
            visualBasicInfo.setProjectHelpFile(helpFile);
        }
        if (vbh.projNameOffset != 0)
        {
            projName = readNullTerminatedAscii(allBytes.data(), allBytes.size(),
                vbHeaderOffset + vbh.projNameOffset, VB_MAX_STRING_LEN, true);
            visualBasicInfo.setProjectName(projName);
        }

        for (size_t i = 0; i < sizeof(vbh.languageDLL) && vbh.languageDLL[i]; i++)
        {
            projLanguageDLL.push_back(vbh.languageDLL[i]);
        }
        for (size_t i = 0; i < sizeof(vbh.backupLanguageDLL) && vbh.backupLanguageDLL[i]; i++)
        {
            projBackupLanguageDLL.push_back(vbh.backupLanguageDLL[i]);
        }
        visualBasicInfo.setLanguageDLL(projLanguageDLL);
        visualBasicInfo.setBackupLanguageDLL(projBackupLanguageDLL);
        visualBasicInfo.setLanguageDLLPrimaryLCID(vbh.LCID1);
        visualBasicInfo.setLanguageDLLSecondaryLCID(vbh.LCID2);

        if (getOffsetFromAddress(vbProjectInfoOffset, vbh.projectInfoAddr))
        {
            parseVisualBasicProjectInfo(vbProjectInfoOffset);
        }

        if (getOffsetFromAddress(vbComDataRegistrationOffset, vbh.COMRegisterDataAddr))
        {
            parseVisualBasicComRegistrationData(vbComDataRegistrationOffset);
        }
    }

    bool PeFormat::getOffsetFromAddress(std::uint64_t& result, std::uint64_t address) const
    {
        const auto& secSeg = getPeSection(address);
        if (!secSeg)
        {
            return false;
        }

        auto secSegAddr = secSeg->getAddress();
        if (secSegAddr > address)
        {
            return false;
        }

        result = secSeg->getOffset() + (address - secSegAddr);
        return true;
    }

    void PeFormat::loadPdbInfo()
    {
        if (!IsValid()) return;
        auto intToHex = [&](uint64_t number) ->std::string {
            std::stringstream ss;
            ss << std::hex << number;
            return ss.str();
        };

        switch (pe_file->getBits()) {
        case 32:
        {
            auto pe_file32 = static_cast<PeLib::PeFileT<32>*>(pe_file.get());
            auto& debug32 = pe_file32->debugDir();

            for (std::size_t i = 0, e = debug32.calcNumberOfEntries(); i < e; ++i)
            {
                std::vector<byte> data;
                data = debug32.getData(i);
                if(!data.size())
                {
                    continue;
                }

                std::string dataString;
                bytesToString(data, dataString);
                const auto size = std::min(data.size(), dataString.length());
                if (size < 4)
                {
                    continue;
                }
                const auto prefix = dataString.substr(0, 4);
                if ((prefix != "RSDS" && prefix != "NB10") || (prefix == "RSDS" && size < PeLib::MINIMAL_PDB_RSDS_INFO_LENGTH) ||
                    (prefix == "NB10" && size < PeLib::MINIMAL_PDB_NB10_INFO_LENGTH))
                {
                    continue;
                }

                unsigned long long timestamp = debug32.getTimeDateStamp(i);
                unsigned long long pointerToRawData = debug32.getPointerToRawData(i);

                const auto isRsds = (prefix == "RSDS");
                auto pdbInfo = std::make_unique<PeLib::PdbInfo>(PeLib::PdbInfo());
                if (!pdbInfo) {
                    continue;
                }
                pdbInfo->setType(prefix);
                pdbInfo->setTimeStamp(timestamp);
                const auto guidOffset = pointerToRawData + prefix.length() + (isRsds ? 0 : 4);
                std::uint64_t res1;
                if (isRsds)
                {
                    std::uint64_t res2, res3, res4, res5;
                    if (getXByteOffset(guidOffset, 4, res1, Endianness::UNKNOWN) && getXByteOffset(guidOffset + 4, 2, res2, Endianness::UNKNOWN) &&
                        getXByteOffset(guidOffset + 6, 2, res3, Endianness::UNKNOWN) && getXByteOffset(guidOffset + 8, 2, res4, getEndianness()) &&
                        getXByteOffset(guidOffset + 10, 6, res5, getEndianness()))
                    {
                        pdbInfo->setGuid(toUpper(intToHex(res1) + "-" + intToHex(res2) + "-" +
                            intToHex(res3) + "-" + intToHex(res4) + "-" + intToHex(res5)));
                    }
                }
                else if (getXByteOffset(guidOffset, 4, res1, Endianness::UNKNOWN))
                {
                    pdbInfo->setGuid(toUpper(intToHex(res1)));
                }

                const auto ageOffset = guidOffset + (isRsds ? 16 : 4);
                if (getXByteOffset(ageOffset, 4, res1, Endianness::UNKNOWN))
                {
                    pdbInfo->setAge(res1);
                }
                if (getNTBSOffset(ageOffset + 4, dataString))
                {
                    pdbInfo->setPath(dataString);
                }
                pdbInfolist.push_back(std::move(pdbInfo));
                break;
            }
        }
        break;
        case 64:
        {
            auto pe_file64 = static_cast<PeLib::PeFileT<64>*>(pe_file.get());
            auto& debug64 = pe_file64->debugDir();

            for (std::size_t i = 0, e = debug64.calcNumberOfEntries(); i < e; ++i)
            {
                std::vector<byte> data;
                data = debug64.getData(i);
                if (!data.size())
                {
                    continue;
                }

                std::string dataString;
                bytesToString(data, dataString);
                const auto size = std::min(data.size(), dataString.length());
                if (size < 4)
                {
                    continue;
                }
                const auto prefix = dataString.substr(0, 4);
                if ((prefix != "RSDS" && prefix != "NB10") || (prefix == "RSDS" && size < PeLib::MINIMAL_PDB_RSDS_INFO_LENGTH) ||
                    (prefix == "NB10" && size < PeLib::MINIMAL_PDB_NB10_INFO_LENGTH))
                {
                    continue;
                }

                unsigned long long timestamp = debug64.getTimeDateStamp(i);
                unsigned long long pointerToRawData = debug64.getPointerToRawData(i);

                const auto isRsds = (prefix == "RSDS");
                auto pdbInfo = std::make_unique<PeLib::PdbInfo>(PeLib::PdbInfo());
                if (!pdbInfo) {
                    continue;
                }
                pdbInfo->setType(prefix);
                pdbInfo->setTimeStamp(timestamp);
                const auto guidOffset = pointerToRawData + prefix.length() + (isRsds ? 0 : 4);
                std::uint64_t res1;
                if (isRsds)
                {
                    std::uint64_t res2, res3, res4, res5;
                    if (getXByteOffset(guidOffset, 4, res1, Endianness::UNKNOWN) && getXByteOffset(guidOffset + 4, 2, res2, Endianness::UNKNOWN) &&
                        getXByteOffset(guidOffset + 6, 2, res3, Endianness::UNKNOWN) && getXByteOffset(guidOffset + 8, 2, res4, getEndianness()) &&
                        getXByteOffset(guidOffset + 10, 6, res5, getEndianness()))
                    {
                        pdbInfo->setGuid(toUpper(intToHex(res1) + "-" + intToHex(res2) + "-" +
                            intToHex(res3) + "-" + intToHex(res4) + "-" + intToHex(res5)));
                    }
                }
                else if (getXByteOffset(guidOffset, 4, res1, Endianness::UNKNOWN))
                {
                    pdbInfo->setGuid(toUpper(intToHex(res1)));
                }

                const auto ageOffset = guidOffset + (isRsds ? 16 : 4);
                if (getXByteOffset(ageOffset, 4, res1, Endianness::UNKNOWN))
                {
                    pdbInfo->setAge(res1);
                }
                if (getNTBSOffset(ageOffset + 4, dataString))
                {
                    pdbInfo->setPath(dataString);
                }
                pdbInfolist.push_back(std::move(pdbInfo));
                break;
            }
        }
        break;
        default:
            ;
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

        return getPeSectionByRva(ep);
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

    const Import* PeFormat::getImport(const std::string& func_name) const
    {
        return importTable ? importTable->getImport(func_name) : nullptr;
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

    const Export* PeFormat::getExport(const std::string& func_name) const
    {
        return exportTable ? exportTable->getExport(func_name) : nullptr;
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

    bool PeFormat::hasImportsTable() const
    {
        if (!importTable)
            return false;
        return importTable->hasImports();
    }

    bool PeFormat::hasExportsTable() const
    {
        if (!exportTable)
            return false;
        return exportTable->hasExports();
    }

    bool PeFormat::hasResources() const
    {
        if (!resourceTable)
            return false;
        return resourceTable->hasResources();
    }

    bool PeFormat::hasSecurity() const
    {
        if (!IsValid()) return false;
        return pe_file->securityDir().hasSecurity();
    }

    bool PeFormat::hasReloc() const
    {
        if (!IsValid()) return false;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::PeFile32* pe_file32 = static_cast<PeLib::PeFile32*>(pe_file.get());
            return (pe_file32->relocDir().size() > 0);
        }
        break;
        case 64:
        {
            PeLib::PeFile64* pe_file64 = static_cast<PeLib::PeFile64*>(pe_file.get());
            return (pe_file64->relocDir().size() > 0);
        }
        break;
        default:
            ;
        }
        return false;
    }

    bool PeFormat::hasTls() const
    {
        if (!tlsInfo)
            return false;
        return true;
    }

    bool PeFormat::hasBoundImportsTable() const
    {
        if (!IsValid()) return false;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::PeFile32* pe_file32 = static_cast<PeLib::PeFile32*>(pe_file.get());
            return (pe_file32->boundImpDir().calcNumberOfModules() > 0);
        }
        break;
        case 64:
        {
            PeLib::PeFile64* pe_file64 = static_cast<PeLib::PeFile64*>(pe_file.get());
            return (pe_file64->boundImpDir().calcNumberOfModules() > 0);
        }
        break;
        default:
            ;
        }
        return false;
    }

    bool PeFormat::hasDelayImportsTable() const
    {
        if (!IsValid()) return false;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::PeFile32* pe_file32 = static_cast<PeLib::PeFile32*>(pe_file.get());
            return (pe_file32->delayImports().getNumberOfFiles() > 0);
        }
        break;
        case 64:
        {
            PeLib::PeFile64* pe_file64 = static_cast<PeLib::PeFile64*>(pe_file.get());
            return (pe_file64->delayImports().getNumberOfFiles() > 0);
        }
        break;
        default:
            ;
        }
        return false;
    }

    bool PeFormat::hasDebug() const
    {
        if (!IsValid()) return false;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::PeFile32* pe_file32 = static_cast<PeLib::PeFile32*>(pe_file.get());
            return (pe_file32->debugDir().size() > 0);
        }
        break;
        case 64:
        {
            PeLib::PeFile64* pe_file64 = static_cast<PeLib::PeFile64*>(pe_file.get());
            return (pe_file64->debugDir().size() > 0);
        }
        break;
        default:
            ;
        }
        return false;
    }

    std::string PeFormat::getSubsystemDesc() const
    {
        std::string subsystem_str;
        uint16_t subsystem = getSubsystem();
        switch (subsystem) {
        case IMAGE_SUBSYSTEM_NATIVE:
            subsystem_str = "native";
            break;
        case IMAGE_SUBSYSTEM_WINDOWS_GUI:
            subsystem_str = "win_gui";
            break;
        case IMAGE_SUBSYSTEM_WINDOWS_CUI:
            subsystem_str = "win_cui";
            break;
        case IMAGE_SUBSYSTEM_OS2_CUI:
            subsystem_str = "os2_cui";
            break;
        case IMAGE_SUBSYSTEM_POSIX_CUI:
            subsystem_str = "posix_cui";
            break;
        case IMAGE_SUBSYSTEM_NATIVE_WINDOWS:
            subsystem_str = "windows";
            break;
        case IMAGE_SUBSYSTEM_WINDOWS_CE_GUI:
            subsystem_str = "ce_gui";
            break;
        case IMAGE_SUBSYSTEM_EFI_APPLICATION:
            subsystem_str = "efi_application";
            break;
        case IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER:
            subsystem_str = "efi_boot_driver";
            break;
        case IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER:
            subsystem_str = "efi_runtime_driver";
            break;
        case IMAGE_SUBSYSTEM_EFI_ROM:
            subsystem_str = "efi_rom";
            break;
        case IMAGE_SUBSYSTEM_XBOX:
            subsystem_str = "xbox";
            break;
        case IMAGE_SUBSYSTEM_WINDOWS_BOOT_APPLICATION:
            subsystem_str = "win_boot_application";
            break;
        case IMAGE_SUBSYSTEM_XBOX_CODE_CATALOG:
            subsystem_str = "xbox_code_catalog";
            break;
        case IMAGE_SUBSYSTEM_UNKNOWN:
        default:
            subsystem_str = "unknown";
        }
        return subsystem_str;
    }

    uint16_t PeFormat::getSubsystem() const
    {
        if (!IsValid()) return 0;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::PeFile32* pe_file32 = static_cast<PeLib::PeFile32*>(pe_file.get());
            return pe_file32->peHeader().getSubsystem();
        }
        break;
        case 64:
        {
            PeLib::PeFile64* pe_file64 = static_cast<PeLib::PeFile64*>(pe_file.get());
            return pe_file64->peHeader().getSubsystem();
        }
        break;
        default:
            ;
        }
        return 0;
    }

    uint16_t PeFormat::getMinorOsVersion() const
    {
        if (!IsValid()) return 0;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::PeFile32* pe_file32 = static_cast<PeLib::PeFile32*>(pe_file.get());
            return pe_file32->peHeader().getMinorOperatingSystemVersion();
        }
        break;
        case 64:
        {
            PeLib::PeFile64* pe_file64 = static_cast<PeLib::PeFile64*>(pe_file.get());
            return pe_file64->peHeader().getMinorOperatingSystemVersion();
        }
        break;
        default:
            ;
        }
        return 0;
    }

    uint16_t PeFormat::getMajorOsVersion() const
    {
        if (!IsValid()) return 0;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::PeFile32* pe_file32 = static_cast<PeLib::PeFile32*>(pe_file.get());
            return pe_file32->peHeader().getMajorOperatingSystemVersion();
        }
        break;
        case 64:
        {
            PeLib::PeFile64* pe_file64 = static_cast<PeLib::PeFile64*>(pe_file.get());
            return pe_file64->peHeader().getMajorOperatingSystemVersion();
        }
        break;
        default:
            ;
        }
        return 0;
    }

    uint16_t PeFormat::getMinorSubsystemVersion() const
    {
        if (!IsValid()) return 0;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::PeFile32* pe_file32 = static_cast<PeLib::PeFile32*>(pe_file.get());
            return pe_file32->peHeader().getMinorSubsystemVersion();
        }
        break;
        case 64:
        {
            PeLib::PeFile64* pe_file64 = static_cast<PeLib::PeFile64*>(pe_file.get());
            return pe_file64->peHeader().getMinorSubsystemVersion();
        }
        break;
        default:
            ;
        }
        return 0;
    }

    uint16_t PeFormat::getMajorSubsystemVersion() const
    {
        if (!IsValid()) return 0;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::PeFile32* pe_file32 = static_cast<PeLib::PeFile32*>(pe_file.get());
            return pe_file32->peHeader().getMajorSubsystemVersion();
        }
        break;
        case 64:
        {
            PeLib::PeFile64* pe_file64 = static_cast<PeLib::PeFile64*>(pe_file.get());
            return pe_file64->peHeader().getMajorSubsystemVersion();
        }
        break;
        default:
            ;
        }
        return 0;
    }

    int32_t PeFormat::getPeHeaderStart() const
    {
        if (!IsValid()) return 0;
        return pe_file->mzHeader().getAddressOfPeHeader();
    }

    uint16_t PeFormat::getCharacteristics() const
    {
        if (!IsValid()) return 0;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::PeFile32* pe_file32 = static_cast<PeLib::PeFile32*>(pe_file.get());
            return pe_file32->peHeader().getCharacteristics();
        }
        break;
        case 64:
        {
            PeLib::PeFile64* pe_file64 = static_cast<PeLib::PeFile64*>(pe_file.get());
            return pe_file64->peHeader().getCharacteristics();
        }
        break;
        default:
            ;
        }
        return 0;
    }

    uint16_t PeFormat::getDllCharacteristics() const
    {
        if (!IsValid()) return 0;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::PeFile32* pe_file32 = static_cast<PeLib::PeFile32*>(pe_file.get());
            return pe_file32->peHeader().getDllCharacteristics();
        }
        break;
        case 64:
        {
            PeLib::PeFile64* pe_file64 = static_cast<PeLib::PeFile64*>(pe_file.get());
            return pe_file64->peHeader().getDllCharacteristics();
        }
        break;
        default:
            ;
        }
        return 0;
    }

    uint32_t PeFormat::getPeSignature() const
    {
        if (!IsValid()) return 0;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::PeFile32* pe_file32 = static_cast<PeLib::PeFile32*>(pe_file.get());
            return pe_file32->peHeader().getNtSignature();
        }
        break;
        case 64:
        {
            PeLib::PeFile64* pe_file64 = static_cast<PeLib::PeFile64*>(pe_file.get());
            return pe_file64->peHeader().getNtSignature();
        }
        break;
        default:
            ;
        }
        return 0;
    }

    uint32_t PeFormat::getMagic() const
    {
        if (!IsValid()) return 0;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::PeFile32* pe_file32 = static_cast<PeLib::PeFile32*>(pe_file.get());
            return pe_file32->peHeader().getMagic();
        }
        break;
        case 64:
        {
            PeLib::PeFile64* pe_file64 = static_cast<PeLib::PeFile64*>(pe_file.get());
            return pe_file64->peHeader().getMagic();
        }
        break;
        default:
            ;
        }
        return 0;
    }

    unsigned int PeFormat::getChecksumFileOffset() const
    {
        if (!IsValid()) return false;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::PeHeaderT<32>& pe_header32 = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->peHeader();
            return pe_header32.getChecksumFileOffset();
        }
        break;
        case 64:
        {
            PeLib::PeHeaderT<64>& pe_header64 = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->peHeader();
            return pe_header64.getChecksumFileOffset();
        }
        break;
        default:
            ;
        }
        return 0;
    }

    bool PeFormat::verifyChecksum() const
    {
        uint32_t checksum = calculateChecksum();
        return (checksum == getChecksum());
    }

    uint32_t PeFormat::calculateChecksum() const
    {
        //Checksum value
        uint64_t checksum = 0;
        if (!IsValid()) return 0;
        try
        {
            //Calculate PE checksum
            uint64_t top = 0xFFFFFFFF;
            top++;

            unsigned int checksum_offset = getChecksumFileOffset();
            size_t filesize = pe_file->getLoadedFileLength();
            //Calculate checksum for each byte of file
            std::vector<unsigned char> result;
            if (!getBytes(result, 0, filesize) && result.size() != filesize)
                return 0;
            const unsigned char* filep = result.data();

            size_t i = 0;
            for (; i + 3 < filesize; i += 4)
            {
                if (i == checksum_offset)
                    continue;
                // Read DWORD from file
                // Pad 0s if the file is not aligned to 4 bytes
                uint32_t* dw2 = (uint32_t*)&filep[i];
                // Calculate checksum
                checksum = (checksum & 0xffffffff) + *dw2 + (checksum >> 32);
                if (checksum > top)
                    checksum = (checksum & 0xffffffff) + (checksum >> 32);
            }

            // file may be not alligned ?
            // calculate the rest of the file, 3,2, or 1 byte(s) remained
            for (; i < filesize; i += 4)
            {
                uint32_t dw = ((uint32_t)(((uint8_t)filep[i]))
                    | (uint32_t)(i + 1 >= filesize ? 0 : ((uint8_t)filep[i + 1] << 8))
                    | (uint32_t)(i + 2 >= filesize ? 0 : ((uint8_t)filep[i + 2]) << 16))
                    | (uint32_t)(i + 3 >= filesize ? 0 : ((uint8_t)filep[i + 3]) << 24);

                // Calculate checksum
                checksum = (checksum & 0xffffffff) + dw + (checksum >> 32);
                if (checksum > top)
                    checksum = (checksum & 0xffffffff) + (checksum >> 32);
            }

            // Finish checksum
            checksum = (checksum & 0xffff) + (checksum >> 16);
            checksum = (checksum)+(checksum >> 16);
            checksum = checksum & 0xffff;

            checksum += static_cast<uint32_t>(filesize);
        }
        catch (const std::exception&)
        {

        }
        //Return checksum
        return static_cast<uint32_t>(checksum);
    }

    uint32_t PeFormat::getSizeofNtHeader() const
    {
        if (!IsValid()) return 0;
        switch (pe_file->getBits()) {
        case 32:
        {
            return sizeof(PeLib::PELIB_IMAGE_NT_HEADERS<32>);
        }
        break;
        case 64:
        {
            return sizeof(PeLib::PELIB_IMAGE_NT_HEADERS<64>);
        }
        break;
        default:
            ;
        }
        return 0;
    }

    uint32_t PeFormat::getBaseOfCode() const
    {
        if (!IsValid()) return 0;
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::PeFile32* pe_file32 = static_cast<PeLib::PeFile32*>(pe_file.get());
            return pe_file32->peHeader().getBaseOfCode();
        }
        break;
        case 64:
        {
            PeLib::PeFile64* pe_file64 = static_cast<PeLib::PeFile64*>(pe_file.get());
            return pe_file64->peHeader().getBaseOfCode();
        }
        break;
        default:
            ;
        }
        return 0;
    }

    PeFormat::ExportDirectoryInfo PeFormat::get_export_directory_info() const
    {
        if (!IsValid())
            return {};
        if (!hasExportsTable())
            return {};

        ExportDirectoryInfo info = {};
        switch (pe_file->getBits()) {
        case 32:
        {
            PeLib::ExportDirectoryT<32>& exports = static_cast<PeLib::PeFileT<32>*>(pe_file.get())->expDir();
            info.name = exports.getName();
            info.characteristics = exports.getCharacteristics();
            info.major_version = exports.getMajorVersion();
            info.minor_version = exports.getMinorVersion();
            info.timestamp = exports.getTimeDateStamp();
            info.number_of_functions = exports.getNumberOfFunctions();
            info.number_of_names = exports.getNumberOfNames();
            info.ordinal_base = exports.getBase();
            info.address_of_functions = exports.getAddressOfFunctions();
            info.address_of_names = exports.getAddressOfNames();
            info.address_of_name_ordinals = exports.getAddressOfNameOrdinals();
        }
        break;
        case 64:
        {
            PeLib::ExportDirectoryT<64>& exports = static_cast<PeLib::PeFileT<64>*>(pe_file.get())->expDir();
            info.name = exports.getName();
            info.characteristics = exports.getCharacteristics();
            info.major_version = exports.getMajorVersion();
            info.minor_version = exports.getMinorVersion();
            info.timestamp = exports.getTimeDateStamp();
            info.number_of_functions = exports.getNumberOfFunctions();
            info.number_of_names = exports.getNumberOfNames();
            info.ordinal_base = exports.getBase();
            info.address_of_functions = exports.getAddressOfFunctions();
            info.address_of_names = exports.getAddressOfNames();
            info.address_of_name_ordinals = exports.getAddressOfNameOrdinals();
        }
        break;
        default:
            ;
        }
        return info;
    }

    bool PeFormat::hasOverlay() const
    {
        return has_overlay_data;
    }

    double PeFormat::caculateEntropy()
    {
        if (!entropy) {
            uint32_t caculate_size = pe_file->getLoadedFileLength();
            if (has_overlay_data) {
                caculate_size -= (uint32_t)overlay_size;
            }
            entropy = pe_file->caculateEntropy(0, caculate_size);
        }
        return entropy;
    }

    std::size_t PeFormat::getNibbleLength() const
    {
        return 4;
    }

    std::size_t PeFormat::getByteLength() const
    {
        return 8;
    }

    std::size_t PeFormat::getNumberOfNibblesInByte() const
    {
        return !getNibbleLength() ? 0 : !(getByteLength() % getNibbleLength()) ? getByteLength() / getNibbleLength() : 0;
    }

    std::size_t PeFormat::nibblesFromBytes(std::size_t bytes) const
    {
        return bytes * getNumberOfNibblesInByte();
    }

    std::size_t PeFormat::bytesFromNibbles(std::size_t nibbles) const
    {
        const auto nibblesInBytes = getNumberOfNibblesInByte();
        return !nibblesInBytes ? 0 : nibbles / nibblesInBytes;
    }

    std::size_t PeFormat::bytesFromNibblesRounded(std::size_t nibbles) const
    {
        const auto nibblesInBytes = getNumberOfNibblesInByte();
        return !nibblesInBytes ? 0 : nibbles / nibblesInBytes + (nibbles % nibblesInBytes ? 1 : 0);
    }

    bool PeFormat::getStringFromEnd(std::string& result, unsigned long long numberOfBytes) const
    {
        numberOfBytes = std::min(numberOfBytes, static_cast<unsigned long long>(getLoadedFileLength()));
        return getHexBytes(result, getLoadedFileLength() - numberOfBytes, numberOfBytes);
    }

    const Resource* PeFormat::getVersionResource() const
    {
        return resourceTable ? resourceTable->getResourceWithType(PeLib::PELIB_RT_VERSION) : nullptr;
    }

    const Resource* PeFormat::getManifestResource() const
    {
        return resourceTable ? resourceTable->getResourceWithType(PeLib::PELIB_RT_MANIFEST) : nullptr;
    }

    bool PeFormat::getNTBSImpl(
        const GetNByteFn& get1ByteFn,
        std::uint64_t address,
        std::string& res, std::size_t size)
    {
        std::uint64_t c = 0;
        auto suc = get1ByteFn(address, 1, c, getEndianness());
        res.clear();

        while (suc && (c || size))
        {
            res += c;
            if (size && res.length() == size)
            {
                break;
            }
            suc = get1ByteFn(++address, 1, c, getEndianness());
        }

        return !res.empty();
    }

    bool PeFormat::getNTBSOffset(std::uint64_t offset, std::string& res, std::size_t size)
    {
        using namespace std::placeholders;

        GetNByteFn get1ByteFn = std::bind(&PeFormat::getXByteOffset, this, _1, _2, _3, _4);
        return getNTBSImpl(get1ByteFn, offset, res, size);
    }

    bool PeFormat::getNTBS(std::uint64_t address, std::string& res, std::size_t size)
    {
        using namespace std::placeholders;

        GetNByteFn get1ByteFn = std::bind(&PeFormat::getXByte,this, _1, _2, _3, _4);
        return getNTBSImpl(get1ByteFn, address, res, size);
    }

    void PeFormat::detectModuleVersionId()
    {
        return;
    }

    /**
     * Detects TypeLib ID (GUID) out of .NET tables.
     */
    void PeFormat::detectTypeLibId()
    {
        return;
    }

    /**
     * Detects and reconstructs .NET types such as classes, methods, fields, properties etc.
     */
    void PeFormat::detectDotnetTypes()
    {
        return;
    }
} // namespace cchips
