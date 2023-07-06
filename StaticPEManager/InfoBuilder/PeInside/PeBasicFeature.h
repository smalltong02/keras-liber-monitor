#pragma once
#include "Handler.h"

namespace cchips {
    class CPeBasicFeatureBuilder : public CPeInsideHandler
    {
    public:
        CPeBasicFeatureBuilder() = default;
        bool Initialize() override;
        ~CPeBasicFeatureBuilder() = default;

        bool Scan(std::unique_ptr<cchips::PeFormat>& pe_format, std::unique_ptr<cchips::CRapidJsonWrapper>& json_result) override;
    };
} // namespace cchips
