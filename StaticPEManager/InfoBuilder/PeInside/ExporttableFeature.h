#pragma once
#include "Handler.h"

namespace cchips {
    class CExporttableFeatureBuilder : public CPeInsideHandler
    {
    public:
        CExporttableFeatureBuilder() = default;
        bool Initialize() override;
        ~CExporttableFeatureBuilder() = default;

        bool Scan(std::unique_ptr<cchips::PeFormat>& pe_format, std::unique_ptr<cchips::CRapidJsonWrapper>& json_result) override;
    };
} // namespace cchips
