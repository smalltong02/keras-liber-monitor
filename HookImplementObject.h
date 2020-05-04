#pragma once
#include <stdio.h>
#include <windows.h>
#include <string>
#include <vector>
#include "HipsCfgObject.h"
#include "SigsObject.h"
#include "MinHook.h"
#include "hookimpl.h"
#include "LogObject.h"
#include "SafePtr.h"
#include "asmfunction.h"
#include "ExceptionThrow.h"

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

#define MACRO_PUSH_PARAMS_(__pnode, __params, __psize, __post_func)   \
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
    __asm {pop  ebx}                            

#define MACRO_CALL_ORGINAL_(__params, __psize, __call_conv, __func, __return) \
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
    __asm {mov  eax, __func}                    \
    __asm {call eax}                            \
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
#define MACRO_PUSH_PARAMS_(__pnode, __params, __psize, __post_func) _callPreProcessing_x64((void*)__pnode, (ULONG_PTR*)__params, __psize, (void*)__post_func);
#define MACRO_CALL_ORGINAL_(__params, __psize, __call_conv, __func, __return) _callOrginApiFunction_x64((ULONG_PTR*)__params, __psize, (int)__call_conv, __func, __return);
#endif // #ifdef _AMD64

    class CHookImplementObject : public CFunctionProtos, public CWmiObjectProtos, public std::enable_shared_from_this<CHookImplementObject>
    {
    public:
#define ADD_PRE_PROCESSING(NAME, FUNC) AddPreProcessing(#NAME, reinterpret_cast<CFunction::func_proto>(FUNC))
#define ADD_POST_PROCESSING(NAME, FUNC) AddPostProcessing(#NAME, reinterpret_cast<CFunction::func_proto>(FUNC))
#define DEL_PRE_PROCESSING(NAME, FUNC) DelPreProcessing(#NAME, reinterpret_cast<CFunction::func_proto>(FUNC))
#define DEL_POST_PROCESSING(NAME, FUNC) DelPostProcessing(#NAME, reinterpret_cast<CFunction::func_proto>(FUNC))

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
        CHookImplementObject() :
            m_bValid(false), m_benable(false), m_configObject(nullptr) {
            CoInitializeEx(0, COINIT_MULTITHREADED);
        }//for hook wmi methods
        ~CHookImplementObject() {
            if (m_threadTlsIdx != TLS_OUT_OF_INDEXES)
                TlsFree(m_threadTlsIdx);
            DisableAllApis();
            UnhookProcessing();
            MH_Uninitialize();
            CoUninitialize();
            //waiting all __hook_node->shared_count == 0
        }
        using IEnumWbemClassObject_Next_Define = HRESULT(*)(IEnumWbemClassObject* This, long lTimeout, ULONG uCount, IWbemClassObject **apObjects, ULONG *puReturned);
        using IWbemClassObject_Get_Define = HRESULT(*)(IWbemClassObject* This, LPCWSTR wszName, long lFlags, VARIANT *pVal, long *pType, long *plFlavor);
        using IWbemClassObject_Put_Define = HRESULT(*)(IWbemClassObject* This, LPCWSTR wszName, long lFlags, VARIANT *pVal, long Type);
        using IWbemClassObject_Next_Define = HRESULT(*)(IWbemClassObject* This, long lFlags, BSTR *strName, VARIANT *pVal, long *pType, long *plFlavor);
        using IWbemServices_ExecMethod_Define = HRESULT(*)(IWbemServices* This, const BSTR strObjectPath, const BSTR strMethodName, long lFlags, IWbemContext *pCtx, PVOID pInParams, IWbemClassObject **ppOutParams, IWbemCallResult **ppCallResult);
        using IWbemServices_ExecQuery_Define = HRESULT(*)(IWbemServices* This, const BSTR strQueryLanguage, const BSTR strQuery, long lFlags, IWbemContext *pCtx, IEnumWbemClassObject **ppEnum);

        using detour_node = struct _detour_node {
            PVOID return_va;
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
        };

        using wmi_methods_define = struct {
            IEnumWbemClassObject_Next_Define enumwbem_next_func;
            IWbemClassObject_Get_Define wbem_get_func;
            IWbemClassObject_Put_Define wbem_put_func;
            IWbemClassObject_Next_Define wbem_next_func;
            IWbemServices_ExecMethod_Define wbem_execmethod_func;
            IWbemServices_ExecQuery_Define wbem_execquery_func;
        };

        bool Initialize(std::shared_ptr<CHipsCfgObject>& configObject);
        static processing_status Preprocessing(CHookImplementObject::hook_node* node_elem, PVOID param_addr, int& params_size, ULONG_PTR& func_return, DWORD entry_count, CLogHandle** __log);
        static processing_status Postprocessing(CHookImplementObject::hook_node* node_elem, PVOID param_addr, int& params_size, ULONG_PTR& func_return, DWORD entry_count, CLogHandle** __log);
#ifdef _AMD64_
        static bool ForwardPropagationArgs(ULONG_PTR* new_addr, CHookImplementObject::REGISTERS* backup_regs, std::shared_ptr<CFunction> function, void* addr);
#endif
        bool HookAllApis();
        bool EnableAllApis() {
            if (!m_benable)
            {
                if (MH_EnableHook(MH_ALL_HOOKS) == MH_OK)
                    m_benable = true;
            }
            return m_benable;
        }
        bool DisableAllApis() {
            if (m_benable)
            {
                if (MH_DisableHook(MH_ALL_HOOKS) == MH_OK)
                    m_benable = false;
            }
            return !m_benable;
        }
        auto& GetHookNodeList() { return m_hookNodeList; }
        auto& GetDelayNodeList() { return m_delayNodeList; }
        const LPVOID GetHookImplementFunction() const { return m_hookImplementFunction; }
        const bool IsTopCallInThread() const {
            ULONG_PTR Index = reinterpret_cast<ULONG_PTR>(TlsGetValue(m_threadTlsIdx));
            if (Index == 0)
                return true;
            else
                return false;
        }
        const ULONG_PTR GetTlsValueForThreadIdx() const {
            ULONG_PTR Index = reinterpret_cast<ULONG_PTR>(TlsGetValue(m_threadTlsIdx));
            TlsSetValue(m_threadTlsIdx, reinterpret_cast<LPVOID>(Index + 1));
            return Index;
        }
        const ULONG_PTR ReleaseTlsValueForThreadIdx() const {
            ULONG_PTR Index = reinterpret_cast<ULONG_PTR>(TlsGetValue(m_threadTlsIdx));
            if (Index > 0) TlsSetValue(m_threadTlsIdx, reinterpret_cast<LPVOID>(Index - 1));
            return Index;
        }
        bool HookProcessing();
        bool UnhookProcessing();

        // define static detour function on here.
        static processing_status WINAPI detour_loadLibraryA(detour_node* node, LPCSTR lpLibFileName);
        static processing_status WINAPI detour_loadLibraryW(detour_node* node, LPCWSTR lpLibFileName);
        static processing_status WINAPI detour_loadLibraryExA(detour_node* node, LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
        static processing_status WINAPI detour_loadLibraryExW(detour_node* node, LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
        static processing_status WINAPI detour_ntQueryInformationProcess(detour_node* node, HANDLE ProcessHandle, DWORD ProcessInformationClass, LPVOID ProcessInformation, ULONG ProcessInformationLength, ULONG* ReturnLength);

        // define static detour for wmi object
        bool HookWmiObjectMethods(int& count);
        static processing_status STDMETHODCALLTYPE detour_IEnumWbemClassObject_Next(detour_node* node, IEnumWbemClassObject* This, long lTimeout, ULONG uCount, IWbemClassObject **apObjects, ULONG *puReturned);
        static processing_status STDMETHODCALLTYPE detour_IWbemClassObject_Get(detour_node* node, IWbemClassObject* This, LPCWSTR wszName, long lFlags, VARIANT *pVal, long *pType, long *plFlavor);
        static processing_status STDMETHODCALLTYPE detour_IWbemClassObject_Put(detour_node* node, IWbemClassObject* This, LPCWSTR wszName, long lFlags, VARIANT *pVal, long Type);
        static processing_status STDMETHODCALLTYPE detour_IWbemClassObject_Next(detour_node* node, IWbemClassObject* This, long lFlags, BSTR *strName, VARIANT *pVal, long *pType, long *plFlavor);
        static processing_status STDMETHODCALLTYPE detour_IWbemServices_ExecMethod(detour_node* node, IWbemServices* This, const BSTR strObjectPath, const BSTR strMethodName, long lFlags, IWbemContext *pCtx, IWbemClassObject *pInParams, IWbemClassObject **ppOutParams, IWbemCallResult **ppCallResult);
        static processing_status STDMETHODCALLTYPE detour_IWbemServices_ExecQuery(detour_node* node, IWbemServices* This, const BSTR strQueryLanguage, const BSTR strQuery, long lFlags, IWbemContext *pCtx, IEnumWbemClassObject **ppEnum);
    private:
        const static int IEnumWbemClassObject_Next_Vtbl_Index = 4;
        const static int IWbemServices_ExecQuery_Vtbl_Index = 20;
        const static int IWbemServices_ExecMethod_Vtbl_Index = 24;
        const static int IWbemClassObject_Get_Vtbl_Index = 4;
        const static int IWbemClassObject_Put_Vtbl_Index = 5;
        const static int IWbemClassObject_Next_Vtbl_Index = 9;
        int GetWmiMethodsDefineSize() { return (sizeof(wmi_methods_define) / sizeof(PVOID)); }
        bool InitializeWmiMethodsDefine();

        bool m_bValid;
        bool m_benable;
        static const LPVOID m_hookImplementFunction;
        // the tls index for implement the function of re-entry calling check in the same thread
        static const DWORD m_threadTlsIdx;
        std::shared_ptr<CHipsCfgObject> m_configObject;
        std::vector<hook_node> m_hookNodeList;
        wmi_methods_define m_wmi_methods_define = {};
        sf::contfree_safe_ptr<std::map<std::string, std::vector<int>>> m_delayNodeList;
        static const std::unique_ptr<CExceptionObject> m_exceptionObject;
    };

    extern std::shared_ptr<CHookImplementObject> g_impl_object;

} // namespace cchips

