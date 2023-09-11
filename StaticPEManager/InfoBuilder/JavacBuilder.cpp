#include "JavacBuilder.h"

namespace cchips {

    bool CJavacBuilder::Initialize(std::unique_ptr<CJsonOptions>& json_options) {
        return true;
    }

    bool CJavacBuilder::Scan(fs::path& file_path, CFileInfo& file_info) {
        try {
            return true;
        }
        catch (const std::exception& e)
        {
        }
        return false;
    }

    bool CJavacBuilder::Scan(const std::string& file_buf, CFileInfo& file_info) {
        try {
            return true;
        }
        catch (const std::exception& e)
        {
        }
        return false;
    }
} // namespace cchips
