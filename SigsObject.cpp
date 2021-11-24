#include "stdafx.h"
#include "SigsObject.h"
#include "utils.h"
#include "FlagsObject.h"
#include "HipsCfgObject.h"
#include "HookImplementObject.h"
#include "rapidjson\document.h"

namespace cchips {
    CFunction::_call_convention CSigsCfgObject::GetCallConvention(const std::string_view& str_call) const
    {
        std::string str(str_call);
        if (_stricmp(str.c_str(), CCS_WINAPI) == 0 ||
            _stricmp(str.c_str(), CCS_STDCALL) == 0)
            return CFunction::call_stdcall;
        else if (_stricmp(str.c_str(), CCS_CDECL) == 0)
            return CFunction::call_cdecl;
        else if (_stricmp(str.c_str(), CCS_FASTCALL) == 0)
            return CFunction::call_fastcall;
        else if (_stricmp(str.c_str(), CCS_VECTORCALL) == 0)
            return CFunction::call_vectorcall;
        return CFunction::call_unknown;
    }

    bool CSigsCfgObject::Initialize(const std::string_view& json_str)
    {
        if (json_str.length() == 0) return false;

        CRapidJsonWrapper document(json_str);
        if (!document.IsValid()) {
            //config data is incorrect.
            error_log("CSigsConfigObject::Initialize failed!");
            return false;
        }

        CFunction::_call_convention call_conv = CFunction::call_unknown;
        if (auto anyvalue(document.GetMember(std::vector<std::string>{SI_CATEGORY}));
            anyvalue.has_value() && anyvalue.type() == typeid(std::string_view))
            m_category_name = std::any_cast<std::string_view>(anyvalue);
        if (auto anyvalue(document.GetMember(std::vector<std::string>{SI_CALL}));
            anyvalue.has_value() && anyvalue.type() == typeid(std::string_view))
            call_conv = GetCallConvention(std::any_cast<std::string_view>(anyvalue));

        if (call_conv == CFunction::call_unknown)
        {
            error_log("CSigsConfigObject::Initialize get a error call convention!");
            return false;
        }

        for (const auto& api_docment : document)
        {
            if (api_docment.value.IsObject())
            {
                if (!InitializeFunction(call_conv, api_docment.name.GetString(), api_docment.value.GetObject(), g_impl_object))
                {
                    error_log("CSigsConfigObject::Initialize InitializeFunction({}) failed!", api_docment.name.GetString());
                    return false;
                }
            }
        }
        m_bValid = true;
        return true;
    }

    bool CSigsCfgObject::InitializePrototype(const std::shared_ptr<CFunction>& func_object, const ConstRapidObject& Object)
    {
        if (!func_object) return false;
        // set signature information

        if (auto anysigs(CRapidJsonWrapper::GetMember(SI_SIGNATURE, Object));
            anysigs.has_value() && anysigs.type() == typeid(ConstRapidObject))
        {
            if (auto anylib(CRapidJsonWrapper::GetMember(std::vector<std::string>{ SI_SIGNATURE, SI_LIBRARY }, Object));
                anylib.has_value() && anylib.type() == typeid(std::string_view))
            {
                func_object->SetLibrary(std::any_cast<std::string_view>(anylib));
            }
            else {
                error_log("CSigsConfigObject::InitializePrototype the library is not correct!");
                return false;
            }
            if (auto anyreturn(CRapidJsonWrapper::GetMember(std::vector<std::string>{ SI_SIGNATURE, SI_RETURN }, Object));
                anyreturn.has_value() && anyreturn.type() == typeid(std::string_view))
            {
                std::shared_ptr<CObObject> return_ptr = CLexerTy::GetInstance().GetIdentifier(std::any_cast<std::string_view>(anyreturn));
                if(!return_ptr)
                {
                    error_log("CSigsConfigObject::InitializePrototype function({}) has invalid return type({})!", func_object->GetName(), std::string(std::any_cast<std::string_view>(anyreturn)));
                    return false;
                }
                if (!func_object->AddReturn(IDENPAIR(SI_RETURN, return_ptr)))
                {
                    error_log("CSigsConfigObject::InitializePrototype ({})AddReturn({}) failed!", func_object->GetName(), std::string(std::any_cast<std::string_view>(anyreturn)));
                    return false;
                }
            }
            else {
                error_log("CSigsConfigObject::InitializePrototype the return type not correct!");
                return false;
            }
            if (auto anyspecial(CRapidJsonWrapper::GetMember(std::vector<std::string>{ SI_SIGNATURE, SI_SPECIAL }, Object));
                anyspecial.has_value() && anyspecial.type() == typeid(bool))
            {
                func_object->SetSpecial(std::any_cast<bool>(anyspecial));
            }
            if (auto anyfeature(CRapidJsonWrapper::GetMember(std::vector<std::string>{ SI_SIGNATURE, SI_FEATURE }, Object));
                anyfeature.has_value() && anyfeature.type() == typeid(std::string_view))
            {
                func_object->SetFeature(std::any_cast<std::string_view>(anyfeature));
            }
            // set class information
            if (auto anyclass(CRapidJsonWrapper::GetMember(SI_CLASS, Object));
                anyclass.has_value() && anyclass.type() == typeid(ConstRapidObject))
            {
                unsigned int class_vtblidx = 0;
                CPrototype::_class_type class_type;
                std::string_view class_name;
                if (auto anytype(CRapidJsonWrapper::GetMember(std::vector<std::string>{ SI_CLASS, SI_CTYPE }, Object));
                    anytype.has_value() && anytype.type() == typeid(std::string_view))
                {
                    class_type = CPrototype::GetClassType(std::any_cast<std::string_view>(anytype));
                    if (auto anyname(CRapidJsonWrapper::GetMember(std::vector<std::string>{ SI_CLASS, SI_CNAME }, Object));
                        anyname.has_value() && anyname.type() == typeid(std::string_view))
                    {
                        class_name = std::any_cast<std::string_view>(anyname);
                        if (auto anyvtblidx(CRapidJsonWrapper::GetMember(std::vector<std::string>{ SI_CLASS, SI_CVTBLIDX }, Object));
                            anyvtblidx.has_value() && anyvtblidx.type() == typeid(int))
                        {
                            class_vtblidx = static_cast<unsigned int>(std::any_cast<int>(anyvtblidx));
                        }
                    }
                    std::string_view class_delay;
                    if (auto anydelay(CRapidJsonWrapper::GetMember(std::vector<std::string>{ SI_CLASS, SI_CDELAY }, Object));
                        anydelay.has_value() && anydelay.type() == typeid(std::string_view))
                    {
                        class_delay = std::any_cast<std::string_view>(anydelay);
                    }
                    std::string_view class_bitmap;
                    if (auto anydelay(CRapidJsonWrapper::GetMember(std::vector<std::string>{ SI_CLASS, SI_BITMAP }, Object));
                        anydelay.has_value() && anydelay.type() == typeid(std::string_view))
                    {
                        class_bitmap = std::any_cast<std::string_view>(anydelay);
                        std::unique_ptr<CPrototype::ClassProto> class_proto = std::make_unique<CPrototype::ClassProto>(class_type, class_name, class_bitmap);
                        func_object->SetClassProto(std::move(class_proto));
                    }
                    else {
                        std::unique_ptr<CPrototype::ClassProto> class_proto = std::make_unique<CPrototype::ClassProto>(class_type, class_name, class_delay, class_vtblidx);
                        func_object->SetClassProto(std::move(class_proto));
                    }
                }
            }
            // set hooks information
            if (auto anyclass(CRapidJsonWrapper::GetMember(SI_HOOKS, Object));
                anyclass.has_value() && anyclass.type() == typeid(ConstRapidObject))
            {
                std::string_view platform;
                std::string_view processes;
                std::string_view category;
                if (auto anytype(CRapidJsonWrapper::GetMember(std::vector<std::string>{ SI_HOOKS, SI_PLATFORM }, Object));
                    anytype.has_value() && anytype.type() == typeid(std::string_view))
                {
                    platform = std::any_cast<std::string_view>(anytype);
                }
                if (auto anytype(CRapidJsonWrapper::GetMember(std::vector<std::string>{ SI_HOOKS, SI_PROC_TYPE }, Object));
                    anytype.has_value() && anytype.type() == typeid(std::string_view))
                {
                    processes = std::any_cast<std::string_view>(anytype);
                }
                if (auto anytype(CRapidJsonWrapper::GetMember(std::vector<std::string>{ SI_HOOKS, SI_PROC_CATEGORY }, Object));
                    anytype.has_value() && anytype.type() == typeid(std::string_view))
                {
                    category = std::any_cast<std::string_view>(anytype);
                }
                std::unique_ptr<CPrototype::HooksProto> hooks_proto = std::make_unique<CPrototype::HooksProto>(platform, processes, category);
                func_object->SetHooksProto(std::move(hooks_proto));
            }
            // set parameter information
            if (auto anyfeature(CRapidJsonWrapper::GetMember(SI_PARAMETERS, Object));
                anyfeature.has_value() && anyfeature.type() == typeid(ConstRapidObject))
            {
                for (auto& param_object : std::any_cast<ConstRapidObject>(anyfeature))
                {
                    if (param_object.value.IsString())
                    {
                        std::shared_ptr<CObObject> param_ptr = CLexerTy::GetInstance().GetIdentifier(param_object.value.GetString());
                        if (!param_ptr)
                        {
                            error_log("CSigsConfigObject::InitializePrototype the parameter {}({}) is a invalid parameter!", param_object.name.GetString(), param_object.value.GetString());
                            return false;
                        }
                        if (!func_object->AddArgument(IDENPAIR(param_object.name.GetString(), param_ptr)))
                        {
                            error_log("CSigsConfigObject::InitializePrototype the function {} add parameter {} failed!", param_object.name.GetString(), param_object.value.GetString());
                            return false;
                        }
                    }
                    else
                    {
                        error_log("CSigsConfigObject::InitializePrototype the param {} not a string!", param_object.name.GetString());
                        return false;
                    }
                }
            }
            return true;
        }
        else {
            error_log("CSigsConfigObject::InitializePrototype the signature not a object!");
        }
        return false;
    }

    bool CSigsCfgObject::InitializeHandle(const std::shared_ptr<CFunction>& func_object, const ConstRapidObject& Object)
    {
        if (!func_object) return false;
        // set handle information
        return true;
    }

    bool CSigsCfgObject::InitializeEnsure(const std::shared_ptr<CFunction>& func_object, const ConstRapidObject& Object)
    {
        if (!func_object) return false;
        // set ensure information
        if (auto anyensure(CRapidJsonWrapper::GetMember(SI_ENSURE, Object));
            anyensure.has_value() && anyensure.type() == typeid(ConstRapidArray))
        {
            for (auto& ensure : std::any_cast<ConstRapidArray>(anyensure))
            {
                if (ensure.IsString())
                {
                    if (!func_object->AddEnsure(ensure.GetString()))
                    {
                        error_log("CSigsCfgObject::InitializeEnsure: add ensure {} failed!", ensure.GetString());
                    }
                }
            }
        }
        return true;
    }

    bool CSigsCfgObject::InitializeLog(const std::shared_ptr<CFunction>& func_object, const ConstRapidObject& Object)
    {
        if (!func_object) return false;
        // set log information
        bool blogging = true;
        if (auto anyprelog(CRapidJsonWrapper::GetMember(SI_PRELOG, Object));
            anyprelog.has_value() && anyprelog.type() == typeid(ConstRapidArray))
        {
            for (auto& prelog : std::any_cast<ConstRapidArray>(anyprelog))
            {
                if (prelog.IsString())
                {
                    if (!func_object->AddLogging(prelog.GetString()))
                    {
                        error_log("CSigsCfgObject::InitializeLog: add logging {} failed!", prelog.GetString());
                    }
                    blogging = false;
                }
            }
        }
        if (auto anypostlog(CRapidJsonWrapper::GetMember(SI_POSTLOG, Object));
            anypostlog.has_value() && anypostlog.type() == typeid(ConstRapidArray))
        {
            for (auto& anypostlog : std::any_cast<ConstRapidArray>(anypostlog))
            {
                if (anypostlog.IsString())
                {
                    if (!func_object->AddLogging(anypostlog.GetString(), false))
                    {
                        error_log("CSigsCfgObject::InitializeLog: add logging {} failed!", anypostlog.GetString());
                    }
                    blogging = false;
                }
            }
        }
        if (auto anylogging(CRapidJsonWrapper::GetMember(SI_LOGGING, Object));
            anylogging.has_value() && anylogging.type() == typeid(bool))
        {
            if (std::any_cast<bool>(anylogging))
            {
                // only record return to r3 log.
                if (!func_object->AddLogging((*func_object).GetReturn().first, false))
                {
                    error_log("CSigsCfgObject::InitializeLog: add logging {} failed!", (*func_object).GetReturn().first);
                }
            }
        }
        return true;
    }

    bool CSigsCfgObject::InitializeCheck(const std::shared_ptr<CFunction>& func_object, const ConstRapidObject& Object)
    {
        if (!func_object) return false;
        // set precheck information
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
            if (auto anydefine(CRapidJsonWrapper::GetMember(SI_CCODE, Object));
                anydefine.has_value() && anydefine.type() == typeid(ConstRapidArray))
            {
                // process CCode.
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
            return;
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
                if (!func_object->AddChecks(package))
                {
                    error_log("CSigsCfgObject::InitializeCheck: function({}) add pre checks failed!", func_object->GetName());
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
                if (!func_object->AddChecks(package, false))
                {
                    error_log("CSigsCfgObject::InitializeCheck: function({}) add post checks failed!", func_object->GetName());
                }
            }
        }
        return true;
    }

    bool CSigsCfgObject::InitializeFunction(CFunction::_call_convention call_conv, const std::string& func_name, const ConstRapidObject& object, std::shared_ptr<CFunctionProtos> func_protos)
    {
        if (!func_protos) return false;
        ASSERT(func_protos != nullptr);

        // create api object
        std::shared_ptr<CFunction> func_object = std::make_shared<CFunction>(call_conv, func_name);
        if (!func_object)
        {
            error_log("CSigsConfigObject::InitializeFunction create func_object({}) failed!", func_name);
            return false;
        }
        if (!InitializePrototype(func_object, object))
        {
            error_log("CSigsConfigObject::InitializePrototype failed!");
            return false;
        }
        if (!InitializeHandle(func_object, object))
        {
            error_log("CSigsConfigObject::InitializeHandle failed!");
            return false;
        }
        if (!InitializeEnsure(func_object, object))
        {
            error_log("CSigsConfigObject::InitializeEnsure failed!");
            return false;
        }
        if (!InitializeLog(func_object, object))
        {
            error_log("CSigsConfigObject::InitializeLog failed!");
            return false;
        }
        if (!InitializeCheck(func_object, object))
        {
            error_log("CSigsConfigObject::InitializeCheck failed!");
            return false;
        }
        if (!func_protos->AddFunction(std::move(func_object)))
        {
            error_log("g_impl_object->AddFunction({}) failed!", func_name);
            return false;
        }
        return true;
    }

} // namespace cchips