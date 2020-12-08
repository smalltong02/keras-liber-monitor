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
        bool IsDangerousCommand(std::string command);
    private:
        _category_type m_category;
        std::unique_ptr<RE2> m_category_pattern;
        std::unique_ptr<RE2> m_dangerous_pattern;

        static std::vector<std::pair<_category_type, std::string>> _category_str_def;
        static std::vector<std::pair<_category_type, std::string>> _dangerous_str_def;
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
        CheckExploitFuncs() = default;
        ~CheckExploitFuncs() = default;
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
} // namespace cchips
