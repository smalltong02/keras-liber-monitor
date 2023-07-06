#pragma once
#include "Handler.h"

namespace cchips {
    class CIconFeatureBuilder : public CPeInsideHandler
    {
    public:
        CIconFeatureBuilder() = default;
        bool Initialize() override { return true; }
        ~CIconFeatureBuilder() = default;

        bool Scan(std::unique_ptr<cchips::PeFormat>& pe_format, std::unique_ptr<cchips::CRapidJsonWrapper>& json_result) override;
    };
} // namespace cchips
