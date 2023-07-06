#pragma once
#include "Handler.h"

namespace cchips {
    class CStatFeatureBuilder : public CPeInsideHandler
    {
    public:
        CStatFeatureBuilder() = default;
        bool Initialize() override { return true; }
        ~CStatFeatureBuilder() = default;

        const static uint32_t _hexstring_min_length;

        bool Scan(std::unique_ptr<cchips::PeFormat>& pe_format, std::unique_ptr<cchips::CRapidJsonWrapper>& json_result) override;
    };
} // namespace cchips
