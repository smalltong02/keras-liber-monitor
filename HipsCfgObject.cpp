//===- HipsCfgObject.h - metadata type implement define -----------------------------*- C++ -*-===//
// 
// This file implements CHipsCfgObject class code.
//
//===------------------------------------------------------------------------------------------===//
#include "stdafx.h"
#include "commutils.h"
#include "HipsCfgObject.h"
#include "utils.h"
#include "rapidjson\document.h"

namespace cchips {

    bool CHipsCfgObject::Initialize(const std::string& json_str)
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

        rapidjson::Document document;
        document.Parse(json_str.c_str());
        if (!document.IsObject() || document.IsNull())
        {
            //config data is incorrect.
            error_log("CHipsConfigObject::Initialize failed!");
            return false;
        }

        // save base header information
        if (document.HasMember(FL_NAME) && document[FL_NAME].IsString())
            m_Info.Name = document[FL_NAME].GetString();
        if (document.HasMember(FL_VERSION) && document[FL_VERSION].IsString())
            m_Info.Version = document[FL_VERSION].GetString();
        if (document.HasMember(FL_CREATEDATE) && document[FL_CREATEDATE].IsString())
            m_Info.CreateDate = document[FL_CREATEDATE].GetString();
        if (document.HasMember(FL_DESCRIPTION) && document[FL_DESCRIPTION].IsString())
            m_Info.Description = document[FL_DESCRIPTION].GetString();

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

    bool CHipsCfgObject::InitializeFlagsObjects(const HMODULE handle, const rapidjson::Document& document)
    {
        bool bret = false;
        if (handle == NULL) return bret;

        std::vector<BYTE> ResBuffer;
        if (document.HasMember(FL_FLAGS) && document[FL_FLAGS].IsArray())
        {
            for (auto& flags_array : document[FL_FLAGS].GetArray())
            {
                DWORD flags_ord = flags_array.GetInt();

                bool result = ExtractResource(handle,
                    __TEXT("JSONRES"),
                    MAKEINTRESOURCE(flags_ord),
                    ResBuffer);
                if (result && ResBuffer.size() > 0)
                {
                    char flags_name[20];
                    sprintf_s(flags_name, "JSONRES_%d", flags_ord);
                    std::unique_ptr<CFlagsCfgObject> flags_object = std::make_unique<CFlagsCfgObject>(flags_name);
                    if (flags_object && flags_object->Initialize(std::string((char*)&ResBuffer[0], ResBuffer.size())))
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
        return bret;
    }

    bool CHipsCfgObject::InitializeSignsObjects(const HMODULE handle, const rapidjson::Document& document)
    {
        bool bret = false;
        if (handle == NULL) return bret;

        std::vector<BYTE> ResBuffer;
        if (document.HasMember(FL_SIGS) && document[FL_SIGS].IsArray())
        {
            for (auto& sigs_array : document[FL_SIGS].GetArray())
            {
                DWORD sigs_ord = sigs_array.GetInt();

                bool result = ExtractResource(handle,
                    __TEXT("JSONRES"),
                    MAKEINTRESOURCE(sigs_ord),
                    ResBuffer);
                if (result && ResBuffer.size() > 0)
                {
                    char sigs_name[20];
                    sprintf_s(sigs_name, "JSONRES_%d", sigs_ord);
                    std::unique_ptr<CSigsCfgObject> sigs_object = std::make_unique<CSigsCfgObject>(sigs_name);
                    if (sigs_object && sigs_object->Initialize(std::string((char*)&ResBuffer[0], ResBuffer.size())))
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
        return bret;
    }

    bool CHipsCfgObject::InitializeWmisObjects(const HMODULE handle, const rapidjson::Document& document)
    {
        bool bret = false;
        if (handle == NULL) return bret;

        std::vector<BYTE> ResBuffer;
        if (document.HasMember(FL_WMIS) && document[FL_WMIS].IsArray())
        {
            for (auto& wmis_array : document[FL_WMIS].GetArray())
            {
                DWORD wmis_ord = wmis_array.GetInt();

                bool result = ExtractResource(handle,
                    __TEXT("JSONRES"),
                    MAKEINTRESOURCE(wmis_ord),
                    ResBuffer);
                if (result && ResBuffer.size() > 0)
                {
                    char wmis_name[20];
                    sprintf_s(wmis_name, "JSONRES_%d", wmis_ord);
                    std::unique_ptr<CWmisCfgObject> wmis_object = std::make_unique<CWmisCfgObject>(wmis_name);
                    if (wmis_object && wmis_object->Initialize(std::string((char*)&ResBuffer[0], ResBuffer.size())))
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
        return bret;
    }

    bool CHipsCfgObject::InitializeComsObjects(const HMODULE handle, const rapidjson::Document& document)
    {
        bool bret = false;
        if (handle == NULL) return bret;

        std::vector<BYTE> ResBuffer;
        if (document.HasMember(FL_COMS) && document[FL_COMS].IsArray())
        {
            for (auto& coms_array : document[FL_COMS].GetArray())
            {
                DWORD coms_ord = coms_array.GetInt();

                bool result = ExtractResource(handle,
                    __TEXT("JSONRES"),
                    MAKEINTRESOURCE(coms_ord),
                    ResBuffer);
                if (result && ResBuffer.size() > 0)
                {
                    char coms_name[20];
                    sprintf_s(coms_name, "JSONRES_%d", coms_ord);
                    std::unique_ptr<CComsCfgObject> coms_object = std::make_unique<CComsCfgObject>(coms_name);
                    if (coms_object && coms_object->Initialize(std::string((char*)&ResBuffer[0], ResBuffer.size())))
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
        return bret;
    }

} // namespace cchips


