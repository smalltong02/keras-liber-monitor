#include "stdafx.h"
#include "ComsObject.h"
#include "utils.h"
#include "HookImplementObject.h"

namespace cchips {

    bool CWmisCfgObject::Initialize(const std::string_view& json_str)
    {
        if (json_str.length() == 0) return false;

        CRapidJsonWrapper document(json_str);
        if (!document.IsValid()) {
            //config data is incorrect.
            error_log("CWmisCfgObject::Initialize failed!");
            return false;
        }
        
        if (auto anyvalue(document.GetMember(std::vector<std::string>{SI_CATEGORY}));
            anyvalue.has_value() && anyvalue.type() == typeid(std::string_view))
            m_category_name = std::any_cast<std::string_view>(anyvalue);

        for (const auto& wmi_docment : document)
        {
            if (wmi_docment.value.IsObject())
            {
                if (!InitializeWmiObject(wmi_docment.name.GetString(), document))
                {
                    error_log("CWmisCfgObject::Initialize InitializeWmiObject({}) failed!", wmi_docment.name.GetString());
                    return false;
                }
            }
        }
        m_bValid = true;
        return true;
    }

    bool CWmisCfgObject::InitializeWmiDatas(const std::shared_ptr<CWmiObject>& wmi_object, const ConstRapidObject& Object)
    {
        if (!wmi_object) return false;
        // set datas information
        if (auto anydata(CRapidJsonWrapper::GetMember(AD_DATA, Object));
            anydata.has_value() && anydata.type() == typeid(ConstRapidObject))
        {
            for (auto& data : std::any_cast<ConstRapidObject>(anydata))
            {
                if (data.value.IsString())
                {
                    std::string_view data_name = data.name.GetString();
                    std::string_view data_iden = data.value.GetString();
                    std::shared_ptr<CObObject> data_ptr = CLexerTy::GetInstance().GetIdentifier(data_iden);
                    if (!data_ptr)
                    {
                        error_log("CWmisCfgObject::InitializeWmiDatas the data {}({}) is invalid type!", data.name.GetString(), data.value.GetString());
                        return false;
                    }
                    if (!wmi_object->AddData(IDENPAIR(data_name, data_ptr)))
                    {
                        error_log("CWmisCfgObject::InitializeWmiDatas the wmi object {} add data {} failed!", wmi_object->GetName(), data.name.GetString());
                        return false;
                    }
                }
                else
                {
                    error_log("CWmisCfgObject::InitializeWmiDatas the data {} not a string!", data.name.GetString());
                    return false;
                }
            }
        }
        return true;
    }

    bool CWmisCfgObject::InitializeCheck(const std::shared_ptr<CWmiObject>& wmi_object, const ConstRapidObject& Object)
    {
        if (!wmi_object) return false;
        auto check_func = [](CFunction::_CheckPackage& package, const ConstRapidObject& Object) {
            if (auto anydefine(CRapidJsonWrapper::GetMember(SI_DEFINE, Object));
                anydefine.has_value() && anydefine.type() == typeid(ConstRapidObject))
            {
                CFunction::_CheckDefine::DefinePair define_pair;
                for (auto& define : std::any_cast<ConstRapidObject>(anydefine))
                {
                    if (!define.value.IsString())
                    {
                        continue;
                    }
                    define_pair.first = define.name.GetString();
                    define_pair.second = define.value.GetString();
                    package.define_pairs.push_back(define_pair);
                }
            }
            if (auto anylog(CRapidJsonWrapper::GetMember(SI_LOG, Object));
                anylog.has_value() && anylog.type() == typeid(ConstRapidArray))
            {
                CFunction::_CheckDefine::CheckLog log;
                for (auto& log_elem : std::any_cast<ConstRapidArray>(anylog))
                {
                    if (log_elem.IsString())
                    {
                        log = log_elem.GetString();
                        package.logs.push_back(log);
                    }
                }
            }
            if (auto anyhandle(CRapidJsonWrapper::GetMember(SI_HANDLE, Object));
                anyhandle.has_value() && anyhandle.type() == typeid(ConstRapidObject))
            {
                CFunction::_CheckDefine::HandlePair handle_pair;
                for (auto& handle : std::any_cast<ConstRapidObject>(anyhandle))
                {
                    if (handle.value.IsString())
                    {
                        handle_pair.first = handle.name.GetString();
                        handle_pair.second = handle.value.GetString();
                        package.handle_pair.push_back(handle_pair);
                    }
                }
            }
            if (auto anycheck(CRapidJsonWrapper::GetMember(SI_CHECK, Object));
                anycheck.has_value() && anycheck.type() == typeid(ConstRapidObject))
            {
                for (auto& check : std::any_cast<ConstRapidObject>(anycheck))
                {
                    package.check_pair.push_back(CFunction::_CheckDefine::CheckPair(check.name.GetString(), check.value.GetString()));
                }
            }
            if (auto anymodify(CRapidJsonWrapper::GetMember(SI_MODIFY, Object));
                anymodify.has_value() && anymodify.type() == typeid(ConstRapidObject))
            {
                for (auto& modify : std::any_cast<ConstRapidObject>(anymodify))
                {
                    package.modify_pair.push_back(CFunction::_CheckDefine::ModifyPair(modify.name.GetString(), modify.value.GetString()));
                }
            }
        };
        // set precheck information
        if (auto anyprecheck(CRapidJsonWrapper::GetMember(SI_PRECHECK, Object));
            anyprecheck.has_value() && anyprecheck.type() == typeid(ConstRapidObject))
        {
            for (auto& inspect_object : std::any_cast<ConstRapidObject>(anyprecheck))
            {
                if (!inspect_object.value.IsObject())
                {
                    continue;
                }
                CFunction::_CheckPackage package;
                check_func(package, inspect_object.value.GetObject());
                if (!wmi_object->AddChecks(package))
                {
                    error_log("CWmisCfgObject::InitializeCheck: wmi object({}) add pre checks failed!", wmi_object->GetName());
                }
            }
        }
        // set postcheck information
        if (auto anypostcheck(CRapidJsonWrapper::GetMember(SI_POSTCHECK, Object));
            anypostcheck.has_value() && anypostcheck.type() == typeid(ConstRapidObject))
        {
            for (auto& inspect_object : std::any_cast<ConstRapidObject>(anypostcheck))
            {
                if (!inspect_object.value.IsObject())
                {
                    continue;
                }
                CFunction::_CheckPackage package;
                check_func(package, inspect_object.value.GetObject());
                if (!wmi_object->AddChecks(package, false))
                {
                    error_log("CWmisCfgObject::InitializeCheck: wmi object({}) add pre checks failed!", wmi_object->GetName());
                }
            }
        }
        return true;
    }

    bool CWmisCfgObject::InitializeWmiMethods(const std::shared_ptr<CWmiObject>& wmi_object, const ConstRapidObject& Object)
    {
        if (!wmi_object) return false;
        // set datas information
        if (auto anymethod(CRapidJsonWrapper::GetMember(COM_METHOD, Object));
            anymethod.has_value() && anymethod.type() == typeid(ConstRapidObject))
        {
            for (auto& methods : std::any_cast<ConstRapidObject>(anymethod))
            {
                if (methods.value.IsObject())
                {
                    if (!CSigsCfgObject::InitializeFunction(CFunction::call_stdcall, methods.name.GetString(), methods.value.GetObject(), wmi_object->GetMethods()))
                    {
                        error_log("CWmisCfgObject::InitializeWmiMethods InitializeMethod({}) failed!", methods.name.GetString());
                        return false;
                    }
                }
                else {
                    error_log("CWmisCfgObject::The Object {} is not Object!", methods.name.GetString());
                    return false;
                }
            }
        }
        return true;
    }

    bool CWmisCfgObject::InitializeWmiObject(const std::string& wmi_object_name, const CRapidJsonWrapper& document)
    {
        if (auto anywmiobj(document.GetMember(wmi_object_name));
            anywmiobj.has_value() && anywmiobj.type() == typeid(ConstRapidObject))
        {
            // create wmi object
            std::shared_ptr<CWmiObject> wmi_object = std::make_shared<CWmiObject>(wmi_object_name);
            if (!wmi_object)
            {
                error_log("CWmisCfgObject::InitializeWmiObject create wmi_object({}) failed!", wmi_object_name);
                return false;
            }
            if (auto anymidl(CRapidJsonWrapper::GetMember(COM_MIDL, std::any_cast<ConstRapidObject>(anywmiobj)));
                anymidl.has_value() && anymidl.type() == typeid(std::string_view))
            {
                std::stringstream ss;
                ss << std::string(std::any_cast<std::string_view>(anymidl));
                if (!wmi_object->SetMidlInterface(ss))
                {
                    error_log("CWmisCfgObject::wmi_object->SetMidlInterface failed!");
                    return false;
                }
            }
            if (!InitializeWmiDatas(wmi_object, std::any_cast<ConstRapidObject>(anywmiobj)))
            {
                error_log("CWmisCfgObject::InitializeWmiObject failed!");
                return false;
            }
            if (!InitializeCheck(wmi_object, std::any_cast<ConstRapidObject>(anywmiobj)))
            {
                error_log("CWmisCfgObject::InitializeCheck failed!");
                return false;
            }
            if (!InitializeWmiMethods(wmi_object, std::any_cast<ConstRapidObject>(anywmiobj)))
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
        else {
            error_log("CWmisCfgObject::InitializeWmiObject the {} not a object!", wmi_object_name);
        }
        return false;
    }

    bool CComsCfgObject::Initialize(const std::string_view& json_str)
    {
        return true;
    }

} // namespace cchips