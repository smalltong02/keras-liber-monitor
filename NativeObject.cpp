#include "stdafx.h"
#include <SubAuth.h>
#include <algorithm>
#include <WbemDisp.h>
#include <WbemProv.h>
#include <WbemTran.h>
#include "NativeObject.h"
#include "CapstoneImpl.h"
#include "ExceptionThrow.h"
#include "CBaseType.h"

namespace cchips {

    const std::vector<std::pair<std::string, std::pair<NativeObject::_native_type, std::string>>> NativeObject::m_native_funcs = {
    {"ntdll", {native_sdt_func, "NtAllocateVirtualMemory"}},
    {"ntdll", {native_sdt_func, "NtFreeVirtualMemory"}},
    {"ntdll", {native_sdt_func, "NtProtectVirtualMemory"}},
    {"ntdll", {native_sdt_func, "NtReadVirtualMemory"}},
    {"ntdll", {native_sdt_func, "NtQueryVirtualMemory"}},
    };
    NativeObject::NtAllocateVirtualMemory_Define NativeObject::_pfn_ntallocatevirtualmemory = nullptr;
    NativeObject::NtFreeVirtualMemory_Define NativeObject::_pfn_ntfreevirtualmemory = nullptr;

    NativeObject::NativeObject()
    {
        if (!cchips::GetCapstoneImplment().IsValid())
            return;

        std::for_each(m_native_funcs.begin(), m_native_funcs.end(), [&](const auto& func_pair) {
            HMODULE hmodule = GetModuleHandle(func_pair.first.c_str());
            if (hmodule) {
                NativeObject::_native_type type = func_pair.second.first;
                std::uint8_t* proc_address = reinterpret_cast<std::uint8_t*>(GetProcAddress(hmodule, func_pair.second.second.c_str()));
                if (proc_address) {

                    switch (type) {
                    case native_sdt_func:
                    {
                        AddNativeSDTFuncToList(func_pair.second.second, proc_address);
                    }
                    break;
                    default:
                        ;
                    }
                }
            }
            });
        _pfn_ntallocatevirtualmemory = reinterpret_cast<NtAllocateVirtualMemory_Define>(GetNativeFunc("NtAllocateVirtualMemory"));
        _pfn_ntfreevirtualmemory = reinterpret_cast<NtFreeVirtualMemory_Define>(GetNativeFunc("NtFreeVirtualMemory"));
        return;
    }

    bool NativeObject::AddNativeSDTFuncToList(std::string_view func_name, std::uint8_t* proc_address)
    {
        uint8_t *code = proc_address;
        size_t code_size = 100;
        size_t insn_size_sum = 0;
        uint64_t address = reinterpret_cast<std::uint64_t>(proc_address);
        std::unique_ptr<CapInsn> insn = std::make_unique<CapInsn>(cchips::GetCapstoneImplment().GetCapHandle());
        while (cchips::GetCapstoneImplment().CsDisasmIter(const_cast<const std::uint8_t**>(reinterpret_cast<std::uint8_t**>(&code)), &code_size, &address, *insn)) {
            insn_size_sum += insn->size();
            if (insn->GetInsnId() == X86_INS_JNE) {
                if (cchips::GetCapstoneImplment().OpInCount(*insn, X86_OP_IMM)) {
                    int index = cchips::GetCapstoneImplment().OpInIndex(*insn, X86_OP_IMM, 1);
                    if (insn->self()->detail) {
                        cs_x86* x86 = &insn->self()->detail->x86;
                        insn_size_sum += (x86->operands[index].imm - address);
                        code_size -= (x86->operands[index].imm - address);
                        code = reinterpret_cast<std::uint8_t*>(x86->operands[index].imm);
                        address = x86->operands[index].imm;
                    }
                }
            }
            else {
                if (cchips::GetCapstoneImplment().InsnInGroup(*insn, X86_GRP_RET)) {
                    std::unique_ptr<std::uint8_t, CDeleter> ptr(reinterpret_cast<std::uint8_t*>(VirtualAlloc(NULL, insn_size_sum,
                        MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE)), CDeleter(insn_size_sum));
                    if (!ptr) return false;
                    memcpy(ptr.get(), proc_address, insn_size_sum);
                    unsigned long old_protect;
                    if (!VirtualProtect(ptr.get(), insn_size_sum, PAGE_EXECUTE_READ, &old_protect))
                        return false;
                    if (ptr) m_native_funcs_list[func_name] = std::move(ptr);
                    return true;
                }
            }
        }
        return false;
    }

    void* NativeObject::mem_malloc(size_t size)
    {
        if (size == 0) {
            return nullptr;
        }
        size_t real_length = size + sizeof(uintptr_t);
        ASSERT(_pfn_ntallocatevirtualmemory);
        void *ptr = nullptr;
        NTSTATUS status = _pfn_ntallocatevirtualmemory(GetCurrentProcess(), &ptr, 0, reinterpret_cast<SIZE_T*>(&real_length), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        if (NT_SUCCESS(status)) {
            if (ptr == nullptr)
                return nullptr;
        }
        else {
            return nullptr;
        }
        memset(ptr, 0, real_length);
        *(uintptr_t *)ptr = size;
        return (uintptr_t *)ptr + 1;
    }

    void* NativeObject::mem_realloc(void *ptr, size_t size)
    {
        void *newptr = mem_malloc(size);
        if (newptr == nullptr) {
            return nullptr;
        }

        if (ptr != nullptr) {
            uintptr_t oldlength = *((uintptr_t *)ptr - 1);
            memcpy(newptr, ptr, std::min(size, oldlength));
            mem_free(ptr);
        }
        return newptr;
    }

    void NativeObject::mem_free(void *ptr)
    {
        if (ptr != NULL) {
            ASSERT(_pfn_ntfreevirtualmemory);
            uintptr_t real_size = *(static_cast<uintptr_t*>(ptr) - 1) + sizeof(uintptr_t);
            void* ptr_ptr = static_cast<uintptr_t*>(ptr) - 1;
            _pfn_ntfreevirtualmemory(GetCurrentProcess(), const_cast<const void**>(&ptr_ptr),
                reinterpret_cast<SIZE_T*>(&real_size), MEM_RELEASE);
        }
    }

    void* NativeObject::native_malloc(size_t size)
    {
        return mem_malloc(size);
    }

    void* NativeObject::native_calloc(size_t nmemb, size_t size)
    {
        return mem_malloc(nmemb * size);
    }

    void* NativeObject::native_realloc(void *ptr, size_t size)
    {
        return mem_realloc(ptr, size);
    }

    void NativeObject::native_free(void *ptr)
    {
        mem_free(ptr);
    }
} // namespace cchips
