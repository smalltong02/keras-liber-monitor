#pragma once
#include "Handler.h"

namespace cchips {
    class CEmbeddedFeatureBuilder : public CPeInsideHandler
    {
    public:
        CEmbeddedFeatureBuilder() = default;
        bool Initialize() override { return true; }
        ~CEmbeddedFeatureBuilder() = default;

        bool Scan(std::unique_ptr<cchips::PeFormat>& pe_format, std::unique_ptr<cchips::CRapidJsonWrapper>& json_result) override;

    private:
        bool EmbeddedPEFromData(std::string_view data) const;
    };
} // namespace cchips
