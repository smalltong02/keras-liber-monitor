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
                vexports->AddMember("Characteristics", export_info.characteristics, allocator);
                vexports->AddMember("Time_Date_Stamp", export_info.timestamp, allocator);
                vexports->AddMember("Major_Version", export_info.major_version, allocator);
                vexports->AddMember("Minor_Version", export_info.minor_version, allocator);
                vexports->AddMember("Dll_Name", cchips::RapidValue(export_tables->getName().c_str(), export_tables->getName().length(), allocator), allocator);
                vexports->AddMember("Base", export_info.characteristics, allocator);
                vexports->AddMember("Number_Of_Functions", export_info.characteristics, allocator);
                vexports->AddMember("Number_Of_Names", export_info.characteristics, allocator);
                vexports->AddMember("Address_Of_Functions", export_info.characteristics, allocator);
                vexports->AddMember("Address_Of_Names", export_info.characteristics, allocator);
                vexports->AddMember("Address_Of_Names_Ordinals", export_info.characteristics, allocator);
            
                cchips::RapidValue functions;
                functions.SetArray();
                for (auto& entry : *export_tables) {
                    functions.PushBack(cchips::RapidValue(entry.getName().c_str(), allocator), allocator);
                }
                vexports->AddMember("Functions", functions, allocator);
                return json_result->AddTopMember("exports", std::move(vexports));
            }
        }
        catch (const std::exception& e)
        {
        }
        return false;
    }
} // namespace cchips
