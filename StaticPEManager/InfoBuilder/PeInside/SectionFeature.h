#pragma once
#include "Handler.h"

namespace cchips {
    class CSectionFeatureBuilder : public CPeInsideHandler
    {
    public:
        CSectionFeatureBuilder() = default;
        bool Initialize() override { return true; }
        ~CSectionFeatureBuilder() = default;

        bool Scan(std::unique_ptr<cchips::PeFormat>& pe_format, std::unique_ptr<cchips::CRapidJsonWrapper>& json_result) override;
    };
} // namespace cchips
