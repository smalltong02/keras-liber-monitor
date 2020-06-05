#include "stdafx.h"
#include "hookimpl.h"
#include "HookImplementObject.h"
#include "utils.h"
#include "Win32WbemClassObject.h"

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

bool process_loadLibrary(CHookImplementObject::detour_node* node, std::string& lib_name, std::shared_ptr<CLogHandle>& log_handle)
{
    if (!lib_name.length()) return false;
    if (!node || !node->log_entry) return processing_skip;
    BEGIN_LOG(lib_name);
    if (g_impl_object)
    {
        //processing delay hooks.
        std::vector<int> ord_list;
        do {
            auto& delay_list = g_impl_object->GetDelayNodeList();
            auto x_safe_delay_list = xlock_safe_ptr(delay_list);
            if (x_safe_delay_list->find(lib_name) == x_safe_delay_list->end()) break;
            ord_list = (*x_safe_delay_list)[lib_name];
            delay_list->erase(x_safe_delay_list->find(lib_name));
        } while (0);

        for (auto i : ord_list)
        {
            if (g_impl_object->GetHookNodeList()[i].bdelayed && g_impl_object->GetHookNodeList()[i].function)
            {
                if (_stricmp((g_impl_object->GetHookNodeList()[i].function)->GetLibrary().c_str(), lib_name.c_str()) == 0)
                {
                    MH_STATUS status;
                    LPVOID ppTarget = nullptr;
                    status = MH_CreateHookApiEx(A2WString((g_impl_object->GetHookNodeList()[i].function)->GetLibrary()).c_str(), (LPCSTR)(g_impl_object->GetHookNodeList()[i].function)->GetName().c_str(), g_impl_object->GetHookImplementFunction(), &(g_impl_object->GetHookNodeList()[i].orgin_api_implfunc), (LPVOID*)&g_impl_object->GetHookNodeList()[i], &ppTarget);
                    if (status == MH_OK && ppTarget)
                    {
                        status = MH_EnableHook(ppTarget);
                        if (status == MH_OK)
                        {
                            LOGGING((g_impl_object->GetHookNodeList()[i].function)->GetName(), "success");
                            error_log("api hook delay create: {} success!", (g_impl_object->GetHookNodeList()[i].function)->GetName());
                        }
                        else
                        {
                            LOGGING((g_impl_object->GetHookNodeList()[i].function)->GetName(), "failed");
                            error_log("api hook delay create: {} failed({})!", (g_impl_object->GetHookNodeList()[i].function)->GetName(), status);
                        }
                    }
                    else
                    {
                        LOGGING((g_impl_object->GetHookNodeList()[i].function)->GetName(), "failed");
                        error_log("api hook delay create: {} failed({})!", (g_impl_object->GetHookNodeList()[i].function)->GetName(), status);
                    }
                    g_impl_object->GetHookNodeList()[i].bdelayed = false;
                }
            }
        }
    }
    END_LOG(log_handle->GetHandle());
    return true;
}

processing_status WINAPI CHookImplementObject::detour_loadLibraryA(CHookImplementObject::detour_node* node, LPCSTR lpLibFileName)
{
    BreakPoint;
    ASSERT(node != nullptr);
    ASSERT(node->return_va != nullptr);
    ASSERT(node->log_entry != nullptr);
    ASSERT(node->function != nullptr);
    if (!node || !node->return_va || !node->log_entry) return processing_skip;
    processing_status status;
    if ((status = node->function->CheckReturn(node)) != processing_continue)
        return status;
    BEGIN_LOG("delay_hooking");
    HMODULE hmod = *reinterpret_cast<HMODULE*>(node->return_va);
    if (hmod != nullptr && lpLibFileName)
    {
        std::string lib_name = lpLibFileName;
        process_loadLibrary(node, lib_name, LOGGER);
    }
    END_LOG(node->log_entry);
    return processing_continue;
}

processing_status WINAPI CHookImplementObject::detour_loadLibraryExA(CHookImplementObject::detour_node* node, LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags)
{
    BreakPoint;
    ASSERT(node != nullptr);
    ASSERT(node->return_va != nullptr);
    ASSERT(node->log_entry != nullptr);
    ASSERT(node->function != nullptr);
    if (!node || !node->return_va || !node->log_entry) return processing_skip;
    processing_status status;
    if ((status = node->function->CheckReturn(node)) != processing_continue)
        return status;
    BEGIN_LOG("delay_hooking");
    HMODULE hmod = *reinterpret_cast<HMODULE*>(node->return_va);
    if (hmod != nullptr && lpLibFileName)
    {
        std::string lib_name = lpLibFileName;
        process_loadLibrary(node, lib_name, LOGGER);
    }
    END_LOG(node->log_entry);
    return processing_continue;
}

processing_status WINAPI CHookImplementObject::detour_loadLibraryW(CHookImplementObject::detour_node* node, LPCWSTR lpLibFileName)
{
    BreakPoint;
    ASSERT(node != nullptr);
    ASSERT(node->return_va != nullptr);
    ASSERT(node->log_entry != nullptr);
    ASSERT(node->function != nullptr);
    if (!node || !node->return_va || !node->log_entry) return processing_skip;
    processing_status status;
    if ((status = node->function->CheckReturn(node)) != processing_continue)
        return status;
    BEGIN_LOG("delay_hooking");
    HMODULE hmod = *reinterpret_cast<HMODULE*>(node->return_va);
    if (hmod != nullptr && lpLibFileName)
    {
        std::string lib_name = W2AString(lpLibFileName);
        process_loadLibrary(node, lib_name, LOGGER);
    }
    END_LOG(node->log_entry);
    return processing_continue;
}

processing_status WINAPI CHookImplementObject::detour_loadLibraryExW(CHookImplementObject::detour_node* node, LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags)
{
    BreakPoint;
    ASSERT(node != nullptr);
    ASSERT(node->return_va != nullptr);
    ASSERT(node->log_entry != nullptr);
    ASSERT(node->function != nullptr);
    if (!node || !node->return_va || !node->log_entry) return processing_skip;
    processing_status status;
    if ((status = node->function->CheckReturn(node)) != processing_continue)
        return status;
    BEGIN_LOG("delay_hooking");
    HMODULE hmod = *reinterpret_cast<HMODULE*>(node->return_va);
    if (hmod != nullptr && lpLibFileName)
    {
        std::string lib_name = W2AString(lpLibFileName);
        process_loadLibrary(node, lib_name, LOGGER);
    }
    END_LOG(node->log_entry);
    return processing_continue;
}

processing_status WINAPI CHookImplementObject::detour_ntQueryInformationProcess(CHookImplementObject::detour_node* node, HANDLE ProcessHandle, DWORD ProcessInformationClass, LPVOID ProcessInformation, ULONG ProcessInformationLength, ULONG* ReturnLength)
{
    BreakPoint;
    ASSERT(node != nullptr);
    ASSERT(node->return_va != nullptr);
    ASSERT(node->log_entry != nullptr);
    if (!node || !node->return_va || !node->log_entry) return processing_skip;
    return processing_continue;
}

bool process_log_for_wmiobject(CHookImplementObject::detour_node* node, const std::shared_ptr<CWmiObject>& wmi_object, IWbemClassObject* pwbem_object, std::shared_ptr<CLogHandle>& log_handle)
{
    HRESULT hr;
    VARIANT variant_value;
    BEGIN_LOG("wmi_log");
    std::string value_string;

    for (const auto& pcheck : wmi_object->GetChecks(false))
    {
        ASSERT(pcheck != nullptr);
        if (pcheck == nullptr)
            continue;
        for (const auto& log : pcheck->GetLogs())
        {
            hr = pwbem_object->Get(CComBSTR(log.first.c_str()), 0, &variant_value, 0, 0);
            if (SUCCEEDED(hr) && GetValueString(variant_value, value_string))
                LOGGING(log.first, value_string);
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
                    std::shared_ptr<CObObject> return_ptr = node->function->GetVarIdentifier(SI_RETURN);
                    ASSERT(return_ptr != nullptr);
                    if (return_ptr)
                    {
                        node->function->SetValue(return_ptr, (char*)node->return_va, std::stringstream("0"));
                    }
                }
            }
        }
    }
    END_LOG(log_handle->GetHandle());
    return true;
}

processing_status STDMETHODCALLTYPE CHookImplementObject::detour_IEnumWbemClassObject_Next(detour_node* node, IEnumWbemClassObject * This, long lTimeout, ULONG uCount, IWbemClassObject **apObjects, ULONG *puReturned)
{
    BreakPoint;
    ASSERT(node != nullptr);
    ASSERT(node->return_va != nullptr);
    ASSERT(node->log_entry != nullptr);
    ASSERT(node->function != nullptr);
    ASSERT(node->hook_implement_object != nullptr);
    if (!node || !node->return_va || !node->log_entry) return processing_skip;
    if (!node->hook_implement_object) return processing_skip;
    if (!apObjects || !(*apObjects)) return processing_continue;

    HRESULT hr;
    VARIANT variant_value;
    std::string class_name;
    IWbemClassObject* pWbemClassObject = *apObjects;
    hr = pWbemClassObject->Get(CComBSTR("CreationClassName"), 0, &variant_value, 0, 0);
    if (!SUCCEEDED(hr) || !GetValueString(variant_value, class_name)) return processing_continue;
    BEGIN_LOG(class_name);
    for (const auto& wmi_object : node->hook_implement_object->GetWmiObjects())
    {
        ASSERT(wmi_object.second != nullptr);
        if (wmi_object.second == nullptr) continue;
        if (_stricmp(wmi_object.first.c_str(), class_name.c_str()) == 0)
        {
            if (node->function->CheckReturn(node) == processing_skip)
            {
                if (!process_duplicate_for_wmiobject(node, wmi_object.second, apObjects, puReturned, LOGGER))
                    return processing_skip;
            }
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
    auto func_checking = [&](CHookImplementObject::detour_node* pnode, const std::unique_ptr<CCheck>& pcheck) ->bool {
        bool bcheck = true;
        for (const auto& check : pcheck->GetChecks())
        {
            ASSERT(pcheck != nullptr);
            if (!check) continue;
            for (const auto& iden : check->GetIdenifierSymbol())
            {
                if (_stricmp(pcheck->GetRealName(iden.first).c_str(), val_name.c_str()) == 0)
                {
                    std::string str_val;
                    if (!GetValueString(vt_value, str_val)) continue;
                    std::stringstream ss; ss << str_val;
                    if (!ss.str().length())
                    {
                        return false;
                    }
                    if (!check->SetIdentifierValue(CExpParsing::ValuePair(iden.first, ss.str())))
                    {
                        return false;
                    }
                    break;
                }
            }
            if (!std::get<bool>(check->EvalExpression()))
            {
                bcheck = false;
                break;
            }
            break;
        }
        return bcheck;
    };

    auto func_modifying = [&](CHookImplementObject::detour_node* pnode, const std::unique_ptr<CCheck>& pcheck) ->bool {
        bool bmodify = true;
        for (const auto& modify : pcheck->GetModifys())
        {
            ASSERT(modify != nullptr);
            if (!modify) continue;
            for (const auto& iden : modify->GetIdenifierSymbol())
            {
                if (_stricmp(pcheck->GetRealName(iden.first).c_str(), val_name.c_str()) == 0)
                {
                    std::stringstream ss; ss << "0";
                    if (!modify->SetIdentifierValue(CExpParsing::ValuePair(iden.first, ss.str())))
                    {
                        return false;
                    }
                    // back propagation
                    std::unique_ptr<CExpParsing::ValuePair> value = std::get<std::unique_ptr<CExpParsing::ValuePair>>(modify->EvalExpression());
                    if (value == nullptr)
                        break;
                    if (_stricmp(iden.first.c_str(), value->first.c_str()) != 0)
                    {
                        ASSERT(0); continue;
                    }
                    if (!SetValueString(value->second.str().c_str(), vt_value))
                        return false;
                    LOGGING(pcheck->GetRealName(iden.first), value->second.str().c_str());
                    break;
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
    BreakPoint;
    ASSERT(node != nullptr);
    ASSERT(node->return_va != nullptr);
    ASSERT(node->log_entry != nullptr);
    ASSERT(node->function != nullptr);
    ASSERT(node->hook_implement_object != nullptr);
    if (!node || !node->return_va || !node->log_entry) return processing_skip;
    processing_status status;
    if ((status = node->function->CheckReturn(node)) != processing_continue)
        return status;
    if (!node->hook_implement_object) return processing_skip;

    HRESULT hr;
    VARIANT variant_value;
    std::string class_name;
    hr = This->Get(CComBSTR("CreationClassName"), 0, &variant_value, 0, 0);
    if (!SUCCEEDED(hr) || !GetValueString(variant_value, class_name)) return processing_continue;
    if (pVal == nullptr) return processing_continue;
    BEGIN_LOG(class_name);
    std::string str_val;
    if (GetValueString(*pVal, str_val))
        LOGGING(W2AString(wszName), str_val);

    for (const auto& wmi_object : node->hook_implement_object->GetWmiObjects())
    {
        ASSERT(wmi_object.second != nullptr);
        if (wmi_object.second == nullptr) continue;
        if (_stricmp(wmi_object.first.c_str(), class_name.c_str()) == 0)
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
    BreakPoint;
    ASSERT(node != nullptr);
    ASSERT(node->return_va != nullptr);
    ASSERT(node->log_entry != nullptr);
    ASSERT(node->function != nullptr);
    ASSERT(node->hook_implement_object != nullptr);
    if (!node || !node->return_va || !node->log_entry) return processing_skip;
    processing_status status;
    if ((status = node->function->CheckReturn(node)) != processing_continue)
        return status;
    if (!node->hook_implement_object) return processing_skip;

    HRESULT hr;
    VARIANT variant_value;
    std::string class_name;
    hr = This->Get(CComBSTR("CreationClassName"), 0, &variant_value, 0, 0);
    if (!SUCCEEDED(hr) || !GetValueString(variant_value, class_name)) return processing_continue;
    if (pVal == nullptr) return processing_continue;
    BEGIN_LOG(class_name);
    std::string str_val;
    if (GetValueString(*pVal, str_val))
        LOGGING(W2AString(wszName), str_val);

    END_LOG(node->log_entry);
    return processing_continue;
}

processing_status STDMETHODCALLTYPE CHookImplementObject::detour_IWbemClassObject_Next(detour_node* node, IWbemClassObject* This, long lFlags, BSTR *strName, VARIANT *pVal, long *pType, long *plFlavor)
{
    BreakPoint;
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
        auto get_argment_value_wmi = [&](const std::string& arg_name) ->std::stringstream {
            std::stringstream ss;
            hr = pInParams->Get(CComBSTR(arg_name.c_str()), 0, &variant_value, 0, 0);
            if (!SUCCEEDED(hr))
            {
                if (hr == WBEM_E_NOT_FOUND)
                    hr = pOutParams->Get(CComBSTR(arg_name.c_str()), 0, &variant_value, 0, 0);
            }
            if (!SUCCEEDED(hr) || !GetValueString(variant_value, value_string))
                return ss;
            ss << value_string;
            return ss;
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
            std::stringstream ss = get_argment_value_wmi(arg.first);
            if (ss.str().length())
            {
                if (arg.second.second->IsReference())
                {
                    size_t size = (ss.str().length() + 1) * arg.second.second->GetMetadataDef()->GetBaseSize();
                    std::shared_ptr<char[]> p(new char[size]());
                    for (int i = 0; i < (size - arg.second.second->GetMetadataDef()->GetBaseSize()); i++) p[i] = '#';
                    args_cache.push_back(p);
                    *(char**)(pparams.get() + (CPrototype::stack_aligned_bytes * ordinal)) = p.get();
                }
                if (!arg.second.second->SetValue(&pparams[CPrototype::stack_aligned_bytes * ordinal], ss))
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
            CHookImplementObject::detour_node method_node = { return_va.get(), pparams.get(), (size_t)function.second->GetArgumentAlignSize(), node->function, node->hook_implement_object, LOGGER->GetHandle() };
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
    BreakPoint;
    ASSERT(node != nullptr);
    ASSERT(node->return_va != nullptr);
    ASSERT(node->log_entry != nullptr);
    ASSERT(node->function != nullptr);
    ASSERT(node->hook_implement_object != nullptr);
    if (!node || !node->return_va || !node->log_entry) return processing_skip;
    processing_status status;
    if ((status = node->function->CheckReturn(node)) != processing_continue)
        return status;
    if (!node->hook_implement_object) return processing_skip;
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
    BreakPoint;
    return processing_continue;
}
