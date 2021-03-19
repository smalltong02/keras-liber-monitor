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

namespace cchips {

    typedef enum _MEMORY_INFORMATION_CLASS {
        MemoryBasicInformation
    } MEMORY_INFORMATION_CLASS;

    enum class Cfg_view_flags
    {
        cfg_simple,
        cfg_info,
        cfg_detail,
        cfg_all,
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

        using NtQueryVirtualMemory_Define = NTSTATUS(WINAPI*)(HANDLE ProcessHandle, CONST VOID *BaseAddress, ULONG MemoryInformationClass, VOID *MemoryInformation, SIZE_T MemoryInformationLength, SIZE_T *ReturnLength);
        using NtAllocateVirtualMemory_Define = NTSTATUS(WINAPI*)(HANDLE ProcessHandle, VOID **BaseAddress, ULONG_PTR ZeroBits, SIZE_T *RegionSize, ULONG AllocationType, ULONG Protect);
        using NtFreeVirtualMemory_Define = NTSTATUS(WINAPI*)(HANDLE ProcessHandle, CONST VOID **BaseAddress, SIZE_T *RegionSize, ULONG FreeType);
        using NtProtectVirtualMemory_Define = NTSTATUS(WINAPI*)(HANDLE ProcessHandle, CONST VOID **BaseAddress, SIZE_T *NumberOfBytesToProtect, ULONG NewAccessProtection, ULONG *OldAccessProtection);
        using NtReadVirtualMemory_Define = NTSTATUS(WINAPI*)(HANDLE ProcessHandle, PVOID BaseAddress, PVOID Buffer, SIZE_T NumberOfBytesToRead, PSIZE_T NumberOfBytesReaded);
        using NtWriteVirtualMemory_Define = NTSTATUS(WINAPI*)(HANDLE ProcessHandle, PVOID BaseAddress, PVOID Buffer, SIZE_T NumberOfBytesToWrite, PSIZE_T NumberOfBytesWritten);

        static NativeObject& GetInstance()
        {
            static NativeObject m_instance;
            return m_instance;
        }

        static void *native_malloc(size_t size);
        static void *native_calloc(size_t nmemb, size_t size);
        static void *native_realloc(void *ptr, size_t size);
        static void native_free(void *ptr);
        static bool native_query(void *ptr, void* mem_info, size_t length, size_t *return_len);
        static bool native_protect(void *ptr, size_t bytes, unsigned long protection, unsigned long *old_protection);
        static bool native_read(void *ptr, void* buffer, size_t bytes, size_t *return_bytes);
        static bool native_write(void *ptr, void* buffer, size_t bytes, size_t *return_bytes);

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
        static bool mem_query(void *ptr, void* mem_info, size_t length, size_t *return_len);
        static bool mem_protect(void *ptr, size_t bytes, unsigned long protection, unsigned long *old_protection);
        static bool mem_read(void *ptr, void* buffer, size_t bytes, size_t *return_bytes);
        static bool mem_write(void *ptr, void* buffer, size_t bytes, size_t *return_bytes);

        std::map<std::string_view, std::unique_ptr<std::uint8_t, CDeleter>> m_native_funcs_list;
        static const std::vector<std::pair<std::string, std::pair<_native_type, std::string>>> m_native_funcs;
        static NtAllocateVirtualMemory_Define _pfn_ntallocatevirtualmemory;
        static NtFreeVirtualMemory_Define _pfn_ntfreevirtualmemory;
        static NtQueryVirtualMemory_Define _pfn_ntqueryvirtualmemory;
        static NtProtectVirtualMemory_Define _pfn_ntprotectvirtualmemory;
        static NtReadVirtualMemory_Define _pfn_ntreadvirtualmemory;
        static NtWriteVirtualMemory_Define _pfn_ntwritevirtualmemory;
    };

#define GetNativeObject() NativeObject::GetInstance()
} // namespace cchips
