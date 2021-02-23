#include "ExceptionThrow.h"
#include "utils.h"
#include "PeInfo.h"

namespace cchips {

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

    FileDetector::file_format FileDetector::DetectFileFormat(const std::uint8_t* base_address) {
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
                    return IsPEHead(base_address) ? file_format::format_pe : file_format::format_dos;
                default:
                    return item.second;
                }
            }
        }
        return file_format::format_unknown;
    }

    bool FileDetector::HasSubstringOnPosition(const std::string &str,
        const std::string &withWhat, std::string::size_type position) 
    {
        return (position < str.length()) && (str.length() - position >= withWhat.length()) &&
            (str.compare(position, withWhat.length(), withWhat) == 0);
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

    bool FileDetector::IsPEHead(const std::uint8_t* base_address)
    {
        DWORD signature = 0;
        byte_array_buffer ba_buffer(base_address, _default_header_size_def);
        std::istream istream(&ba_buffer);
        std::unique_ptr<PeLib::PeFile> file(ReadImageMemory(istream));
        if (!file) return false;
        tls_check_struct *tls = check_get_tls();
        if (tls) {
            tls->active = 1;
            if (setjmp(tls->jb) == 0) {
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

    const std::uint8_t* FileDetector::GetEntryPoint(const std::uint8_t* base_address)
    {
        file_format format = DetectFileFormat(base_address);

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

    const size_t FileDetector::GetSizeOfImage(const std::uint8_t* base_address)
    {
        file_format format = DetectFileFormat(base_address);

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
} // namespace cchips
