#include "UnpackerFeature.h"

namespace cchips {
    bool CUnpackerFeatureBuilder::Scan(std::unique_ptr<cchips::PeFormat>& pe_format, std::unique_ptr<cchips::CRapidJsonWrapper>& json_result)
    {
        if (!pe_format || !json_result)
            return false;
        try {

            return true;
        }
        catch (const std::exception& e)
        {
        }
        return false;
    }
} // namespace cchips
