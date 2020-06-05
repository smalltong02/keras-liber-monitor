#include "stdafx.h"
#include "utils.h"
#include "HookImplementObject.h"

namespace cchips {

    const LPVOID CHookImplementObject::m_hookImplementFunction = (LPVOID)_hookImplementFunction;
    std::shared_ptr<CHookImplementObject> g_impl_object = std::make_shared<CHookImplementObject>();
    const DWORD CHookImplementObject::m_threadTlsIdx = TlsAlloc();
    const std::unique_ptr<CExceptionObject> CHookImplementObject::m_exceptionObject(CExceptionObject::GetInstance());

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

        MACRO_PROCESSING_STACK_(__backup_regs, __stack, __hook_node, __addr)
            if (__hook_node != nullptr && __addr != nullptr && (__object = __hook_node->hook_implement_object.get()) != nullptr)
            {
                // safe remove hook when shared_count = 0
                InterlockedIncrement(&__hook_node->shared_count);
                __psize = 0; __return = 0;
                ULONG_PTR entry_count = __object->GetTlsValueForThreadIdx();
                CLogHandle* __log = nullptr;
                processing_status __status = __object->Preprocessing(__hook_node, __addr, __psize, __return, entry_count, &__log);
                if (__status != processing_exit)
                {
                    void* __func = __hook_node->orgin_api_implfunc;
                    char** __params = nullptr;
                    __params = (char**)((ULONG_PTR)__addr + __psize);
                    CFunction::_call_convention __call_conv = __hook_node->function->GetCallConv();

                    MACRO_CALL_ORGINAL_(__params, __psize, __call_conv, __func, __return)
                    if(__status == processing_continue)
                        __object->Postprocessing(__hook_node, __addr, __psize, __return, entry_count, &__log);
                }
                __object->ReleaseTlsValueForThreadIdx();
                InterlockedDecrement(&__hook_node->shared_count);
            }
        MACRO_PROCESSING_RETURN(__backup_regs, __psize, __return)
    }
#pragma code_seg()
#pragma comment(linker, "/SECTION:.wrtext,ERW")
#endif // #ifdef _X86_

    bool CHookImplementObject::Initialize(std::shared_ptr<CHipsCfgObject>& configObject)
    {
        CoInitializeEx(0, COINIT_MULTITHREADED);

        m_configObject = configObject;
        m_drivermgr = std::make_unique<CDriverMgr>();

        if (m_threadTlsIdx == TLS_OUT_OF_INDEXES)
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

    bool CHookImplementObject::HookAllApis()
    {
        if (!m_bValid)
            return false;
        ASSERT(m_bValid);
        int count = 0;
        m_hookNodeList.resize(GetFunctionCounts() + GetWmiMethodsDefineSize() + 5, hook_node({}));

        for (const auto& func : GetFunctions())
        {
            m_hookNodeList[count].bdelayed = false;
            m_hookNodeList[count].orgin_api_implfunc = nullptr;
            m_hookNodeList[count].hook_implement_object = shared_from_this();
            m_hookNodeList[count].function = func.second;

            MH_STATUS status;
            status = MH_CreateHookApiEx(A2WString((*func.second).GetLibrary()).c_str(), (LPCSTR)(*func.second).GetName().c_str(), m_hookImplementFunction, &(m_hookNodeList[count].orgin_api_implfunc), (LPVOID*)&m_hookNodeList[count], NULL);
            if (status == MH_OK)
            {
                debug_log("api hook create: '{}' success!", (*func.second).GetName());
            }
            else
            {
                if (status == MH_ERROR_MODULE_NOT_FOUND)
                {
                    // delayed hook;
                    m_hookNodeList[count].bdelayed = true;
                    (*m_delayNodeList)[(*m_hookNodeList[count].function).GetLibrary()].push_back(count);
                }
                else
                {
                    error_log("api hook create: '{}' failed({})!", (*func.second).GetName(), status);
                }
            }
            count++;
        }

        if (GetWmiObjectCounts())
        {
            HookWmiObjectMethods(count);
        }

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
        if (hook_implement_object->IsFilterThread(std::this_thread::get_id())) return processing_skip;
        if (params_size == 0) params_size = (int)func_object->GetArgumentAlignSize();
#ifdef _X86_
        char* __params = reinterpret_cast<char*>(reinterpret_cast<ULONG_PTR>(param_addr) + CFunction::stack_aligned_bytes);
#endif
#ifdef _AMD64_
        char* __params = reinterpret_cast<char*>(param_addr);
#endif

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
            MACRO_PUSH_PARAMS_(pnode, __rev_params, params_size, pre_func, status)
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
            MACRO_PUSH_PARAMS_(pnode, __rev_params, params_size, post_func, status)
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
        // processing post hook
        ADD_POST_PROCESSING(LoadLibraryA, detour_loadLibraryA);
        ADD_POST_PROCESSING(LoadLibraryW, detour_loadLibraryW);
        ADD_POST_PROCESSING(LoadLibraryExA, detour_loadLibraryExA);
        ADD_POST_PROCESSING(LoadLibraryExW, detour_loadLibraryExW);
        // processing wmi hook
        ADD_POST_PROCESSING(IEnumWbemClassObject_Next, detour_IEnumWbemClassObject_Next);
        ADD_POST_PROCESSING(IWbemClassObject_Get, detour_IWbemClassObject_Get);
        ADD_POST_PROCESSING(IWbemClassObject_Put, detour_IWbemClassObject_Put);
        ADD_POST_PROCESSING(IWbemClassObject_Next, detour_IWbemClassObject_Next);
        ADD_POST_PROCESSING(IWbemServices_ExecMethod, detour_IWbemServices_ExecMethod);
        ADD_POST_PROCESSING(IWbemServices_ExecQuery, detour_IWbemServices_ExecQuery);
        return true;
    }

    bool CHookImplementObject::UnhookProcessing()
    {
        //processing pre hook
        DEL_PRE_PROCESSING(NtQueryInformationProcess, detour_ntQueryInformationProcess);
        // processing post hook
        DEL_POST_PROCESSING(LoadLibraryA, detour_loadLibraryA);
        DEL_POST_PROCESSING(LoadLibraryW, detour_loadLibraryW);
        DEL_POST_PROCESSING(LoadLibraryExA, detour_loadLibraryExA);
        DEL_POST_PROCESSING(LoadLibraryExW, detour_loadLibraryExW);
        // processing wmi hook
        DEL_POST_PROCESSING(IEnumWbemClassObject_Next, detour_IEnumWbemClassObject_Next);
        DEL_POST_PROCESSING(IWbemClassObject_Get, detour_IWbemClassObject_Get);
        DEL_POST_PROCESSING(IWbemClassObject_Put, detour_IWbemClassObject_Put);
        DEL_POST_PROCESSING(IWbemClassObject_Next, detour_IWbemClassObject_Next);
        DEL_POST_PROCESSING(IWbemServices_ExecMethod, detour_IWbemServices_ExecMethod);
        DEL_POST_PROCESSING(IWbemServices_ExecQuery, detour_IWbemServices_ExecQuery);
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
            // safe remove hook when shared_count = 0
            InterlockedIncrement(&__hook_node->shared_count);
            __psize = 0; __return = 0;
            ULONG_PTR entry_count = __object->GetTlsValueForThreadIdx();
            CLogHandle* __log = nullptr;
            void* __new_addr = reinterpret_cast<void*>(reinterpret_cast<ULONG_PTR>(__addr) - 0x270);
            bool __bforward = __object->ForwardPropagationArgs(reinterpret_cast<ULONG_PTR*>(__new_addr), __backup_regs, __hook_node->function, __addr);

            processing_status __status = processing_continue;
            if (__bforward) __status = __object->Preprocessing(__hook_node, __new_addr, __psize, __return, entry_count, &__log);
            if (__status != processing_exit)
            {
                if (__psize == 0) __psize = (int)__hook_node->function->GetArgumentAlignSize();
                void* __func = __hook_node->orgin_api_implfunc;
                char** __params = (char**)(__new_addr);
                CFunction::_call_convention __call_conv = __hook_node->function->GetCallConv();
                ASSERT(__call_conv == CFunction::_call_convention::call_stdcall); // now only support stdcall, actually is fastcall on x64.
                MACRO_CALL_ORGINAL_(__params, __psize, __call_conv, __func, &__return)
                if(__status == processing_continue)
                    if (__bforward) __object->Postprocessing(__hook_node, __new_addr, __psize, __return, entry_count, &__log);
            }
            __object->ReleaseTlsValueForThreadIdx();
            InterlockedDecrement(&__hook_node->shared_count);
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

    bool CHookImplementObject::InitializeWmiMethodsDefine()
    {
        if (m_wmi_methods_define.enumwbem_next_func &&
            m_wmi_methods_define.wbem_next_func &&
            m_wmi_methods_define.wbem_get_func &&
            m_wmi_methods_define.wbem_put_func &&
            m_wmi_methods_define.wbem_execmethod_func &&
            m_wmi_methods_define.wbem_execquery_func)
            return true;

        bool bret = false;
        IWbemLocator* pWbemLoc = nullptr;
        IEnumWbemClassObject* pEnumClsObj = nullptr;
        IWbemClassObject* pWbemClsObj = nullptr;
        IWbemServices* pWbemSvc = nullptr;
        HRESULT hr = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pWbemLoc);
        if (SUCCEEDED(hr) && pWbemLoc != nullptr)
        {
            hr = pWbemLoc->ConnectServer(CComBSTR("ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pWbemSvc);
            if (SUCCEEDED(hr) && pWbemSvc != nullptr)
            {
                if (!m_wmi_methods_define.wbem_execmethod_func)
                    m_wmi_methods_define.wbem_execmethod_func = reinterpret_cast<IWbemServices_ExecMethod_Define>(*(reinterpret_cast<ULONG_PTR*>(*reinterpret_cast<ULONG_PTR*>(pWbemSvc)) + IWbemServices_ExecMethod_Vtbl_Index));
                if (!m_wmi_methods_define.wbem_execquery_func)
                    m_wmi_methods_define.wbem_execquery_func = reinterpret_cast<IWbemServices_ExecQuery_Define>(*(reinterpret_cast<ULONG_PTR*>(*reinterpret_cast<ULONG_PTR*>(pWbemSvc)) + IWbemServices_ExecQuery_Vtbl_Index));
                CoSetProxyBlanket(pWbemSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);

                CComBSTR query("SELECT * FROM ");
                ULONG uReturn;
                query += CComBSTR("Win32_DiskDrive");
                hr = pWbemSvc->ExecQuery(CComBSTR("WQL"), query, WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                    0, &pEnumClsObj);
                if (SUCCEEDED(hr) && pEnumClsObj != nullptr)
                {
                    if (!m_wmi_methods_define.enumwbem_next_func)
                        m_wmi_methods_define.enumwbem_next_func = reinterpret_cast<IEnumWbemClassObject_Next_Define>(*(reinterpret_cast<ULONG_PTR*>(*reinterpret_cast<ULONG_PTR*>(pEnumClsObj)) + IEnumWbemClassObject_Next_Vtbl_Index));
                    hr = pEnumClsObj->Next(WBEM_INFINITE, 1, &pWbemClsObj, &uReturn);
                    if (SUCCEEDED(hr) && pWbemClsObj != nullptr)
                    {
                        if (!m_wmi_methods_define.wbem_get_func)
                            m_wmi_methods_define.wbem_get_func = reinterpret_cast<IWbemClassObject_Get_Define>(*(reinterpret_cast<ULONG_PTR*>(*reinterpret_cast<ULONG_PTR*>(pWbemClsObj)) + IWbemClassObject_Get_Vtbl_Index));
                        if (!m_wmi_methods_define.wbem_put_func)
                            m_wmi_methods_define.wbem_put_func = reinterpret_cast<IWbemClassObject_Put_Define>(*(reinterpret_cast<ULONG_PTR*>(*reinterpret_cast<ULONG_PTR*>(pWbemClsObj)) + IWbemClassObject_Put_Vtbl_Index));
                        if (!m_wmi_methods_define.wbem_next_func)
                            m_wmi_methods_define.wbem_next_func = reinterpret_cast<IWbemClassObject_Next_Define>(*(reinterpret_cast<ULONG_PTR*>(*reinterpret_cast<ULONG_PTR*>(pWbemClsObj)) + IWbemClassObject_Next_Vtbl_Index));
                        pWbemClsObj->Release();
                        bret = true;
                    }
                    pEnumClsObj->Release();
                }
                pWbemSvc->Release();
            }
            pWbemLoc->Release();
        }
        return bret;
    }

    bool CHookImplementObject::HookWmiObjectMethods(int& count)
    {
        if (!InitializeWmiMethodsDefine()) return false;

        ASSERT(m_wmi_methods_define.enumwbem_next_func != nullptr);
        ASSERT(m_wmi_methods_define.wbem_next_func != nullptr);
        ASSERT(m_wmi_methods_define.wbem_get_func != nullptr);
        ASSERT(m_wmi_methods_define.wbem_put_func != nullptr);
        ASSERT(m_wmi_methods_define.wbem_execmethod_func != nullptr);
        ASSERT(m_wmi_methods_define.wbem_execquery_func != nullptr);

        if (!m_wmi_methods_define.enumwbem_next_func ||
            !m_wmi_methods_define.wbem_next_func ||
            !m_wmi_methods_define.wbem_get_func ||
            !m_wmi_methods_define.wbem_put_func ||
            !m_wmi_methods_define.wbem_execmethod_func ||
            !m_wmi_methods_define.wbem_execquery_func)
            return false;

        auto create_function_object = [](CFunction::_call_convention call_conv, const std::string& func_name, const std::string& library, const std::string& feature, const std::vector<std::pair<std::string, std::string>>& args_list, const std::string& ret_iden) ->std::shared_ptr<CFunction> {
            std::unique_ptr<CFunction> func_object = std::make_unique<CFunction>(call_conv, func_name);
            ASSERT(func_object);
            if (!func_object) return nullptr;
            func_object->SetLibrary(library);
            func_object->SetFeature(feature);
            std::shared_ptr<CObObject> return_ptr = func_object->GetTyIdentifier(ret_iden);
            if (!return_ptr) return nullptr;
            if (!func_object->AddReturn(IDENPAIR(SI_RETURN, return_ptr))) return nullptr;
            for (auto& arg : args_list)
            {
                std::shared_ptr<CObObject> param_ptr = func_object->GetTyIdentifier(arg.second);
                if (!param_ptr) return false;
                if (!func_object->AddArgument(IDENPAIR(arg.first, param_ptr))) return nullptr;
            }
            return std::shared_ptr<CFunction>(func_object.release());
        };
        auto create_mhhook_wmi_method = [](std::shared_ptr<CFunction>& method_func, std::shared_ptr<CHookImplementObject>& impl_object, std::vector<hook_node>& hook_node_list, int& count, const LPVOID target_addr, const LPVOID hookImplementFunction) ->bool {
            if (!impl_object->AddFunction(method_func)) return false;
            hook_node_list[count].bdelayed = false;
            hook_node_list[count].orgin_api_implfunc = nullptr;
            hook_node_list[count].hook_implement_object = impl_object;
            hook_node_list[count].function = method_func;
            MH_STATUS status;
            status = MH_CreateHook(target_addr, hookImplementFunction, &(hook_node_list[count].orgin_api_implfunc), (LPVOID*)&hook_node_list[count]);
            if (status == MH_OK)
            {
                debug_log("wmi method hook create: '{}' success!", method_func->GetName());
            }
            else
            {
                error_log("wmi method hook create: '{}' failed({})!", method_func->GetName(), status);
            }
            count++;
            return (status == MH_OK);
        };

        std::vector<std::pair<std::string, std::string>> enumwbem_next_args = { {"This", "LPVOID"}, {"lTimeout", "LONG"}, {"uCount", "ULONG"}, {"apObjects", "LPVOID*"}, {"puReturned", "ULONG*"} };
        std::shared_ptr<CFunction> enumwbem_next_func_object(create_function_object(CFunction::call_stdcall, "IEnumWbemClassObject_Next", "ole32.dll", WMI_ENUMWBEM_NEXT, enumwbem_next_args, "HRESULT"));
        if (!enumwbem_next_func_object) return false;
        if (!create_mhhook_wmi_method(enumwbem_next_func_object, shared_from_this(), m_hookNodeList, count, m_wmi_methods_define.enumwbem_next_func, m_hookImplementFunction)) return false;

        std::vector<std::pair<std::string, std::string>> wbem_get_args = { {"This", "LPVOID"}, {"wszName", "LPCWSTR"}, {"lFlags", "LONG"}, {"pVal", "LPVOID"}, {"pType", "LONG*"}, {"plFlavor", "LONG*"} };
        std::shared_ptr<CFunction> wbem_get_func_object(create_function_object(CFunction::call_stdcall, "IWbemClassObject_Get", "ole32.dll", WMI_WBEM_GET, wbem_get_args, "HRESULT"));
        if (!wbem_get_func_object) return false;
        if (!create_mhhook_wmi_method(wbem_get_func_object, shared_from_this(), m_hookNodeList, count, m_wmi_methods_define.wbem_get_func, m_hookImplementFunction)) return false;

        std::vector<std::pair<std::string, std::string>> wbem_put_args = { {"This", "LPVOID"}, {"wszName", "LPCWSTR"}, {"lFlags", "LONG"}, {"pVal", "LPVOID"}, {"Type", "LONG"} };
        std::shared_ptr<CFunction> wbem_put_func_object(create_function_object(CFunction::call_stdcall, "IWbemClassObject_Put", "ole32.dll", WMI_WBEM_PUT, wbem_put_args, "HRESULT"));
        if (!wbem_put_func_object) return false;
        if (!create_mhhook_wmi_method(wbem_put_func_object, shared_from_this(), m_hookNodeList, count, m_wmi_methods_define.wbem_put_func, m_hookImplementFunction)) return false;

        std::vector<std::pair<std::string, std::string>> wbem_next_args = { {"This", "LPVOID"}, {"lFlags", "LONG"}, {"strName", "BSTR*"}, {"pVal", "LPVOID"}, {"pType", "LONG*"}, {"plFlavor", "LONG*"} };
        std::shared_ptr<CFunction> wbem_next_func_object(create_function_object(CFunction::call_stdcall, "IWbemClassObject_Next", "ole32.dll", WMI_WBEM_NEXT, wbem_next_args, "HRESULT"));
        if (!wbem_next_func_object) return false;
        if (!create_mhhook_wmi_method(wbem_next_func_object, shared_from_this(), m_hookNodeList, count, m_wmi_methods_define.wbem_next_func, m_hookImplementFunction)) return false;

        std::vector<std::pair<std::string, std::string>> wbem_execmethod_args = { {"This", "LPVOID"}, {"strObjectPath", "BSTR"}, {"strMethodName", "BSTR"}, {"lFlags", "LONG"}, {"pCtx", "LPVOID"}, {"pInParams", "LPVOID"}, {"ppOutParams", "LPVOID*"}, {"ppCallResult", "LPVOID*"} };
        std::shared_ptr<CFunction> wbem_execmethod_func_object(create_function_object(CFunction::call_stdcall, "IWbemServices_ExecMethod", "ole32.dll", WMI_WBEM_EXECMETHOD, wbem_execmethod_args, "HRESULT"));
        if (!wbem_execmethod_func_object) return false;
        if (!create_mhhook_wmi_method(wbem_execmethod_func_object, shared_from_this(), m_hookNodeList, count, m_wmi_methods_define.wbem_execmethod_func, m_hookImplementFunction)) return false;

        std::vector<std::pair<std::string, std::string>> wbem_execquery_args = { {"This", "LPVOID"}, {"strQueryLanguage", "BSTR"}, {"strQuery", "BSTR"}, {"lFlags", "LONG"}, {"pCtx", "LPVOID"}, {"ppEnum", "LPVOID*"} };
        std::shared_ptr<CFunction> wbem_execquery_func_object(create_function_object(CFunction::call_stdcall, "IWbemServices_ExecQuery", "ole32.dll", WMI_WBEM_EXECQUERY, wbem_execquery_args, "HRESULT"));
        if (!wbem_execquery_func_object) return false;
        if (!create_mhhook_wmi_method(wbem_execquery_func_object, shared_from_this(), m_hookNodeList, count, m_wmi_methods_define.wbem_execquery_func, m_hookImplementFunction)) return false;
        return true;
    }

} // namespace cchips



