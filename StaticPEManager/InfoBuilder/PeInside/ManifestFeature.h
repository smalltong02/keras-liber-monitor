#pragma once
#include "Handler.h"

namespace cchips {
    class CManifestFeatureBuilder : public CPeInsideHandler
    {
    public:
        CManifestFeatureBuilder() = default;
        bool Initialize() override { return true; }
        ~CManifestFeatureBuilder() = default;

        bool Scan(std::unique_ptr<cchips::PeFormat>& pe_format, std::unique_ptr<cchips::CRapidJsonWrapper>& json_result) override;
    };
} // namespace cchips
