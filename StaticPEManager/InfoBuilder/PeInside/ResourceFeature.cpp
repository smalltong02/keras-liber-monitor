#include "ResourceFeature.h"
#include "utf8.h"

namespace cchips {
    std::vector<std::string> CResourceFeatureBuilder::_resource_info_strings = {
        "CompanyName",
        "FileDescription",
        "FileVersion",
        "InternalName",
        "LegalCopyright",
        "OriginalFileName",
    };

    bool CResourceFeatureBuilder::Scan(std::unique_ptr<cchips::PeFormat>& pe_format, std::unique_ptr<cchips::CRapidJsonWrapper>& json_result)
    {
        if (!pe_format || !json_result)
            return false;
        try {
            if (pe_format->hasResources()) {
                const auto& res_table = pe_format->getResourceTable();
                resource_info res_info;
                size_t nums_str = res_table->getNumberOfStrings();
                for (int index = 0; index < nums_str; index++) {
                    auto info = res_table->getString(index);
                    if (!info) {
                        continue;
                    }
                    if (info->first.empty() || info->second.empty()) {
                        continue;
                    }
                    if (areEqualCaseInsensitive(info->first, "CompanyName")) {
                        if (contain_invalid_utf8char(info->second)) {
                            res_info.company_name = stringto_hexstring(info->second);
                        }
                        else {
                            res_info.company_name = info->second;
                        }
                        continue;
                    }
                    if (areEqualCaseInsensitive(info->first, "FileDescription")) {
                        if (contain_invalid_utf8char(info->second)) {
                            res_info.file_description = stringto_hexstring(info->second);
                        }
                        else {
                            res_info.file_description = info->second;
                        }
                        continue;
                    }
                    if (areEqualCaseInsensitive(info->first, "FileVersion")) {
                        if (contain_invalid_utf8char(info->second)) {
                            res_info.file_version = stringto_hexstring(info->second);
                        }
                        else {
                            res_info.file_version = info->second;
                        }
                        continue;
                    }
                    if (areEqualCaseInsensitive(info->first, "InternalName")) {
                        if (contain_invalid_utf8char(info->second)) {
                            res_info.internal_name = stringto_hexstring(info->second);
                        }
                        else {
                            res_info.internal_name = info->second;
                        }
                        continue;
                    }
                    if (areEqualCaseInsensitive(info->first, "LegalCopyright")) {
                        if (contain_invalid_utf8char(info->second)) {
                            res_info.legal_copyright = stringto_hexstring(info->second);
                        }
                        else {
                            res_info.legal_copyright = info->second;
                        }
                        continue;
                    }
                    if (areEqualCaseInsensitive(info->first, "OriginalFileName")) {
                        if (contain_invalid_utf8char(info->second)) {
                            res_info.original_file_name = stringto_hexstring(info->second);
                        }
                        else {
                            res_info.original_file_name = info->second;
                        }
                        continue;
                    }
                    if (areEqualCaseInsensitive(info->first, "ProductName")) {
                        if (contain_invalid_utf8char(info->second)) {
                            res_info.product_name = stringto_hexstring(info->second);
                        }
                        else {
                            res_info.product_name = info->second;
                        }
                        continue;
                    }
                    if (areEqualCaseInsensitive(info->first, "ProductVersion")) {
                        if (contain_invalid_utf8char(info->second)) {
                            res_info.product_version = stringto_hexstring(info->second);
                        }
                        else {
                            res_info.product_version = info->second;
                        }
                        continue;
                    }
                    if (areEqualCaseInsensitive(info->first, "Comments")) {
                        if (contain_invalid_utf8char(info->second)) {
                            res_info.comments = stringto_hexstring(info->second);
                        }
                        else {
                            res_info.comments = info->second;
                        }
                        continue;
                    }
                    if (areEqualCaseInsensitive(info->first, "LegalTrademarks")) {
                        if (contain_invalid_utf8char(info->second)) {
                            res_info.legal_trademarks = stringto_hexstring(info->second);
                        }
                        else {
                            res_info.legal_trademarks = info->second;
                        }
                        continue;
                    }
                    if (areEqualCaseInsensitive(info->first, "PrivateBuild")) {
                        if (contain_invalid_utf8char(info->second)) {
                            res_info.private_build = stringto_hexstring(info->second);
                        }
                        else {
                            res_info.private_build = info->second;
                        }
                        continue;
                    }
                    if (areEqualCaseInsensitive(info->first, "SpecialBuild")) {
                        if (contain_invalid_utf8char(info->second)) {
                            res_info.special_build = stringto_hexstring(info->second);
                        }
                        else {
                            res_info.special_build = info->second;
                        }
                    }
                }

                auto& allocator = json_result->GetAllocator();
                std::unique_ptr<cchips::RapidValue> vresinfo = std::make_unique<cchips::RapidValue>();
                if (!vresinfo) return false;
                vresinfo->SetObject();

                if (!res_info.company_name.empty()) {
                    vresinfo->AddMember("companyname", cchips::RapidValue(res_info.company_name.c_str(), allocator), allocator);
                }
                if (!res_info.file_description.empty()) {
                    vresinfo->AddMember("description", cchips::RapidValue(res_info.file_description.c_str(), allocator), allocator);
                }
                if (!res_info.file_version.empty()) {
                    vresinfo->AddMember("fileversion", cchips::RapidValue(res_info.file_version.c_str(), allocator), allocator);
                }
                if (!res_info.internal_name.empty()) {
                    vresinfo->AddMember("internalname", cchips::RapidValue(res_info.internal_name.c_str(), allocator), allocator);
                }
                if (!res_info.legal_copyright.empty()) {
                    vresinfo->AddMember("copyright", cchips::RapidValue(res_info.legal_copyright.c_str(), allocator), allocator);
                }
                if (!res_info.original_file_name.empty()) {
                    vresinfo->AddMember("originalname", cchips::RapidValue(res_info.original_file_name.c_str(), allocator), allocator);
                }
                if (!res_info.product_name.empty()) {
                    vresinfo->AddMember("productname", cchips::RapidValue(res_info.product_name.c_str(), allocator), allocator);
                }
                if (!res_info.product_version.empty()) {
                    vresinfo->AddMember("productversion", cchips::RapidValue(res_info.product_version.c_str(), allocator), allocator);
                }
                if (!res_info.comments.empty()) {
                    vresinfo->AddMember("comments", cchips::RapidValue(res_info.comments.c_str(), allocator), allocator);
                }
                if (!res_info.legal_trademarks.empty()) {
                    vresinfo->AddMember("trademarks", cchips::RapidValue(res_info.legal_trademarks.c_str(), allocator), allocator);
                }
                if (!res_info.private_build.empty()) {
                    vresinfo->AddMember("privatebuild", cchips::RapidValue(res_info.private_build.c_str(), allocator), allocator);
                }
                if (!res_info.special_build.empty()) {
                    vresinfo->AddMember("specialbuild", cchips::RapidValue(res_info.special_build.c_str(), allocator), allocator);
                }
                return json_result->AddTopMember("resourceinfo", std::move(vresinfo));
            }
            return true;
        }
        catch (const std::exception& e)
        {
        }
        return false;
    }
} // namespace cchips
