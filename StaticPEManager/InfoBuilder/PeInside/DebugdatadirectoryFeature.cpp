#include "DebugdatadirectoryFeature.h"

namespace cchips {
    bool CDebugdatadirectoryFeatureBuilder::Scan(std::unique_ptr<cchips::PeFormat>& pe_format, std::unique_ptr<cchips::CRapidJsonWrapper>& json_result)
    {
        if (!pe_format || !json_result)
            return false;
        try {
            if (pe_format->hasDebug()) {
                const auto& pdbinfo = pe_format->getPdbInfo();
                if (pdbinfo.size()) {
                    auto& allocator = json_result->GetAllocator();
                    std::unique_ptr<cchips::RapidValue> vpdbinfos = std::make_unique<cchips::RapidValue>();
                    if (!vpdbinfos) return false;
                    vpdbinfos->SetObject();

                    for (auto& pdb : pdbinfo) {
                        if (!pdb) {
                            continue;
                        }
                        cchips::RapidValue vpdb;
                        vpdb.SetObject();
                        vpdb.AddMember("type", RapidValue(pdb->getType().c_str(), allocator), allocator);
                        vpdb.AddMember("timestamp", RapidValue(pdb->getTimeStamp()), allocator);
                        vpdb.AddMember("age", RapidValue(pdb->getAge()), allocator);
                        vpdb.AddMember("path", RapidValue(pdb->getPath().c_str(), allocator), allocator);
                        auto guid = pdb->getGuid();
                        if (!guid.length())
                            guid = "unknown";
                        vpdbinfos->AddMember(cchips::RapidValue(guid.c_str(), allocator), vpdb, allocator);
                    }
                    return json_result->AddTopMember("pdbinfos", std::move(vpdbinfos));
                }
            }
            return true;
        }
        catch (const std::exception& e)
        {
        }
        return false;
    }
} // namespace cchips
