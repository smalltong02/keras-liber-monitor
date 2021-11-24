#include "stdafx.h"
#include "hookimpl.h"
#include "HookImplementObject.h"
#include "utils.h"
#include "Win32WbemClassObject.h"
#include "Win32EnumWbemClassObject.h"

using namespace cchips;

processing_status WINAPI detour_getTickCount(CHookImplementObject::detour_node* node)
{
    BreakPoint;
    return processing_continue;
}

processing_status WINAPI detour_getModuleHandle(CHookImplementObject::detour_node* node, LPCWSTR lpModuleName)
{
    BreakPoint;
    return processing_continue;
}

processing_status WINAPI CHookImplementObject::detour_coInitializeEx(CHookImplementObject::detour_node* node, LPVOID pvReserved, DWORD dwCoInit)
{
    POST_BEGIN(node)
    //processing delay hooks.
    std::vector<int> ord_list;
    do {
        std::lock_guard<std::recursive_mutex> lock_guard(g_impl_object->GetRecursiveMutex());
        auto& delay_list = g_impl_object->GetDelayNodeList();
        if (delay_list.find("WMI") == delay_list.end()) break;
        ord_list = delay_list["WMI"];
    } while (0);

    for (auto i : ord_list)
    {
        if (g_impl_object->GetHookNodeList()[i].bdelayed && 
            g_impl_object->GetHookNodeList()[i].function &&
            g_impl_object->GetHookNodeList()[i].function->GetClassProto() &&
            g_impl_object->GetHookNodeList()[i].function->GetClassProto()->GetClassType() == CPrototype::class_wmi)
        {
            g_impl_object->HookApi(g_impl_object->GetHookNodeList()[i]);
        }
    }
    return processing_continue;
}

processing_status WINAPI CHookImplementObject::detour_coUninitialize(detour_node* node)
{
    PRE_BEGIN(node)
    std::vector<int> ord_list;
    do {
        std::lock_guard<std::recursive_mutex> lock_guard(g_impl_object->GetRecursiveMutex());
        auto& delay_list = g_impl_object->GetDelayNodeList();
        if (delay_list.find("WMI") == delay_list.end()) break;
        ord_list = delay_list["WMI"];
    } while (0);

    for (auto i : ord_list)
    {
        if (!node->hook_implement_object->GetHookNodeList()[i].bdelayed  &&
            g_impl_object->GetHookNodeList()[i].function &&
            g_impl_object->GetHookNodeList()[i].function->GetClassProto() &&
            g_impl_object->GetHookNodeList()[i].function->GetClassProto()->GetClassType() == CPrototype::class_wmi)
        {
            g_impl_object->RemoveApi(g_impl_object->GetHookNodeList()[i]);
        }
    }
    {
        std::lock_guard<std::recursive_mutex> lock_guard(g_impl_object->GetRecursiveMutex());
        node->hook_implement_object->ClearWmiInterfaceDefine();
    }
    return processing_continue;
}

processing_status WINAPI CHookImplementObject::detour_coInitializeSecurity(detour_node* node, PSECURITY_DESCRIPTOR pSecDesc, LONG cAuthSvc, SOLE_AUTHENTICATION_SERVICE *asAuthSvc, 
    void *pReserved1, DWORD dwAuthnLevel, DWORD dwImpLevel, void *pAuthList, DWORD dwCapabilities, void *pReserved3)
{
    PRE_BEGIN(node)
    std::shared_ptr<CObObject> return_ptr = node->function->GetIdentifier(SI_RETURN);
    if (!return_ptr) return processing_continue;
    std::any anyvalue = return_ptr->GetValue(static_cast<char*>(node->return_va));
    if (!anyvalue.has_value() || anyvalue.type() != typeid(HRESULT))
        return processing_continue;
    if (HRESULT hr = std::any_cast<HRESULT>(anyvalue); hr == RPC_E_TOO_LATE) {
        anyvalue = S_OK;
        return_ptr->SetValue(static_cast<char*>(node->return_va), anyvalue);
    }
    return processing_continue;
}

bool process_loadLibrary(CHookImplementObject::detour_node* node, std::string& lib_name, std::shared_ptr<CLogHandle>& log_handle)
{
    if (!lib_name.length()) return false;
    if (!node || !node->log_entry) return processing_skip;
    BEGIN_LOG(lib_name);
    std::string lower_name = StringToLower(lib_name);
    if (!lower_name.length()) return false;
    if (g_impl_object)
    {
        //processing delay hooks.
        std::vector<int> ord_list;
        do {
            std::lock_guard<std::recursive_mutex> lock_guard(g_impl_object->GetRecursiveMutex());
            auto& delay_list = g_impl_object->GetDelayNodeList();
            if (delay_list.find(lower_name) == delay_list.end()) break;
            ord_list = delay_list[lower_name];
            delay_list.erase(delay_list.find(lower_name));
        } while (0);

        for (auto i : ord_list)
        {
            if (g_impl_object->GetHookNodeList()[i].bdelayed && g_impl_object->GetHookNodeList()[i].function)
            {
                if (_stricmp((g_impl_object->GetHookNodeList()[i].function)->GetLibrary().c_str(), lib_name.c_str()) == 0)
                {
                    g_impl_object->HookApi(g_impl_object->GetHookNodeList()[i]);
                }
            }
        }
    }
    END_LOG(log_handle->GetHandle());
    return true;
}

void CALLBACK CHookImplementObject::detour_ldrDllNotification(ULONG reason, const LDR_DLL_NOTIFICATION_DATA *notification, void *param)
{
    if (!m_disable_hook && notification != NULL) {
        if (reason == LDR_DLL_NOTIFICATION_REASON_LOADED) {
            if (notification->Loaded.BaseDllName != nullptr &&
                notification->Loaded.BaseDllName->Buffer != nullptr &&
                notification->Loaded.BaseDllName->Length > 0)
            {
                std::unique_ptr<CLogHandle> log_handle = std::make_unique<CLogHandle>(DEBUG_FEATURE, CLogObject::logtype::log_debug);
                if (!log_handle) return;
                BEGIN_LOG("delay_hooking");
                std::wstring lib_name = { notification->Loaded.BaseDllName->Buffer, notification->Loaded.BaseDllName->Length/sizeof(wchar_t)};
                lib_name = lib_name.c_str();
                if (g_impl_object) {
                    g_impl_object->InitializeVbeModuleInfo(W2AString(lib_name), notification->Loaded.DllBase);
                }
                detour_node node = { nullptr, nullptr, nullptr, 0, nullptr, nullptr, log_handle->GetHandle() };
                debug_log("load module: {}", lib_name);
                process_loadLibrary(&node, W2AString(lib_name), LOGGER);
                END_LOG(log_handle->GetHandle());
            }
        }
        else if (reason == LDR_DLL_NOTIFICATION_REASON_UNLOADED) {
            if (notification->Unloaded.BaseDllName != nullptr &&
                notification->Unloaded.BaseDllName->Buffer != nullptr &&
                notification->Unloaded.BaseDllName->Length > 0)
            {
                std::wstring lib_name = { notification->Unloaded.BaseDllName->Buffer, notification->Unloaded.BaseDllName->Length };
                debug_log("unload module: {}", lib_name);
            }
        }
    }
    return;
}

processing_status WINAPI CHookImplementObject::detour_ntQueryInformationProcess(CHookImplementObject::detour_node* node, HANDLE ProcessHandle, DWORD ProcessInformationClass, LPVOID ProcessInformation, ULONG ProcessInformationLength, ULONG* ReturnLength)
{
    PRE_BEGIN(node)
    return processing_continue;
}

processing_status WINAPI CHookImplementObject::detour_getProcAddress(CHookImplementObject::detour_node* node, HMODULE hModule, LPCSTR  lpProcName)
{
    POST_BEGIN(node)
    //if (hModule == nullptr) return processing_skip;
    if (((DWORD_PTR)lpProcName) <= 0xffff) {
        BEGIN_LOG("get_ordinal");
        std::stringstream smod, ss;
        smod << std::hex << "0x" << hModule;
        ss << (DWORD_PTR)lpProcName;
        LOGGING("hModule", smod.str());
        LOGGING("Ordinal", ss.str());
        END_LOG(node->log_entry);
    }
    else {
        BEGIN_LOG("get_procname");
        std::stringstream smod, ss;
        smod << std::hex << "0x" << hModule;
        ss << lpProcName;
        LOGGING("hModule", smod.str());
        LOGGING("procName", ss.str());
        END_LOG(node->log_entry);
    }
    return processing_continue;
}

void process_com_communication(CHookImplementObject::detour_node* node, REFCLSID rclsid, DWORD dwClsContext)
{
    WCHAR *clsid_str, *progid_str;
    std::string clisid_string, progid_string;
    std::string local_server32, inproc_server32;
    HRESULT hr = StringFromCLSID(rclsid, &clsid_str);
    if (FAILED(hr)) return;
    ProgIDFromCLSID(rclsid, &progid_str);
    clisid_string = to_byte_string(clsid_str);
    local_server32 = CommonFuncsObject::GetLocalServer32FromClsid(clisid_string);
    if (!local_server32.length()) return;
    inproc_server32 = CommonFuncsObject::GetInprocServer32FromClsid(clisid_string);
    if (!inproc_server32.length())
        inproc_server32 = to_byte_string(progid_str);

    BEGIN_LOG("com_communication");
    LOGGING("Clsid", clisid_string);
    LOGGING("LocalServer32", local_server32);
    LOGGING("CLSCTX", std::to_string(dwClsContext));
    // add service path to driver
    DWORD BytesReturned = 0;
    std::string service_path = CommonFuncsObject::GetComServicePath(local_server32);
    if (node->hook_implement_object->GetDriverMgr()->IoControl(IOCTL_HIPS_SETTARGETCMD, &service_path[0], sizeof(DWORD), NULL, 0, &BytesReturned))
    {
        debug_log("add service path({}) to driver success!", local_server32);
    }
    else
        debug_log("add service path({}) to driver failed!", local_server32);
    if (inproc_server32.length())
        LOGGING("InprocServer32", inproc_server32);
    END_LOG(node->log_entry);
    if (clsid_str)
        CoTaskMemFree(clsid_str);
    if (progid_str)
        CoTaskMemFree(progid_str);
    return;
}

processing_status WINAPI CHookImplementObject::detour_coCreateInstance(detour_node* node, REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID *ppv)
{
    PRE_BEGIN(node)
    if (pUnkOuter != nullptr || dwClsContext != CLSCTX_LOCAL_SERVER) return processing_continue;
    if (CommonFuncsObject::IsKnownWMIClsid(rclsid)) return processing_continue;
    
    process_com_communication(node, rclsid, dwClsContext);
    return processing_continue;
}

processing_status WINAPI CHookImplementObject::detour_coCreateInstance_post(detour_node* node, REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID *ppv)
{
    POST_BEGIN(node)
    if (!ppv) return processing_continue;
    if( !node->hook_implement_object->GetCategoryPtr() ||
        node->hook_implement_object->GetCategoryPtr()->GetCurrentCategory() != CategoryObject::category_ps)
        return processing_continue;
    if (rclsid == CLSID_WbemLocator &&
        dwClsContext == CLSCTX_INPROC_SERVER &&
        riid == IID_IWbemLocator) {
        //processing delay hooks.
        std::vector<int> ord_list;
        do {
            std::lock_guard<std::recursive_mutex> lock_guard(g_impl_object->GetRecursiveMutex());
            auto& delay_list = g_impl_object->GetDelayNodeList();
            if (delay_list.find("WMI") == delay_list.end()) break;
            ord_list = delay_list["WMI"];
        } while (0);

        for (auto i : ord_list)
        {
            if (g_impl_object->GetHookNodeList()[i].bdelayed &&
                g_impl_object->GetHookNodeList()[i].function &&
                g_impl_object->GetHookNodeList()[i].function->GetClassProto() &&
                g_impl_object->GetHookNodeList()[i].function->GetClassProto()->GetClassType() == CPrototype::class_wmi)
            {
                g_impl_object->HookApi(g_impl_object->GetHookNodeList()[i]);
            }
        }
    }

    return processing_continue;
}

processing_status WINAPI CHookImplementObject::detour_coCreateInstanceEx(detour_node* node, REFCLSID Clsid, IUnknown *punkOuter, DWORD dwClsCtx, COSERVERINFO *pServerInfo, DWORD dwCount, MULTI_QI *pResults)
{
    PRE_BEGIN(node)
    if(punkOuter != nullptr || dwClsCtx != CLSCTX_LOCAL_SERVER || pServerInfo != nullptr) return processing_continue;
    if (CommonFuncsObject::IsKnownWMIClsid(Clsid)) return processing_continue;
    
    process_com_communication(node, Clsid, dwClsCtx);
    return processing_continue;
}

bool process_log_for_wmiobject(CHookImplementObject::detour_node* node, const std::shared_ptr<CWmiObject>& wmi_object, IWbemClassObject* pwbem_object, std::shared_ptr<CLogHandle>& log_handle)
{
    HRESULT hr;
    VARIANT variant_value;
    BEGIN_LOG("wmi_log");

    for (const auto& pcheck : wmi_object->GetChecks(false))
    {
        ASSERT(pcheck != nullptr);
        if (pcheck == nullptr)
            continue;
        for (const auto& log : pcheck->GetLogs())
        {
            hr = pwbem_object->Get(CComBSTR(log.first.c_str()), 0, &variant_value, 0, 0);
            if (SUCCEEDED(hr)) {
                if (std::stringstream ss = OutputAnyValue(GetVariantValue(variant_value)); ss.str().length())
                    LOGGING(log.first, ss.str());
            }
        }
    }
    END_LOG(log_handle->GetHandle());
    return true;
}

bool process_duplicate_for_wmiobject(CHookImplementObject::detour_node* node, const std::shared_ptr<CWmiObject>& wmi_object, IWbemClassObject **apObjects, ULONG *puReturned, std::shared_ptr<CLogHandle>& log_handle)
{
    bool duplicate = false;
    ASSERT(apObjects);
    ASSERT(puReturned);
    if (apObjects == nullptr) return false;
    if (puReturned == nullptr) return false;
    BEGIN_LOG("duplicate");
    for (const auto& pcheck : wmi_object->GetChecks(false))
    {
        ASSERT(pcheck != nullptr);
        if (pcheck == nullptr)
            continue;
        for (const auto& handle : pcheck->GetHandles())
        {
            if (handle.second == CCheck::handle_duplicate)
            {
                duplicate = true;
                break;
            }
        }
        if (duplicate)
        {
            idIWbemClassObject* p_IWbemClassObject = (idIWbemClassObject *)malloc(sizeof(idIWbemClassObject));
            if (p_IWbemClassObject)
            {
                p_IWbemClassObject->lpVtbl = (idIWbemClassObjectVtbl *)malloc(sizeof(idIWbemClassObjectVtbl));
                if (p_IWbemClassObject->lpVtbl)
                {
                    memset(p_IWbemClassObject->lpVtbl, 0, sizeof(idIWbemClassObjectVtbl));
                    InitializeWin32WbemClassObject(p_IWbemClassObject, wmi_object);
                    *apObjects = (IWbemClassObject*)p_IWbemClassObject;
                    *puReturned = 1;
                    //std::shared_ptr<CObObject> return_ptr = node->function->GetIdentifier(SI_RETURN);
                    //ASSERT(return_ptr != nullptr);
                    //if (return_ptr)
                    //{
                    //    std::any anyvalue = return_ptr->GetCurValue();
                    //    if(AssignAnyType(anyvalue, 0))
                    //        return_ptr->SetValue((char*)node->return_va, anyvalue);
                    //}
                }
            }
        }
    }
    END_LOG(log_handle->GetHandle());
    return true;
}

processing_status STDMETHODCALLTYPE CHookImplementObject::detour_IEnumWbemClassObject_Next(detour_node* node, IEnumWbemClassObject * This, long lTimeout, ULONG uCount, IWbemClassObject **apObjects, ULONG *puReturned)
{
    POST_BEGIN(node)
    if (!apObjects || !(*apObjects)) return processing_continue;

    HRESULT hr;
    VARIANT variant_value;
    std::stringstream class_name;
    IWbemClassObject* pWbemClassObject = *apObjects;
    hr = pWbemClassObject->Get(CComBSTR("__CLASS"), 0, &variant_value, 0, 0);
    if (!SUCCEEDED(hr)) return processing_continue;
    if (class_name = OutputAnyValue(GetVariantValue(variant_value)); !class_name.str().length())
        return processing_continue;
    BEGIN_LOG(class_name.str());
    for (const auto& wmi_object : node->hook_implement_object->GetWmiObjects())
    {
        ASSERT(wmi_object.second != nullptr);
        if (wmi_object.second == nullptr) continue;
        if (_stricmp(wmi_object.first.c_str(), class_name.str().c_str()) == 0)
        {
            if (!process_log_for_wmiobject(node, wmi_object.second, pWbemClassObject, LOGGER))
                return processing_skip;
            break;
        }
    }
    END_LOG(node->log_entry);
    return processing_continue;
}

bool process_check_for_wmiobject(CHookImplementObject::detour_node* node, const std::shared_ptr<CWmiObject>& wmi_object, const std::string& val_name, VARIANT& vt_value, std::shared_ptr<CLogHandle>& log_handle)
{
    BEGIN_LOG("wmi_modify");
    auto func_checking = [&](CHookImplementObject::detour_node* pnode, const std::shared_ptr<CCheck>& pcheck) ->bool {
        bool bcheck = true;
        bool bSetValue = false;
        CExpParsing::_ValueMap value_map;
        for (const auto& check : pcheck->GetChecks())
        {
            ASSERT(pcheck != nullptr);
            if (!check) continue;
            for (const auto& iden : check->GetIdenifierSymbol())
            {
                if (_stricmp(pcheck->GetRealName(iden.first).c_str(), val_name.c_str()) == 0)
                {
                    if (iden.second == nullptr) return false;
                    std::any cur_anyvalue = iden.second->GetCurValue();
                    std::any anyvalue = GetVariantValue(vt_value);
                    if (!anyvalue.has_value()) 
                        return false;
                    if (AssignAnyType(cur_anyvalue, anyvalue) && !check->SetIdentifierValue(value_map, CExpParsing::ValuePair(iden.first, cur_anyvalue)))
                    {
                        return false;
                    }
                    else
                        bSetValue = true;
                    break;
                }
            }
            if (auto variant_value = check->EvalExpression(value_map); variant_value.index() == 0 && !std::get<ULONGLONG>(variant_value))
            {
                bcheck = false;
                break;
            }
            break;
        }
        return bcheck;
    };

    auto func_modifying = [&](CHookImplementObject::detour_node* pnode, const std::shared_ptr<CCheck>& pcheck) ->bool {
        bool bmodify = true;
        for (const auto& modify : pcheck->GetModifys())
        {
            ASSERT(modify != nullptr);
            if (!modify) continue;
            for (const auto& iden : modify->GetIdenifierSymbol())
            {
                if (_stricmp(pcheck->GetRealName(iden.first).c_str(), val_name.c_str()) == 0)
                {
                    bool binvalid_value = false;
                    std::any anyvalue = GetVariantValue(vt_value);
                    if (!anyvalue.has_value()) {
                        binvalid_value = true;
                        anyvalue = iden.second->GetCurValue();
                    }
                    if (!anyvalue.has_value())
                        return false;
                    AssignAnyType(anyvalue, 0);
                    CExpParsing::_ValueMap value_map;
                    if (!modify->SetIdentifierValue(value_map, CExpParsing::ValuePair(iden.first, anyvalue)))
                    {
                        return false;
                    }
                    // back propagation
                    auto variant_value = modify->EvalExpression(value_map);
                    if (variant_value.index() != 1) break;
                    std::unique_ptr<CExpParsing::ValuePair> value = std::move(std::get<std::unique_ptr<CExpParsing::ValuePair>>(variant_value));
                    if (value == nullptr)
                        break;
                    if (_stricmp(iden.first.c_str(), value->first.c_str()) != 0)
                    {
                        ASSERT(0); continue;
                    }
                    std::any any_new_val = anyvalue;
                    if (AssignAnyType(any_new_val, value->second))
                    {
                        if (binvalid_value) {
                            vt_value.vt = ConvertObTypeToVarintType(iden.second->GetName());
                        }
                        if (!SetVariantValue(vt_value, any_new_val))
                            return false;
                        if(std::stringstream ss = OutputAnyValue(any_new_val); ss.str().length())
                            LOGGING(pcheck->GetRealName(iden.first), ss.str());
                        break;
                    }
                    else
                        return false;
                }
            }
            break;
        }
        return bmodify;
    };

    for (const auto& pcheck : wmi_object->GetChecks(false))
    {
        ASSERT(pcheck != nullptr);
        if (pcheck == nullptr)
            continue;

        if (func_checking(node, pcheck))
        {
            if (!func_modifying(node, pcheck))
                error_log("modifying error!");
        }
    }
    END_LOG(log_handle->GetHandle());
    return true;
}

processing_status STDMETHODCALLTYPE CHookImplementObject::detour_IWbemClassObject_Get(detour_node* node, IWbemClassObject* This, LPCWSTR wszName, long lFlags, VARIANT *pVal, long *pType, long *plFlavor)
{
    PRE_BEGIN(node)
    HRESULT hr;
    VARIANT variant_value;
    std::stringstream class_name;
    error_log("detour_IWbemClassObject_Get: 11111111111111");
    hr = This->Get(CComBSTR("__CLASS"), 0, &variant_value, 0, 0);
    if (!SUCCEEDED(hr)) return processing_continue;
    if (class_name = OutputAnyValue(GetVariantValue(variant_value)); !class_name.str().length())
        return processing_continue;
    if (pVal == nullptr) return processing_continue;
    BEGIN_LOG(class_name.str());
    if (std::stringstream str_val = OutputAnyValue(GetVariantValue(*pVal)); str_val.str().length())
        LOGGING(W2AString(wszName), str_val.str());

    for (const auto& wmi_object : node->hook_implement_object->GetWmiObjects())
    {
        ASSERT(wmi_object.second != nullptr);
        if (wmi_object.second == nullptr) continue;
        if (_stricmp(wmi_object.first.c_str(), class_name.str().c_str()) == 0)
        {
            if (!process_check_for_wmiobject(node, wmi_object.second, W2AString(wszName), *pVal, LOGGER))
                return processing_skip;
            break;
        }
    }
    END_LOG(node->log_entry);
    return processing_continue;
}

processing_status STDMETHODCALLTYPE CHookImplementObject::detour_IWbemClassObject_Put(detour_node* node, IWbemClassObject* This, LPCWSTR wszName, long lFlags, VARIANT *pVal, long Type)
{
    POST_BEGIN(node)
    HRESULT hr;
    VARIANT variant_value;
    std::stringstream class_name;
    hr = This->Get(CComBSTR("__CLASS"), 0, &variant_value, 0, 0);
    if (!SUCCEEDED(hr)) return processing_continue;
    if (class_name = OutputAnyValue(GetVariantValue(variant_value)); !class_name.str().length())
        return processing_continue;
    if (pVal == nullptr) return processing_continue;
    BEGIN_LOG(class_name.str());
    if (std::stringstream str_val = OutputAnyValue(GetVariantValue(*pVal)); str_val.str().length())
        LOGGING(W2AString(wszName), str_val.str());

    END_LOG(node->log_entry);
    return processing_continue;
}

processing_status STDMETHODCALLTYPE CHookImplementObject::detour_IWbemClassObject_Next(detour_node* node, IWbemClassObject* This, long lFlags, BSTR *strName, VARIANT *pVal, long *pType, long *plFlavor)
{
    POST_BEGIN(node)
    return processing_continue;
}

bool process_methods_for_wmiobject(CHookImplementObject::detour_node* node, const std::shared_ptr<CWmiObject>& wmi_object, const std::string& method_name, IWbemClassObject *pInParams, IWbemClassObject *pOutParams, std::shared_ptr<CLogHandle>& log_handle)
{
    HRESULT hr;
    VARIANT variant_value;
    std::string value_string;
    std::vector<std::shared_ptr<char[]>> args_cache;

    BEGIN_LOG(method_name);
    auto forward_propagation_args_wmi = [&](std::shared_ptr<char[]>& pparams, const std::shared_ptr<CFunction>& function) {
        auto get_argment_value_wmi = [&](const std::string& arg_name) ->std::any {
            hr = pInParams->Get(CComBSTR(arg_name.c_str()), 0, &variant_value, 0, 0);
            if (!SUCCEEDED(hr))
            {
                if (hr == WBEM_E_NOT_FOUND)
                    hr = pOutParams->Get(CComBSTR(arg_name.c_str()), 0, &variant_value, 0, 0);
            }
            if (SUCCEEDED(hr))
                return GetVariantValue(variant_value);
            return {};
        };

        ASSERT(pparams != nullptr);
        ASSERT(function != nullptr);
        if (!pparams || !function) return false;
        size_t arg_total_sizes = function->GetArgumentSize();
        for (const auto& arg : function->GetArguments())
        {
            ASSERT(arg.second.second);
            if (!arg.second.second) return false;
            size_t arg_size = arg.second.second->GetObSize();
            ASSERT(arg_size != 0);
            arg_total_sizes -= arg_size;
            ASSERT(arg_total_sizes >= 0 && arg_total_sizes <= 0x100);
            int ordinal = function->GetArgumentOrdinal(arg.first);
            ASSERT(ordinal != -1);
            if (ordinal == -1) return false;
            std::any anyvalue = get_argment_value_wmi(arg.first);
            if (anyvalue.has_value())
            {
                if (anyvalue.type() == typeid(std::string))
                {
                    size_t size = (std::any_cast<std::string>(anyvalue).length() + 1) * arg.second.second->GetMetadataDef()->GetObSize();
                    std::shared_ptr<char[]> p(new char[size]());
                    if (!p) return false;
                    for (int i = 0; i < (size - arg.second.second->GetMetadataDef()->GetObSize()); i++) p[i] = '#';
                    args_cache.push_back(p);
                    *(char**)(pparams.get() + (CPrototype::stack_aligned_bytes * ordinal)) = p.get();
                }
                else {
                    std::any anytmp = arg.second.second->GetCurValue();
                    if (AssignAnyType(anytmp, anyvalue))
                        anyvalue = anytmp;
                }
                if (!arg.second.second->SetValue(&pparams[CPrototype::stack_aligned_bytes * ordinal], anyvalue))
                {
                    ASSERT(0);
                    return false;
                }
            }
        }
        return true;
    };
    for (const auto& function : *wmi_object->GetMethods())
    {
        ASSERT(function.second != nullptr);
        if (!function.second) continue;
        if (_stricmp(method_name.c_str(), function.first.c_str()) == 0)
        {
            std::shared_ptr<char[]> pparams(new char[function.second->GetArgumentAlignSize()]());
            if (!pparams) continue;
            if (!forward_propagation_args_wmi(pparams, function.second)) continue;
            std::shared_ptr<HRESULT> return_va = std::make_shared<HRESULT>(S_FALSE);
            hr = pOutParams->Get(CComBSTR("ReturnValue"), 0, &variant_value, 0, 0);
            if (SUCCEEDED(hr)) *return_va = variant_value.uintVal;
            CHookImplementObject::detour_node method_node = { return_va.get(), nullptr, pparams.get(), (size_t)function.second->GetArgumentAlignSize(), node->function, node->hook_implement_object, LOGGER->GetHandle() };
            if (function.second->PostprocessingLog(&method_node) == processing_skip)
                continue;
            if (function.second->PostprocessingChecks(&method_node) == processing_skip)
                continue;
            break;
        }
    }

    END_LOG(log_handle->GetHandle());
    return true;
}

processing_status STDMETHODCALLTYPE CHookImplementObject::detour_IWbemServices_ExecMethod(detour_node* node, IWbemServices* This, const BSTR strObjectPath, const BSTR strMethodName, long lFlags, IWbemContext *pCtx, IWbemClassObject *pInParams, IWbemClassObject **ppOutParams, IWbemCallResult **ppCallResult)
{
    POST_BEGIN(node)
    std::shared_ptr<PVOID> log_handle = node->log_entry;

    if (strObjectPath == nullptr) return processing_continue;
    if (pInParams == nullptr) return processing_continue;
    if (!ppOutParams || !(*ppOutParams)) return processing_continue;
    BEGIN_LOG("Methods");

    for (const auto& wmi_object : node->hook_implement_object->GetWmiObjects())
    {
        ASSERT(wmi_object.second != nullptr);
        if (wmi_object.second == nullptr) continue;
        if (_stricmp(wmi_object.first.c_str(), W2AString(strObjectPath).c_str()) == 0)
        {
            if (!process_methods_for_wmiobject(node, wmi_object.second, W2AString(strMethodName), pInParams, *ppOutParams, LOGGER))
                return processing_skip;
            break;
        }
    }
    END_LOG(node->log_entry);
    return processing_continue;
}

processing_status STDMETHODCALLTYPE CHookImplementObject::detour_IWbemServices_ExecQuery(detour_node* node, IWbemServices* This, const BSTR strQueryLanguage, const BSTR strQuery, long lFlags, IWbemContext *pCtx, IEnumWbemClassObject **ppEnum)
{
    PRE_BEGIN(node)
    std::shared_ptr<PVOID> log_handle = node->log_entry;

    if (strQueryLanguage == nullptr) return processing_continue;
    if (strQuery == nullptr) return processing_continue;
    BEGIN_LOG("ExecQuery");
    if (std::stringstream str_val = OutputAnyValue(std::wstring(strQueryLanguage)); str_val.str().length())
        LOGGING("strQueryLanguage", str_val.str());
    if (std::stringstream str_val = OutputAnyValue(std::wstring(strQuery)); str_val.str().length())
        LOGGING("strQuery", str_val.str());
    if (std::stringstream str_val = OutputAnyValue(lFlags); str_val.str().length())
        LOGGING("lFlags", str_val.str());
    END_LOG(node->log_entry);
    return processing_continue;
}

processing_status STDMETHODCALLTYPE CHookImplementObject::detour_IWbemServices_Post_ExecQuery(detour_node* node, IWbemServices* This, const BSTR strQueryLanguage, const BSTR strQuery, long lFlags, IWbemContext *pCtx, IEnumWbemClassObject **ppEnum)
{
    POST_BEGIN(node)
    if (ppEnum == nullptr || *ppEnum == nullptr) return processing_skip;
    std::shared_ptr<PVOID> log_handle = node->log_entry;
    error_log("detour_IWbemServices_Post_ExecQuery: 11111111111111");
    if (std::stringstream str_val = OutputAnyValue(std::wstring(strQuery)); str_val.str().length()) {
        if (std::string object_str; RE2::FullMatch(str_val.str().c_str(), "^.*(?i)from (.*)$", &object_str) && object_str.length()) {
            for (const auto& wmi_object : node->hook_implement_object->GetWmiObjects())
            {
                ASSERT(wmi_object.second != nullptr);
                if (wmi_object.second == nullptr) continue;
                if (_stricmp(wmi_object.first.c_str(), object_str.c_str()) == 0)
                {
                    error_log("detour_IWbemServices_Post_ExecQuery: 222222222222222");
                    for (const auto& pcheck : wmi_object.second->GetChecks(false))
                    {
                        ASSERT(pcheck != nullptr);
                        if (pcheck == nullptr)
                            continue;
                        bool duplicate = false;
                        for (const auto& handle : pcheck->GetHandles())
                        {
                            if (handle.second == CCheck::handle_duplicate)
                            {
                                duplicate = true;
                                break;
                            }
                        }
                        if (duplicate)
                        {
                            error_log("detour_IWbemServices_Post_ExecQuery: 3333333333333333");
                            idIEnumWbemClassObject* p_IEnumWbemClassObject = (idIEnumWbemClassObject *)malloc(sizeof(idIEnumWbemClassObject));
                            if (p_IEnumWbemClassObject)
                            {
                                p_IEnumWbemClassObject->lpVtbl = (idIEnumWbemClassObjectVtbl *)malloc(sizeof(idIEnumWbemClassObjectVtbl));
                                if (p_IEnumWbemClassObject->lpVtbl)
                                {
                                    memset(p_IEnumWbemClassObject->lpVtbl, 0, sizeof(idIEnumWbemClassObjectVtbl));
                                    InitializeWin32EnumWbemClassObject(p_IEnumWbemClassObject, *ppEnum, wmi_object.second);
                                    *ppEnum = (IEnumWbemClassObject*)p_IEnumWbemClassObject;
                                    error_log("detour_IWbemServices_Post_ExecQuery: 4444444444444444444");
                                }
                            }
                            break;
                        }
                    }
                    break;
                }
            }
        }
    }
    return processing_continue;
}

// for sample 4cd6ac6a04eb5234757e84ebf401caf7
processing_status WINAPI CHookImplementObject::detour_samplePatch1(detour_node* node, ULONG param1, ULONG param2)
{
    PRE_BEGIN(node)
        processing_status status = processing_continue;
    if (param1 == 0xE6E607E4 && param2 == 0x1C) {
        do {
            PVOID address = (PVOID)(*((ULONG*)0x00431FB0) + param2 * 4);
            if (address) {
                memset(address, 0, 4);
            }
            node->hook_implement_object->DisableAllApis();
        } while (0);
    }
    return status;
}
