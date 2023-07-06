#pragma once
#include "Handler.h"

namespace cchips {
    class CDatadirectoryFeatureBuilder : public CPeInsideHandler
    {
    public:
        CDatadirectoryFeatureBuilder() = default;
        bool Initialize() override { return true; }
        ~CDatadirectoryFeatureBuilder() = default;

        bool Scan(std::unique_ptr<cchips::PeFormat>& pe_format, std::unique_ptr<cchips::CRapidJsonWrapper>& json_result) override;
    };
} // namespace cchips
