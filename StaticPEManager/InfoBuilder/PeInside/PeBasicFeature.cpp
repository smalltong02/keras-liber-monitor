#include "PeBasicFeature.h"

namespace cchips {
    bool CPeBasicFeatureBuilder::Initialize()
    {
        return true;
    }

    bool CPeBasicFeatureBuilder::Scan(std::unique_ptr<cchips::PeFormat>& pe_format, std::unique_ptr<cchips::CRapidJsonWrapper>& json_result)
    {
        if (!pe_format || !json_result)
            return false;
        try {
            auto& allocator = json_result->GetAllocator();
            std::unique_ptr<cchips::RapidValue> vpeinfo = std::make_unique<cchips::RapidValue>();
            if (!vpeinfo) return false;
            vpeinfo->SetObject();
            std::string exe_type = "exe";
            if (pe_format->isDll()) {
                exe_type = "dll";
            }
            else if (pe_format->isSys()) {
                exe_type = "sys";
            }
            vpeinfo->AddMember("exe_type", cchips::RapidValue(exe_type.c_str(), allocator), allocator);
            vpeinfo->AddMember("has_imports", cchips::RapidValue(pe_format->hasImportsTable()), allocator);
            vpeinfo->AddMember("has_exports", cchips::RapidValue(pe_format->hasExportsTable()), allocator);
            vpeinfo->AddMember("has_resources", cchips::RapidValue(pe_format->hasResources()), allocator);
            vpeinfo->AddMember("has_security", cchips::RapidValue(pe_format->hasSecurity()), allocator);
            vpeinfo->AddMember("has_reloc", cchips::RapidValue(pe_format->hasReloc()), allocator);
            vpeinfo->AddMember("has_tls", cchips::RapidValue(pe_format->hasTls()), allocator);
            vpeinfo->AddMember("has_bound_import", cchips::RapidValue(pe_format->hasBoundImportsTable()), allocator);
            vpeinfo->AddMember("has_delay_import", cchips::RapidValue(pe_format->hasDelayImportsTable()), allocator);
            vpeinfo->AddMember("is_dotnet", cchips::RapidValue(pe_format->isDotNet()), allocator);
            vpeinfo->AddMember("has_debug", cchips::RapidValue(pe_format->hasDebug()), allocator);
            vpeinfo->AddMember("get_subsystem", cchips::RapidValue(pe_format->getSubsystem()), allocator);
            vpeinfo->AddMember("get_minor_os_version", cchips::RapidValue(pe_format->getMinorOsVersion()), allocator);
            vpeinfo->AddMember("get_major_os_version", cchips::RapidValue(pe_format->getMajorOsVersion()), allocator);
            vpeinfo->AddMember("get_minor_subsystem_version", cchips::RapidValue(pe_format->getMinorSubsystemVersion()), allocator);
            vpeinfo->AddMember("get_major_subsystem_version", cchips::RapidValue(pe_format->getMajorSubsystemVersion()), allocator);
            vpeinfo->AddMember("get_pe_header_start", cchips::RapidValue(pe_format->getPeHeaderStart()), allocator);
            vpeinfo->AddMember("get_file_alignment", cchips::RapidValue(pe_format->getFileAlignment()), allocator);
            vpeinfo->AddMember("get_size_of_image", cchips::RapidValue(pe_format->getSizeOfImage()), allocator);
            unsigned long long ep_address = 0; pe_format->getEpAddress(ep_address);
            vpeinfo->AddMember("get_ep", cchips::RapidValue(ep_address), allocator);
            vpeinfo->AddMember("get_number_of_rvas_and_sizes", cchips::RapidValue(pe_format->getDeclaredNumberOfDataDirectories()), allocator);
            vpeinfo->AddMember("get_characteristics", cchips::RapidValue(pe_format->getCharacteristics()), allocator);
            vpeinfo->AddMember("get_dll_characteristics", cchips::RapidValue(pe_format->getDllCharacteristics()), allocator);
            vpeinfo->AddMember("get_size_of_headers", cchips::RapidValue(pe_format->getSizeOfHeaders()), allocator);
            vpeinfo->AddMember("get_size_of_optional_header", cchips::RapidValue(pe_format->getOptionalHeaderSize()), allocator);
            vpeinfo->AddMember("get_pe_signature", cchips::RapidValue(pe_format->getPeSignature()), allocator);
            vpeinfo->AddMember("get_magic", cchips::RapidValue(pe_format->getMagic()), allocator);
            unsigned long long base_address = 0; pe_format->getImageBaseAddress(base_address);
            vpeinfo->AddMember("get_image_base", cchips::RapidValue(base_address), allocator);
            vpeinfo->AddMember("get_heap_size_commit", cchips::RapidValue(pe_format->getSizeOfHeapCommit()), allocator);
            vpeinfo->AddMember("get_heap_size_reserve", cchips::RapidValue(pe_format->getSizeOfHeapReserve()), allocator);
            vpeinfo->AddMember("get_stack_size_commit", cchips::RapidValue(pe_format->getSizeOfStackCommit()), allocator);
            vpeinfo->AddMember("get_stack_size_reserve", cchips::RapidValue(pe_format->getSizeOfStackReserve()), allocator);
            vpeinfo->AddMember("get_checksum", cchips::RapidValue(pe_format->getChecksum()), allocator);
            vpeinfo->AddMember("verify_checksum", cchips::RapidValue(pe_format->verifyChecksum()), allocator);
            vpeinfo->AddMember("get_time_date_stamp", cchips::RapidValue(pe_format->getTimeStamp()), allocator);
            unsigned long long machine_code = 0; pe_format->getMachineCode(machine_code);
            vpeinfo->AddMember("get_machine", cchips::RapidValue(machine_code), allocator);
            vpeinfo->AddMember("get_sizeof_nt_header", cchips::RapidValue(pe_format->getSizeofNtHeader()), allocator);
            vpeinfo->AddMember("get_base_of_code", cchips::RapidValue(pe_format->getBaseOfCode()), allocator);
            vpeinfo->AddMember("get_number_of_sections", cchips::RapidValue(pe_format->getDeclaredNumberOfSections()), allocator);
            vpeinfo->AddMember("get_section_alignment", cchips::RapidValue(pe_format->getSectionAlignment()), allocator);
            vpeinfo->AddMember("has_overlay", cchips::RapidValue(pe_format->hasOverlay()), allocator);
            vpeinfo->AddMember("entropy", cchips::RapidValue(pe_format->caculateEntropy()), allocator);
        
            return json_result->AddTopMember("peinfo", std::move(vpeinfo));
        }
        catch (const std::exception& e)
        {
        }
        return false;
    }
} // namespace cchips
