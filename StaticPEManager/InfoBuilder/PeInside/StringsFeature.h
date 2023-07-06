#pragma once
#include "Handler.h"

namespace cchips {
    class CStringsFeatureBuilder : public CPeInsideHandler
    {
    public:
        CStringsFeatureBuilder() = default;
        bool Initialize() override;
        ~CStringsFeatureBuilder() = default;

        bool Scan(std::unique_ptr<cchips::PeFormat>& pe_format, std::unique_ptr<cchips::CRapidJsonWrapper>& json_result) override;

    private:
        inline bool IsAscii(uint8_t byte) { return ((byte >= 32 && byte < 127) || (byte >= 9 && byte <= 13)); }
        inline bool IsUnicode(uint16_t hw) { return ((hw >= 0x0020 && hw <= 0x007E)); }
    };
} // namespace cchips
