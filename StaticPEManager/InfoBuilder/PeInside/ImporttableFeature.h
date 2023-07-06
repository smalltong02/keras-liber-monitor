#pragma once
#include "Handler.h"

namespace cchips {
    class CImporttableFeatureBuilder : public CPeInsideHandler
    {
    public:
        CImporttableFeatureBuilder() = default;
        bool Initialize() override;
        ~CImporttableFeatureBuilder() = default;

        bool Scan(std::unique_ptr<cchips::PeFormat>& pe_format, std::unique_ptr<cchips::CRapidJsonWrapper>& json_result) override;
    };
} // namespace cchips
