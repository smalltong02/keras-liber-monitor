#include "stdafx.h"
#include "ComsObject.h"
#include "utils.h"
#include "HookImplementObject.h"

namespace cchips {

    bool CWmisCfgObject::Initialize(const std::string& json_str)
    {
        if (json_str.length() == 0) return false;

        rapidjson::Document document;
        document.Parse(json_str.c_str());
        if (!document.IsObject() || document.IsNull())
        {
            //config data is incorrect.
            error_log("CWmisCfgObject::Initialize failed!");
            return false;
        }

        if (document.HasMember(SI_CATEGORY) && document[SI_CATEGORY].IsString())
            m_category_name = document[SI_CATEGORY].GetString();

        for (const auto& wmi_docment : document.GetObject())
        {
            if (_stricmp(wmi_docment.name.GetString(), SI_CATEGORY) == 0)
                continue;
            else if (_stricmp(wmi_docment.name.GetString(), SI_CALL) == 0)
                continue;
            else
            {
                if (!InitializeWmiObject(wmi_docment.name.GetString(), wmi_docment.value))
                {
                    error_log("CWmisCfgObject::Initialize InitializeWmiObject({}) failed!", wmi_docment.name.GetString());
                    return false;
                }
            }
        }
        m_bValid = true;
        return true;
    }

    bool CWmisCfgObject::InitializeWmiDatas(const std::shared_ptr<CWmiObject>& wmi_object, const rapidjson::Value& Object)
    {
        if (!wmi_object) return false;
        // set datas information
        if (Object.HasMember(AD_DATA) && Object[AD_DATA].IsObject())
        {
            for (auto& data_object : Object[AD_DATA].GetObject())
            {
                if (data_object.value.IsString())
                {
                    std::string data_name = data_object.name.GetString();
                    std::string data_iden = data_object.value.GetString();
                    std::shared_ptr<CObObject> data_ptr = wmi_object->GetTyIdentifier(data_iden);
                    if (!data_ptr)
                    {
                        error_log("CWmisCfgObject::InitializeWmiDatas the data {}({}) is invalid type!", data_name, data_iden);
                        return false;
                    }
                    if (!wmi_object->AddData(IDENPAIR(data_name, data_ptr)))
                    {
                        error_log("CWmisCfgObject::InitializeWmiDatas the wmi object {} add data {} failed!", wmi_object->GetName(), data_name);
                        return false;
                    }
                }
                else
                {
                    error_log("CWmisCfgObject::InitializeWmiDatas the data {} not a string!", data_object.name.GetString());
                    return false;
                }
            }
        }
        return true;
    }

    bool CWmisCfgObject::InitializeCheck(const std::shared_ptr<CWmiObject>& wmi_object, const rapidjson::Value& Object)
    {
        if (!wmi_object) return false;

        // set precheck information
        if (Object.HasMember(SI_PRECHECK) && Object[SI_PRECHECK].IsObject())
        {
            for (auto& inspect_object : Object[SI_PRECHECK].GetObject())
            {
                if (!inspect_object.value.IsObject())
                {
                    continue;
                }
                CFunction::_CheckPackage package;
                if (inspect_object.value.HasMember(SI_DEFINE) && inspect_object.value[SI_DEFINE].IsObject())
                {
                    CFunction::_CheckDefine::DefinePair define_pair;
                    for (auto& check_object : inspect_object.value[SI_DEFINE].GetObject())
                    {
                        if (!check_object.value.IsString())
                        {
                            continue;
                        }
                        define_pair.first = check_object.name.GetString();
                        define_pair.second = check_object.value.GetString();
                        package.define_pairs.push_back(define_pair);
                    }
                }

                if (inspect_object.value.HasMember(SI_LOG) && inspect_object.value[SI_LOG].IsArray())
                {
                    CFunction::_CheckDefine::CheckLog log;
                    for (auto& array_element : inspect_object.value[SI_LOG].GetArray())
                    {
                        if (array_element.IsString())
                        {
                            log = array_element.GetString();
                            package.logs.push_back(log);
                        }
                    }
                }

                if (inspect_object.value.HasMember(SI_HANDLE) && inspect_object.value[SI_HANDLE].IsObject())
                {
                    CFunction::_CheckDefine::HandlePair handle_pair;
                    for (auto& handle_object : inspect_object.value[SI_HANDLE].GetObject())
                    {
                        if (handle_object.value.IsString())
                        {
                            handle_pair.first = handle_object.name.GetString();
                            handle_pair.second = handle_object.value.GetString();
                            package.handle_pair.push_back(handle_pair);
                        }
                    }
                }

                if (inspect_object.value.HasMember(SI_CHECK) && inspect_object.value[SI_CHECK].IsObject())
                {
                    for (auto& check_object : inspect_object.value[SI_CHECK].GetObject())
                    {
                        package.check_pair.push_back(CFunction::_CheckDefine::CheckPair(check_object.name.GetString(), check_object.value.GetString()));
                    }
                }

                if (inspect_object.value.HasMember(SI_MODIFY) && inspect_object.value[SI_MODIFY].IsObject())
                {
                    for (auto& modify_object : inspect_object.value[SI_MODIFY].GetObject())
                    {
                        package.modify_pair.push_back(CFunction::_CheckDefine::ModifyPair(modify_object.name.GetString(), modify_object.value.GetString()));
                    }
                }
                if (!wmi_object->AddChecks(package))
                    error_log("CWmisCfgObject::InitializeCheck: wmi object({}) add pre checks failed!", wmi_object->GetName());
            }
        }
        // set postcheck information
        if (Object.HasMember(SI_POSTCHECK) && Object[SI_POSTCHECK].IsObject())
        {
            for (auto& inspect_object : Object[SI_POSTCHECK].GetObject())
            {
                if (!inspect_object.value.IsObject())
                {
                    continue;
                }
                CFunction::_CheckPackage package;
                if (inspect_object.value.HasMember(SI_DEFINE) && inspect_object.value[SI_DEFINE].IsObject())
                {
                    CFunction::_CheckDefine::DefinePair define_pair;
                    for (auto& check_object : inspect_object.value[SI_DEFINE].GetObject())
                    {
                        if (!check_object.value.IsString())
                        {
                            continue;
                        }
                        define_pair.first = check_object.name.GetString();
                        define_pair.second = check_object.value.GetString();
                        package.define_pairs.push_back(define_pair);
                    }
                }

                if (inspect_object.value.HasMember(SI_LOG) && inspect_object.value[SI_LOG].IsArray())
                {
                    CFunction::_CheckDefine::CheckLog log;
                    for (auto& array_element : inspect_object.value[SI_LOG].GetArray())
                    {
                        if (array_element.IsString())
                        {
                            log = array_element.GetString();
                            package.logs.push_back(log);
                        }
                    }
                }

                if (inspect_object.value.HasMember(SI_HANDLE) && inspect_object.value[SI_HANDLE].IsArray())
                {
                    CFunction::_CheckDefine::HandlePair handle_pair;
                    for (auto& handle_object : inspect_object.value[SI_HANDLE].GetObject())
                    {
                        if (handle_object.value.IsString())
                        {
                            handle_pair.first = handle_object.name.GetString();
                            handle_pair.second = handle_object.value.GetString();
                            package.handle_pair.push_back(handle_pair);
                        }
                    }
                }

                if (inspect_object.value.HasMember(SI_CHECK) && inspect_object.value[SI_CHECK].IsObject())
                {
                    for (auto& check_object : inspect_object.value[SI_CHECK].GetObject())
                    {
                        package.check_pair.push_back(CFunction::_CheckDefine::CheckPair(check_object.name.GetString(), check_object.value.GetString()));
                    }
                }

                if (inspect_object.value.HasMember(SI_MODIFY) && inspect_object.value[SI_MODIFY].IsObject())
                {
                    for (auto& modify_object : inspect_object.value[SI_MODIFY].GetObject())
                    {
                        package.modify_pair.push_back(CFunction::_CheckDefine::ModifyPair(modify_object.name.GetString(), modify_object.value.GetString()));
                    }
                }
                if (!wmi_object->AddChecks(package, false))
                    error_log("CWmisCfgObject::InitializeCheck: wmi object({}) add pre checks failed!", wmi_object->GetName());
            }
        }

        return true;
    }

    bool CWmisCfgObject::InitializeWmiMethods(const std::shared_ptr<CWmiObject>& wmi_object, const rapidjson::Value& Object)
    {
        if (!wmi_object) return false;
        // set datas information
        if (Object.HasMember(COM_METHOD) && Object[COM_METHOD].IsObject())
        {
            for (auto& method_object : Object[COM_METHOD].GetObject())
            {
                if (!CSigsCfgObject::InitializeFunction(CFunction::call_stdcall, method_object.name.GetString(), method_object.value, wmi_object->GetMethods()))
                {
                    error_log("CWmisCfgObject::InitializeWmiMethods InitializeMethod({}) failed!", method_object.name.GetString());
                    return false;
                }
            }
        }
        return true;
    }

    bool CWmisCfgObject::InitializeWmiObject(const std::string& wmi_object_name, const rapidjson::Value& Object)
    {
        if (!Object.IsObject() || Object.IsNull())
        {
            error_log("CWmisCfgObject::InitializeWmiObject the {} not a object!", wmi_object_name);
            return false;
        }
        // create wmi object
        std::shared_ptr<CWmiObject> wmi_object = std::make_shared<CWmiObject>(wmi_object_name);
        if (!wmi_object)
        {
            error_log("CWmisCfgObject::InitializeWmiObject create wmi_object({}) failed!", wmi_object_name);
            return false;
        }
        if (Object.HasMember(COM_MIDL) && Object[COM_MIDL].IsString())
        {
            std::stringstream ss;
            ss << Object[COM_MIDL].GetString();
            if (!wmi_object->SetMidlInterface(ss))
            {
                error_log("CWmisCfgObject::wmi_object->SetMidlInterface failed!");
                return false;
            }
        }
        if (!InitializeWmiDatas(wmi_object, Object))
        {
            error_log("CWmisCfgObject::InitializeWmiObject failed!");
            return false;
        }
        if (!InitializeCheck(wmi_object, Object))
        {
            error_log("CWmisCfgObject::InitializeCheck failed!");
            return false;
        }
        if (!InitializeWmiMethods(wmi_object, Object))
        {
            error_log("CWmisCfgObject::InitializeWmiObject failed!");
            return false;
        }
        if (!g_impl_object || !g_impl_object->AddWmiObject(wmi_object))
        {
            error_log("g_impl_object->AddWmiObject({}) failed!", wmi_object->GetName());
            return false;
        }
        return true;
    }

    bool CComsCfgObject::Initialize(const std::string& json_str)
    {
        return true;
    }

} // namespace cchips