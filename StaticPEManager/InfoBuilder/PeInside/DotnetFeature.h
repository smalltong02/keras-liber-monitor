#pragma once
#include "Handler.h"

namespace cchips {
    class CDotnetFeatureBuilder : public CPeInsideHandler
    {
    public:
        CDotnetFeatureBuilder() = default;
        bool Initialize() override { return true; }
        ~CDotnetFeatureBuilder() = default;

        bool Scan(std::unique_ptr<cchips::PeFormat>& pe_format, std::unique_ptr<cchips::CRapidJsonWrapper>& json_result) override;
    };
} // namespace cchips
