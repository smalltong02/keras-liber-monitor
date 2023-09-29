#include "EmbeddedFeature.h"

namespace cchips {
    bool CEmbeddedFeatureBuilder::Scan(std::unique_ptr<cchips::PeFormat>& pe_format, std::unique_ptr<cchips::CRapidJsonWrapper>& json_result)
    {
        if (!pe_format || !json_result)
            return false;
        try {
            const auto& restable = pe_format->getResourceTable();
            if (restable) {
                auto& allocator = json_result->GetAllocator();
                std::unique_ptr<cchips::RapidValue> vembedded = std::make_unique<cchips::RapidValue>();
                if (!vembedded) return false;
                vembedded->SetObject();
                for (int index = 0; index < restable->getNumberOfResources(); index++) {
                    const auto* resource = restable->getResource(index);
                    if (resource) {
                        const auto bytes = resource->getBytes(0, resource->getLoadedSize());
                        if (!bytes.size())
                            continue;
                        if (EmbeddedPEFromData(bytes)) {
                            cchips::RapidValue embedded_res;
                            embedded_res.SetObject();
                            size_t id = 0;
                            std::string name = resource->getName();
                            if (!name.length()) name = resource->getType();
                            if (!name.length()) {
                                if (resource->getNameId(id)) {
                                    name = std::to_string(id);
                                }
                            }
                            if (!name.length()) {
                                resource->getTypeId(id);
                                name = std::to_string(id);
                            }
                            embedded_res.AddMember("offset", cchips::RapidValue(resource->getOffset()), allocator);
                            embedded_res.AddMember("size", cchips::RapidValue(resource->getSizeInFile()), allocator);
                            vembedded->AddMember(cchips::RapidValue(name.c_str(), allocator), embedded_res, allocator);
                        }
                    }
                }
                return json_result->AddTopMember("embeddedpe", std::move(vembedded));
            }
            return true;
        }
        catch (const std::exception& e)
        {
        }
        return false;
    }

    bool CEmbeddedFeatureBuilder::EmbeddedPEFromData(std::string_view data) const
    {
        bool bfind = false;
        size_t DosHeader = PeLib::PELIB_IMAGE_DOS_HEADER().size();
        size_t DataDirectory = PeLib::PELIB_IMAGE_DATA_DIRECTORY().size() * PeLib::PELIB_IMAGE_NUMBEROF_DIRECTORY_ENTRIES;
        size_t NtHeader = PeLib::PELIB_IMAGE_NT_HEADERS<32>().size() + DataDirectory;
        if (data.size() < (DosHeader + NtHeader)) {
            return false;
        }
        uint32_t DosHeaderLastPos = data.size() - DosHeader - NtHeader;
        uint32_t NtHeaderLastPos = data.size() - NtHeader;

        for (size_t pos = 0; (pos = data.find("MZ", pos)) != std::string::npos;) {
            if (pos >= DosHeaderLastPos)
                break;
            const PeLib::PELIB_IMAGE_DOS_HEADER* dos_header = reinterpret_cast<const PeLib::PELIB_IMAGE_DOS_HEADER*>(&data[pos]);
            if (pos + dos_header->e_lfanew >= NtHeaderLastPos)
            {
                pos += sizeof("MZ"); continue;
            }
            const PeLib::PELIB_IMAGE_NT_HEADERS<32>* nt_header = reinterpret_cast<const PeLib::PELIB_IMAGE_NT_HEADERS<32>*>(&data[pos + dos_header->e_lfanew]);
            if (nt_header->Signature != PeLib::PELIB_IMAGE_NT_SIGNATURE ||
                (nt_header->OptionalHeader.Magic != PeLib::PELIB_IMAGE_NT_OPTIONAL_HDR32_MAGIC &&
                    nt_header->OptionalHeader.Magic != PeLib::PELIB_IMAGE_NT_OPTIONAL_HDR64_MAGIC &&
                    nt_header->OptionalHeader.Magic != PeLib::PELIB_IMAGE_ROM_OPTIONAL_HDR_MAGIC)) {
                pos += sizeof("MZ"); continue;
            }
            bfind = true;
            break;
        }
        return bfind;
    }
} // namespace cchips
