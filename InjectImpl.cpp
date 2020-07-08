#include "InjectImpl.h"

namespace cchips {

    CInjectProcess::_os_type CInjectProcess::m_os_type = os_type_invalid;
    CInjectProcess::define_IsWow64Process CInjectProcess::m_lpfn_IsWow64Process = nullptr;
    CInjectProcess::define_GetNativeSystemInfo CInjectProcess::m_lpfn_GetNativeSystemInfo = nullptr;
    CInjectProcess::define_NtSuspendProcess CInjectProcess::m_lpfn_NtSuspendProcess = nullptr;
    CInjectProcess::define_NtResumeProcess CInjectProcess::m_lpfn_NtResumeProcess = nullptr;
    FARPROC CInjectProcess::m_lpfn_LdrLoadDll = nullptr;
    FARPROC CInjectProcess::m_lpfn_GetLastError = nullptr;

    FARPROC CDllInjectorObject::m_lpfn_LdrLoadDll = nullptr;
    FARPROC CDllInjectorObject::m_lpfn_GetLastError = nullptr;

    const std::vector<BYTE> CInjectProcess::m_shellcode_LoadLibrary_32 = {
        0x55,                                           // push ebp
        0x8B, 0xEC,                                     // mov  ebp, esp
        0x83, 0xEC, 0x08,                               // sub  esp, 8                                ; hmodule: -8, ret: -4
        0x33, 0xC0,                                     // xor  eax, eax
        0x89, 0x45, 0xFC,                               // mov  dword ptr[ebp - 4], eax
        0x56,                                           // push esi
        0x8B, 0x75, 0x08,                               // mov  esi, [ebp + 8]                        ; load_info
        0x8B, 0x0E,                                     // mov  ecx, [esi]
        0x85, 0xC9,                                     // test ecx, ecx                              ; if(load_info->ldr_load_dll == nullptr) goto worker_exit
        0x74, 0x24,                                     // jz   worker_exit
        0x83, 0x7E, 0x0C, 0x00,                         // cmp  dword ptr[esi + 0Ch], 0               ; if(load_info->replacedll.Buffer == nullptr) goto worker_exit
        0x74, 0x1E,                                     // jz   worker_exit
        0x8D, 0x45, 0xF8,                               // lea  eax, [ebp - 8]
        0x50,                                           // push eax
        0x8D, 0x46, 0x08,                               // lea  eax, [esi + 8]
        0x50,                                           // push eax
        0x6A, 0x00,                                     // push 0
        0x6A, 0x00,                                     // push 0
        0xFF, 0xD1,                                     // call ecx                                   ; ldr_load_dll(NULL, 0, &load_info->replacedll, &hmodule);
        0x85, 0xC0,                                     // test eax, eax
        0x74, 0x0C,                                     // jz   worker_exit
        0x8B, 0x46, 0x04,                               // mov  eax, [esi + 4]
        0xFF, 0xD0,                                     // call eax                                   ; get_last_error();
        0x5E,                                           // pop  esi
        0x8B, 0xE5,                                     // mov  esp, ebp
        0x5D,                                           // pop  ebp
        0xC2, 0x04, 0x00,                               // ret  4
        //worker_exit:
        0x33, 0xC0,                                     // xor  eax, eax
        0x5E,                                           // pop  esi
        0x8B, 0xE5,                                     // mov  esp, ebp
        0x5D,                                           // pop  ebp
        0xC2, 0x04, 0x00,                               // ret  4
    };

    //---------------------------------------------------------------
    // shellcode on x64 platform
    //_loadLibraryWorker_x64 proc
    //    push    rbx
    //    sub     rsp, 30h; hmodule: -18, ret : -10
    //    xor rax, rax
    //    mov[rsp + 28h], rax
    //    mov     rax, [rcx]
    //    mov     rbx, rcx
    //    test    rax, rax; if (load_info->ldr_load_dll == nullptr) goto worker_exit
    //    jz      worker_exit
    //    cmp     qword ptr[rcx + 18h], 0; if (load_info->replacedll.Buffer == nullptr) goto worker_exit
    //    jz      worker_exit
    //    lea     r8, [rcx + 10h]
    //    xor edx, edx
    //    xor     ecx, ecx
    //    lea     r9, [rsp + 20h]
    //    call    rax; ldr_load_dll(NULL, 0, &load_info->replacedll, &hmodule);
    //    test    eax, eax
    //    jz      worker_exit
    //    call    qword ptr[rbx + 8]; get_last_error();
    //    add     rsp, 30h
    //    pop     rbx
    //    ret
    //    worker_exit :
    //    xor     eax, eax
    //    add     rsp, 30h
    //    pop     rbx
    //    ret
    //_loadLibraryWorker_x64 endp
    //---------------------------------------------------------------
    const std::vector<BYTE> CInjectProcess::m_shellcode_LoadLibrary_64 = {
        0x53, 0x48, 0x83, 0xec, 0x30, 0x48, 0x33, 0xc0, 0x48, 
        0x89, 0x44, 0x24, 0x28, 0x48, 0x8b, 0x01, 0x48, 0x8b, 
        0xd9, 0x48, 0x85, 0xc0, 0x74, 0x23, 0x48, 0x83, 0x79, 
        0x18, 0x00, 0x74, 0x1c, 0x4c, 0x8d, 0x41, 0x10, 0x33, 
        0xd2, 0x33, 0xc9, 0x4c, 0x8d, 0x4c, 0x24, 0x20, 0xff, 
        0xd0, 0x85, 0xc0, 0x74, 0x09, 0xff, 0x53, 0x08, 0x48, 
        0x83, 0xc4, 0x30, 0x5b, 0xc3, 0x33, 0xc0, 0x48, 0x83, 
        0xc4, 0x30, 0x5b, 0xc3 };

} // namespace cchips
