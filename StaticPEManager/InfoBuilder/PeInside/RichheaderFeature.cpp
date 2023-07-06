#include "RichheaderFeature.h"

namespace cchips {
    bool CRichheaderFeatureBuilder::Scan(std::unique_ptr<cchips::PeFormat>& pe_format, std::unique_ptr<cchips::CRapidJsonWrapper>& json_result)
    {
        if (!pe_format || !json_result)
            return false;
        try {
            auto& richheader = pe_format->getRichHeader();
            if (richheader.hasRecords()) {
                auto& allocator = json_result->GetAllocator();
                std::unique_ptr<cchips::RapidValue> vrichinfo = std::make_unique<cchips::RapidValue>();
                if (!vrichinfo) return false;
                vrichinfo->SetObject();
                for (const auto& lnk : richheader.getLinkerInfo()) {
                    cchips::RapidValue productinfo;
                    productinfo.SetObject();
                    productinfo.AddMember("productid", cchips::RapidValue(lnk.getProductId()), allocator);
                    productinfo.AddMember("productbuild", cchips::RapidValue(lnk.getProductBuild()), allocator);
                    productinfo.AddMember("numberofuses", cchips::RapidValue(lnk.getNumberOfUses()), allocator);
                    if(lnk.getProductName().length())
                        productinfo.AddMember("productname", cchips::RapidValue(lnk.getProductName().c_str(), allocator), allocator);
                    if(lnk.getVisualStudioName().length())
                        productinfo.AddMember("vsname", cchips::RapidValue(lnk.getVisualStudioName().c_str(), allocator), allocator);
                    
                    vrichinfo->AddMember(RapidValue(std::to_string(lnk.getProductId()).c_str(), allocator), productinfo, allocator);
                }
                return json_result->AddTopMember("richheader", std::move(vrichinfo));
            }
            return true;
        }
        catch (const std::exception& e)
        {
        }
        return false;
    }
} // namespace cchips
