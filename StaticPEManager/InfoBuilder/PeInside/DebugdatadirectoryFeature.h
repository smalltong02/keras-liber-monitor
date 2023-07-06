#pragma once
#include "Handler.h"

namespace cchips {
    class CDebugdatadirectoryFeatureBuilder : public CPeInsideHandler
    {
    public:
        CDebugdatadirectoryFeatureBuilder() = default;
        bool Initialize() override { return true; }
        ~CDebugdatadirectoryFeatureBuilder() = default;

        bool Scan(std::unique_ptr<cchips::PeFormat>& pe_format, std::unique_ptr<cchips::CRapidJsonWrapper>& json_result) override;
    };
} // namespace cchips
