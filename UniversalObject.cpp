#include "stdafx.h"
#include <SubAuth.h>
#include <algorithm>
#include <WbemDisp.h>
#include <WbemProv.h>
#include <WbemTran.h>
#include "UniversalObject.h"
#include "CapstoneImpl.h"
#include "ExceptionThrow.h"
#include "CBaseType.h"

namespace cchips {
    std::vector<std::pair<CategoryObject::_category_type, std::string>> CategoryObject::_category_str_def = {
        {category_browser, {"^(?i)firefox.exe$|^(?i)chrome.exe$|^(?i)iexplore.exe$|^(?i)opera.exe$|^(?i)plugin-container.exe$|^(?i)opera_plugin_wrapper.exe$|^(?i)opera_plugin_wrapper_32.exe$|^(?i)FlashPlayerPlugin_\\w*.exe$"}},
        {category_office, {"^(?i)powerpnt.exe$|^(?i)winword.exe$|^(?i)excel.exe$|^(?i)eqnedt32.exe$"}},
        {category_pdf, {"^(?i)acrord32.exe$|^(?i)acrobat.exe$|^(?i)foxit reader.exe$"}},
        {category_java, {"^(?i)java.exe$|^(?i)javaw.exe$|^(?i)javaws.exe$"}},
        {category_misc, {"^(?i)helpctr.exe$|^(?i)hh.exe$|^(?i)wscript.exe$|^(?i)winhlp32.exe$|loaddll.exe$"}},
        {category_ps, {"^(?i)powershell.exe$"}},
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
        {category_ps, {"category_powershell"}},
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

    CommonFuncsObject::SetGUID CommonFuncsObject::_known_wmi_clsid_def = {
        CLSID_WbemLocator,
        CLSID_WbemContext,
        CLSID_UnsecuredApartment,
        CLSID_WbemClassObject,
        CLSID_MofCompiler,
        CLSID_WbemStatusCodeText,
        CLSID_WbemBackupRestore,
        CLSID_WbemRefresher,
        CLSID_WbemObjectTextSrc,
        CLSID_WbemDefPath,
        CLSID_WbemQuery,
        CLSID_SWbemLocator,
        CLSID_SWbemNamedValueSet,
        CLSID_SWbemObjectPath,
        CLSID_SWbemLastError,
        CLSID_SWbemSink,
        CLSID_SWbemDateTime,
        CLSID_SWbemRefresher,
        CLSID_SWbemServices,
        CLSID_SWbemServicesEx,
        CLSID_SWbemObject,
        CLSID_SWbemObjectEx,
        CLSID_SWbemObjectSet,
        CLSID_SWbemNamedValue,
        CLSID_SWbemQualifier,
        CLSID_SWbemQualifierSet,
        CLSID_SWbemProperty,
        CLSID_SWbemPropertySet,
        CLSID_SWbemMethod,
        CLSID_SWbemMethodSet,
        CLSID_SWbemEventSource,
        CLSID_SWbemSecurity,
        CLSID_SWbemPrivilege,
        CLSID_SWbemPrivilegeSet,
        CLSID_SWbemRefreshableItem,
        CLSID_WbemAdministrativeLocator,
        CLSID_WbemAuthenticatedLocator,
        CLSID_WbemUnauthenticatedLocator,
        CLSID_WbemDecoupledRegistrar,
        CLSID_WbemDecoupledBasicEventProvider,
        CLSID_WbemLevel1Login,
        CLSID_WbemLocalAddrRes,
        CLSID_WbemUninitializedClassObject,
        CLSID_WbemDCOMTransport,
    };

    CommonFuncsObject::_proc_type CommonFuncsObject::m_proc_type = CommonFuncsObject::proc_type_invalid;
    CommonFuncsObject::_os_type CommonFuncsObject::m_os_type = CommonFuncsObject::os_type_invalid;
    CommonFuncsObject::IsWow64Process_Define CommonFuncsObject::m_lpfn_IsWow64Process = nullptr;
    CommonFuncsObject::GetNativeSystemInfo_Define CommonFuncsObject::m_lpfn_GetNativeSystemInfo = nullptr;
    bool CommonFuncsObject::m_is_dotnet_owner = false;

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
            size == 0 ||
            m_heap_block_table.size() == 0)
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

    bool CommonFuncsObject::IsKnownWMIClsid(const GUID& clsid)
    {
        if (!_known_wmi_clsid_def.size())
            return false;
        const auto& it = _known_wmi_clsid_def.find(clsid);
        if (it == _known_wmi_clsid_def.end())
            return false;
        return true;
    }

    std::string CommonFuncsObject::GetLocalServer32FromClsid(const std::string& clsid)
    {
        if (!clsid.length()) return {};
        std::stringstream root_key_str;
        root_key_str << "SOFTWARE\\Classes\\CLSID\\" << clsid << "\\LocalServer32";
        HKEY key_handle;
        LSTATUS status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, root_key_str.str().c_str(), 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &key_handle);
        if (status == ERROR_SUCCESS) {
            DWORD cbData = 0;
            DWORD Type = 0;
            status = RegQueryValueEx(key_handle, NULL, NULL, NULL, NULL, &cbData);
            if ((status == ERROR_SUCCESS || status == ERROR_MORE_DATA) && cbData > 0) {
                std::unique_ptr<BYTE[]> buffer(new BYTE[cbData]);
                if (buffer) {
                    status = RegQueryValueEx(key_handle, NULL, NULL, &Type, buffer.get(), &cbData);
                    if (status == ERROR_SUCCESS) {
                        if (Type == REG_SZ) {
                            RegCloseKey(key_handle);
                            return std::string(reinterpret_cast<char*>(buffer.get()));
                        }
                    }
                }
            }
            RegCloseKey(key_handle);
        }
        return {};
    }

    std::string CommonFuncsObject::GetInprocServer32FromClsid(const std::string& clsid)
    {
        if (!clsid.length()) return {};
        std::stringstream root_key_str;
        root_key_str << "SOFTWARE\\Classes\\CLSID\\" << clsid << "\\InprocServer32";
        HKEY key_handle;
        LSTATUS status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, root_key_str.str().c_str(), 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &key_handle);
        if (status == ERROR_SUCCESS) {
            DWORD cbData = 0;
            DWORD Type = 0;
            status = RegQueryValueEx(key_handle, NULL, NULL, NULL, NULL, &cbData);
            if ((status == ERROR_SUCCESS || status == ERROR_MORE_DATA) && cbData > 0) {
                std::unique_ptr<BYTE[]> buffer(new BYTE[cbData]);
                if (buffer) {
                    status = RegQueryValueEx(key_handle, NULL, NULL, &Type, buffer.get(), &cbData);
                    if (status == ERROR_SUCCESS) {
                        if (Type == REG_SZ) {
                            RegCloseKey(key_handle);
                            return std::string(reinterpret_cast<char*>(buffer.get()));
                        }
                    }
                }
            }
            RegCloseKey(key_handle);
        }
        return {};
    }

    std::string CommonFuncsObject::GetComServicePath(const std::string& local_server32)
    {
        std::string service_path = local_server32;
        if (service_path.length()) {
            // try to delete /automation
#define STR_TAIL_AUTOMATION " /automation"
            if (EndsWith(service_path, STR_TAIL_AUTOMATION, false)) {
                service_path = service_path.substr(0, service_path.length() - sizeof(STR_TAIL_AUTOMATION) + 1);
            }
            if (service_path.length() > 2) {
                if (service_path[0] == '"' && service_path[service_path.length() - 1] == '"') {
                    service_path = service_path.substr(1, service_path.length()-2);
                }
            }
            return service_path;
        }

        return local_server32;
    }
} // namespace cchips
