#pragma once
#include <Windows.h>
#include <cstdio>
#include <string>
#include <vector>
#include <queue>
#include <map>
#include <memory>
#include <functional>
#include <variant>
#include "Psapi.h"
#include "re2\re2.h"
#include "utils.h"

namespace cchips {
    class CategoryObject
    {
    public:
        enum _category_type {
            category_normal = 0,
            category_browser,
            category_office,
            category_pdf,
            category_java,
            category_misc,
            category_test,
        };

        CategoryObject() : m_category(category_normal), m_category_pattern(nullptr), m_dangerous_pattern(nullptr) { InitGetCategory(); }
        ~CategoryObject() = default;
        void InitGetCategory();
        bool IsMatchCategory(const std::string& category_pattern) const;
        bool IsDangerousCommand(std::string& command) const;
    private:
        _category_type m_category;
        std::unique_ptr<RE2> m_category_pattern;
        std::unique_ptr<RE2> m_dangerous_pattern;

        static std::vector<std::pair<_category_type, std::string>> _category_str_def;
        static std::vector<std::pair<_category_type, std::string>> _dangerous_str_def;
        static std::vector<std::pair<_category_type, std::string>> _category_name_def;
    };

    class CheckExploitFuncs
    {
    public:
        enum _exploit_reason {
            e_reason_null = 0,
            e_reason_stack_corrupted,
            e_reason_run_on_non_image,
            e_reason_suspicious_process,
            e_reason_danger_process,
            e_reason_allocate_exec_stack,
            e_reason_disable_dep,
            e_reason_exception,
            e_reason_rop,
            e_reason_heap_spray,
            e_reason_app_abort,
            e_reason_unlock_vba,
            e_reason_vbe_suspicious,
        };
        enum _callInsnIds {
            X86_INS_CALL,
            X86_INS_LCALL,
        };
        using heap_block = struct {
            ULONG_PTR    addr;
            ULONG_PTR    base;
            ULONG_PTR    size;
            ULONG_PTR    bound;
            PVOID        handle;
        };
        CheckExploitFuncs() = delete;
        ~CheckExploitFuncs() = delete;
        static bool AddressInModule(ULONG_PTR addr, MODULEINFO mi);
        static bool CheckStackInstruction(ULONG_PTR stack);
        static bool ValidStackPointer(ULONG_PTR stack);
        static bool IsValidImage(ULONG_PTR addr);
        static _exploit_reason CheckROPAttack(ULONG_PTR addr);
        static _exploit_reason CheckReturnAddress(ULONG_PTR addr);
        static bool CheckHeapSpray(PVOID hHeap, PVOID base, ULONG_PTR size);
        static void FreeHeapSpray(PVOID hHeap, PVOID base);
        static void DestroyHeapSpray(PVOID hHeap);
    private:
        static bool ValidHeapBlock(heap_block* block) {
            if (!block) return false;
            if (block->addr == 0 ||
                block->base == 0 ||
                block->bound == 0 ||
                block->handle == nullptr ||
                block->size == 0)
                return false;
            return true;
        }
        static std::vector<heap_block> m_heap_block_table;
        static bool CheckNOPSlide(heap_block* block);
        static bool DoCheckHeapSpray(ULONG_PTR base, ULONG_PTR bound);
    };

    class CommonFuncsObject {
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
        CommonFuncsObject() = delete;
        ~CommonFuncsObject() = delete;

        using IsWow64Process_Define = BOOL(WINAPI*)(HANDLE, PBOOL);
        using GetNativeSystemInfo_Define = VOID(WINAPI*)(LPSYSTEM_INFO lpSystemInfo);

        static bool IsX64Process(HANDLE hprocess);
        static bool Is64BitOS();
        static bool IsMatchCurrentOS(const std::string& platform_pattern);
        static bool IsMatchProcType(const std::string& type_pattern);
        static bool IsDotnetOwner(void);
    private:
        static _proc_type m_proc_type;
        static _os_type m_os_type;
        static bool m_is_dotnet_owner;
        static IsWow64Process_Define m_lpfn_IsWow64Process;
        static GetNativeSystemInfo_Define m_lpfn_GetNativeSystemInfo;
        static std::vector<std::pair<_os_type, std::string>> _os_type_def;
        static std::vector<std::pair<_proc_type, std::string>> _proc_type_def;
    };

    class NativeObject {
    public:
        using _native_type = enum {
            native_sdt_func = 0,
        };
        struct CDeleter
        {
            SIZE_T data_size = 0;
            CDeleter() = default;
            CDeleter(size_t size) : CDeleter() { data_size = static_cast<SIZE_T>(size); }
            __forceinline void operator()(void* data)
            {
                //if (data_size) {
                //    VirtualFree(static_cast<LPVOID>(data), data_size, MEM_RELEASE);
                //}
            }
        };

        using NtQueryVirtualMemory_Define = NTSTATUS(WINAPI*)(HANDLE ProcessHandle,CONST VOID *BaseAddress, ULONG MemoryInformationClass, VOID *MemoryInformation, SIZE_T MemoryInformationLength, SIZE_T *ReturnLength);
        using NtAllocateVirtualMemory_Define = NTSTATUS(WINAPI*)(HANDLE ProcessHandle, VOID **BaseAddress, ULONG_PTR ZeroBits, SIZE_T *RegionSize, ULONG AllocationType, ULONG Protect);
        using NtFreeVirtualMemory_Define = NTSTATUS(WINAPI*)(HANDLE ProcessHandle, CONST VOID **BaseAddress, SIZE_T *RegionSize, ULONG FreeType);
        using NtProtectVirtualMemory_Define = NTSTATUS(WINAPI*)(HANDLE ProcessHandle, CONST VOID **BaseAddress, SIZE_T *NumberOfBytesToProtect, ULONG NewAccessProtection, ULONG *OldAccessProtection);
        using NtReadVirtualMemory_Define = NTSTATUS(WINAPI*)(HANDLE ProcessHandle, PVOID BaseAddress, PVOID Buffer, SIZE_T NumberOfBytesToRead, PSIZE_T NumberOfBytesReaded);

        static NativeObject& GetInstance()
        {
            static NativeObject m_instance;
            return m_instance;
        }

        static void *native_malloc(size_t size);
        static void *native_calloc(size_t nmemb, size_t size);
        static void *native_realloc(void *ptr, size_t size);
        static void native_free(void *ptr);

        std::uint8_t* GetNativeFunc(std::string_view func_name) {
            if (m_native_funcs_list.size() == 0) return nullptr;
            auto it = m_native_funcs_list.find(func_name);
            if (it == m_native_funcs_list.end())
                return nullptr;
            return it->second.get();
        }
        bool Success() const {
            if (m_native_funcs_list.size() != m_native_funcs.size())
                return false;
            for (const auto& func : m_native_funcs_list) {
                if (!func.first.length() || func.second == nullptr)
                    return false;
            }
            return true;
        }
    private:
        NativeObject();
        ~NativeObject() = default;
        NativeObject(const NativeObject&) = delete;
        NativeObject& operator=(const NativeObject&) = delete;
        bool AddNativeSDTFuncToList(std::string_view func_name, std::uint8_t* proc_address);

        static void *mem_malloc(size_t size);
        static void *mem_realloc(void *ptr, size_t size);
        static void mem_free(void *ptr);

        std::map<std::string_view, std::unique_ptr<std::uint8_t, CDeleter>> m_native_funcs_list;
        static const std::vector<std::pair<std::string, std::pair<_native_type, std::string>>> m_native_funcs;
        static NtAllocateVirtualMemory_Define _pfn_ntallocatevirtualmemory;
        static NtFreeVirtualMemory_Define _pfn_ntfreevirtualmemory;

    };

#define GetNativeObject() NativeObject::GetInstance()
} // namespace cchips
