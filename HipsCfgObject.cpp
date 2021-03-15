//===- HipsCfgObject.h - metadata type implement define -----------------------------*- C++ -*-===//
// 
// This file implements CHipsCfgObject class code.
//
//===------------------------------------------------------------------------------------------===//
#include "stdafx.h"
#include "commutils.h"
#include "HipsCfgObject.h"
#include "utils.h"
#include "PackageWrapper.h"
#include "rapidjson\document.h"

namespace cchips {

    bool CHipsCfgObject::Initialize(const std::string_view& json_str)
    {
        if (m_bValid)
        {
            return true;
        }

        if (json_str.length() == 0) return false;

        HMODULE module_handle = NULL;
        if(g_is_dll_module)
            module_handle = GetModuleHandle(__TEXT("hipshook.dll"));
        else
            module_handle = GetModuleHandle(NULL);
        if (module_handle == NULL)
        {
            error_log("CHipsConfigObject::Initialize can't get module handle!");
            return false;
        }

        CRapidJsonWrapper document(json_str);
        if (!document.IsValid()) {
            //config data is incorrect.
            error_log("CHipsConfigObject::Initialize failed!");
            return false;
        }

        if (auto anyvalue(document.GetMember(std::vector<std::string>{FL_NAME})); 
            anyvalue.has_value() && anyvalue.type() == typeid(std::string_view))
            m_Info.Name = std::any_cast<std::string_view>(anyvalue);
        if (auto anyvalue(document.GetMember(std::vector<std::string>{FL_VERSION})); 
            anyvalue.has_value() && anyvalue.type() == typeid(std::string_view))
            m_Info.Version = std::any_cast<std::string_view>(anyvalue);
        if (auto anyvalue(document.GetMember(std::vector<std::string>{FL_CREATEDATE}));
            anyvalue.has_value() && anyvalue.type() == typeid(std::string_view))
            m_Info.CreateDate = std::any_cast<std::string_view>(anyvalue);
        if (auto anyvalue(document.GetMember(std::vector<std::string>{FL_DESCRIPTION}));
            anyvalue.has_value() && anyvalue.type() == typeid(std::string_view))
            m_Info.Description = std::any_cast<std::string_view>(anyvalue);
        if (auto anyvalue(document.GetMember(std::vector<std::string>{FL_CONFIG}));
            anyvalue.has_value() && anyvalue.type() == typeid(ConstRapidObject)) {
            if (auto modevalue(document.GetMember(std::vector<std::string>{FL_CONFIG, FL_MODE}));
                modevalue.has_value() && modevalue.type() == typeid(int)) {
                m_hips_mode = (_hips_mode)std::any_cast<int>(modevalue);
            }
        }

        if (!InitializeFlagsObjects(module_handle, document))
            return false;
        if (!InitializeSignsObjects(module_handle, document))
            return false;
        if (!InitializeWmisObjects(module_handle, document))
            return false;
        if (!InitializeComsObjects(module_handle, document))
            return false;

        m_bValid = true;
        return true;
    }

    bool CHipsCfgObject::InitializeFlagsObjects(const HMODULE handle, const CRapidJsonWrapper& document)
    {
        if (handle == NULL) 
            return false;

        bool bret = false;
        if (auto anyvalue(document.GetMember(FL_FLAGS)); 
            anyvalue.has_value() && anyvalue.type() == typeid(ConstRapidArray))
        {
            std::vector<BYTE> ResBuffer;
            for (auto& flags_elem : std::any_cast<ConstRapidArray>(anyvalue))
            {
                if (flags_elem.IsInt())
                {
                    DWORD flags_ord = flags_elem.GetInt();

                    bool result = ExtractResource(handle,
                        __TEXT("JSONRES"),
                        MAKEINTRESOURCE(flags_ord),
                        ResBuffer);
                    if (result && ResBuffer.size() > 0)
                    {
                        std::ostringstream flags_name;
                        flags_name << "JSONRES_" << flags_ord;
                        std::unique_ptr<CFlagsCfgObject> flags_object = std::make_unique<CFlagsCfgObject>(flags_name.str());
                        if (flags_object && flags_object->Initialize(std::string_view((char*)&ResBuffer[0], ResBuffer.size())))
                        {
                            m_FlagsObjects.push_back(std::move(flags_object));
                        }
                        bret = true;
                    }
                    else
                    {
                        error_log("CHipsConfigObject::Initialize extract(flags-{}) failed!", flags_ord);
                        break;
                    }
                }
            }
        }
        return bret;
    }

    bool CHipsCfgObject::InitializeSignsObjects(const HMODULE handle, const CRapidJsonWrapper& document)
    {
        if (handle == NULL) 
            return false;

        bool bret = false;
        if (auto anyvalue(document.GetMember(FL_SIGS));
            anyvalue.has_value() && anyvalue.type() == typeid(ConstRapidArray))
        {
            std::vector<BYTE> ResBuffer;
            for (auto& sigs_elem : std::any_cast<ConstRapidArray>(anyvalue))
            {
                if (sigs_elem.IsInt())
                {
                    DWORD sigs_ord = sigs_elem.GetInt();
                        
                    bool result = ExtractResource(handle,
                        __TEXT("JSONRES"),
                        MAKEINTRESOURCE(sigs_ord),
                        ResBuffer);
                    if (result && ResBuffer.size() > 0)
                    {
                        std::ostringstream sigs_name;
                        sigs_name << "JSONRES_" << sigs_ord;
                        std::unique_ptr<CSigsCfgObject> sigs_object = std::make_unique<CSigsCfgObject>(sigs_name.str());
                        if (sigs_object && sigs_object->Initialize(std::string_view((char*)&ResBuffer[0], ResBuffer.size())))
                        {
                            m_SigsObjects.push_back(std::move(sigs_object));
                        }
                        bret = true;
                    }
                    else
                    {
                        error_log("CHipsConfigObject::Initialize extract(sigs-{}) failed!", sigs_ord);
                        break;
                    }
                }
            }
        }
        return bret;
    }

    bool CHipsCfgObject::InitializeWmisObjects(const HMODULE handle, const CRapidJsonWrapper& document)
    {
        if (handle == NULL) 
            return false;

        bool bret = false;
        if (auto anyvalue(document.GetMember(FL_WMIS));
            anyvalue.has_value() && anyvalue.type() == typeid(ConstRapidArray))
        {
            std::vector<BYTE> ResBuffer;
            for (auto& wmis_elem : std::any_cast<ConstRapidArray>(anyvalue))
            {
                if (wmis_elem.IsInt())
                {
                    DWORD wmis_ord = wmis_elem.GetInt();

                    bool result = ExtractResource(handle,
                        __TEXT("JSONRES"),
                        MAKEINTRESOURCE(wmis_ord),
                        ResBuffer);
                    if (result && ResBuffer.size() > 0)
                    {
                        std::ostringstream wmis_name;
                        wmis_name << "JSONRES_" << wmis_ord;
                        std::unique_ptr<CWmisCfgObject> wmis_object = std::make_unique<CWmisCfgObject>(wmis_name.str());
                        if (wmis_object && wmis_object->Initialize(std::string_view((char*)&ResBuffer[0], ResBuffer.size())))
                        {
                            m_WmisObjects.push_back(std::move(wmis_object));
                        }
                        bret = true;
                    }
                    else
                    {
                        error_log("CHipsConfigObject::Initialize extract(wmis-{}) failed!", wmis_ord);
                        break;
                    }
                }
            }
        }
        return bret;
    }

    bool CHipsCfgObject::InitializeComsObjects(const HMODULE handle, const CRapidJsonWrapper& document)
    {
        if (handle == NULL) 
            return false;

        bool bret = false;
        if (auto anyvalue(document.GetMember(FL_COMS));
            anyvalue.has_value() && anyvalue.type() == typeid(ConstRapidArray))
        {
            std::vector<BYTE> ResBuffer;
            for (auto& coms_elem : std::any_cast<ConstRapidArray>(anyvalue))
            {
                if (coms_elem.IsInt())
                {
                    DWORD coms_ord = coms_elem.GetInt();

                    bool result = ExtractResource(handle,
                        __TEXT("JSONRES"),
                        MAKEINTRESOURCE(coms_ord),
                        ResBuffer);
                    if (result && ResBuffer.size() > 0)
                    {
                        std::ostringstream coms_name;
                        coms_name << "JSONRES_" << coms_ord;
                        std::unique_ptr<CComsCfgObject> coms_object = std::make_unique<CComsCfgObject>(coms_name.str());
                        if (coms_object && coms_object->Initialize(std::string_view((char*)&ResBuffer[0], ResBuffer.size())))
                        {
                            m_ComsObjects.push_back(std::move(coms_object));
                        }
                        bret = true;
                    }
                    else
                    {
                        error_log("CHipsConfigObject::Initialize extract(coms-{}) failed!", coms_ord);
                        break;
                    }
                }
            }
        }
        return bret;
    }

} // namespace cchips


