#pragma once
#include "Handler.h"

namespace cchips {
    class CRichheaderFeatureBuilder : public CPeInsideHandler
    {
    public:
        CRichheaderFeatureBuilder() = default;
        bool Initialize() override { return true; }
        ~CRichheaderFeatureBuilder() = default;

        bool Scan(std::unique_ptr<cchips::PeFormat>& pe_format, std::unique_ptr<cchips::CRapidJsonWrapper>& json_result) override;
    };
} // namespace cchips
