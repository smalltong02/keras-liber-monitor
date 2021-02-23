#pragma once
#include <Windows.h>
#include <psapi.h>
#include <subauth.h>
#include <string>
#include <list>
#include <vector>
#include <map>
#include "SpecialLog.h"

namespace cchips {

    inline std::string to_byte_string(const std::wstring& input)
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        return converter.to_bytes(input);
    }

    inline std::wstring to_wide_string(const std::string& input)
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        return converter.from_bytes(input);
    }

    class CInjectProcess
    {
    public:
        using _os_type = enum {
            os_type_invalid = 0,
            os_type_x32 = 1,
            os_type_x64 = 2,
        };
        using _proc_type = enum {
            proc_type_invalid = 0,
            proc_type_x32 = 1,
            proc_type_x64 = 2,
        };
        using _LoadInformation = struct {
            FARPROC ldr_load_dll;
            FARPROC get_last_error;
            UNICODE_STRING replacedll;
        };
        typedef BOOL(WINAPI* define_IsWow64Process) (HANDLE, PBOOL);
        typedef VOID(WINAPI* define_GetNativeSystemInfo)(LPSYSTEM_INFO lpSystemInfo);
        typedef LONG(NTAPI* define_NtSuspendProcess)(IN HANDLE ProcessHandle);
        typedef LONG(NTAPI* define_NtResumeProcess)(IN HANDLE ProcessHandle);
        typedef NTSTATUS(NTAPI* define_LdrLoadDll)(IN PWCHAR PathToFile, IN ULONG Flags, IN PUNICODE_STRING ModuleFileName, OUT PHANDLE ModuleHandle);

        CInjectProcess(DWORD_PTR pid) : m_pid(pid), m_proc_type(proc_type_invalid), m_h_process(nullptr) {
            Initialize();
        }
        ~CInjectProcess() { 
            if (m_h_process)
            {
                CloseHandle(m_h_process);
            }
        }
        void Initialize() {
            m_h_process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_pid);
            if (!m_h_process) return;
            m_process_name = [](HANDLE& hprocess) ->std::string {
                char process_path[MAX_PATH] = {};
                if (GetModuleFileNameExA(hprocess, NULL, process_path, MAX_PATH) == 0)
                    return "";
                return process_path;
            }(m_h_process);

            if (!m_process_name.length())
            {
                CloseHandle(m_h_process);
                m_h_process = nullptr;
                return;
            }

            if (!Is64BitOS())
                m_proc_type = proc_type_x32;
            else
            {
                if (m_lpfn_IsWow64Process == nullptr)
                {
                    m_lpfn_IsWow64Process = (define_IsWow64Process)GetProcAddress(
                        GetModuleHandleA("kernel32"), "IsWow64Process");
                }

                if (m_lpfn_IsWow64Process)
                {
                    BOOL wow64_process;
                    if (m_lpfn_IsWow64Process(m_h_process, &wow64_process))
                    {
                        // It is maybe incorrect on ARM.
                        if (wow64_process)
                            m_proc_type = proc_type_x32;
                        else
                            m_proc_type = proc_type_x64;
                        return;
                    }
                }
                CloseHandle(m_h_process);
                m_h_process = nullptr;
            }
        }
        bool Suspend() {
            if (IsValid())
            {
                if (m_lpfn_NtSuspendProcess == nullptr)
                {
                    m_lpfn_NtSuspendProcess = (define_NtSuspendProcess)GetProcAddress(
                        GetModuleHandleA("ntdll"), "NtSuspendProcess");
                }

                if (m_lpfn_NtSuspendProcess)
                {
                    if (m_lpfn_NtSuspendProcess(m_h_process) == 0) // NtStatus = STATUS_SUCCESS
                        return true;
                }
            }
            return false;
        }
        bool Resume() {
            if (IsValid())
            {
                if (m_lpfn_NtResumeProcess == nullptr)
                {
                    m_lpfn_NtResumeProcess = (define_NtResumeProcess)GetProcAddress(
                        GetModuleHandleA("ntdll"), "NtResumeProcess");
                }

                if (m_lpfn_NtResumeProcess)
                {
                    if (m_lpfn_NtResumeProcess(m_h_process) == 0) // NtStatus = STATUS_SUCCESS
                        return true;
                }
            }
            return false;
        }
        bool Terminate() {
            if (IsValid())
            {
                if (TerminateProcess(m_h_process, 0))
                {
                    CloseHandle(m_h_process);
                    m_h_process = nullptr;
                    return true;
                }
            }
            return false;
        }
        bool ReadData(const void* addr, void* data, size_t length) const {
            if (IsValid())
            {
                DWORD_PTR bytes_read;
                if (ReadProcessMemory(m_h_process, addr, data, length,
                    &bytes_read) == FALSE || bytes_read != length) {
                    return false;
                }
                return true;
            }
            return false;
        }
        void* WriteData(const void* data, size_t length) const {
            if (IsValid())
            {
                void *addr = VirtualAllocEx(m_h_process, NULL, length,
                    MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
                if (addr == NULL) {
                    return nullptr;
                }

                DWORD_PTR bytes_written;
                if (WriteProcessMemory(m_h_process, addr, data, length,
                    &bytes_written) == FALSE || bytes_written != length) {
                    FreeData(addr, length);
                    return nullptr;
                }
                return addr;
            }
            return nullptr;
        }
        bool FreeData(void* addr, size_t length) const {
            if (IsValid())
            {
                if (addr != nullptr && length != 0) {
                    VirtualFreeEx(m_h_process, addr, length, MEM_RELEASE);
                    return true;
                }
            }
            return false;
        }
        bool InjectRemoteThread(PAPCFUNC shellcode_addr, ULONG_PTR shellcode_data) {
            if (IsValid())
            {
                HANDLE remote_thread = CreateRemoteThread(m_h_process, NULL, 0, (LPTHREAD_START_ROUTINE)shellcode_addr, (void*)shellcode_data, CREATE_SUSPENDED, NULL);
                if (remote_thread == nullptr)
                    return false;
                ResumeThread(remote_thread);
                /*DWORD ret = */WaitForSingleObject(remote_thread, INFINITE);
                //if (ret == WAIT_TIMEOUT)
                //    return false;
                return true;
            }
            return false;
        }
        //bool InjectApcToMainThread(PAPCFUNC shellcode_addr, ULONG_PTR shellcode_data) {
        //    if (IsValid())
        //    {
        //        if (QueueUserAPC((PAPCFUNC)shellcode_addr, m_h_thread,
        //            (ULONG_PTR)shellcode_data) == 0)
        //            return false;
        //    }
        //    return true;
        //}
        _proc_type GetProcType() const {
            if (IsValid())
                return m_proc_type;
            return proc_type_invalid;
        }
        static bool Is64BitOS() {
            if (m_os_type == os_type_x64) return true;
            else if (m_os_type == os_type_x32) return false;

            SYSTEM_INFO si;
            if (!m_lpfn_GetNativeSystemInfo)
                m_lpfn_GetNativeSystemInfo = (define_GetNativeSystemInfo)GetProcAddress(GetModuleHandleA("kernel32.dll"), "GetNativeSystemInfo");
            if (m_lpfn_GetNativeSystemInfo != nullptr)
            {
                m_lpfn_GetNativeSystemInfo(&si);

                if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ||
                    si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)
                {
                    m_os_type = os_type_x64;
                    return true;
                }
            }
            m_os_type = os_type_x32;
            return false;
        }
        static const void * GetRealAddress(const void *func) {
            LPBYTE code = (LPBYTE)func;
            if (code[0] != 0xE9)
                return func;
            int rel_addr = *(int *)(code + 1);
            return (const void *)((ULONG_PTR)func + rel_addr + 5);
        }
        static DWORD LoadLibraryWorker(_LoadInformation* load_info) {
            HANDLE hmodule;
            DWORD ret = 0;
            if (load_info->get_last_error != nullptr && load_info->ldr_load_dll != nullptr && load_info->replacedll.Buffer != nullptr)
            {
                if (((define_LdrLoadDll)load_info->ldr_load_dll)(NULL, 0, &load_info->replacedll, &hmodule) == 0)
                    ret = load_info->get_last_error();
            }
            return ret;
        }
        const std::vector<BYTE>& GetShellCode() const {
            if (m_proc_type == proc_type_x64)
                return m_shellcode_LoadLibrary_64;
            return m_shellcode_LoadLibrary_32;
        }
        bool IsWow64Process(HANDLE hprocess, BOOL& wow64_process) {
            if (!m_lpfn_IsWow64Process)
                return false;
            return m_lpfn_IsWow64Process(hprocess, &wow64_process);
        }
    private:
        bool IsValid() const {
            if (m_h_process != nullptr)
                return true;
            return false;
        }

        DWORD_PTR m_pid;
        HANDLE m_h_process;
        std::string m_process_name;
        _proc_type m_proc_type;
        static _os_type m_os_type;
        static define_IsWow64Process m_lpfn_IsWow64Process;
        static define_GetNativeSystemInfo m_lpfn_GetNativeSystemInfo;
        static define_NtSuspendProcess m_lpfn_NtSuspendProcess;
        static define_NtResumeProcess m_lpfn_NtResumeProcess;
        static FARPROC m_lpfn_LdrLoadDll;
        static FARPROC m_lpfn_GetLastError;
        const static std::vector<BYTE> m_shellcode_LoadLibrary_32;
        const static std::vector<BYTE> m_shellcode_LoadLibrary_64;
    };

    class CDllInjectorObject
    {
    public:
        using _inject_mode = enum {
            inject_invalid = 0,
            inject_remotethread = 1,
            inject_apc,
        };
        using _dll_type = enum {
            dll_type_x32 = 0,
            dll_type_x64 = 1,
            dll_type_wow64 = 2,
        };
        struct _InjectDllInfo {
            _InjectDllInfo() = default;
            _InjectDllInfo(_inject_mode mode, _dll_type type, const std::string& dll_string) : inject_mode(mode), dll_type(type), replace_dll(dll_string) { }
            _inject_mode inject_mode;
            _dll_type dll_type;
            std::string replace_dll;
        };

        CDllInjectorObject() {
            if(!m_lpfn_LdrLoadDll)
                m_lpfn_LdrLoadDll = (FARPROC)GetProcAddress(GetModuleHandleA("ntdll.dll"), "LdrLoadDll");
            if(!m_lpfn_GetLastError)
                m_lpfn_GetLastError = (FARPROC)GetProcAddress(GetModuleHandleA("ntdll.dll"), "RtlGetLastWin32Error");
        }
        ~CDllInjectorObject() = default;

        void InitializeSamplePath(const std::string& path) { m_sample_path = path; }
        void AddDllInfo(std::unique_ptr<_InjectDllInfo>& p_dll_info) { 
            if (!p_dll_info) return;
            m_injectinfo_list.push_back(std::move(p_dll_info));
            return;
        }
        const std::string GetDllPath(_dll_type dll_type) const {
            for (const auto& info : m_injectinfo_list)
            {
                if (info->dll_type == dll_type)
                    return info->replace_dll;
            }
            return std::string("unknown.dll");
        }
        bool InjectProcess(DWORD_PTR pid, _inject_mode mode = inject_remotethread) {
            if (!m_lpfn_LdrLoadDll) return false;
            if (!m_lpfn_GetLastError) return false;
            auto it = m_injectproc_list.find(pid);
            // check if process has been injected.
            if (it != m_injectproc_list.end()) return true;
            std::unique_ptr<CInjectProcess> proc_ptr = std::make_unique<CInjectProcess>(pid);
            if (!proc_ptr) return false;
            BOOL wow64_process = FALSE;

            if(proc_ptr->Is64BitOS())   // current don't allow x32 -> x64 or x64 -> x32.
            {
                if (proc_ptr->IsWow64Process(GetCurrentProcess(), wow64_process))
                {
                    if (!wow64_process && proc_ptr->GetProcType() == CInjectProcess::proc_type_x32)
                        return false;
                    if (wow64_process && proc_ptr->GetProcType() == CInjectProcess::proc_type_x64)
                        return false;
                }
            }
            std::string inject_dll_name = [](std::vector<std::unique_ptr<_InjectDllInfo>>& list, std::unique_ptr<CInjectProcess>& proc) ->std::string {
                for (const auto& it : list)
                {
                    if (CInjectProcess::Is64BitOS())
                    {
                        if (proc->GetProcType() == CInjectProcess::proc_type_x32)
                        {
                            if (it->dll_type == dll_type_wow64)
                                return it->replace_dll;
                        }
                        else if (proc->GetProcType() == CInjectProcess::proc_type_x64)
                        {
                            if (it->dll_type == dll_type_x64)
                                return it->replace_dll;
                        }
                    }
                    else
                    {
                        if (it->dll_type == dll_type_x32)
                            return it->replace_dll;
                    }
                }
                return "";
            }(m_injectinfo_list, proc_ptr);
            if (!inject_dll_name.length()) return false;
            if (!m_lpfn_LdrLoadDll || !m_lpfn_GetLastError) return false;

            CInjectProcess::_LoadInformation load_info = {};
            load_info.ldr_load_dll = m_lpfn_LdrLoadDll;
            load_info.get_last_error = m_lpfn_GetLastError;
            std::wstring wide_name = cchips::to_wide_string(inject_dll_name);
            if (!wide_name.length()) return false;
            load_info.replacedll.Length = (USHORT)(wide_name.length() * sizeof(wchar_t));
            load_info.replacedll.MaximumLength = (USHORT)((wide_name.length() + 1) * sizeof(wchar_t));
            load_info.replacedll.Buffer = (wchar_t*)proc_ptr->WriteData(wide_name.c_str(), load_info.replacedll.MaximumLength);
            if (!load_info.replacedll.Buffer) return false;
            void* shellcode_data = proc_ptr->WriteData(&load_info, sizeof(load_info));
            if (!shellcode_data) { proc_ptr->FreeData(load_info.replacedll.Buffer, load_info.replacedll.MaximumLength); return false; }
            void* shellcode_addr = proc_ptr->WriteData(&proc_ptr->GetShellCode()[0], 0x1000);
            if (!shellcode_addr) { proc_ptr->FreeData(load_info.replacedll.Buffer, load_info.replacedll.MaximumLength); proc_ptr->FreeData(shellcode_data, sizeof(load_info)); return false; }
            if (!proc_ptr->InjectRemoteThread((PAPCFUNC)shellcode_addr, (ULONG_PTR)shellcode_data))
            {
                proc_ptr->FreeData(load_info.replacedll.Buffer, load_info.replacedll.MaximumLength); 
                proc_ptr->FreeData(shellcode_data, sizeof(load_info));
                proc_ptr->FreeData(shellcode_addr, 0x1000);
                return false;
            }
            m_injectproc_list[pid] = std::move(proc_ptr);
            return true;
        }
    private:
        std::vector<std::unique_ptr<_InjectDllInfo>> m_injectinfo_list;
        std::map<DWORD_PTR, std::unique_ptr<CInjectProcess>> m_injectproc_list;
        std::string m_sample_path;  // maybe it is not a exe
        static FARPROC m_lpfn_LdrLoadDll;
        static FARPROC m_lpfn_GetLastError;
    };

} // namespace cchips
