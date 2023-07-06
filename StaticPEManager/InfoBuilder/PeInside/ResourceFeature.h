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
    };
} // namespace cchips
