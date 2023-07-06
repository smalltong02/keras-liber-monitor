#pragma once
#include "Handler.h"

namespace cchips {
    class CPackerFeatureBuilder : public CPeInsideHandler
    {
    public:
        CPackerFeatureBuilder() = default;
        bool Initialize() override { return true; }
        ~CPackerFeatureBuilder() = default;

        bool Scan(std::unique_ptr<cchips::PeFormat>& pe_format, std::unique_ptr<cchips::CRapidJsonWrapper>& json_result) override;
    };
} // namespace cchips
