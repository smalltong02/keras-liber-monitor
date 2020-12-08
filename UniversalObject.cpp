#include "stdafx.h"
#include "UniversalObject.h"

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

    std::vector<CheckExploitFuncs::heap_block> CheckExploitFuncs::m_heap_block_table = {
        { 0x0A0A0A0A },
        { 0x0B0B0B0B },
        { 0x0C0C0C0C },
        { 0x0D0D0D0D },
    };

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

    bool CategoryObject::IsDangerousCommand(std::string command)
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

} // namespace cchips
