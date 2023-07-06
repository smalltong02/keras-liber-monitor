#pragma once
#include "InfoBuilder.h"
#include "PassStructure.h"
#include "PassSupport.h"

namespace cchips {
    class CPeinsnflowInfoBuilder : public CInfoBuilder
    {
    public:
        CPeinsnflowInfoBuilder() { UpdateInfoType(CJsonOptions::_info_type::info_type_peinsnflow); }
        ~CPeinsnflowInfoBuilder() = default;

        bool Initialize(std::unique_ptr<CJsonOptions>& json_options) {
            if (!json_options)
                return false;
            if (!json_options->GetOptionsInfo(GetInfoType(), std::pair<unsigned char*, size_t>(reinterpret_cast<unsigned char*>(&m_options_info), sizeof(m_options_info))))
                return false;
            return true;
        }
        bool Scan(fs::path& file_path, CFileInfo& file_info) { 
            try {
                std::unique_ptr<cchips::CRapidJsonWrapper> json_result;
                json_result = std::make_unique<cchips::CRapidJsonWrapper>("{}");
                if (!json_result)
                    return false;
                if (fs::is_regular_file(file_path)) {
                    std::shared_ptr<Module> fmodule = std::make_shared<Module>();
                    fmodule->SetPrecachePath(file_path.string());
                    fmodule->SetModuleName(file_path.filename().string());
                    GetPassRegistry().sequence(GetPassRegistry().sequence_passes_define);
                    GetPassRegistry().run(fmodule);
                    if (fmodule->Valid()) {
                        //auto logpath = file_path.filename();
                        //logpath = fs::path(".\\").append(logpath.replace_extension(".json").string());
                        //logpath = fs::path("C:\\work\\output_result\\").append(logpath.replace_extension(".json").string());
                        fmodule->dump(json_result);
                    }
                }
                file_info.SetInsnFlowInfo(std::move(json_result));
                return true;
            }
            catch (const std::exception& e)
            {
            }
            return false;
        }

    private:
        CJsonOptions::_peinsnflow_info m_options_info;
    };
} // namespace cchips
