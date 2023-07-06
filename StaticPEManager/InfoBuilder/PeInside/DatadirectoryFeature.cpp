#include "DatadirectoryFeature.h"

namespace cchips {
    bool CDatadirectoryFeatureBuilder::Scan(std::unique_ptr<cchips::PeFormat>& pe_format, std::unique_ptr<cchips::CRapidJsonWrapper>& json_result)
    {
        if (!pe_format || !json_result)
            return false;
        try {
            unsigned long long reladdr = 0;
            unsigned long long size = 0;
            auto& allocator = json_result->GetAllocator();
            std::unique_ptr<cchips::RapidValue> vdatadir = std::make_unique<cchips::RapidValue>();
            if (!vdatadir) return false;
            vdatadir->SetObject();

            pe_format->getDataDirectoryRelative(PeLib::PELIB_IMAGE_DIRECTORY_ENTRY_EXPORT, reladdr, size);
            {
                cchips::RapidValue exportdata;
                exportdata.SetObject();
                exportdata.AddMember("reladdr", cchips::RapidValue(reladdr), allocator);
                exportdata.AddMember("size", cchips::RapidValue(size), allocator);
                vdatadir->AddMember("export_directory", exportdata, allocator);
            }
            pe_format->getDataDirectoryRelative(PeLib::PELIB_IMAGE_DIRECTORY_ENTRY_IMPORT, reladdr, size);
            {
                cchips::RapidValue importdata;
                importdata.SetObject();
                importdata.AddMember("reladdr", cchips::RapidValue(reladdr), allocator);
                importdata.AddMember("size", cchips::RapidValue(size), allocator);
                vdatadir->AddMember("import_directory", importdata, allocator);
            }
            pe_format->getDataDirectoryRelative(PeLib::PELIB_IMAGE_DIRECTORY_ENTRY_RESOURCE, reladdr, size);
            {
                cchips::RapidValue resourcedata;
                resourcedata.SetObject();
                resourcedata.AddMember("reladdr", cchips::RapidValue(reladdr), allocator);
                resourcedata.AddMember("size", cchips::RapidValue(size), allocator);
                vdatadir->AddMember("resource_directory", resourcedata, allocator);
            }
            pe_format->getDataDirectoryRelative(PeLib::PELIB_IMAGE_DIRECTORY_ENTRY_EXCEPTION, reladdr, size);
            {
                cchips::RapidValue exceptiondata;
                exceptiondata.SetObject();
                exceptiondata.AddMember("reladdr", cchips::RapidValue(reladdr), allocator);
                exceptiondata.AddMember("size", cchips::RapidValue(size), allocator);
                vdatadir->AddMember("exception_directory", exceptiondata, allocator);
            }
            pe_format->getDataDirectoryRelative(PeLib::PELIB_IMAGE_DIRECTORY_ENTRY_SECURITY, reladdr, size);
            {
                cchips::RapidValue securitydata;
                securitydata.SetObject();
                securitydata.AddMember("reladdr", cchips::RapidValue(reladdr), allocator);
                securitydata.AddMember("size", cchips::RapidValue(size), allocator);
                vdatadir->AddMember("security_directory", securitydata, allocator);
            }
            pe_format->getDataDirectoryRelative(PeLib::PELIB_IMAGE_DIRECTORY_ENTRY_BASERELOC, reladdr, size);
            {
                cchips::RapidValue baserelocdata;
                baserelocdata.SetObject();
                baserelocdata.AddMember("reladdr", cchips::RapidValue(reladdr), allocator);
                baserelocdata.AddMember("size", cchips::RapidValue(size), allocator);
                vdatadir->AddMember("basereloc_directory", baserelocdata, allocator);
            }
            pe_format->getDataDirectoryRelative(PeLib::PELIB_IMAGE_DIRECTORY_ENTRY_DEBUG, reladdr, size);
            {
                cchips::RapidValue debugdata;
                debugdata.SetObject();
                debugdata.AddMember("reladdr", cchips::RapidValue(reladdr), allocator);
                debugdata.AddMember("size", cchips::RapidValue(size), allocator);
                vdatadir->AddMember("debug_directory", debugdata, allocator);
            }
            pe_format->getDataDirectoryRelative(PeLib::PELIB_IMAGE_DIRECTORY_ENTRY_ARCHITECTURE, reladdr, size);
            {
                cchips::RapidValue architecturedata;
                architecturedata.SetObject();
                architecturedata.AddMember("reladdr", cchips::RapidValue(reladdr), allocator);
                architecturedata.AddMember("size", cchips::RapidValue(size), allocator);
                vdatadir->AddMember("architecture_directory", architecturedata, allocator);
            }
            pe_format->getDataDirectoryRelative(PeLib::PELIB_IMAGE_DIRECTORY_ENTRY_GLOBALPTR, reladdr, size);
            {
                cchips::RapidValue globalptrdata;
                globalptrdata.SetObject();
                globalptrdata.AddMember("reladdr", cchips::RapidValue(reladdr), allocator);
                globalptrdata.AddMember("size", cchips::RapidValue(size), allocator);
                vdatadir->AddMember("globalptr_directory", globalptrdata, allocator);
            }
            pe_format->getDataDirectoryRelative(PeLib::PELIB_IMAGE_DIRECTORY_ENTRY_TLS, reladdr, size);
            {
                cchips::RapidValue tlsdata;
                tlsdata.SetObject();
                tlsdata.AddMember("reladdr", cchips::RapidValue(reladdr), allocator);
                tlsdata.AddMember("size", cchips::RapidValue(size), allocator);
                vdatadir->AddMember("tls_directory", tlsdata, allocator);
            }
            pe_format->getDataDirectoryRelative(PeLib::PELIB_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG, reladdr, size);
            {
                cchips::RapidValue loadconfigdata;
                loadconfigdata.SetObject();
                loadconfigdata.AddMember("reladdr", cchips::RapidValue(reladdr), allocator);
                loadconfigdata.AddMember("size", cchips::RapidValue(size), allocator);
                vdatadir->AddMember("loadconfig_directory", loadconfigdata, allocator);
            }
            pe_format->getDataDirectoryRelative(PeLib::PELIB_IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT, reladdr, size);
            {
                cchips::RapidValue boundimportdata;
                boundimportdata.SetObject();
                boundimportdata.AddMember("reladdr", cchips::RapidValue(reladdr), allocator);
                boundimportdata.AddMember("size", cchips::RapidValue(size), allocator);
                vdatadir->AddMember("boundimport_directory", boundimportdata, allocator);
            }
            pe_format->getDataDirectoryRelative(PeLib::PELIB_IMAGE_DIRECTORY_ENTRY_IAT, reladdr, size);
            {
                cchips::RapidValue iatdata;
                iatdata.SetObject();
                iatdata.AddMember("reladdr", cchips::RapidValue(reladdr), allocator);
                iatdata.AddMember("size", cchips::RapidValue(size), allocator);
                vdatadir->AddMember("iat_directory", iatdata, allocator);
            }
            pe_format->getDataDirectoryRelative(PeLib::PELIB_IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT, reladdr, size);
            {
                cchips::RapidValue delayimportdata;
                delayimportdata.SetObject();
                delayimportdata.AddMember("reladdr", cchips::RapidValue(reladdr), allocator);
                delayimportdata.AddMember("size", cchips::RapidValue(size), allocator);
                vdatadir->AddMember("delayimport_directory", delayimportdata, allocator);
            }
            pe_format->getDataDirectoryRelative(PeLib::PELIB_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR, reladdr, size);
            {
                cchips::RapidValue comdesdata;
                comdesdata.SetObject();
                comdesdata.AddMember("reladdr", cchips::RapidValue(reladdr), allocator);
                comdesdata.AddMember("size", cchips::RapidValue(size), allocator);
                vdatadir->AddMember("comdes_directory", comdesdata, allocator);
            }
            return json_result->AddTopMember("datadirectory", std::move(vdatadir));
        }
        catch (const std::exception& e)
        {
        }
        return false;
    }
} // namespace cchips
