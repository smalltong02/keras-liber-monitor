#include "SectionFeature.h"

namespace cchips {
    bool CSectionFeatureBuilder::Scan(std::unique_ptr<cchips::PeFormat>& pe_format, std::unique_ptr<cchips::CRapidJsonWrapper>& json_result)
    {
        if (!pe_format || !json_result)
            return false;
        try {
			const auto seccounter = pe_format->getDeclaredNumberOfSections();
            if (!seccounter)
                return true;
            auto& allocator = json_result->GetAllocator();
            std::unique_ptr<cchips::RapidValue> vsections = std::make_unique<cchips::RapidValue>();
            if (!vsections) return false;
            vsections->SetObject();
			for (std::size_t i = 0; i < seccounter; ++i)
			{
				const auto sec = pe_format->getPeSection(i);
				if (sec)
				{
                    cchips::RapidValue vsection;
                    vsection.SetObject();
                    vsection.AddMember("index", RapidValue(sec->getIndex()), allocator);
                    vsection.AddMember("type", RapidValue((unsigned int)sec->getType()), allocator);
                    vsection.AddMember("characteristics", RapidValue(sec->getPeCoffFlags()), allocator);
                    vsection.AddMember("raw_data_point", RapidValue(sec->getOffset()), allocator);
                    vsection.AddMember("raw_data_size", RapidValue(sec->getSizeInFile()), allocator);
                    unsigned long long sizeinmemory = 0;
                    sec->getSizeInMemory(sizeinmemory);
                    vsection.AddMember("virtual_size", RapidValue(sizeinmemory), allocator);
                    vsection.AddMember("hasdata", RapidValue(sec->isSomeData()), allocator);
                    vsection.AddMember("hascode", RapidValue(sec->isSomeCode()), allocator);
                    vsection.AddMember("dataonly", RapidValue(sec->isDataOnly()), allocator);
                    vsection.AddMember("readonly", RapidValue(sec->isReadOnly()), allocator);
                    vsection.AddMember("bbs", RapidValue(sec->isBss()), allocator);
                    vsection.AddMember("debug", RapidValue(sec->isDebug()), allocator);
                    vsection.AddMember("info", RapidValue(sec->isInfo()), allocator);
                    std::string_view bytes = sec->getBytes(0, sizeinmemory);
                    vsection.AddMember("entropy", RapidValue(getEntropy(bytes)), allocator);
                    vsections->AddMember(cchips::RapidValue(sec->getName().c_str(), allocator), vsection, allocator);
				}
			}
            return json_result->AddTopMember("sections", std::move(vsections));
        }
        catch (const std::exception& e)
        {
        }
        return false;
    }
} // namespace cchips
