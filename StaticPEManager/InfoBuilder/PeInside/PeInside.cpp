#include <fstream>
#include "PeInside.h"
#include "PeBasicFeature.h"
#include "StatFeature.h"
#include "CrossRefFeature.h"
#include "PackerFeature.h"
#include "DatadirectoryFeature.h"
#include "DebugdatadirectoryFeature.h"
#include "EmbeddedFeature.h"
#include "ImporttableFeature.h"
#include "ExporttableFeature.h"
#include "IconFeature.h"
#include "ManifestFeature.h"
#include "ResourceFeature.h"
#include "RichheaderFeature.h"
#include "SectionFeature.h"
#include "StringsFeature.h"
#include "UnpackerFeature.h"
#include "DotnetFeature.h"

namespace cchips {
    CPeInside::CPeInside()
    {
        return;
    }

    CPeInside::~CPeInside()
    {
        return;
    }

    bool CPeInside::Initialize(CJsonOptions::_peinside_info& info)
    {
        m_options_info = info;
        if (m_options_info.bpebasicfeature) {
            std::unique_ptr<CPeInsideHandler> handler = std::make_unique<CPeBasicFeatureBuilder>();
            if (handler && handler->Initialize()) {
                m_handlers.push_back(std::move(handler));
            }
        }
        if (m_options_info.bstatfeature) {
            std::unique_ptr<CPeInsideHandler> handler = std::make_unique<CStatFeatureBuilder>();
            if (handler && handler->Initialize()) {
                m_handlers.push_back(std::move(handler));
            }
        }
        if (m_options_info.bcrossreffeature) {
            std::unique_ptr<CPeInsideHandler> handler = std::make_unique<CCrossRefFeatureBuilder>();
            if (handler && handler->Initialize()) {
                m_handlers.push_back(std::move(handler));
            }
        }
        if (m_options_info.bpackerfeature) {
            std::unique_ptr<CPeInsideHandler> handler = std::make_unique<CPackerFeatureBuilder>();
            if (handler && handler->Initialize()) {
                m_handlers.push_back(std::move(handler));
            }
        }
        if (m_options_info.bdatadirectoryfeature) {
            std::unique_ptr<CPeInsideHandler> handler = std::make_unique<CDatadirectoryFeatureBuilder>();
            if (handler && handler->Initialize()) {
                m_handlers.push_back(std::move(handler));
            }
        }
        if (m_options_info.bdebugdatadirectoryfeature) {
            std::unique_ptr<CPeInsideHandler> handler = std::make_unique<CDebugdatadirectoryFeatureBuilder>();
            if (handler && handler->Initialize()) {
                m_handlers.push_back(std::move(handler));
            }
        }
        if (m_options_info.bembeddedfeature) {
            std::unique_ptr<CPeInsideHandler> handler = std::make_unique<CEmbeddedFeatureBuilder>();
            if (handler && handler->Initialize()) {
                m_handlers.push_back(std::move(handler));
            }
        }
        if (m_options_info.bimporttablefeature) {
            std::unique_ptr<CPeInsideHandler> handler = std::make_unique<CImporttableFeatureBuilder>();
            if (handler && handler->Initialize()) {
                m_handlers.push_back(std::move(handler));
            }
        }
        if (m_options_info.bexporttablefeature) {
            std::unique_ptr<CPeInsideHandler> handler = std::make_unique<CExporttableFeatureBuilder>();
            if (handler && handler->Initialize()) {
                m_handlers.push_back(std::move(handler));
            }
        }
        if (m_options_info.biconfeature) {
            std::unique_ptr<CPeInsideHandler> handler = std::make_unique<CIconFeatureBuilder>();
            if (handler && handler->Initialize()) {
                m_handlers.push_back(std::move(handler));
            }
        }
        if (m_options_info.bmanifestfeature) {
            std::unique_ptr<CPeInsideHandler> handler = std::make_unique<CManifestFeatureBuilder>();
            if (handler && handler->Initialize()) {
                m_handlers.push_back(std::move(handler));
            }
        }
        if (m_options_info.bresourcefeature) {
            std::unique_ptr<CPeInsideHandler> handler = std::make_unique<CResourceFeatureBuilder>();
            if (handler && handler->Initialize()) {
                m_handlers.push_back(std::move(handler));
            }
        }
        if (m_options_info.brichheaderfeature) {
            std::unique_ptr<CPeInsideHandler> handler = std::make_unique<CRichheaderFeatureBuilder>();
            if (handler && handler->Initialize()) {
                m_handlers.push_back(std::move(handler));
            }
        }
        if (m_options_info.bsectionfeature) {
            std::unique_ptr<CPeInsideHandler> handler = std::make_unique<CSectionFeatureBuilder>();
            if (handler && handler->Initialize()) {
                m_handlers.push_back(std::move(handler));
            }
        }
        if (m_options_info.bstringsfeature) {
            std::unique_ptr<CPeInsideHandler> handler = std::make_unique<CStringsFeatureBuilder>();
            if (handler && handler->Initialize()) {
                m_handlers.push_back(std::move(handler));
            }
        }
        if (m_options_info.bunpackerfeature) {
            std::unique_ptr<CPeInsideHandler> handler = std::make_unique<CUnpackerFeatureBuilder>();
            if (handler && handler->Initialize()) {
                m_handlers.push_back(std::move(handler));
            }
        }
        if (m_options_info.bdotnetfeature) {
            std::unique_ptr<CPeInsideHandler> handler = std::make_unique<CDotnetFeatureBuilder>();
            if (handler && handler->Initialize()) {
                m_handlers.push_back(std::move(handler));
            }
        }
        return true;
    }

    bool CPeInside::Scan(fs::path& path)
    {
        try {
            std::unique_ptr<PeLib::PeFile> file = GetPeFile(path);
            if (!file)
                return false;
            std::unique_ptr<cchips::PeFormat> pe_format = std::make_unique<cchips::PeFormat>(std::move(file));
            if (!pe_format)
                return false;
            m_json_result = std::make_unique<cchips::CRapidJsonWrapper>("{}");
            if (!m_json_result)
                return false;
            for (auto& handler : m_handlers) {
                handler->Scan(pe_format, m_json_result);
            }
            return true;
        }
        catch (const std::exception& e)
        {
        }
        return false;
    }

    bool CPeInside::ScanPeriod(fs::path& path)
    {
        try {
            std::unique_ptr<PeLib::PeFile> file = GetPeFile(path);
            if (!file)
                return false;
            std::unique_ptr<cchips::PeFormat> pe_format = std::make_unique<cchips::PeFormat>(std::move(file));
            if (!pe_format)
                return false;
            m_json_result = std::make_unique<cchips::CRapidJsonWrapper>("{}");
            if (!m_json_result)
                return false;
            for (auto& handler : m_handlers) {
                handler->ScanPeriod(pe_format, m_json_result);
            }
            return true;
        }
        catch (const std::exception& e)
        {
        }
        return false;
    }

    std::unique_ptr<PeLib::PeFile> CPeInside::GetPeFile(fs::path& path)
    {
        PeLib::PeFile32 pefile(path.string());
        if (pefile.readMzHeader() != PeLib::ERROR_NONE) {
            return nullptr;
        }
        if (!pefile.mzHeader().isValid()) {
            return nullptr;
        }
        if (pefile.readPeHeader() != PeLib::ERROR_NONE) {
            return nullptr;
        }
        if (!pefile.peHeader().isValid()) {
            return nullptr;
        }
        WORD machine = pefile.peHeader().getMachine();
        WORD magic = pefile.peHeader().getMagic();

        if ((machine == PeLib::PELIB_IMAGE_FILE_MACHINE_AMD64
            || machine == PeLib::PELIB_IMAGE_FILE_MACHINE_IA64)
            && magic == PeLib::PELIB_IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
            return std::make_unique<PeLib::PeFile64>(path.string());
        }
        return std::make_unique<PeLib::PeFile32>(path.string());
    }
} // namespace cchips
