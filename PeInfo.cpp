#include "ExceptionThrow.h"
#include "utils.h"
#include "PeInfo.h"
#include <filesystem>

namespace cchips {
    namespace fs = std::filesystem;

    const std::map<std::pair<std::size_t, std::string>, FileDetector::file_format> FileDetector::_magic_format_map = {
        {{0, "MZ"}, FileDetector::file_format::format_pe},
        {{0, "ZM"}, FileDetector::file_format::format_pe},
        {{0, "\x7F""ELF"}, FileDetector::file_format::format_elf},
        {{0, ":"}, FileDetector::file_format::format_intel_hex},
        {{0, "\xFE""\xED""\xFA""\xCE"}, FileDetector::file_format::format_macho}, // Mach-O
        {{0, "\xFE""\xED""\xFA""\xCF"}, FileDetector::file_format::format_macho}, // Mach-O
        {{0, "\xCE""\xFA""\xED""\xFE"}, FileDetector::file_format::format_macho}, // Mach-O
        {{0, "\xCF""\xFA""\xED""\xFE"}, FileDetector::file_format::format_macho}, // Mach-O
        {{0, "\xCA""\xFE""\xBA""\xBE"}, FileDetector::file_format::format_macho}  // Mach-O fat binary
    };

    const std::map<std::pair<std::size_t, std::string>, FileDetector::file_format> FileDetector::_unknown_format_map = {
        {{0, "\x7""\x1""\x64""\x00"}, FileDetector::file_format::format_unknown}, // a.out
        {{0, "PS-X EXE"}, FileDetector::file_format::format_unknown}, // PS-X
        {{257, "ustar"}, FileDetector::file_format::format_unknown} // tar
    };

    const std::map<FileDetector::file_format, std::string> FileDetector::_file_format_map = {
        {format_undetectable, "undetectable"},
        {format_unknown, "unknown"},
        {format_pe, "pe"},
        {format_dos, "dos"},
        {format_elf, "elf"},
        {format_coff, "coff"},
        {format_macho, "macho"},
        {format_intel_hex, "intel_hex"},
        {format_raw, "raw"}
    };

    byte_array_buffer::int_type byte_array_buffer::underflow()
    {
        if (current_ == end_)
        {
            return traits_type::eof();
        }

        return traits_type::to_int_type(*current_);
    }

    byte_array_buffer::int_type byte_array_buffer::uflow()
    {
        if (current_ == end_)
        {
            return traits_type::eof();
        }

        return traits_type::to_int_type(*current_++);
    }

    byte_array_buffer::int_type byte_array_buffer::pbackfail(int_type ch)
    {
        if (current_ == begin_ || (ch != traits_type::eof() && ch != current_[-1]))
        {
            return traits_type::eof();
        }

        return traits_type::to_int_type(*--current_);
    }

    std::streamsize byte_array_buffer::showmanyc()
    {
        assert(std::less_equal<const std::uint8_t *>()(current_, end_));
        return end_ - current_;
    }

    std::streampos byte_array_buffer::seekoff(
        std::streamoff off,
        std::ios_base::seekdir way,
        std::ios_base::openmode which)
    {
        if (way == std::ios_base::beg)
        {
            current_ = begin_ + off;
        }
        else if (way == std::ios_base::cur)
        {
            current_ += off;
        }
        else if (way == std::ios_base::end)
        {
            current_ = end_;
        }

        if (current_ < begin_ || current_ > end_)
        {
            return -1;
        }

        return current_ - begin_;
    }

    std::streampos byte_array_buffer::seekpos(
        std::streampos sp,
        std::ios_base::openmode which)
    {
        current_ = begin_ + sp;

        if (current_ < begin_ || current_ > end_)
        {
            return -1;
        }

        return current_ - begin_;
    }

    FileDetector::file_format FileDetector::DetectFileFormat(const std::uint8_t* base_address, size_t size)
    {
        size_t magic_size = 0;
        for (const auto &formatMap : { _magic_format_map, _unknown_format_map })
        {
            for (const auto &item : formatMap)
            {
                magic_size = (std::max)(magic_size, item.first.first + item.first.second.length());
            }
        }

        std::string magic;
        tls_check_struct *tls = check_get_tls();
        if (tls) {
            tls->active = 1;
            if (setjmp(tls->jb) == 0) {
                magic.resize(magic_size);
                memcpy(&magic[0], base_address, magic_size);
                tls->active = 0;
            }
            else {
                //execption occur
                tls->active = 0;
                return file_format::format_undetectable;
            }
        }

        for (const auto &item : _unknown_format_map)
        {
            if (HasSubstringOnPosition(magic, item.first.second, item.first.first))
            {
                return file_format::format_unknown;
            }
        }

        for (const auto &item : _magic_format_map)
        {
            if (HasSubstringOnPosition(magic, item.first.second, item.first.first))
            {
                switch (item.second)
                {
                case file_format::format_pe:
                    return IsPEHead(base_address, size) ? file_format::format_pe : file_format::format_dos;
                default:
                    return item.second;
                }
            }
        }
        return file_format::format_unknown;
    }

    FileDetector::file_format FileDetector::DetectFileFormat(const std::string& path) {
        file_format format = file_format::format_undetectable;
        std::vector<std::uint8_t> context_buffer;
        if (!LoadFile(path, context_buffer))
            return format;
        return DetectFileFormat(&context_buffer[0], context_buffer.size());
    }

    std::string FileDetector::GetFileFormat(const std::uint8_t* base_address, size_t size)
    {
        FileDetector::file_format format = DetectFileFormat(base_address, size);
        return _file_format_map.at(format);
    }

    std::string FileDetector::GetFileFormat(const std::string& path)
    {
        std::vector<std::uint8_t> context_buffer;
        if (!LoadFile(path, context_buffer))
            return _file_format_map.at(file_format::format_undetectable);
        return GetFileFormat(&context_buffer[0], context_buffer.size());
    }

    bool FileDetector::HasSubstringOnPosition(const std::string &str,
        const std::string &withWhat, std::string::size_type position) 
    {
        return (position < str.length()) && (str.length() - position >= withWhat.length()) &&
            (str.compare(position, withWhat.length(), withWhat) == 0);
    }

    PeLib::PeFile* FileDetector::ReadImageFile(const std::string& path)
    {
        if (path.empty()) {
            return nullptr;
        }
        if (!fs::exists(path)) {
            return nullptr;
        }
        if (!fs::is_regular_file(path)) {
            return nullptr;
        }

        PeLib::PeFile32 pefile(path);

        // Attempt to read the DOS file header.
        if (pefile.readMzHeader() != PeLib::ERROR_NONE)
        {
            return nullptr;
        }

        // Verify the DOS header
        if (!pefile.mzHeader().isValid())
        {
            return nullptr;
        }

        // Read PE header. Note that at this point, we read the header as if
        // it was 32-bit PE file.
        if (pefile.readPeHeader() != PeLib::ERROR_NONE)
        {
            return nullptr;
        }
        if (!pefile.peHeader().isValid())
        {
            return nullptr;
        }

        WORD machine = pefile.peHeader().getMachine();
        WORD magic = pefile.peHeader().getMagic();

        // jk2012-02-20: make the PEFILE32 be the default return value
        if ((machine == PeLib::PELIB_IMAGE_FILE_MACHINE_AMD64
            || machine == PeLib::PELIB_IMAGE_FILE_MACHINE_IA64)
            && magic == PeLib::PELIB_IMAGE_NT_OPTIONAL_HDR64_MAGIC)
        {
            return new PeLib::PeFile64(path);
        }
        else
        {
            return new PeLib::PeFile32(path);
        }
    }

    PeLib::PeFile* FileDetector::ReadImageMemory(std::istream& istream) 
    {
        PeLib::PeFile32 pefile(istream, PeLib::PeFileT<32>::loading_memory);

        // Attempt to read the DOS file header.
        if (pefile.readMzHeader() != PeLib::ERROR_NONE)
        {
            return nullptr;
        }

        // Verify the DOS header
        if (!pefile.mzHeader().isValid())
        {
            return nullptr;
        }

        // Read PE header. Note that at this point, we read the header as if
        // it was 32-bit PE file.
        if (pefile.readPeHeader() != PeLib::ERROR_NONE)
        {
            return nullptr;
        }
        if (!pefile.peHeader().isValid())
        {
            return nullptr;
        }

        WORD machine = pefile.peHeader().getMachine();
        WORD magic = pefile.peHeader().getMagic();

        // jk2012-02-20: make the PEFILE32 be the default return value
        if ((machine == PeLib::PELIB_IMAGE_FILE_MACHINE_AMD64
            || machine == PeLib::PELIB_IMAGE_FILE_MACHINE_IA64)
            && magic == PeLib::PELIB_IMAGE_NT_OPTIONAL_HDR64_MAGIC)
        {
            return new PeLib::PeFile64(istream, PeLib::PeFileT<64>::loading_memory);
        }
        else
        {
            return new PeLib::PeFile32(istream, PeLib::PeFileT<32>::loading_memory);
        }
    }

    bool FileDetector::IsPEHead(const std::uint8_t* base_address, size_t size)
    {
        DWORD signature = 0;
        byte_array_buffer ba_buffer(base_address, size);
        std::istream istream(&ba_buffer);
        tls_check_struct *tls = check_get_tls();
        if (tls) {
            tls->active = 1;
            if (setjmp(tls->jb) == 0) {
                std::unique_ptr<PeLib::PeFile> file(ReadImageMemory(istream));
                if (!file) return false;
                file->readMzHeader();
                file->readPeHeader();
                switch (file->getBits())
                {
                case 32:
                    signature = static_cast<PeLib::PeFileT<32>*>(file.get())->peHeader().getNtSignature();
                    break;
                case 64:
                    signature = static_cast<PeLib::PeFileT<64>*>(file.get())->peHeader().getNtSignature();
                    break;
                default:;
                }
                tls->active = 0;
            }
            else {
                //execption occur
                tls->active = 0;
                return false;
            }
        }

        return signature == _pe_header_signature_def || signature == _pe_header_little_endian_signature_def;
    }

    bool FileDetector::IsPEHead(const std::string& path)
    {
        std::vector<std::uint8_t> context_buffer;
        if (!LoadFile(path, context_buffer))
            return nullptr;
        return IsPEHead(&context_buffer[0]);
    }

    const std::uint8_t* FileDetector::GetEntryPoint(const std::uint8_t* base_address, size_t size)
    {
        file_format format = DetectFileFormat(base_address, size);

        switch (format)
        {
        case file_format::format_pe:
        {
            byte_array_buffer ba_buffer(base_address, _default_header_size_def);
            std::istream istream(&ba_buffer);
            std::unique_ptr<PeLib::PeFile> file(ReadImageMemory(istream));
            if (!file) return false;
            file->readMzHeader();
            file->readPeHeader();
            switch (file->getBits())
            {
            case 32:
                return (base_address + static_cast<PeLib::PeFileT<32>*>(file.get())->peHeader().getAddressOfEntryPoint());
            case 64:
                return (base_address + static_cast<PeLib::PeFileT<64>*>(file.get())->peHeader().getAddressOfEntryPoint());
            default:
                ;
            }
            return nullptr;
        }
        break;
        case format_dos:
        case format_elf:
        case format_coff:
        case format_macho:
        default:
            break;
        }
        return nullptr;
    }

    const std::uint8_t* FileDetector::GetEntryPoint(const std::string& path)
    {
        std::vector<std::uint8_t> context_buffer;
        if (!LoadFile(path, context_buffer))
            return nullptr;
        return GetEntryPoint(&context_buffer[0]);
    }

    const size_t FileDetector::GetSizeOfImage(const std::uint8_t* base_address, size_t size)
    {
        file_format format = DetectFileFormat(base_address);

        switch (format)
        {
        case file_format::format_pe:
        {
            byte_array_buffer ba_buffer(base_address, size);
            std::istream istream(&ba_buffer);
            std::unique_ptr<PeLib::PeFile> file(ReadImageMemory(istream));
            if (!file) return false;
            file->readMzHeader();
            file->readPeHeader();
            switch (file->getBits())
            {
            case 32:
                return static_cast<PeLib::PeFileT<32>*>(file.get())->peHeader().getSizeOfImage();
            case 64:
                return static_cast<PeLib::PeFileT<64>*>(file.get())->peHeader().getSizeOfImage();
            default:
                ;
            }
            return 0;
        }
        break;
        case format_dos:
        case format_elf:
        case format_coff:
        case format_macho:
        default:
            break;
        }
        return 0;
    }

    const size_t FileDetector::GetSizeOfImage(const std::string& path)
    {
        std::vector<std::uint8_t> context_buffer;
        if (!LoadFile(path, context_buffer))
            return 0;
        return GetSizeOfImage(&context_buffer[0]);
    }

    std::unique_ptr<CapInsn> FileDetector::GetAsmInstruction(std::uint8_t* insn_addr)
    {
        ASSERT(insn_addr);
        if (!insn_addr) return 0;
        if (!cchips::GetCapstoneImplment().IsValid())
            return nullptr;

        tls_check_struct *tls = check_get_tls();
        if (tls) {
            tls->active = 1;
            if (setjmp(tls->jb) == 0) {
                cs_insn *insn = nullptr;
                size_t count =
                    cs_disasm_ex(cchips::GetCapstoneImplment().GetCapHandle(), reinterpret_cast<const uint8_t*>(insn_addr), 16, reinterpret_cast<uintptr_t>(insn_addr), 1, &insn);
                if (count == 0) return nullptr;
                if (!insn) return nullptr;
                std::unique_ptr<CapInsn> cap_insn = std::make_unique<CapInsn>(insn, count);
                return cap_insn;
                tls->active = 0;
            }
            else {
                //execption occur
                tls->active = 0;
            }
        }
        return nullptr;
    }

    bool FileDetector::LoadFile(const std::string& path, std::vector<std::uint8_t>& context_buffer) 
    {
        if (path.empty()) {
            return false;
        }
        if (!fs::exists(path)) {
            return false;
        }
        if (!fs::is_regular_file(path)) {
            return false;
        }
        context_buffer.resize(0);

        auto filesize = fs::file_size(path);
        if (filesize == static_cast<uintmax_t>(-1) || filesize == 0) {
            return false;
        }
        std::ifstream infile;
        infile.open(path, std::ios::in | std::ios::binary | std::ios::ate);
        if (!infile.is_open()) {
            return false;
        }
        infile.seekg(0, std::ios::beg);
        context_buffer.resize(filesize);
        if (context_buffer.size() != filesize) {
            context_buffer.resize(0);
            return false;
        }
        infile.read((char*)&context_buffer[0], filesize);
        auto readed = infile.tellg();
        if (!readed) {
            context_buffer.resize(0);
            return false;
        }
        return true;
    }
} // namespace cchips
