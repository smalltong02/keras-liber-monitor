#include "ExporttableFeature.h"

namespace cchips {
    bool CExporttableFeatureBuilder::Initialize()
    {
        return true;
    }

    bool CExporttableFeatureBuilder::Scan(std::unique_ptr<cchips::PeFormat>& pe_format, std::unique_ptr<cchips::CRapidJsonWrapper>& json_result)
    {
        if (!pe_format || !json_result)
            return false;

        try {
            if (pe_format->hasExportsTable()) {

                auto export_info = pe_format->get_export_directory_info();
                auto& export_tables = pe_format->getExportTable();
                if (!export_tables)
                    return false;
            
                auto& allocator = json_result->GetAllocator();
                std::unique_ptr<cchips::RapidValue> vexports = std::make_unique<cchips::RapidValue>();
                if (!vexports) return false;
                vexports->SetObject();
                vexports->AddMember("characteristics", export_info.characteristics, allocator);
                vexports->AddMember("time_date_stamp", export_info.timestamp, allocator);
                vexports->AddMember("major_version", export_info.major_version, allocator);
                vexports->AddMember("minor_version", export_info.minor_version, allocator);
                vexports->AddMember("dll_name", cchips::RapidValue(export_tables->getName().c_str(), export_tables->getName().length(), allocator), allocator);
                vexports->AddMember("base", export_info.characteristics, allocator);
                vexports->AddMember("number_of_functions", export_info.characteristics, allocator);
                vexports->AddMember("number_of_names", export_info.characteristics, allocator);
                vexports->AddMember("address_of_Functions", export_info.characteristics, allocator);
                vexports->AddMember("address_of_names", export_info.characteristics, allocator);
                vexports->AddMember("address_of_names_ordinals", export_info.characteristics, allocator);
            
                cchips::RapidValue functions;
                functions.SetArray();
                for (auto& entry : *export_tables) {
                    functions.PushBack(cchips::RapidValue(entry.getName().c_str(), allocator), allocator);
                }
                vexports->AddMember("functions", functions, allocator);
                return json_result->AddTopMember("exports", std::move(vexports));
            }
        }
        catch (const std::exception& e)
        {
        }
        return false;
    }
} // namespace cchips
