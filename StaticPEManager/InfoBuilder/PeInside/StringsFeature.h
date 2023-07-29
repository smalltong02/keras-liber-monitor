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
        bool UnporcessSection(const std::shared_ptr<PeCoffSection> sec, const std::shared_ptr<PeCoffSection> res) const;
        inline bool IsAscii(uint8_t byte) const { return ((byte >= 32 && byte < 127) || (byte >= 9 && byte <= 13)); }
        inline bool IsUnicode(uint16_t hw) const { return ((hw >= 0x0020 && hw <= 0x007E)); }
    };
} // namespace cchips
