#pragma once
#include <stdio.h>
#include <windows.h>
#include <string>
#include <vector>
#include <execution>
#include "Psapi.h"
#include "HipsCfgObject.h"
#include "SigsObject.h"
#include "MinHook.h"
#include "hookimpl.h"
#include "LogObject.h"
#include "SafePtr.h"
#include "asmfunction.h"
#include "ExceptionThrow.h"
#include "UniversalObject.h"
#include "NativeObject.h"
#include "PassManager.h"
#include "drivermgr.h"
#include "ntapi.h"

namespace cchips {

#ifdef _X86_

#define __CACHE_SIZE 100h	// reserve 256 bytes on stack

#define MACRO_PROCESSING_STACK_(__backup_regs, __stack, __hook_node, __addr) \
    __asm {pushfd}                             \
    __asm {push eax}                           \
    __asm {push ebx}                           \
    __asm {push ecx}                           \
    __asm {push edx}                           \
    __asm {push esi}                           \
    __asm {push edi}                           \
    __asm {push ebp}                           \
    __asm {mov  ebp, esp}                      \
    __asm {sub  esp, __CACHE_SIZE}             \
    __asm {mov eax, 0}                         \
    __asm {mov eax, [ebp + 0]}                 \
    __asm {mov __backup_regs.ebp, eax}         \
    __asm {mov eax, [ebp + 4]}                 \
    __asm {mov __backup_regs.edi, eax}         \
    __asm {mov eax, [ebp + 8]}                 \
    __asm {mov __backup_regs.esi, eax}         \
    __asm {mov eax, [ebp + 12]}                \
    __asm {mov __backup_regs.edx, eax}         \
    __asm {mov eax, [ebp + 16]}                \
    __asm {mov __backup_regs.ecx, eax}         \
    __asm {mov eax, [ebp + 20]}                \
    __asm {mov __backup_regs.ebx, eax}         \
    __asm {mov eax, [ebp + 24]}                \
    __asm {mov __backup_regs.eax, eax}         \
    __asm {mov eax, [ebp + 28]}                \
    __asm {mov __backup_regs.efl, eax}         \
    __asm {mov  __stack, ebp}                  \
    __asm {mov  eax, dword ptr[ebp + 32]}      \
    __asm {mov  __hook_node, eax}              \
    __asm {lea  eax, dword ptr[ebp + 36]}      \
    __asm {mov  __addr, eax}                   \
    __asm {lea  eax, dword ptr[ebp + 40]}      \
    __asm {mov  __backup_regs.esp, eax}

#define MACRO_PUSH_PARAMS_(__pnode, __params, __psize, __post_func, __status)   \
    __asm {push ebx}                            \
    for (int i = 0; i < __psize / 4; i++)       \
    {                                           \
        __asm {mov  eax, i}                     \
        __asm {imul ebx, eax, 4}                \
        __asm {mov  eax, __params}              \
        __asm {sub  eax, ebx}                   \
        __asm {mov  eax, dword ptr [eax]}       \
        __asm {push eax}                        \
    }                                           \
    __asm {mov eax, __pnode}                    \
    __asm {push eax}                            \
    __asm {mov  eax, __post_func}               \
    __asm {mov  eax, dword ptr [eax]}           \
    __asm {call eax}                            \
    __asm {pop  ebx}                            \
    __asm {mov __status, eax}

#define MACRO_CALL_ORGINAL_(__backup_regs, __params, __psize, __call_conv, __func, __return) \
    __asm {push ebx}                            \
    for (int i = 0; i < __psize / 4; i++)       \
    {                                           \
        __asm {mov  eax, i}                     \
        __asm {imul ebx, eax, 4}                \
        __asm {mov  eax, __params}              \
        __asm {sub  eax, ebx}                   \
        __asm {mov  eax, dword ptr [eax]}       \
        __asm {push eax}                        \
    }                                           \
    __asm {call __func}                            \
    __asm {mov  __return, eax}                  \
    __asm {cmp  __psize, 0}                     \
    __asm {jz   std_quit}                       \
    __asm {cmp  __call_conv, 1}                 \
    __asm {jz   cdecl_quit}                     \
    __asm {jmp  std_quit}                       \
    __asm {cdecl_quit:}                         \
    __asm {lea  eax, fix_stack}                 \
    __asm {add  eax, 2}                         \
    __asm {mov  ebx, __psize}                   \
    __asm {mov  byte ptr[eax], bl}              \
    __asm {fix_stack:}                          \
    __asm {add  esp, 4}                         \
    __asm {std_quit:}                           \
    __asm {pop  ebx}                            

#define MACRO_PROCESSING_RETURN(__backup_regs, __nsize, __return) \
    __asm {push ebx}                            \
    __asm {lea  eax, quit}                      \
    __asm {mov  ebx, __nsize}                   \
    __asm {add  eax, 1}                         \
    __asm {mov  word ptr[eax], bx}              \
    __asm {pop  ebx}                            \
    __asm {mov  ebx, __backup_regs.ebx}         \
    __asm {mov  ecx, __backup_regs.ecx}         \
    __asm {mov  edx, __backup_regs.edx}         \
    __asm {mov  esi, __backup_regs.esi}         \
    __asm {mov  edi, __backup_regs.edi}         \
    __asm {mov  eax, __backup_regs.esp}         \
    __asm {sub  eax, 4}                         \
    __asm {mov  __backup_regs.esp, eax}         \
    __asm {push __backup_regs.efl}              \
    __asm {popfd}                               \
    __asm {mov  eax, __return}                  \
    __asm {mov  esp, __backup_regs.esp}         \
    __asm {mov  ebp, __backup_regs.ebp}         \
    __asm {quit:}                               \
    __asm {ret 4}                               

    extern void STDMETHODCALLTYPE _hookImplementFunction(void);
#endif	// #ifdef _X86_

#ifdef _AMD64_
#define MACRO_PUSH_PARAMS_(__pnode, __params, __psize, __post_func, __status) _callPreProcessing_x64((void*)__pnode, (ULONG_PTR*)__params, __psize, (void*)__post_func, __status);
#define MACRO_CALL_ORGINAL_(__params, __psize, __call_conv, __func, __return) _callOrginApiFunction_x64((ULONG_PTR*)__params, __psize, (int)__call_conv, __func, __return);
#endif // #ifdef _AMD64

    class CHookImplementObject : public CFunctionProtos, public CWmiObjectProtos, public std::enable_shared_from_this<CHookImplementObject>
    {
    public:
#define ADD_PRE_PROCESSING(NAME, FUNC) AddPreProcessing(#NAME, reinterpret_cast<CFunction::func_proto>(FUNC))
#define ADD_POST_PROCESSING(NAME, FUNC) AddPostProcessing(#NAME, reinterpret_cast<CFunction::func_proto>(FUNC))
#define DEL_PRE_PROCESSING(NAME, FUNC) DelPreProcessing(#NAME, reinterpret_cast<CFunction::func_proto>(FUNC))
#define DEL_POST_PROCESSING(NAME, FUNC) DelPostProcessing(#NAME, reinterpret_cast<CFunction::func_proto>(FUNC))

#define PRE_BEGIN(de_node) \
BreakPoint; \
ASSERT(de_node != nullptr); \
ASSERT(de_node->return_va != nullptr); \
ASSERT(de_node->log_entry != nullptr); \
ASSERT(de_node->function != nullptr); \
ASSERT(de_node->hook_implement_object != nullptr); \
if (!de_node || !de_node->return_va || !de_node->log_entry || !de_node->hook_implement_object) return processing_skip;

#define EXPLOIT_PRE_BEGIN(de_node) \
PRE_BEGIN(de_node) \
if (!de_node->return_addr) return processing_skip;

#define POST_BEGIN(de_node) \
BreakPoint; \
ASSERT(de_node != nullptr); \
ASSERT(de_node->return_va != nullptr); \
ASSERT(de_node->log_entry != nullptr); \
ASSERT(de_node->function != nullptr); \
ASSERT(de_node->hook_implement_object != nullptr); \
if (!de_node || !de_node->return_va || !de_node->log_entry || !de_node->hook_implement_object) return processing_skip; \
if (processing_status status; (status = de_node->function->CheckReturn(de_node)) != processing_continue) return status;

#define EXPLOIT_POST_BEGIN(de_node) \
POST_BEGIN(de_node) \
if (!de_node->return_addr) return processing_skip;

#define CHECK_COMMON_EXPLOIT() \
if (DWORD index_; !CheckExploitFuncs::ValidStackPointer(reinterpret_cast<ULONG_PTR>(&index_))) {    \
    exploit_log("\"API\": \"{}\", \"reason\": {}", node->function->GetName(), CheckExploitFuncs::e_reason_stack_corrupted); \
    return processing_continue; \
} else { \
    CheckExploitFuncs::_exploit_reason reason = CheckExploitFuncs::e_reason_null; \
    reason = CheckExploitFuncs::CheckReturnAddress(reinterpret_cast<ULONG_PTR>(node->return_addr)); \
    if (reason != CheckExploitFuncs::e_reason_null) { \
            exploit_log("\"API\": \"{}\", \"reason\": {}", node->function->GetName(), reason); \
            return processing_continue; \
    } \
}

        using REGISTERS = struct __REGISTERS {
#ifdef _X86_
            DWORD eax;
            DWORD ebx;
            DWORD ecx;
            DWORD edx;
            DWORD esi;
            DWORD edi;
            DWORD efl;
            DWORD esp;
            DWORD ebp;
#endif
#ifdef _AMD64_
            LONGLONG elf;
            LONGLONG rsp;
            LONGLONG rbp;
            LONGLONG r15;
            LONGLONG r14;
            LONGLONG r13;
            LONGLONG r12;
            LONGLONG r11;
            LONGLONG r10;
            LONGLONG r9;
            LONGLONG r8;
            LONGLONG rdi;
            LONGLONG rsi;
            LONGLONG rdx;
            LONGLONG rcx;
            LONGLONG rbx;
#endif
        };
        enum _filter_type {
            filter_invalid = -1,
            filter_threadid = 0,
        };
        CHookImplementObject() :
            m_bValid(false), m_benable(false), m_configObject(nullptr), m_drivermgr(nullptr), m_bwmi_success(false) {
        }//for hook wmi methods
        ~CHookImplementObject() {
            if (m_bValid)
            {
                DisableAllApis();
                UnhookProcessing();
                MH_Uninitialize();
            }
            //waiting all __hook_node->shared_count == 0
        }
        using LdrRegisterDllNotification_Define = NTSTATUS (WINAPI*)(ULONG Flags, LDR_DLL_NOTIFICATION_FUNCTION LdrDllNotificationFunction, VOID *Context, VOID **Cookie);
        using CoCreateInstance_Define = HRESULT (WINAPI*)(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID *ppv);
        using CoInitializeSecurity_Define = HRESULT(WINAPI*)(PSECURITY_DESCRIPTOR pSecDesc, LONG cAuthSvc, SOLE_AUTHENTICATION_SERVICE *asAuthSvc, void *pReserved1, DWORD dwAuthnLevel, DWORD dwImpLevel, void *pAuthList, DWORD dwCapabilities, void *pReserved3);
        using CoSetProxyBlanket_Define = HRESULT(WINAPI*) (IUnknown *pProxy, DWORD dwAuthnSvc, DWORD dwAuthzSvc, OLECHAR *pServerPrincName, DWORD dwAuthnLevel, DWORD dwImpLevel, RPC_AUTH_IDENTITY_HANDLE pAuthInfo, DWORD dwCapabilities);
        using IEnumWbemClassObject_Next_Define = HRESULT(*)(IEnumWbemClassObject* This, long lTimeout, ULONG uCount, IWbemClassObject **apObjects, ULONG *puReturned);
        using IWbemClassObject_Get_Define = HRESULT(*)(IWbemClassObject* This, LPCWSTR wszName, long lFlags, VARIANT *pVal, long *pType, long *plFlavor);
        using IWbemClassObject_Put_Define = HRESULT(*)(IWbemClassObject* This, LPCWSTR wszName, long lFlags, VARIANT *pVal, long Type);
        using IWbemClassObject_Next_Define = HRESULT(*)(IWbemClassObject* This, long lFlags, BSTR *strName, VARIANT *pVal, long *pType, long *plFlavor);
        using IWbemServices_ExecMethod_Define = HRESULT(*)(IWbemServices* This, const BSTR strObjectPath, const BSTR strMethodName, long lFlags, IWbemContext *pCtx, PVOID pInParams, IWbemClassObject **ppOutParams, IWbemCallResult **ppCallResult);
        using IWbemServices_ExecQuery_Define = HRESULT(*)(IWbemServices* This, const BSTR strQueryLanguage, const BSTR strQuery, long lFlags, IWbemContext *pCtx, IEnumWbemClassObject **ppEnum);
        using RtlAllocateHeap_Define = PVOID(WINAPI*)(PVOID HeapHandle, ULONG Flags, SIZE_T Size);
        using RtlFreeHeap_Define = BOOLEAN(WINAPI*)(PVOID HeapHandle, ULONG Flags, PVOID BaseAddress);
        using RtlDestroyHeap_Define = PVOID(WINAPI*)(PVOID HeapHandle);

        using detour_node = struct _detour_node {
            PVOID return_va;
            char* return_addr;
            char* pparams;
            size_t params_size;
            std::shared_ptr<CFunction> function;
            std::shared_ptr<CHookImplementObject> hook_implement_object;
            std::shared_ptr<PVOID> log_entry;
        };

        using hook_node = struct _hook_node {
            bool bdelayed;
            volatile LONG shared_count;
            std::shared_ptr<CFunction> function;
            std::shared_ptr<CHookImplementObject> hook_implement_object;
            void* orgin_api_implfunc;
            void* ppTarget;
        };

        using ole32_api_define = struct {
            CoCreateInstance_Define cocreateinstance_func;
            CoInitializeSecurity_Define coinitializesecurity_func;
            CoSetProxyBlanket_Define cosetproxyblanket_func;
        };
        using notification_api_define = struct {
            LdrRegisterDllNotification_Define ldrregisterdllnotification_func;
        };
        using heap_allocation_define = struct {
            RtlAllocateHeap_Define rtlallocateheap_func;
            RtlFreeHeap_Define rtlfreeheap_func;
            RtlDestroyHeap_Define rtldestroyheap_func;
        };
        using error_offset_define = struct {
            DWORD m_win32_error_offset;
            DWORD m_nt_status_offset;
        };

        using special_module_info = struct {
            MODULEINFO m_self_info;
            MODULEINFO m_vbe_info;
        };

        using last_error_define = struct {
            DWORD nt_status;
            DWORD last_error;
        };

        bool Running();
        bool InitializeErrorOffset();
        bool InitializeSpecialModuleInfo();
        bool InitializeSelfModuleInfo();
        bool InitializeVbeModuleInfo();
        bool InitializeVbeModuleInfo(const std::string& lib_name, PVOID vbe_base);
        bool InitializeCapstoneEngine();
        special_module_info& GetSpecialModuleInfo() { return m_special_module_info; }
        bool Initialize(std::shared_ptr<CHipsCfgObject> configObject);
        static processing_status Preprocessing(CHookImplementObject::hook_node* node_elem, PVOID param_addr, PVOID return_addr, int& params_size, ULONG_PTR& func_return, DWORD entry_count, CLogHandle** __log);
        static processing_status Postprocessing(CHookImplementObject::hook_node* node_elem, PVOID param_addr, PVOID return_addr, int& params_size, ULONG_PTR& func_return, DWORD entry_count, CLogHandle** __log);
#ifdef _AMD64_
        static bool ForwardPropagationArgs(ULONG_PTR* new_addr, CHookImplementObject::REGISTERS* backup_regs, std::shared_ptr<CFunction> function, void* addr);
#endif
        void AddFilterThread(std::thread::id thread_id) { 
            std::lock_guard<std::recursive_mutex> lock_guard(m_filterids_mutex);
            if (m_filterids.find(thread_id) != m_filterids.end())
                return;
            m_filterids[thread_id] = filter_threadid;
            return;
        }
        void DelFilterThread(std::thread::id thread_id) {
            std::lock_guard<std::recursive_mutex> lock_guard(m_filterids_mutex);
            if (m_filterids.find(thread_id) != m_filterids.end())
            {
                if (m_filterids[thread_id] == filter_threadid)
                    m_filterids.erase(thread_id);
            }
            return;
        }
        bool IsFilterThread(std::thread::id thread_id) {
            std::lock_guard<std::recursive_mutex> lock_guard(m_filterids_mutex);
            if (m_filterids.find(thread_id) != m_filterids.end())
                return true;
            return false;
        }
        bool TraceModule();
        const std::unique_ptr<CDriverMgr>& GetDriverMgr() const { return m_drivermgr; }
        const std::unique_ptr<CategoryObject>& GetCategoryPtr() const { return m_category_ob_ptr; }
        bool ShouldbeHooked(const std::shared_ptr<CFunction>& function);
        MH_STATUS HookApi(hook_node& node);
        void RemoveApi(hook_node& node);
        bool HookAllApis();
        bool EnableAllApis() {
            if (!m_benable)
            {
                m_disable_hook = false;
                if (MH_EnableHook(MH_ALL_HOOKS) == MH_OK)
                    m_benable = true;
            }
            return m_benable;
        }
        bool DisableAllApis() {
            if (m_benable)
            {
                m_disable_hook = true;
                if (MH_DisableHook(MH_ALL_HOOKS) == MH_OK)
                    m_benable = false;
            }
            return !m_benable;
        }
        bool IsDisableHook() const { return m_disable_hook.load(); }
        auto& GetHookNodeList() { return m_hookNodeList; }
        auto& GetDelayNodeList() { return m_delayNodeList; }
        auto& GetRecursiveMutex() { return m_recursive_mutex; }
        const LPVOID GetHookImplementFunction() const { return m_hookImplementFunction; }
        const bool IsTopCallInThread() const {
            if (m_threadTlsCount == 0)
                return true;
            else
                return false;
        }
        const ULONG_PTR GetTlsValueForThreadIdx() const {
            if (readtls(TLS_TIB) != 0)
                return m_threadTlsCount++;
            else
                return 1;
        }
        const ULONG_PTR ReleaseTlsValueForThreadIdx() const {
            if (readtls(TLS_TIB) != 0) {
                if (m_threadTlsCount > 0)
                    return m_threadTlsCount--;
            }
            return 0;
        }
        void GetLastError(last_error_define& error) { 
            error.last_error = 0;
            error.nt_status = 0;
            if (!m_error_offset.m_win32_error_offset || !m_error_offset.m_nt_status_offset) return;
            error.last_error = *(DWORD *)(readtls(TLS_TEB) + m_error_offset.m_win32_error_offset);
            error.nt_status = *(DWORD *)(readtls(TLS_TEB) + m_error_offset.m_nt_status_offset);
            return;
        }
        void SetLastError(const last_error_define& error) { 
            if (!m_error_offset.m_win32_error_offset || !m_error_offset.m_nt_status_offset) return;
            *(DWORD *)(readtls(TLS_TEB) + m_error_offset.m_win32_error_offset) = error.last_error;
            *(DWORD *)(readtls(TLS_TEB) + m_error_offset.m_nt_status_offset) = error.nt_status;
            return;
        }
        bool HookProcessing();
        bool UnhookProcessing();
        bool GetbWmiProcessing() { return m_bwmi_success; }
        void ClearWmiInterfaceDefine() { m_wmi_interface_define.clear(); }

        static void CALLBACK detour_ldrDllNotification(ULONG reason, const LDR_DLL_NOTIFICATION_DATA *notification, void *param);
        // define static detour function on here.
        static processing_status WINAPI detour_ntQueryInformationProcess(detour_node* node, HANDLE ProcessHandle, DWORD ProcessInformationClass, LPVOID ProcessInformation, ULONG ProcessInformationLength, ULONG* ReturnLength);
        static processing_status WINAPI detour_coInitializeEx(detour_node* node, LPVOID pvReserved, DWORD dwCoInit);
        static processing_status WINAPI detour_coInitializeSecurity(detour_node* node, PSECURITY_DESCRIPTOR pSecDesc, LONG cAuthSvc, SOLE_AUTHENTICATION_SERVICE *asAuthSvc, void *pReserved1, DWORD dwAuthnLevel, DWORD dwImpLevel, void *pAuthList, DWORD dwCapabilities, void *pReserved3);
        static processing_status WINAPI detour_coUninitialize(detour_node* node);
        static processing_status WINAPI detour_getProcAddress(detour_node* node, HMODULE hModule, LPCSTR  lpProcName);
        static processing_status WINAPI detour_coCreateInstance(detour_node* node, REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID *ppv);
        static processing_status WINAPI detour_coCreateInstanceEx(detour_node* node,REFCLSID Clsid, IUnknown *punkOuter, DWORD dwClsCtx, COSERVERINFO *pServerInfo, DWORD dwCount, MULTI_QI *pResults);

        // define static detour for wmi object
        static processing_status STDMETHODCALLTYPE detour_IEnumWbemClassObject_Next(detour_node* node, IEnumWbemClassObject* This, long lTimeout, ULONG uCount, IWbemClassObject **apObjects, ULONG *puReturned);
        static processing_status STDMETHODCALLTYPE detour_IWbemClassObject_Get(detour_node* node, IWbemClassObject* This, LPCWSTR wszName, long lFlags, VARIANT *pVal, long *pType, long *plFlavor);
        static processing_status STDMETHODCALLTYPE detour_IWbemClassObject_Put(detour_node* node, IWbemClassObject* This, LPCWSTR wszName, long lFlags, VARIANT *pVal, long Type);
        static processing_status STDMETHODCALLTYPE detour_IWbemClassObject_Next(detour_node* node, IWbemClassObject* This, long lFlags, BSTR *strName, VARIANT *pVal, long *pType, long *plFlavor);
        static processing_status STDMETHODCALLTYPE detour_IWbemServices_ExecMethod(detour_node* node, IWbemServices* This, const BSTR strObjectPath, const BSTR strMethodName, long lFlags, IWbemContext *pCtx, IWbemClassObject *pInParams, IWbemClassObject **ppOutParams, IWbemCallResult **ppCallResult);
        static processing_status STDMETHODCALLTYPE detour_IWbemServices_ExecQuery(detour_node* node, IWbemServices* This, const BSTR strQueryLanguage, const BSTR strQuery, long lFlags, IWbemContext *pCtx, IEnumWbemClassObject **ppEnum);
        static processing_status STDMETHODCALLTYPE detour_IWbemServices_Post_ExecQuery(detour_node* node, IWbemServices* This, const BSTR strQueryLanguage, const BSTR strQuery, long lFlags, IWbemContext *pCtx, IEnumWbemClassObject **ppEnum);
        // define static detour for exploit
        static processing_status WINAPI detour_virtualAlloc(detour_node* node, LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect);
        static processing_status WINAPI detour_virtualAllocEx(detour_node* node, HANDLE hProcess, LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect);
        static processing_status WINAPI detour_virtualProtectEx(detour_node* node, HANDLE hProcess, LPVOID lpAddress, SIZE_T dwSize, DWORD flNewProtect, LPDWORD lpflOldProtect);
        static processing_status WINAPI detour_createProcessA(detour_node* node, LPCSTR lpApplicationName, LPSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCSTR lpCurrentDirectory, LPSTARTUPINFOA lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation);
        static processing_status WINAPI detour_createProcessW(detour_node* node, LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation);
        static processing_status WINAPI detour_createProcessInternalW(detour_node* node, HANDLE hToken, LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation, PHANDLE hNewToken);
        static processing_status WINAPI detour_dialogBoxParamA(detour_node* node, HINSTANCE hInstance, LPCSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam);
        static processing_status WINAPI detour_dialogBoxParamW(detour_node* node, HINSTANCE hInstance, LPCWSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam);
        static processing_status WINAPI detour_createFileA(detour_node* node, LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
        static processing_status WINAPI detour_createFileW(detour_node* node, LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
        static processing_status WINAPI detour_copyFileA(detour_node* node, LPCSTR lpExistingFileName, LPCSTR lpNewFileName, BOOL bFailIfExists);
        static processing_status WINAPI detour_copyFileW(detour_node* node, LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, BOOL bFailIfExists);
        static processing_status WINAPI detour_shellExecuteA(detour_node* node, HWND hwnd, LPCSTR lpOperation, LPCSTR lpFile, LPCSTR lpParameters, LPCSTR lpDirectory, INT nShowCmd);
        static processing_status WINAPI detour_shellExecuteW(detour_node* node, HWND hwnd, LPCWSTR lpOperation, LPCWSTR lpFile, LPCWSTR lpParameters, LPCWSTR lpDirectory, INT nShowCmd);
        static processing_status WINAPI detour_URLDownloadToFileA(detour_node* node, LPUNKNOWN pCaller, LPCSTR szURL, LPCSTR szFileName, DWORD dwReserved, LPBINDSTATUSCALLBACK lpfnCB);
        static processing_status WINAPI detour_URLDownloadToFileW(detour_node* node, LPUNKNOWN pCaller, LPCWSTR szURL, LPCWSTR szFileName, DWORD dwReserved, LPBINDSTATUSCALLBACK lpfnCB);
        static processing_status WINAPI detour_URLDownloadToCacheFileA(detour_node* node, LPUNKNOWN lpUnkcaller, LPCSTR szURL, LPCSTR szFileName, DWORD cchFileName, DWORD dwReserved, IBindStatusCallback *pBSC);
        static processing_status WINAPI detour_URLDownloadToCacheFileW(detour_node* node, LPUNKNOWN lpUnkcaller, LPCSTR szURL, LPCWSTR szFileName, DWORD cchFileName, DWORD dwReserved, IBindStatusCallback *pBSC);
        static processing_status WINAPI detour_winExec(detour_node* node, LPCSTR lpCmdLine, UINT uCmdShow);
        static processing_status WINAPI detour_loadLibraryA(detour_node* node, LPCSTR lpLibFileName);
        static processing_status WINAPI detour_loadLibraryW(detour_node* node, LPCWSTR lpLibFileName);
        static processing_status WINAPI detour_loadLibraryExA(detour_node* node, LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
        static processing_status WINAPI detour_loadLibraryExW(detour_node* node, LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
        static processing_status WINAPI detour_ntMapViewOfSection(detour_node* node, HANDLE SectionHandle, HANDLE ProcessHandle, PVOID *BaseAddress, ULONG ZeroBits, ULONG CommitSize, PLARGE_INTEGER SectionOffset, PULONG ViewSize, UINT InheritDisposition, ULONG AllocationType, ULONG Protect);
        static processing_status WINAPI detour_WSAStartup(detour_node* node, WORD wVersionRequired, LPWSADATA lpWSAData);
        static processing_status WINAPI detour_socket(detour_node* node, int af, int type, int protocol);
        static processing_status WINAPI detour_connect(detour_node* node, SOCKET s, const sockaddr *name, int namelen);

        // Use traditional hooks to handle heap APIs.
        static PVOID WINAPI detour_rtlAllocateHeap(PVOID HeapHandle, ULONG Flags, SIZE_T Size);
        static BOOLEAN WINAPI detour_rtlFreeHeap(PVOID HeapHandle, ULONG Flags, PVOID BaseAddress);
        static PVOID WINAPI detour_rtlDestroyHeap(PVOID HeapHandle);

        bool IsHookMode() const { 
            if (m_configObject)
                return m_configObject->IsHookMode();
            return false;
        }
        bool IsTraceMode() const {
            if (m_configObject)
                return m_configObject->IsTraceMode();
            return false;
        }
    private:
        MH_STATUS HookNormalApi(hook_node& node);
        MH_STATUS HookSpecialApi(hook_node& node);
        void RemoveNormalApi(hook_node& node);
        void RemoveSpecialApi(hook_node& node);
        bool InitializeWmiMethodsDefine(bool bdelayed = true);
        bool InitializeNotificationDefine() {
            if (m_notification_api_define.ldrregisterdllnotification_func) {
                PVOID Cookie = nullptr;
                NTSTATUS status = m_notification_api_define.ldrregisterdllnotification_func(0, detour_ldrDllNotification, nullptr, &Cookie);
                if (status >= 0)
                    return true;
            }
            return false;
        }

        bool m_bValid;
        std::atomic_bool m_benable;
        static std::atomic_bool m_disable_hook;
        std::atomic_flag m_bwmi_processing = ATOMIC_FLAG_INIT;
        bool m_bwmi_success;
        static const LPVOID m_hookImplementFunction;
        // the tls index for implement the function of re-entry calling check in the same thread
        static thread_local ULONG_PTR m_threadTlsCount;
        std::shared_ptr<CHipsCfgObject> m_configObject;
        std::vector<hook_node> m_hookNodeList;
        std::map<std::string, ULONG_PTR*> m_wmi_interface_define;
        ole32_api_define m_ole32_api_define = {};
        notification_api_define m_notification_api_define = {};
        static heap_allocation_define m_heap_allocation_define;
        std::recursive_mutex m_recursive_mutex;
        std::map<std::string, std::vector<int>> m_delayNodeList;
        std::recursive_mutex m_filterids_mutex;
        std::map<std::thread::id, _filter_type> m_filterids;
        static const std::unique_ptr<CExceptionObject> m_exceptionObject;
        std::unique_ptr< CDriverMgr> m_drivermgr;
        error_offset_define m_error_offset = {};
        special_module_info m_special_module_info = {};
        std::unique_ptr<CategoryObject> m_category_ob_ptr;
        const static std::vector<std::string> _vbe_module_name_def;
    };

    extern std::shared_ptr<CHookImplementObject> g_impl_object;

} // namespace cchips

