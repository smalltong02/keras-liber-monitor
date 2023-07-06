#pragma once
#include "Handler.h"

namespace cchips {
    class CUnpackerFeatureBuilder : public CPeInsideHandler
    {
    public:
        CUnpackerFeatureBuilder() = default;
        bool Initialize() override { return true; }
        ~CUnpackerFeatureBuilder() = default;

        bool Scan(std::unique_ptr<cchips::PeFormat>& pe_format, std::unique_ptr<cchips::CRapidJsonWrapper>& json_result) override;
    };
} // namespace cchips
