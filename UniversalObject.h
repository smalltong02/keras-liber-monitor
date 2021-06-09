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
#include <set>
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
            category_ps,
            category_test,
        };

        CategoryObject() : m_category(category_normal), m_category_pattern(nullptr), m_dangerous_pattern(nullptr) { InitGetCategory(); }
        ~CategoryObject() = default;
        void InitGetCategory();
        bool IsMatchCategory(const std::string& category_pattern) const;
        bool IsDangerousCommand(std::string& command) const;
        _category_type GetCurrentCategory() const { return m_category; }
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

        typedef struct {
            bool operator()(const GUID& lhs, const GUID& rhs) const {
                return (memcmp(&lhs, &rhs, sizeof(GUID)) > 0 ? true : false);
            }
        }guid_compare;
        typedef std::set<GUID, guid_compare> SetGUID;

        CommonFuncsObject() = delete;
        ~CommonFuncsObject() = delete;

        using IsWow64Process_Define = BOOL(WINAPI*)(HANDLE, PBOOL);
        using GetNativeSystemInfo_Define = VOID(WINAPI*)(LPSYSTEM_INFO lpSystemInfo);

        static bool IsX64Process(HANDLE hprocess);
        static bool Is64BitOS();
        static bool IsMatchCurrentOS(const std::string& platform_pattern);
        static bool IsMatchProcType(const std::string& type_pattern);
        static bool IsDotnetOwner(void);
        static bool IsKnownWMIClsid(const GUID& clsid);
        static std::string GetComServicePath(const std::string& local_server32);
        static std::string GetLocalServer32FromClsid(const std::string& clsid);
        static std::string GetInprocServer32FromClsid(const std::string& clsid);
    private:
        static _proc_type m_proc_type;
        static _os_type m_os_type;
        static bool m_is_dotnet_owner;
        static IsWow64Process_Define m_lpfn_IsWow64Process;
        static GetNativeSystemInfo_Define m_lpfn_GetNativeSystemInfo;
        static std::vector<std::pair<_os_type, std::string>> _os_type_def;
        static std::vector<std::pair<_proc_type, std::string>> _proc_type_def;
        static SetGUID _known_wmi_clsid_def;
    };
} // namespace cchips
