#include "stdafx.h"
#include "utils.h"
#include "HookImplementObject.h"

namespace cchips {

    const LPVOID CHookImplementObject::m_hookImplementFunction = (LPVOID)_hookImplementFunction;
    std::shared_ptr<CHookImplementObject> g_impl_object = std::make_shared<CHookImplementObject>();
    thread_local ULONG_PTR CHookImplementObject::m_threadTlsCount = 0;
    std::atomic_bool CHookImplementObject::m_disable_hook = true;
    const std::unique_ptr<CExceptionObject> CHookImplementObject::m_exceptionObject = (CExceptionObject::GetInstance());

#ifdef _X86_
#pragma code_seg(".wrtext")
    __declspec(naked) void STDMETHODCALLTYPE _hookImplementFunction(void)
    {
        ULONG* __stack;
        void* __addr;
        CHookImplementObject::REGISTERS __backup_regs;
        CHookImplementObject::hook_node* __hook_node;
        CHookImplementObject* __object;
        ULONG_PTR __return;
        int __psize;

        MACRO_PROCESSING_STACK_(__backup_regs, __stack, __hook_node, __addr);
        if (__hook_node != nullptr && __addr != nullptr && (__object = __hook_node->hook_implement_object.get()) != nullptr)
        {
            CHookImplementObject::last_error_define error;
            __hook_node->hook_implement_object->GetLastError(error);
            // safe remove hook when shared_count = 0
            InterlockedIncrement(&__hook_node->shared_count);
            processing_status __status = processing_continue;
            if (__hook_node->hook_implement_object->IsDisableHook())
                __status = processing_skip;
            __psize = 0; __return = 0;
            ULONG_PTR entry_count = -1;
            if(__status == processing_continue)
                entry_count = __object->GetTlsValueForThreadIdx();
            CLogHandle* __log = nullptr;
            if (__status == processing_continue)
                __status = __object->Preprocessing(__hook_node, __addr, __psize, __return, entry_count, &__log);
            if (__status != processing_exit)
            {
                void* __func = __hook_node->orgin_api_implfunc;
                char** __params = nullptr;
                __params = (char**)((ULONG_PTR)__addr + __psize);
                CFunction::_call_convention __call_conv = __hook_node->function->GetCallConv();

                MACRO_CALL_ORGINAL_(__params, __psize, __call_conv, __func, __return);
                __hook_node->hook_implement_object->GetLastError(error);
                if(__status == processing_continue)
                    __object->Postprocessing(__hook_node, __addr, __psize, __return, entry_count, &__log);
            }
            if(entry_count != -1)
                __object->ReleaseTlsValueForThreadIdx();
            InterlockedDecrement(&__hook_node->shared_count);
            __hook_node->hook_implement_object->SetLastError(error);
        }
        MACRO_PROCESSING_RETURN(__backup_regs, __psize, __return);
    }
#pragma code_seg()
#pragma comment(linker, "/SECTION:.wrtext,ERW")
#endif // #ifdef _X86_

    bool CHookImplementObject::InitializeErrorOffset()
    {
        FARPROC pRtlGetLastWin32Error = GetProcAddress(
            GetModuleHandleA("ntdll"), "RtlGetLastWin32Error");

        FARPROC pRtlGetLastNtStatus = GetProcAddress(
            GetModuleHandleA("ntdll"), "RtlGetLastNtStatus");

        if (!pRtlGetLastWin32Error || !pRtlGetLastNtStatus)
            return false;

        m_error_offset.m_win32_error_offset = NativeFetchMovEaxImmOffset(
            (const char *)pRtlGetLastWin32Error);
        if (m_error_offset.m_win32_error_offset == 0) {
            return false;
        }

        m_error_offset.m_nt_status_offset = NativeFetchMovEaxImmOffset(
            (const char *)pRtlGetLastNtStatus);
        if (m_error_offset.m_nt_status_offset == 0) {
            return false;
        }
        return true;
    }

    bool CHookImplementObject::Initialize(std::shared_ptr<CHipsCfgObject> configObject)
    {
        m_configObject = std::move(configObject);
        m_drivermgr = std::make_unique<CDriverMgr>();

        if (!InitializeErrorOffset())
            return m_bValid;
        m_notification_api_define.ldrregisterdllnotification_func = reinterpret_cast<LdrRegisterDllNotification_Define>(GetProcAddress(GetModuleHandleA("ntdll"), "LdrRegisterDllNotification"));
        if (!m_notification_api_define.ldrregisterdllnotification_func)
            return m_bValid;

        if (m_hookImplementFunction != nullptr &&
            m_exceptionObject != nullptr)
        {
            MH_STATUS Status = MH_Initialize();
            if (Status == MH_OK)
            {
                m_bValid = true;
            }
        }
        return m_bValid;
    }

    MH_STATUS CHookImplementObject::HookApi(hook_node& node)
    {
        ASSERT(node.hook_implement_object);
        ASSERT(node.function);

        if (node.function->GetClassType() != CPrototype::class_normal)
            return HookSpecialApi(node);
        return HookNormalApi(node);
    }

    MH_STATUS CHookImplementObject::HookNormalApi(hook_node& node)
    {
        MH_STATUS status = MH_CreateHookApiEx(A2WString((*node.function).GetLibrary()).c_str(), (LPCSTR)(*node.function).GetName().c_str(), m_hookImplementFunction, &(node.orgin_api_implfunc), (LPVOID*)&node, &node.ppTarget);
        if (status == MH_OK)
        {
            if (node.bdelayed && node.ppTarget)
            {
                status = MH_EnableHook(node.ppTarget);
                if (status == MH_OK)
                {
                    error_log("api hook delay create: {} success!", node.function->GetName());
                }
                else
                {
                    error_log("api hook delay create: {} failed({})!", node.function->GetName(), status);
                }
                node.bdelayed = false;
            }
            else
            {
                error_log("api hook create: '{}' success!", (*node.function).GetName());
            }
            return status;
        }
        else
        {
            error_log("api hook create: '{}' failed({})!", (*node.function).GetName(), status);
        }
        return status;
    }

    MH_STATUS CHookImplementObject::HookSpecialApi(hook_node& node)
    {
        MH_STATUS status = MH_UNKNOWN;
        CPrototype::_class_type type = node.function->GetClassType();
        switch (type)
        {
        case CPrototype::class_wmi:
        {
            if (m_wmi_interface_define.size() == 0)
                InitializeWmiMethodsDefine(node.bdelayed);
            if (!node.function->GetClassProto())
                break;
            auto it = m_wmi_interface_define.find(node.function->GetClassProto()->GetClassNam());
            if (it == m_wmi_interface_define.end())
            {
                if (!node.bdelayed)
                    status = MH_ERROR_MODULE_NOT_FOUND;
                break;
            }
            unsigned int index = node.function->GetClassProto()->GetVtblIdx();
            PVOID target_addr = reinterpret_cast<PVOID>(*(it->second + index));
            ASSERT(target_addr);
            if (!target_addr)
                break;
            status = MH_CreateHook(target_addr, node.hook_implement_object->GetHookImplementFunction(), &(node.orgin_api_implfunc), (LPVOID*)&node);
            if (status == MH_OK)
            {
                if (node.bdelayed)
                {
                    node.ppTarget = target_addr;
                    status = MH_EnableHook(target_addr);
                    if (status == MH_OK)
                    {
                        error_log("wmi method hook create: '{}' success!", node.function->GetName());
                    }
                    else
                    {
                        error_log("wmi method hook create: '{}' failed({})!", node.function->GetName(), status);
                    }
                    node.bdelayed = false;
                }
                else
                {
                    error_log("wmi method hook create: '{}' success!", node.function->GetName());
                }
                return status;
            }
            else
            {
                error_log("wmi method hook create: '{}' failed({})!", node.function->GetName(), status);
                return status;
            }
        }
        break;

        case CPrototype::class_vbs:
        case CPrototype::class_ps:
        case CPrototype::class_kvm:
        case CPrototype::class_cshape:
        default:
            break;
        }
        return status;
    }

    void CHookImplementObject::RemoveNormalApi(hook_node& node)
    {
        if (!node.bdelayed && node.ppTarget)
        {
            MH_STATUS status = MH_RemoveHook(node.ppTarget);
            if(status == MH_OK)
                node.bdelayed = true;
        }
        return;
    }

    void CHookImplementObject::RemoveSpecialApi(hook_node& node)
    {
        CPrototype::_class_type type = node.function->GetClassType();
        switch (type)
        {
        case CPrototype::class_wmi:
        {
            if (!node.bdelayed && node.ppTarget)
            {
                MH_STATUS status = MH_RemoveHook(node.ppTarget);
                if (status == MH_OK)
                    node.bdelayed = true;
            }
        }
        break;

        case CPrototype::class_vbs:
        case CPrototype::class_ps:
        case CPrototype::class_kvm:
        case CPrototype::class_cshape:
        default:
            break;
        }
        return;
    }

    void CHookImplementObject::RemoveApi(hook_node& node)
    {
        ASSERT(node.hook_implement_object);
        ASSERT(node.function);

        if (node.function->GetClassType() != CPrototype::class_normal)
            return RemoveSpecialApi(node);
        return RemoveNormalApi(node);
    }

    bool CHookImplementObject::HookAllApis()
    {
        ASSERT(m_bValid);
        if (!m_bValid)
            return false;
        if (!InitializeNotificationDefine())
            return false;
        int count = 0;
        m_hookNodeList.resize(GetFunctionCounts(), hook_node({}));

        std::for_each(GetFunctions().begin(), GetFunctions().end(), [&](const auto& func) {
            m_hookNodeList[count].bdelayed = false;
            m_hookNodeList[count].orgin_api_implfunc = nullptr;
            m_hookNodeList[count].hook_implement_object = shared_from_this();
            m_hookNodeList[count].function = func.second;

            if (MH_STATUS status = HookApi(m_hookNodeList[count]); status == MH_ERROR_MODULE_NOT_FOUND)
            {
                // delayed hook;
                m_hookNodeList[count].bdelayed = true;
                if(!(*m_hookNodeList[count].function).GetClassProto() || (*m_hookNodeList[count].function).GetClassProto()->GetClassType() == CPrototype::class_normal)
                    m_delayNodeList[StringToLower((*m_hookNodeList[count].function).GetLibrary())].push_back(count);
                else
                    m_delayNodeList[(*m_hookNodeList[count].function).GetClassProto()->GetTypeName()].push_back(count);
                error_log("api hook: '{}' be add to delay_list!", (*func.second).GetName());
            }
            count++;
        });

        if (HookProcessing())
            EnableAllApis();
        return true;
    }

    processing_status CHookImplementObject::Preprocessing(CHookImplementObject::hook_node* node_elem, PVOID param_addr, int& params_size, ULONG_PTR& func_return, DWORD entry_count, CLogHandle** __log)
    {
        ASSERT(node_elem->function != nullptr && node_elem->hook_implement_object != nullptr && node_elem->orgin_api_implfunc != nullptr);
        if (node_elem->function == nullptr || node_elem->hook_implement_object == nullptr || node_elem->orgin_api_implfunc == nullptr)
            return processing_skip;
        if ((*__log) != nullptr) (*__log) = nullptr;
        std::shared_ptr<CFunction> func_object = node_elem->function;
        std::shared_ptr<CHookImplementObject> hook_implement_object = node_elem->hook_implement_object;
        if (params_size == 0) params_size = (int)func_object->GetArgumentAlignSize();
        if (hook_implement_object->IsFilterThread(std::this_thread::get_id())) 
            return processing_skip;
        if (func_object->GetSpecial())
        {
            // If there are API hooks in the upper layer that have been processed, this hook will be processed again when Special is true
            // please carefully when used Special flag, because it maybe cause re-entry.
        }
        else
        {
            // API call has been processed or logged, so didn't need to more processed for avoid re-entry.
            if (entry_count)
                return processing_skip;
        }
#ifdef _X86_
        char* __params = reinterpret_cast<char*>(reinterpret_cast<ULONG_PTR>(param_addr) + CFunction::stack_aligned_bytes);
#endif
#ifdef _AMD64_
        char* __params = reinterpret_cast<char*>(param_addr);
#endif
        std::unique_ptr<CLogHandle> log_handle = std::make_unique<CLogHandle>(func_object->GetFeature(), CLogObject::logtype::log_invalid);
        ASSERT(log_handle != nullptr);
        detour_node node = { &func_return, __params, (size_t)params_size, node_elem->function, node_elem->hook_implement_object, log_handle->GetHandle() };
        detour_node* pnode = &node;
#ifdef _X86_
        char** __rev_params = (char**)((ULONG_PTR)param_addr + params_size);
#endif
#ifdef _AMD64_
        char** __rev_params = (char**)(param_addr);
#endif
        processing_status status = processing_skip;
        for (const auto& pre_func : func_object->GetPreProcessing())
        {
#ifdef _X86_
            MACRO_PUSH_PARAMS_(pnode, __rev_params, params_size, pre_func, status)
#endif
#ifdef _AMD64_
            MACRO_PUSH_PARAMS_(pnode, __rev_params, params_size, pre_func, (ULONG*)&status)
#endif
            if (status != processing_continue)
                return status;
        }

        do {
            status = node_elem->function->ProcessingEnsure(pnode);
            if (status != processing_continue) break;
            status = node_elem->function->PreprocessingLog(pnode);
            if (status != processing_continue) break;
            status = node_elem->function->PreprocessingChecks(pnode);
            if (status != processing_continue) break;
        } while (0);

        if (status == processing_exit)
        {
            if (log_handle->LogCounts())
            {
                log_handle->AddLog(LOGPAIR("API", func_object->GetName()));
                log_handle->FreeHandle(CLogObject::logtype::log_event);
            }
        }
        else
            *__log = log_handle.release();

        return status;
    }

    processing_status CHookImplementObject::Postprocessing(CHookImplementObject::hook_node* node_elem, PVOID param_addr, int& params_size, ULONG_PTR& func_return, DWORD entry_count, CLogHandle** __log)
    {
        ASSERT(node_elem->function != nullptr && node_elem->hook_implement_object != nullptr && node_elem->orgin_api_implfunc != nullptr && __log != nullptr);

        std::unique_ptr<CLogHandle> log_handle(*__log);
        std::shared_ptr<CFunction> func_object = node_elem->function;
        std::shared_ptr<CHookImplementObject> hook_implement_object = node_elem->hook_implement_object;
        if (params_size == 0) params_size = (int)func_object->GetArgumentAlignSize();
#ifdef _X86_
        char* __params = reinterpret_cast<char*>(reinterpret_cast<ULONG_PTR>(param_addr) + CFunction::stack_aligned_bytes);
#endif
#ifdef _AMD64_
        char* __params = reinterpret_cast<char*>(param_addr);
#endif
        ASSERT(log_handle != nullptr);
        detour_node node = { &func_return, __params, (size_t)params_size, node_elem->function, node_elem->hook_implement_object, log_handle->GetHandle() };
        detour_node* pnode = &node;
#ifdef _X86_
        char** __rev_params = (char**)((ULONG_PTR)param_addr + params_size);
#endif
#ifdef _AMD64_
        char** __rev_params = (char**)(param_addr);
#endif
        processing_status status = processing_skip;
        for (const auto& post_func : func_object->GetPostProcessing())
        {
#ifdef _X86_
            MACRO_PUSH_PARAMS_(pnode, __rev_params, params_size, post_func, status)
#endif
#ifdef _AMD64_
                MACRO_PUSH_PARAMS_(pnode, __rev_params, params_size, post_func, (ULONG*)&status)
#endif
            if (status != processing_continue)
                return status;
        }
        do {
            status = node_elem->function->CheckReturn(pnode);
            if (status != processing_continue) break;
            status = node_elem->function->PostprocessingLog(pnode);
            if (status != processing_continue) break;
            status = node_elem->function->PostprocessingChecks(pnode);
            if (status != processing_continue) break;
        } while (0);
        if (log_handle->LogCounts())
        {
            log_handle->AddLog(LOGPAIR("API", func_object->GetName()));
            log_handle->FreeHandle(CLogObject::logtype::log_event);
        }
        return status;
    }

    bool CHookImplementObject::HookProcessing()
    {
        //processing pre hook
        ADD_PRE_PROCESSING(NtQueryInformationProcess, detour_ntQueryInformationProcess);
        ADD_PRE_PROCESSING(CoUninitialize, detour_coUninitialize);
        // processing post hook
        ADD_POST_PROCESSING(CoInitializeEx, detour_coInitializeEx);
        ADD_POST_PROCESSING(GetProcAddress, detour_getProcAddress);
        ADD_POST_PROCESSING(CoInitializeSecurity, detour_coInitializeSecurity);
        // processing wmi hook
        ADD_PRE_PROCESSING(IWbemServices_ExecQuery, detour_IWbemServices_ExecQuery);
        ADD_POST_PROCESSING(IWbemServices_ExecQuery, detour_IWbemServices_Post_ExecQuery);
        ADD_POST_PROCESSING(IEnumWbemClassObject_Next, detour_IEnumWbemClassObject_Next);
        ADD_POST_PROCESSING(IWbemClassObject_Get, detour_IWbemClassObject_Get);
        ADD_POST_PROCESSING(IWbemClassObject_Put, detour_IWbemClassObject_Put);
        ADD_POST_PROCESSING(IWbemClassObject_Next, detour_IWbemClassObject_Next);
        ADD_POST_PROCESSING(IWbemServices_ExecMethod, detour_IWbemServices_ExecMethod);
        
        return true;
    }

    bool CHookImplementObject::UnhookProcessing()
    {
        //processing pre hook
        DEL_PRE_PROCESSING(NtQueryInformationProcess, detour_ntQueryInformationProcess);
        DEL_PRE_PROCESSING(CoUninitialize, detour_coUninitialize);
        // processing post hook
        DEL_POST_PROCESSING(CoInitializeEx, detour_coInitializeEx);
        DEL_POST_PROCESSING(CoInitializeSecurity, detour_coInitializeSecurity);
        // processing wmi hook
        DEL_PRE_PROCESSING(IWbemServices_ExecQuery, detour_IWbemServices_ExecQuery);
        DEL_POST_PROCESSING(IWbemServices_ExecQuery, detour_IWbemServices_Post_ExecQuery);
        DEL_POST_PROCESSING(IEnumWbemClassObject_Next, detour_IEnumWbemClassObject_Next);
        DEL_POST_PROCESSING(IWbemClassObject_Get, detour_IWbemClassObject_Get);
        DEL_POST_PROCESSING(IWbemClassObject_Put, detour_IWbemClassObject_Put);
        DEL_POST_PROCESSING(IWbemClassObject_Next, detour_IWbemClassObject_Next);
        DEL_POST_PROCESSING(IWbemServices_ExecMethod, detour_IWbemServices_ExecMethod);
        return true;
    }

#ifdef _AMD64_
    extern "C" ULONG_PTR _hookImplementFunction_x64(void* p_register_stack)
    {
        CHookImplementObject::REGISTERS* __backup_regs = (CHookImplementObject::REGISTERS*)p_register_stack;
        ASSERT(__backup_regs != nullptr);
        LONGLONG __stack = __backup_regs->rsp;
        ASSERT(reinterpret_cast<void*>(__stack + 0x80) != nullptr);
        void* __addr = reinterpret_cast<void*>(__stack + 0x80);
        ASSERT(reinterpret_cast<CHookImplementObject::hook_node**>(__stack + 0x70) != nullptr);
        CHookImplementObject::hook_node* __hook_node = *reinterpret_cast<CHookImplementObject::hook_node**>(__stack + 0x70);
        CHookImplementObject* __object = nullptr;
        ULONG_PTR __return = 0;
        if (__hook_node != nullptr && __addr != nullptr && (__object = __hook_node->hook_implement_object.get()) != nullptr)
        {
            int __psize = 0;
            CHookImplementObject::last_error_define error;
            __hook_node->hook_implement_object->GetLastError(error);
            // safe remove hook when shared_count = 0
            InterlockedIncrement(&__hook_node->shared_count);
            __psize = 0; __return = 0;
            ULONG_PTR entry_count = -1;
            processing_status __status = processing_continue;
            if (__hook_node->hook_implement_object->IsDisableHook())
                __status = processing_skip;
            if (__status == processing_continue)
                entry_count = __object->GetTlsValueForThreadIdx();
            CLogHandle* __log = nullptr;
            void* __new_addr = reinterpret_cast<void*>(reinterpret_cast<ULONG_PTR>(__addr) - 0x270);
            bool __bforward = __object->ForwardPropagationArgs(reinterpret_cast<ULONG_PTR*>(__new_addr), __backup_regs, __hook_node->function, __addr);
            if (__status == processing_continue)
                if (__bforward) __status = __object->Preprocessing(__hook_node, __new_addr, __psize, __return, entry_count, &__log);
            if (__status != processing_exit)
            {
                if (__psize == 0) __psize = (int)__hook_node->function->GetArgumentAlignSize();
                void* __func = __hook_node->orgin_api_implfunc;
                char** __params = (char**)(__new_addr);
                CFunction::_call_convention __call_conv = __hook_node->function->GetCallConv();
                ASSERT(__call_conv == CFunction::_call_convention::call_stdcall); // now only support stdcall, actually is fastcall on x64.
                MACRO_CALL_ORGINAL_(__params, __psize, __call_conv, __func, &__return);
                __hook_node->hook_implement_object->GetLastError(error);
                if(__status == processing_continue)
                    if (__bforward) __object->Postprocessing(__hook_node, __new_addr, __psize, __return, entry_count, &__log);
            }
            if (entry_count != -1)
                __object->ReleaseTlsValueForThreadIdx();
            InterlockedDecrement(&__hook_node->shared_count);
            __hook_node->hook_implement_object->SetLastError(error);
        }
        return __return;
    }

    bool CHookImplementObject::ForwardPropagationArgs(ULONG_PTR* new_addr, CHookImplementObject::REGISTERS* backup_regs, std::shared_ptr<CFunction> function, void* addr)
    {
        ASSERT(new_addr != nullptr);
        ASSERT(backup_regs != nullptr);
        ASSERT(function != nullptr);
        ASSERT(addr != nullptr);
        if (!new_addr || !backup_regs || !function || !addr) return false;
        int count = 0;
        size_t arg_total_sizes = function->GetArgumentSize();
        for (const auto& arg : function->GetArguments())
        {
            ASSERT(arg.second.second);
            if (!arg.second.second) return false;
            LONGLONG arg_value = 0;
            size_t arg_size = arg.second.second->GetObSize();
            ASSERT(arg_size != 0);
            arg_total_sizes -= arg_size;
            ASSERT(arg_total_sizes >= 0 && arg_total_sizes <= 0x100);
            switch (count)
            {
            case 0: {arg_value = backup_regs->rcx; } break;
            case 1: {arg_value = backup_regs->rdx; } break;
            case 2: {arg_value = backup_regs->r8; }  break;
            case 3: {arg_value = backup_regs->r9; }  break;
            default: {arg_value = reinterpret_cast<ULONG_PTR*>(addr)[count]; }
            }
            new_addr[count++] = arg_value;
        }
        return true;
    }
#endif // #define _AMD64_

    bool CHookImplementObject::InitializeWmiMethodsDefine(bool bdelayed)
    {
        if (!m_ole32_api_define.cocreateinstance_func ||
            !m_ole32_api_define.cosetproxyblanket_func)
        {
            HMODULE hOle32 = GetModuleHandle("ole32");
            if (hOle32 == nullptr) return false;
            if (!m_ole32_api_define.coinitializesecurity_func)
                m_ole32_api_define.coinitializesecurity_func = reinterpret_cast<CoInitializeSecurity_Define>(GetProcAddress(hOle32, "CoInitializeSecurity"));
            if (!m_ole32_api_define.cocreateinstance_func)
                m_ole32_api_define.cocreateinstance_func = reinterpret_cast<CoCreateInstance_Define>(GetProcAddress(hOle32, "CoCreateInstance"));
            if (!m_ole32_api_define.cosetproxyblanket_func)
                m_ole32_api_define.cosetproxyblanket_func = reinterpret_cast<CoSetProxyBlanket_Define>(GetProcAddress(hOle32, "CoSetProxyBlanket"));
        }
        if (!m_ole32_api_define.coinitializesecurity_func ||
            !m_ole32_api_define.cocreateinstance_func ||
            !m_ole32_api_define.cosetproxyblanket_func)
            return false;
        if (m_wmi_interface_define.size())
            return true;

        bool bret = false;
        IWbemLocator* pWbemLoc = nullptr;
        IEnumWbemClassObject* pEnumClsObj = nullptr;
        IWbemClassObject* pWbemClsObj = nullptr;
        IWbemServices* pWbemSvc = nullptr;
        HRESULT hr = m_ole32_api_define.coinitializesecurity_func(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
        if (SUCCEEDED(hr))
        {
            hr = m_ole32_api_define.cocreateinstance_func(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pWbemLoc);
                if (SUCCEEDED(hr) && pWbemLoc != nullptr)
                {
                    m_wmi_interface_define[std::string("IWbemLocator")] = reinterpret_cast<ULONG_PTR*>(*reinterpret_cast<ULONG_PTR*>(pWbemLoc));
                        hr = pWbemLoc->ConnectServer(CComBSTR("ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pWbemSvc);
                        if (SUCCEEDED(hr) && pWbemSvc != nullptr)
                        {
                            m_wmi_interface_define[std::string("IWbemServices")] = reinterpret_cast<ULONG_PTR*>(*reinterpret_cast<ULONG_PTR*>(pWbemSvc));
                                m_ole32_api_define.cosetproxyblanket_func(pWbemSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
                                CComBSTR query("SELECT * FROM ");
                            ULONG uReturn;
                            query += CComBSTR("Win32_DiskDrive");
                            hr = pWbemSvc->ExecQuery(CComBSTR("WQL"), query, WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                                0, &pEnumClsObj);
                            if (SUCCEEDED(hr) && pEnumClsObj != nullptr)
                            {
                                m_wmi_interface_define[std::string("IEnumWbemClassObject")] = reinterpret_cast<ULONG_PTR*>(*reinterpret_cast<ULONG_PTR*>(pEnumClsObj));
                                hr = pEnumClsObj->Next(WBEM_INFINITE, 1, &pWbemClsObj, &uReturn);
                                if (SUCCEEDED(hr) && pWbemClsObj != nullptr)
                                {
                                    m_wmi_interface_define[std::string("IWbemClassObject")] = reinterpret_cast<ULONG_PTR*>(*reinterpret_cast<ULONG_PTR*>(pWbemClsObj));
                                    pWbemClsObj->Release();
                                    bret = true;
                                }
                                pEnumClsObj->Release();
                            }
                            pWbemSvc->Release();
                        }
                    pWbemLoc->Release();
                }
        }
        return bret;
    }

} // namespace cchips



