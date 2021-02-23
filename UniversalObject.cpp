#include "stdafx.h"
#include <SubAuth.h>
#include <algorithm>
#include "UniversalObject.h"
#include "CapstoneImpl.h"
#include "ExceptionThrow.h"

namespace cchips {
    std::vector<std::pair<CategoryObject::_category_type, std::string>> CategoryObject::_category_str_def = {
        {category_browser, {"^(?i)firefox.exe$|^(?i)chrome.exe$|^(?i)iexplore.exe$|^(?i)opera.exe$|^(?i)plugin-container.exe$|^(?i)opera_plugin_wrapper.exe$|^(?i)opera_plugin_wrapper_32.exe$|^(?i)FlashPlayerPlugin_\\w*.exe$"}},
        {category_office, {"^(?i)powerpnt.exe$|^(?i)winword.exe$|^(?i)excel.exe$|^(?i)eqnedt32.exe$"}},
        {category_pdf, {"^(?i)acrord32.exe$|^(?i)acrobat.exe$|^(?i)foxit reader.exe$"}},
        {category_java, {"^(?i)java.exe$|^(?i)javaw.exe$|^(?i)javaws.exe$"}},
        {category_misc, {"^(?i)helpctr.exe$|^(?i)hh.exe$|^(?i)wscript.exe$|^(?i)winhlp32.exe$|loaddll.exe$"}},
        {category_test, {"^(?i)hipshooktest.exe$"}},
    };

    std::vector<std::pair<CategoryObject::_category_type, std::string>> CategoryObject::_dangerous_str_def = {
        {category_browser, {"^(?i)wscript.exe$|^(?i)cscript.exe$|^(?i)powershell.exe$|^(?i)net.exe$|^(?i)regsvr32.exe$"}},
        {category_office, {"^(?i)cmd.exe$|^(?i)wscript.exe$|^(?i)cscript.exe$|^(?i)powershell.exe$|^(?i)net.exe$|^(?i)regsvr32.exe$"}},
        {category_pdf, {"^(?i)cmd.exe$|^(?i)wscript.exe$|^(?i)cscript.exe$|^(?i)powershell.exe$|^(?i)net.exe$|^(?i)regsvr32.exe$"}},
        {category_java, {"^(?i)wscript.exe$|^(?i)cscript.exe$|^(?i)powershell.exe$|^(?i)net.exe$|^(?i)regsvr32.exe$"}},
        {category_misc, {"^(?i)powershell.exe$|^(?i)net.exe$|^(?i)regsvr32.exe$"}},
        {category_test, {"^(?i)notepad.exe$"}},
    };

    std::vector<std::pair<CategoryObject::_category_type, std::string>> CategoryObject::_category_name_def = {
        {category_normal, {"category_normal"}},
        {category_browser, {"category_browser"}},
        {category_office, {"category_office"}},
        {category_pdf, {"category_pdf"}},
        {category_java, {"category_java"}},
        {category_misc, {"category_misc"}},
        {category_test, {"category_test"}},
    };

    std::vector<CheckExploitFuncs::heap_block> CheckExploitFuncs::m_heap_block_table = {
        { 0x0A0A0A0A },
        { 0x0B0B0B0B },
        { 0x0C0C0C0C },
        { 0x0D0D0D0D },
    };

    std::vector<std::pair<CommonFuncsObject::_os_type, std::string>> CommonFuncsObject::_os_type_def = {
        {os_type_x32, {"x32"}}, {os_type_x64, {"x64"}},
    };

    std::vector<std::pair<CommonFuncsObject::_proc_type, std::string>> CommonFuncsObject::_proc_type_def = {
        {proc_type_x32, {"32-bit"}}, {proc_type_x64, {"64-bit"}},
    };

    CommonFuncsObject::_proc_type CommonFuncsObject::m_proc_type = CommonFuncsObject::proc_type_invalid;
    CommonFuncsObject::_os_type CommonFuncsObject::m_os_type = CommonFuncsObject::os_type_invalid;
    CommonFuncsObject::IsWow64Process_Define CommonFuncsObject::m_lpfn_IsWow64Process = nullptr;
    CommonFuncsObject::GetNativeSystemInfo_Define CommonFuncsObject::m_lpfn_GetNativeSystemInfo = nullptr;
    bool CommonFuncsObject::m_is_dotnet_owner = false;

    const std::vector<std::pair<std::string, std::pair<NativeObject::_native_type, std::string>>> NativeObject::m_native_funcs = {
        {"ntdll", {native_sdt_func, "NtAllocateVirtualMemory"}},
        {"ntdll", {native_sdt_func, "NtFreeVirtualMemory"}},
        {"ntdll", {native_sdt_func, "NtProtectVirtualMemory"}},
        {"ntdll", {native_sdt_func, "NtReadVirtualMemory"}},
        {"ntdll", {native_sdt_func, "NtQueryVirtualMemory"}},
    };
    NativeObject::NtAllocateVirtualMemory_Define NativeObject::_pfn_ntallocatevirtualmemory = nullptr;
    NativeObject::NtFreeVirtualMemory_Define NativeObject::_pfn_ntfreevirtualmemory = nullptr;

    void CategoryObject::InitGetCategory()
    {
        char buffer[MAX_PATH];

        DWORD nret = GetModuleFileName(NULL, buffer, MAX_PATH);
        if (nret == 0) return;
        std::string filename = buffer;
        size_t npos = filename.rfind('\\');
        if (npos != std::string::npos) {
            filename = filename.substr(npos + 1);
        }
        if (!filename.size()) return;

        for (const auto& category : _category_str_def) {
            std::unique_ptr<RE2> pattern_ptr = std::make_unique<RE2>(category.second, RE2::Quiet);
            if (!pattern_ptr) break;
            if (RE2::FullMatch(filename, *pattern_ptr)) {
                m_category_pattern = std::move(pattern_ptr);
                m_category = category.first;
                break;
            }
        }

        if (m_category != category_normal && m_category_pattern) {
            for (const auto& dangerous : _dangerous_str_def) {
                if (m_category == dangerous.first) {
                    m_dangerous_pattern = std::make_unique<RE2>(dangerous.second, RE2::Quiet);
                    break;
                }
            }
        }
    }

    bool CategoryObject::IsMatchCategory(const std::string& category_pattern) const
    {
        if (!category_pattern.length()) return false;
        std::unique_ptr<RE2> pattern_ptr = std::make_unique<RE2>(category_pattern, RE2::Quiet);
        if (!pattern_ptr) return false;
        std::string cur_category_str;
        for (const auto& name : _category_name_def) {
            if (m_category == name.first) {
                cur_category_str = name.second;
                break;
            }
        }
        if (!cur_category_str.length()) return false;
        if (RE2::FullMatch(cur_category_str, *pattern_ptr)) {
            return true;
        }
        return false;
    }

    bool CategoryObject::IsDangerousCommand(std::string& command) const
    {
        if (!m_dangerous_pattern) return false;
        if (!command.size()) return false;
        size_t npos = command.rfind('\\');
        if (npos != std::string::npos) {
            command = command.substr(npos + 1);
        }
        if (!command.size()) return false;
        npos = command.find(' ');
        std::string cmd_name;
        while (npos != std::string::npos) {
            cmd_name = command.substr(0, npos);
            command = command.substr(npos + 1);
            if (RE2::FullMatch(cmd_name, *m_dangerous_pattern)) {
                return true;
            }
            npos = command.find(' ');
        }
        cmd_name = command;
        if (RE2::FullMatch(cmd_name, *m_dangerous_pattern)) {
            return true;
        }
        return false;
    }

    bool CheckExploitFuncs::AddressInModule(ULONG_PTR addr, MODULEINFO mi)
    {
        if (addr != 0 && mi.lpBaseOfDll != nullptr && mi.SizeOfImage != 0)
        {
            if (addr >= reinterpret_cast<ULONG_PTR>(mi.lpBaseOfDll) && addr < (reinterpret_cast<ULONG_PTR>(mi.lpBaseOfDll) + mi.SizeOfImage))
            {
                return true;
            }
        }
        return false;
    }

    bool CheckExploitFuncs::CheckStackInstruction(ULONG_PTR stack) {
        bool bsuspicious = false;
        if (stack)
        {
            PUCHAR p_instruction = (PUCHAR)stack - 6;

            tls_check_struct *tls = check_get_tls();
            if (tls) {
                tls->active = 1;
                if (setjmp(tls->jb) == 0) {
                    if (p_instruction[1] != ASM_CALL_RELATIVE)
                    {
                        if (((WORD*)p_instruction)[0] != ASM_CALL_ABSOLUTE &&
                            ((WORD*)p_instruction)[0] != ASM_JMP_ABSOLUTE)
                        {
                            bsuspicious = true;
                        }
                    }
                }
                tls->active = 0;
            }
        }
        return bsuspicious;
    }

    bool CheckExploitFuncs::ValidStackPointer(ULONG_PTR stack)
    {
        ULONG_PTR base = 0;
        ULONG_PTR limit = 0;

        if (stack == 0)
            return true;

        GetStackRange(&base, &limit);
        if (base == 0 || limit == 0) return true;
        if (stack >= limit && stack < base)
            return true;
        return false;
    }

    bool CheckExploitFuncs::IsValidImage(ULONG_PTR addr)
    {
        MEMORY_BASIC_INFORMATION mbi;
        unsigned char * ptr;
        if (!addr) return true;

        if (VirtualQueryEx(GetCurrentProcess(), reinterpret_cast<LPCVOID>(addr), &mbi, sizeof(MEMORY_BASIC_INFORMATION))) {
            if (mbi.Type & MEM_IMAGE)
                return true;

            if (mbi.AllocationBase) {
                if (VirtualQueryEx(GetCurrentProcess(), mbi.AllocationBase, &mbi, sizeof(MEMORY_BASIC_INFORMATION))) {
                    if (mbi.State & MEM_RESERVE)
                        return true;

                    if (mbi.State & MEM_COMMIT) {
                        ptr = (unsigned char *)mbi.AllocationBase;
                        if (ptr[0] == 'M' && ptr[1] == 'Z')
                            return true;
                    }
                }
            }
        }
        return false;
    }

    CheckExploitFuncs::_exploit_reason CheckExploitFuncs::CheckROPAttack(ULONG_PTR addr)
    {
        char* ptr;
        char* ret;
        int opsize;

        if (addr == 0) return e_reason_null;

        ret = reinterpret_cast<char*>(addr);
        for (ptr = ret - 0x10; ptr < ret; ptr++) {
            opsize = GetCallOpSize(reinterpret_cast<LPCVOID>(ptr));
            if (opsize > 0 && (ptr + opsize) == ret)
                return e_reason_null;
        }
        return e_reason_rop;
    }

    CheckExploitFuncs::_exploit_reason CheckExploitFuncs::CheckReturnAddress(ULONG_PTR addr)
    {
        if (!addr) {
            return e_reason_null;
        }

        if (!IsValidImage(addr)) {
            return e_reason_run_on_non_image;
        }

        return CheckROPAttack(addr);
    }

    bool CheckExploitFuncs::CheckNOPSlide(heap_block* block)
    {
        if (!ValidHeapBlock(block))
            return false;
        ULONG_PTR check_size = block->bound - block->addr;
        DWORD slide_nop = 0;
        DWORD* ptr = reinterpret_cast<DWORD*>(block->addr);

        if (check_size < 0x100)
            return false;

        slide_nop = *ptr;

        if (
            slide_nop != 0x90909090        /* Regular nop slide */
            && slide_nop != 0x0A0A0A0A
            && slide_nop != 0x0C0C0C0C    /* Sample MD5: dacc9f48a60f88cf519249031ff469c1 */
            && slide_nop != 0x0D0D0D0D
            )
            return false;

        check_size = 0x100;

        for (int i = 1; i < (check_size / sizeof(DWORD)); i ++) {
            if (*(ptr+i) != slide_nop) {
                return false;
            }
        }

        /* FIXME: do we need to check nop slide and shell code further? */
        return true;
    }

    bool CheckExploitFuncs::DoCheckHeapSpray(ULONG_PTR base, ULONG_PTR bound)
    {
        CheckExploitFuncs::heap_block* block0, *block;
        ULONG_PTR size;
        PVOID heap;

        /* we make sure all block heap handle should be the same. */
        block0 = &m_heap_block_table[0];
        heap = block0->handle;
        size = block0->size;
#define HEAP_SPRAY_BLOCK_SIZE 0x2000
        if (!heap || size < HEAP_SPRAY_BLOCK_SIZE) {
            return false;
        }

        for (int count = 1; count < m_heap_block_table.size(); count++) {
            block = &m_heap_block_table[count];
            if (block->handle != heap || block->size != size) {
                return false;
            }

            if (memcmp((void *)block0->base, (void *)block->base, size) != 0) {
                return false;
            }
        }

        return CheckNOPSlide(block0);
    }

    bool CheckExploitFuncs::CheckHeapSpray(PVOID hHeap, PVOID base, ULONG_PTR size)
    {
        if (hHeap == nullptr ||
            base == nullptr ||
            size == 0)
            return false;

        CheckExploitFuncs::heap_block* block;
        ULONG_PTR bound;
        bool retval = false;
        bound = reinterpret_cast<ULONG_PTR>(base) + size;

        for (int count = 0; count < m_heap_block_table.size(); count++) {
            block = &m_heap_block_table[count];
            if (block->addr >= reinterpret_cast<ULONG_PTR>(base) && block->addr < bound) {
                block->base = reinterpret_cast<ULONG_PTR>(base);
                block->size = size;
                block->bound = bound;
                block->handle = hHeap;
                return retval;
            }
        }

        if (reinterpret_cast<ULONG_PTR>(base) <= m_heap_block_table[m_heap_block_table.size() - 1].addr)
            return retval;

        block = &m_heap_block_table[0];

        if ((hHeap == block->handle) && (size == block->size) && (reinterpret_cast<ULONG_PTR>(base) > block->addr)) {
            retval = DoCheckHeapSpray(reinterpret_cast<ULONG_PTR>(base), bound);
        }
        return retval;
    }

    void CheckExploitFuncs::FreeHeapSpray(PVOID hHeap, PVOID base)
    {
        CheckExploitFuncs::heap_block* block;
        if (hHeap == nullptr || base == nullptr)
            return;

        for (int count = 0; count < m_heap_block_table.size(); count++) {
            block = &m_heap_block_table[count];
            if (block->base && block->base == reinterpret_cast<ULONG_PTR>(base) && block->handle == hHeap) {
                block->base = block->size = block->bound = 0;
                block->handle = nullptr;
                break;
            }
        }
        return;
    }

    void CheckExploitFuncs::DestroyHeapSpray(PVOID hHeap)
    {
        CheckExploitFuncs::heap_block* block;
        if (hHeap == nullptr)
            return;
        for (int count = 0; count < m_heap_block_table.size(); count++) {
            block = &m_heap_block_table[count];
            if (block->handle == hHeap) {
                block->base = block->size = block->bound = 0;
                block->handle = nullptr;
            }
        }
    }

    bool CommonFuncsObject::IsX64Process(HANDLE hprocess) {
        bool bcurproc = false;
        if (hprocess == GetCurrentProcess())
            bcurproc = true;
        if (bcurproc) {
            if (m_proc_type == proc_type_x32) return false;
            else if (m_proc_type == proc_type_x64) return true;
        }
        if (!Is64BitOS()) {
            if(bcurproc)
                m_proc_type = proc_type_x32;
            return false;
        }
        if (m_lpfn_IsWow64Process == nullptr) {
            m_lpfn_IsWow64Process = (IsWow64Process_Define)GetProcAddress(
                GetModuleHandleA("kernel32"), "IsWow64Process");
        }
        BOOL wow64_process = false;
        if (!m_lpfn_IsWow64Process) {
            if(bcurproc)
                m_proc_type = proc_type_x32;
            return false;
        }
        BOOL bret = m_lpfn_IsWow64Process(hprocess, &wow64_process);
        if (!bret) {
            if (bcurproc)
                m_proc_type = proc_type_x32;
            return false;
        }
        if (wow64_process) {
            if (bcurproc)
                m_proc_type = proc_type_x32;
            return false;
        }
        if (bcurproc)
            m_proc_type = proc_type_x64;
        return true;
    }

    bool CommonFuncsObject::Is64BitOS() {
        if (m_os_type == os_type_x64) return true;
        else if (m_os_type == os_type_x32) return false;

        SYSTEM_INFO si;
        if (!m_lpfn_GetNativeSystemInfo)
            m_lpfn_GetNativeSystemInfo = (GetNativeSystemInfo_Define)GetProcAddress(GetModuleHandleA("kernel32.dll"), "GetNativeSystemInfo");
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

    bool CommonFuncsObject::IsMatchCurrentOS(const std::string& platform_pattern)
    {
        Is64BitOS();
        std::string cur_os_type;
        std::unique_ptr<RE2> pattern_ptr = std::make_unique<RE2>(platform_pattern, RE2::Quiet);
        if (!pattern_ptr) return false;
        for (const auto& os_type : _os_type_def) {
            if (m_os_type == os_type.first) {
                cur_os_type = os_type.second;
                break;
            }
        }
        if (!cur_os_type.length()) return false;
        if (RE2::FullMatch(cur_os_type, *pattern_ptr)) {
            return true;
        }
        return false;
    }

    bool CommonFuncsObject::IsMatchProcType(const std::string& type_pattern)
    {
        IsX64Process(GetCurrentProcess());
        std::string cur_proc_type;
        std::unique_ptr<RE2> pattern_ptr = std::make_unique<RE2>(type_pattern, RE2::Quiet);
        if (!pattern_ptr) return false;
        for (const auto& type : _proc_type_def) {
            if (m_proc_type == type.first) {
                cur_proc_type = type.second;
                break;
            }
        }
        if (!cur_proc_type.length()) return false;
        if (RE2::FullMatch(cur_proc_type, *pattern_ptr)) {
            return true;
        }
        return false;
    }

    bool CommonFuncsObject::IsDotnetOwner(void)
    {
        static std::once_flag dotnet_owner_flag;
        std::call_once(dotnet_owner_flag, [](bool& b_dotnet_owner) {
            static LPCSTR clr_dll_table[] = { "clr.dll", "clrjit.dll", "mscorwks.dll" };
            b_dotnet_owner = false;
            for (int count = 0; count < (sizeof(clr_dll_table) / sizeof(clr_dll_table[0])); count++) {
                if (GetModuleHandle(clr_dll_table[count])) {
                    b_dotnet_owner = true;
                    break;
                }
            }
            return;
            }, m_is_dotnet_owner);

        return m_is_dotnet_owner;
    }

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
        while (cchips::GetCapstoneImplment().CsDisasmIter(const_cast<const std::uint8_t**>(reinterpret_cast<std::uint8_t**>(&code)), &code_size, &address, insn)) {
            insn_size_sum += insn->size();
            if (insn->GetInsnId() == X86_INS_JNE) {
                if (cchips::GetCapstoneImplment().OpInCount(insn, X86_OP_IMM)) {
                    int index = cchips::GetCapstoneImplment().OpInIndex(insn, X86_OP_IMM, 1);
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
                if (cchips::GetCapstoneImplment().InsnInGroup(insn, X86_GRP_RET)) {
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
        if (NT_SUCCESS(_pfn_ntallocatevirtualmemory(GetCurrentProcess(), &ptr, 0, reinterpret_cast<SIZE_T*>(&real_length), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE))) {
            if (ptr == nullptr)
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
