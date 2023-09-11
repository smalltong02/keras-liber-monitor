#pragma once
#include <stdio.h>
#include <windows.h>
#include <string>
#include <map>
#include <algorithm>
#include "PeLib.h"
#include "CapstoneImpl.h"

// a part of code be ported from retdec

namespace cchips {

    class byte_array_buffer : public std::streambuf
    {
    public:
        byte_array_buffer(const std::uint8_t* begin, const std::uint8_t* end) : begin_(begin), end_(end), current_(begin_) { assert(std::less_equal<const std::uint8_t *>()(begin_, end_)); }
        byte_array_buffer(const std::uint8_t* data, const std::size_t size) : byte_array_buffer(data, data + size) {}

    private:
        int_type underflow();
        int_type uflow();
        int_type pbackfail(int_type ch);
        std::streamsize showmanyc();

        std::streampos seekoff(
            std::streamoff off,
            std::ios_base::seekdir way,
            std::ios_base::openmode which = std::ios_base::in | std::ios_base::out);
        std::streampos seekpos(std::streampos sp,
            std::ios_base::openmode which = std::ios_base::in | std::ios_base::out);

        // copy ctor and assignment not implemented;
        // copying not allowed
        byte_array_buffer(const byte_array_buffer &) = delete;
        byte_array_buffer &operator= (const byte_array_buffer &) = delete;

    private:
        const std::uint8_t* const begin_ = nullptr;
        const std::uint8_t* const end_ = nullptr;
        const std::uint8_t* current_ = nullptr;
    };

    class FileDetector
    {
    public:
        enum file_format
        {
            format_undetectable,
            format_unknown,
            format_pe,
            format_dos,
            format_elf,
            format_coff,
            format_macho,
            format_intel_hex,
            format_pyc,
            format_javac,
            format_raw
        };
        enum pe_file_type
        {
            PEFILE32 = 32,
            PEFILE64 = 64,
            PEFILE_UNKNOWN = 0
        };

        FileDetector() = delete;
        ~FileDetector() = delete;

        const static size_t _default_header_size_def = 4096; // 4k

        static std::string GetFileFormat(const std::uint8_t* base_address, size_t size = _default_header_size_def);
        static std::string GetFileFormat(const std::string& path);
        static file_format DetectFileFormat(const std::uint8_t* base_address, size_t size = _default_header_size_def);
        static file_format DetectFileFormat(const std::string& path);
        static const std::uint8_t* GetEntryPoint(const std::uint8_t* base_address, size_t size = _default_header_size_def);
        static const std::uint8_t* GetEntryPoint(const std::string& path);
        static const size_t GetSizeOfImage(const std::uint8_t* base_address, size_t size = _default_header_size_def);
        static const size_t GetSizeOfImage(const std::string& path);
        static std::unique_ptr<CapInsn> GetAsmInstruction(std::uint8_t* insn_addr);
        static PeLib::PeFile* ReadImageMemory(std::istream& istream);
        static PeLib::PeFile* ReadImageFile(const std::string& path);
        static PeLib::PeFile* ReadFileMemory(std::istream& istream);
    private:
        static bool LoadFile(const std::string& path, std::vector<std::uint8_t>& context_buffer);
        static bool HasSubstringOnPosition(const std::string &str,
            const std::string &withWhat, std::string::size_type position);
        static bool IsPEHead(const std::uint8_t* base_address, size_t size = _default_header_size_def);
        static bool IsPEHead(const std::string& path);
        static bool IsPycFormat(const std::string& magic);
        static bool IsJavacFormat(const std::string& magic);
        const static WORD _pe_header_signature_def = 0x4550;
        const static DWORD _pe_header_little_endian_signature_def = 0x50450000;
        const static std::map<std::pair<std::size_t, std::string>, file_format> _magic_format_map;
        const static std::map<std::pair<std::size_t, std::string>, file_format> _unknown_format_map;
        const static std::map<file_format, std::string> _file_format_map;
    };

    class FileInfo
    {
    public:
        

        FileInfo(const char* base_address) { ; }
        ~FileInfo() = default;
    private:

    };
} // namespace cchips
