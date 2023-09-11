#pragma once
#include "Handler.h"

namespace cchips {
    class CResourceFeatureBuilder : public CPeInsideHandler
    {
    public:
        CResourceFeatureBuilder() = default;
        bool Initialize() override { return true; }
        ~CResourceFeatureBuilder() = default;

        bool Scan(std::unique_ptr<cchips::PeFormat>& pe_format, std::unique_ptr<cchips::CRapidJsonWrapper>& json_result) override;

    private:
        using resource_info = struct {

            std::string company_name;
            std::string file_description;
            std::string file_version;
            std::string internal_name;
            std::string legal_copyright;
            std::string original_file_name;
            std::string product_name;
            std::string product_version;
            std::string comments;
            std::string legal_trademarks;
            std::string private_build;
            std::string special_build;

        };

        static std::vector<std::string> _resource_info_strings;
    };
} // namespace cchips
