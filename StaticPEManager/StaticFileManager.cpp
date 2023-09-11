#include "StaticFileManager.h"

bool g_IsNT = true;
HINSTANCE g_hInstance = 0;
CStdOutStream* g_ErrStream = nullptr;
const CExternalCodecs* g_ExternalCodecs_Ptr = nullptr;

namespace cchips {
    const mongocxx::instance CStaticFileManager::_mongodb_instance{};
    const std::map<std::uint32_t, bool> CStaticFileManager::_codecs_formats = {
        {0, true}, {0x1, false}, {0x2, false}, {0x3, true}, {0x4, true}, {0x5, false}, {0x6, false}, {0x7, true}, {0x8, false}, {0x9, true}, {0xa, true},
        {0xb, false}, {0xc, true}, {0xd, false}, {0xe, false}, {0xf, false}, {0x10, false}, {0x11, false}, {0x12, false}, {0x13, false}, {0x14, false}, {0x15, true},
        {0x16, true}, {0x17, true}, {0x18, false}, {0x19, false}, {0x1a, true}, {0x1b, false}, {0x1c, false}, {0x1d, true}, {0x1e, false}, {0x1f, true}, {0x20, true}, 
        {0x21, true}, {0x22, true}, {0x23, true}, {0x24, false}, {0x25, false}, {0x26, false}, {0x27, false}, {0x28, false}, {0x29, false}, {0x2a, true}, {0x2b, true},
        {0x2c, true}, {0x2d, true}, {0x2e, true}, {0x2f, true}, {0x30, true}, {0x31, true}, {0x32, true}, {0x33, true}, {0x34, true}, {0x35, true}, {0x36, true},
        {0x37, true}, {0x38, true}, {0x39, true}, {0x3a, true}, {0x3b, true}, {0x3c, true}, {0x3d, true}, {0x3e, true}, {0x3f, true}, {0x40, false},
    };
    const std::vector<std::string> CStaticFileManager::_suffix_compress = {
        ".zip", ".rar", ".7z", ".tar", ".gz", ".cab", ".msi", ".iso", ".bz2", ".xz", ".tgz", ".z", ".tar.bz2", ".tar.xz"
    };

    bool CStaticFileManager::c7z_callback(const FString& path, CReadArcItem& item, const std::vector<unsigned char>& data) {
        try {
            if (item.IsDir) {
                return false;
            }
            fs::path package_path = GetCStaticFileManager().fstring_to_string(path);
            fs::path name = GetCStaticFileManager().fstring_to_string(item.Path);
            if (GetCStaticFileManager().OutputConsoleFlag()) {
                printf("    --> extract: %s\n", name.string().c_str());
            }
            std::string_view data_view((char*)data.data(), data.size());
            if (is_suffix_compress(name.extension())) {
                return GetCStaticFileManager().scanpackage_data(std::string(data_view), GetCStaticFileManager().m_temp_output, package_path, name);
            }
            return GetCStaticFileManager().scanmem_callback(std::string(data_view), GetCStaticFileManager().m_temp_output, false, package_path, name);
        }
        catch (const std::exception& e) {
            printf("CStaticFileManager::c7z_callback crash: %s\n", e.what());
            return false;
        }
    }
    bool CStaticFileManager::is_suffix_compress(const fs::path& extension) {
        if (extension.empty()) {
            return false;
        }
        std::string ext_str = extension.string();
        std::transform(ext_str.begin(), ext_str.end(), ext_str.begin(), ::tolower);
        for (auto& ext : _suffix_compress) {
            if (ext_str == ext) {
                return true;
            }
        }
        return false;
    }
    bool CStaticFileManager::extractc7z_format(std::uint32_t formatindex) {
        auto find = _codecs_formats.find(formatindex);
        if (find != _codecs_formats.end()) {
            return find->second;
        }
        return false;
    }
} // namespace cchips
