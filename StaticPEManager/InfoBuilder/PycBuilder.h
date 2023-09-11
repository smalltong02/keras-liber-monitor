#pragma once
#include "InfoBuilder.h"
#include "pyc_module.h"
#include <filesystem>

namespace cchips {
    namespace fs = std::filesystem;

    class CPycBuilder : public CInfoBuilder
    {
    public:
        using pyc_flags = enum {
            pyc_source,
            pyc_package,
            pyc_disassemble,
            pyc_decompyle,
        };
        using py_filetype = enum {
            py_init,
            py_main,
            py_init_c,
            py_main_c,
            py_init_i,
            py_requirements,
            py_about,
        };

        CPycBuilder() { UpdateInfoType(CJsonOptions::_info_type::info_type_pyc); }
        ~CPycBuilder() = default;
        bool Initialize(std::unique_ptr<CJsonOptions>& json_options);
        bool Scan(const std::string& file_buf, CFileInfo& file_info);
        bool Scan(fs::path& file_path, CFileInfo& file_info);
    private:
        static const std::map<std::string, py_filetype> _py_special_files;
        static const std::vector<std::string> _pyc_flag_names;
        bool CommonPycScan(CFileInfo& file_info);
        bool CommonSrcScan(CFileInfo& file_info);
        bool CommonSrcScan(const std::string& file_buf, CFileInfo& file_info);
        bool ParsePycObject(PycRef<PycObject> obj, int indent, std::ostringstream& oss);
        fs::path m_input;
        PycModule m_pycmod;
        pyc_flags m_flag;
    };
} // namespace cchips
