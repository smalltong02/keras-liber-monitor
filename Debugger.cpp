#include "stdafx.h"
#include "Debugger.h"
#include "NativeObject.h"
#include "utils.h"
#include "ExceptionThrow.h"

namespace cchips {

    BYTE Debugger::_breakpoint_int3 = 0xCC;
    BYTE Debugger::_breakpoint_long_int3[2] = { 0xCD, 0x03 };
    BYTE Debugger::_breakpoint_ud2[2] = { 0 };

    std::uint64_t Debugger::Modifier::getXip() const
    {
        if (!valid()) return 0;
#ifdef _X86_
        return m_context->Eip;
#endif
#ifdef _AMD64_
        return m_context->Rip;
#endif
        return 0;
    }

    std::uint64_t Debugger::Modifier::getXsp() const
    {
        if (!valid()) return 0;
#ifdef _X86_
        return m_context->Esp;
#endif
#ifdef _AMD64_
        return m_context->Rsp;
#endif
        return 0;
    }

    std::uint64_t Debugger::Modifier::getXbp() const
    {
        if (!valid()) return 0;
#ifdef _X86_
        return m_context->Ebp;
#endif
#ifdef _AMD64_
        return m_context->Rbp;
#endif
        return 0;
    }

    std::uint64_t Debugger::Modifier::getXdi() const
    {
        if (!valid()) return 0;
#ifdef _X86_
        return m_context->Edi;
#endif
#ifdef _AMD64_
        return m_context->Rdi;
#endif
        return 0;
    }

    std::uint64_t Debugger::Modifier::getXsi() const
    {
        if (!valid()) return 0;
#ifdef _X86_
        return m_context->Esi;
#endif
#ifdef _AMD64_
        return m_context->Rsi;
#endif
        return 0;
    }

    std::uint64_t Debugger::Modifier::getXax() const
    {
        if (!valid()) return 0;
#ifdef _X86_
        return m_context->Eax;
#endif
#ifdef _AMD64_
        return m_context->Rax;
#endif
        return 0;
    }

    std::uint64_t Debugger::Modifier::getXbx() const
    {
        if (!valid()) return 0;
#ifdef _X86_
        return m_context->Ebx;
#endif
#ifdef _AMD64_
        return m_context->Rbx;
#endif
        return 0;
    }

    std::uint64_t Debugger::Modifier::getXcx() const
    {
        if (!valid()) return 0;
#ifdef _X86_
        return m_context->Ecx;
#endif
#ifdef _AMD64_
        return m_context->Rcx;
#endif
        return 0;
    }

    std::uint64_t Debugger::Modifier::getXdx() const
    {
        if (!valid()) return 0;
#ifdef _X86_
        return m_context->Edx;
#endif
#ifdef _AMD64_
        return m_context->Rdx;
#endif
        return 0;
    }

    std::uint32_t Debugger::Modifier::getEflags() const
    {
        if (!valid()) return 0;
        return m_context->EFlags;
    }

    std::uint64_t Debugger::Modifier::getDr0() const
    {
        if (!valid()) return 0;
        return m_context->Dr0;
    }

    std::uint64_t Debugger::Modifier::getDr1() const
    {
        if (!valid()) return 0;
        return m_context->Dr1;
    }

    std::uint64_t Debugger::Modifier::getDr2() const
    {
        if (!valid()) return 0;
        return m_context->Dr2;
    }

    std::uint64_t Debugger::Modifier::getDr3() const
    {
        if (!valid()) return 0;
        return m_context->Dr3;
    }

    std::uint64_t Debugger::Modifier::getDr6() const
    {
        if (!valid()) return 0;
        return m_context->Dr6;
    }

    std::uint64_t Debugger::Modifier::getDr7() const
    {
        if (!valid()) return 0;
        return m_context->Dr7;
    }

    std::uint32_t Debugger::Modifier::getGs() const
    {
        if (!valid()) return 0;
        return m_context->SegGs;
    }

    std::uint32_t Debugger::Modifier::getFs() const
    {
        if (!valid()) return 0;
        return m_context->SegFs;
    }

    std::uint32_t Debugger::Modifier::getEs() const
    {
        if (!valid()) return 0;
        return m_context->SegEs;
    }

    std::uint32_t Debugger::Modifier::getDs() const
    {
        if (!valid()) return 0;
        return m_context->SegDs;
    }

    std::uint32_t Debugger::Modifier::getCs() const
    {
        if (!valid()) return 0;
        return m_context->SegCs;
    }

    std::uint32_t Debugger::Modifier::getSs() const
    {
        if (!valid()) return 0;
        return m_context->SegSs;
    }

    std::uint32_t Debugger::Modifier::getCFlags() const
    {
        if (!valid()) return 0;
        return m_context->ContextFlags;
    }

    std::uint32_t Debugger::Modifier::getMxCsr() const
    {
        if (!valid()) return 0;
#ifdef _AMD64_
        return m_context->MxCsr;
#endif
        return 0;
    }

    std::uint64_t Debugger::Modifier::getR8() const
    {
        if (!valid()) return 0;
#ifdef _AMD64_
        return m_context->R8;
#endif
        return 0;
    }

    std::uint64_t Debugger::Modifier::getR9() const
    {
        if (!valid()) return 0;
#ifdef _AMD64_
        return m_context->R9;
#endif
        return 0;
    }

    std::uint64_t Debugger::Modifier::getR10() const
    {
        if (!valid()) return 0;
#ifdef _AMD64_
        return m_context->R10;
#endif
        return 0;
    }

    std::uint64_t Debugger::Modifier::getR11() const
    {
        if (!valid()) return 0;
#ifdef _AMD64_
        return m_context->R11;
#endif
        return 0;
    }

    std::uint64_t Debugger::Modifier::getR12() const
    {
        if (!valid()) return 0;
#ifdef _AMD64_
        return m_context->R12;
#endif
        return 0;
    }

    std::uint64_t Debugger::Modifier::getR13() const
    {
        if (!valid()) return 0;
#ifdef _AMD64_
        return m_context->R13;
#endif
        return 0;
    }

    std::uint64_t Debugger::Modifier::getR14() const
    {
        if (!valid()) return 0;
#ifdef _AMD64_
        return m_context->R14;
#endif
        return 0;
    }

    std::uint64_t Debugger::Modifier::getR15() const
    {
        if (!valid()) return 0;
#ifdef _AMD64_
        return m_context->R15;
#endif
        return 0;
    }

    M128A Debugger::Modifier::getXmm0() const
    {
        if (!valid()) return {};
#ifdef _AMD64_
        return m_context->Xmm0;
#endif
        return {};
    }

    M128A Debugger::Modifier::getXmm1() const
    {
        if (!valid()) return {};
#ifdef _AMD64_
        return m_context->Xmm1;
#endif
        return {};
    }

    M128A Debugger::Modifier::getXmm2() const
    {
        if (!valid()) return {};
#ifdef _AMD64_
        return m_context->Xmm2;
#endif
        return {};
    }

    M128A Debugger::Modifier::getXmm3() const
    {
        if (!valid()) return {};
#ifdef _AMD64_
        return m_context->Xmm3;
#endif
        return {};
    }

    M128A Debugger::Modifier::getXmm4() const
    {
        if (!valid()) return {};
#ifdef _AMD64_
        return m_context->Xmm4;
#endif
        return {};
    }

    M128A Debugger::Modifier::getXmm5() const
    {
        if (!valid()) return {};
#ifdef _AMD64_
        return m_context->Xmm5;
#endif
        return {};
    }

    M128A Debugger::Modifier::getXmm6() const
    {
        if (!valid()) return {};
#ifdef _AMD64_
        return m_context->Xmm6;
#endif
        return {};
    }

    M128A Debugger::Modifier::getXmm7() const
    {
        if (!valid()) return {};
#ifdef _AMD64_
        return m_context->Xmm7;
#endif
        return {};
    }

    M128A Debugger::Modifier::getXmm8() const
    {
        if (!valid()) return {};
#ifdef _AMD64_
        return m_context->Xmm8;
#endif
        return {};
    }

    M128A Debugger::Modifier::getXmm9() const
    {
        if (!valid()) return {};
#ifdef _AMD64_
        return m_context->Xmm9;
#endif
        return {};
    }

    M128A Debugger::Modifier::getXmm10() const
    {
        if (!valid()) return {};
#ifdef _AMD64_
        return m_context->Xmm10;
#endif
        return {};
    }

    M128A Debugger::Modifier::getXmm11() const
    {
        if (!valid()) return {};
#ifdef _AMD64_
        return m_context->Xmm11;
#endif
        return {};
    }

    M128A Debugger::Modifier::getXmm12() const
    {
        if (!valid()) return {};
#ifdef _AMD64_
        return m_context->Xmm12;
#endif
        return {};
    }

    M128A Debugger::Modifier::getXmm13() const
    {
        if (!valid()) return {};
#ifdef _AMD64_
        return m_context->Xmm13;
#endif
        return {};
    }

    M128A Debugger::Modifier::getXmm14() const
    {
        if (!valid()) return {};
#ifdef _AMD64_
        return m_context->Xmm14;
#endif
        return {};
    }

    M128A Debugger::Modifier::getXmm15() const
    {
        if (!valid()) return {};
#ifdef _AMD64_
        return m_context->Xmm15;
#endif
        return {};
    }

    bool Debugger::Modifier::setXip(std::uint64_t val)
    {
        if (!valid()) return false;
#ifdef _X86_
        m_context->Eip = val;
#endif
#ifdef _AMD64_
        m_context->Rip = val;
#endif
        return true;
    }

    bool Debugger::Modifier::setXsp(std::uint64_t val)
    {
        if (!valid()) return false;
#ifdef _X86_
        m_context->Esp = val;
#endif
#ifdef _AMD64_
        m_context->Rsp = val;
#endif
        return true;
    }

    bool Debugger::Modifier::setXbp(std::uint64_t val)
    {
        if (!valid()) return false;
#ifdef _X86_
        m_context->Ebp = val;
#endif
#ifdef _AMD64_
        m_context->Rbp = val;
#endif
        return true;
    }

    bool Debugger::Modifier::setXdi(std::uint64_t val)
    {
        if (!valid()) return false;
#ifdef _X86_
        m_context->Edi = val;
#endif
#ifdef _AMD64_
        m_context->Rdi = val;
#endif
        return true;
    }

    bool Debugger::Modifier::setXsi(std::uint64_t val)
    {
        if (!valid()) return false;
#ifdef _X86_
        m_context->Esi = val;
#endif
#ifdef _AMD64_
        m_context->Rsi = val;
#endif
        return true;
    }

    bool Debugger::Modifier::setXax(std::uint64_t val)
    {
        if (!valid()) return false;
#ifdef _X86_
        m_context->Eax = val;
#endif
#ifdef _AMD64_
        m_context->Rax = val;
#endif
        return true;
    }

    bool Debugger::Modifier::setXbx(std::uint64_t val)
    {
        if (!valid()) return false;
#ifdef _X86_
        m_context->Ebx = val;
#endif
#ifdef _AMD64_
        m_context->Rbx = val;
#endif
        return true;
    }

    bool Debugger::Modifier::setXcx(std::uint64_t val)
    {
        if (!valid()) return false;
#ifdef _X86_
        m_context->Ecx = val;
#endif
#ifdef _AMD64_
        m_context->Rcx = val;
#endif
        return true;
    }

    bool Debugger::Modifier::setXdx(std::uint64_t val)
    {
        if (!valid()) return false;
#ifdef _X86_
        m_context->Edx = val;
#endif
#ifdef _AMD64_
        m_context->Rdx = val;
#endif
        return true;
    }

    bool Debugger::Modifier::setEflags(std::uint32_t val)
    {
        if (!valid()) return false;
        m_context->EFlags = val;
        return true;
    }

    bool Debugger::Modifier::setDr0(std::uint64_t val)
    {
        if (!valid()) return false;
        m_context->Dr0 = val;
        return true;
    }

    bool Debugger::Modifier::setDr1(std::uint64_t val)
    {
        if (!valid()) return false;
        m_context->Dr1 = val;
        return true;
    }

    bool Debugger::Modifier::setDr2(std::uint64_t val)
    {
        if (!valid()) return false;
        m_context->Dr2 = val;
        return true;
    }

    bool Debugger::Modifier::setDr3(std::uint64_t val)
    {
        if (!valid()) return false;
        m_context->Dr3 = val;
        return true;
    }

    bool Debugger::Modifier::setDr6(std::uint64_t val)
    {
        if (!valid()) return false;
        m_context->Dr6 = val;
        return true;
    }

    bool Debugger::Modifier::setDr7(std::uint64_t val)
    {
        if (!valid()) return false;
        m_context->Dr7 = val;
        return true;
    }

    bool Debugger::Modifier::setGs(std::uint32_t val)
    {
        if (!valid()) return false;
        m_context->SegGs = val;
        return true;
    }

    bool Debugger::Modifier::setFs(std::uint32_t val)
    {
        if (!valid()) return false;
        m_context->SegFs = val;
        return true;
    }

    bool Debugger::Modifier::setEs(std::uint32_t val)
    {
        if (!valid()) return false;
        m_context->SegEs = val;
        return true;
    }

    bool Debugger::Modifier::setDs(std::uint32_t val)
    {
        if (!valid()) return false;
        m_context->SegDs = val;
        return true;
    }

    bool Debugger::Modifier::setCs(std::uint32_t val)
    {
        if (!valid()) return false;
        m_context->SegCs = val;
        return true;
    }

    bool Debugger::Modifier::setSs(std::uint32_t val)
    {
        if (!valid()) return false;
        m_context->SegSs = val;
        return true;
    }

    // only x64
    bool Debugger::Modifier::setCFlags(std::uint32_t val)
    {
        if (!valid()) return false;
        m_context->ContextFlags = val;
        return true;
    }

    bool Debugger::Modifier::setMxCsr(std::uint32_t val)
    {
        if (!valid()) return false;
#ifdef _X86_
        return false;
#endif
#ifdef _AMD64_
        m_context->MxCsr = val;
#endif
        return true;
    }

    bool Debugger::Modifier::setR8(std::uint64_t val)
    {
        if (!valid()) return false;
#ifdef _X86_
        return false;
#endif
#ifdef _AMD64_
        m_context->R8 = val;
#endif
        return true;
    }

    bool Debugger::Modifier::setR9(std::uint64_t val)
    {
        if (!valid()) return false;
#ifdef _X86_
        return false;
#endif
#ifdef _AMD64_
        m_context->R9 = val;
#endif
        return true;
    }

    bool Debugger::Modifier::setR10(std::uint64_t val)
    {
        if (!valid()) return false;
#ifdef _X86_
        return false;
#endif
#ifdef _AMD64_
        m_context->R10 = val;
#endif
        return true;
    }

    bool Debugger::Modifier::setR11(std::uint64_t val)
    {
        if (!valid()) return false;
#ifdef _X86_
        return false;
#endif
#ifdef _AMD64_
        m_context->R11 = val;
#endif
        return true;
    }

    bool Debugger::Modifier::setR12(std::uint64_t val)
    {
        if (!valid()) return false;
#ifdef _X86_
        return false;
#endif
#ifdef _AMD64_
        m_context->R12 = val;
#endif
        return true;
    }

    bool Debugger::Modifier::setR13(std::uint64_t val)
    {
        if (!valid()) return false;
#ifdef _X86_
        return false;
#endif
#ifdef _AMD64_
        m_context->R13 = val;
#endif
        return true;
    }

    bool Debugger::Modifier::setR14(std::uint64_t val)
    {
        if (!valid()) return false;
#ifdef _X86_
        return false;
#endif
#ifdef _AMD64_
        m_context->R14 = val;
#endif
        return true;
    }

    bool Debugger::Modifier::setR15(std::uint64_t val)
    {
        if (!valid()) return false;
#ifdef _X86_
        return false;
#endif
#ifdef _AMD64_
        m_context->R15 = val;
#endif
        return true;
    }

    bool Debugger::Modifier::setXmm0(const M128A& val)
    {
        if (!valid()) return false;
#ifdef _X86_
        return false;
#endif
#ifdef _AMD64_
        m_context->Xmm0 = val;
#endif
        return true;
    }

    bool Debugger::Modifier::setXmm1(const M128A& val)
    {
        if (!valid()) return false;
#ifdef _X86_
        return false;
#endif
#ifdef _AMD64_
        m_context->Xmm1 = val;
#endif
        return true;
    }

    bool Debugger::Modifier::setXmm2(const M128A& val)
    {
        if (!valid()) return false;
#ifdef _X86_
        return false;
#endif
#ifdef _AMD64_
        m_context->Xmm2 = val;
#endif
        return true;
    }

    bool Debugger::Modifier::setXmm3(const M128A& val)
    {
        if (!valid()) return false;
#ifdef _X86_
        return false;
#endif
#ifdef _AMD64_
        m_context->Xmm3 = val;
#endif
        return true;
    }

    bool Debugger::Modifier::setXmm4(const M128A& val)
    {
        if (!valid()) return false;
#ifdef _X86_
        return false;
#endif
#ifdef _AMD64_
        m_context->Xmm4 = val;
#endif
        return true;
    }

    bool Debugger::Modifier::setXmm5(const M128A& val)
    {
        if (!valid()) return false;
#ifdef _X86_
        return false;
#endif
#ifdef _AMD64_
        m_context->Xmm5 = val;
#endif
        return true;
    }

    bool Debugger::Modifier::setXmm6(const M128A& val)
    {
        if (!valid()) return false;
#ifdef _X86_
        return false;
#endif
#ifdef _AMD64_
        m_context->Xmm6 = val;
#endif
        return true;
    }

    bool Debugger::Modifier::setXmm7(const M128A& val)
    {
        if (!valid()) return false;
#ifdef _X86_
        return false;
#endif
#ifdef _AMD64_
        m_context->Xmm7 = val;
#endif
        return true;
    }

    bool Debugger::Modifier::setXmm8(const M128A& val)
    {
        if (!valid()) return false;
#ifdef _X86_
        return false;
#endif
#ifdef _AMD64_
        m_context->Xmm8 = val;
#endif
        return true;
    }

    bool Debugger::Modifier::setXmm9(const M128A& val)
    {
        if (!valid()) return false;
#ifdef _X86_
        return false;
#endif
#ifdef _AMD64_
        m_context->Xmm9 = val;
#endif
        return true;
    }

    bool Debugger::Modifier::setXmm10(const M128A& val)
    {
        if (!valid()) return false;
#ifdef _X86_
        return false;
#endif
#ifdef _AMD64_
        m_context->Xmm10 = val;
#endif
        return true;
    }

    bool Debugger::Modifier::setXmm11(const M128A& val)
    {
        if (!valid()) return false;
#ifdef _X86_
        return false;
#endif
#ifdef _AMD64_
        m_context->Xmm11 = val;
#endif
        return true;
    }

    bool Debugger::Modifier::setXmm12(const M128A& val)
    {
        if (!valid()) return false;
#ifdef _X86_
        return false;
#endif
#ifdef _AMD64_
        m_context->Xmm12 = val;
#endif
        return true;
    }

    bool Debugger::Modifier::setXmm13(const M128A& val)
    {
        if (!valid()) return false;
#ifdef _X86_
        return false;
#endif
#ifdef _AMD64_
        m_context->Xmm13 = val;
#endif
        return true;
    }

    bool Debugger::Modifier::setXmm14(const M128A& val)
    {
        if (!valid()) return false;
#ifdef _X86_
        return false;
#endif
#ifdef _AMD64_
        m_context->Xmm14 = val;
#endif
        return true;
    }

    bool Debugger::Modifier::setXmm15(const M128A& val)
    {
        if (!valid()) return false;
#ifdef _X86_
        return false;
#endif
#ifdef _AMD64_
        m_context->Xmm15 = val;
#endif
        return true;
    }

    bool Debugger::Initialize()
    {
        de_callback access_callback(std::bind(&Debugger::bp_accessviolation_cb, this, std::placeholders::_1));
        if (!setCustomHandler(de_access_violation, access_callback))
            return false;
        de_callback step_callback(std::bind(&Debugger::bp_singlestep_cb, this, std::placeholders::_1));
        if (!setCustomHandler(de_single_step, step_callback))
            return false;
        de_callback breakpoint_callback(std::bind(&Debugger::bp_breakpoint_cb, this, std::placeholders::_1));
        if (!setCustomHandler(de_breakpoint, breakpoint_callback))
            return false;
        de_callback ud2_callback(std::bind(&Debugger::bp_ud2_cb, this, std::placeholders::_1));
        if (!setCustomHandler(de_illegal_instruction, ud2_callback))
            return false;
        return true;
    }

    bool Debugger::hideDebugger()
    {
        return true;
    }

    bool Debugger::unhideDebugger()
    {
        return true;
    }

    long Debugger::Dispatch(struct _EXCEPTION_POINTERS *ep)
    {
        long ret = EXCEPTION_CONTINUE_SEARCH;
        if (!ep) return ret;
        if (!ep->ExceptionRecord) return ret;
        if (!ep->ContextRecord) return ret;
        //std::cout << "exception addr: " << std::hex << ep->ExceptionRecord->ExceptionAddress << std::endl;
        DWORD e_code = ep->ExceptionRecord->ExceptionCode;
        de_type detype = de_unknown;
        if (CExceptionObject::IsBreakPoint(e_code)) {
            detype = de_breakpoint;
        }
        else if (CExceptionObject::IsSingleStep(e_code)) {
            detype = de_single_step;
        }
        else if (CExceptionObject::IsAccessViolation(e_code)) {
            detype = de_access_violation;
        }
        else if (CExceptionObject::IsPageGuardViolation(e_code)) {
            detype = de_guard_page_violation;
        }
        else if (CExceptionObject::IsPageError(e_code)) {
            detype = de_page_error;
        }

        std::shared_ptr<Modifier> modp = std::make_shared<Modifier>(ep->ContextRecord);
        auto& find = m_de_list.find(detype);
        if (find == m_de_list.end())
            return ret;
        for (auto& callback : find->second) {
            //std::cout << "Dispatch call! detype: " << detype << std::endl;
            ret |= callback(modp);
        }
        return ret;
    }

    bool Debugger::setBreakPoint(std::uint64_t address, de_callback callback)
    {
        if (!address) return false;
        bp_entry entry;
        entry.address = address;
        auto& find = m_bp_list.find(entry);
        if (find != m_bp_list.end())
            return false;
        entry.bactive = 1;
        entry.type = bp_int3;
        entry.res_size = 1;

        tls_check_struct *tls = check_get_tls();
        if (tls) {
            tls->active = 1;
            if (setjmp(tls->jb) == 0) {
                entry.restore_bytes.resize(1);
                entry.restore_bytes[0] = *reinterpret_cast<BYTE*>(address);
                tls->active = 0;
            }
            else {
                //execption occur
                tls->active = 0;
                return false;
            }
        }

        if (NativeObject::native_protect((void*)address, 1, PAGE_EXECUTE_READWRITE, &entry.old_protection)) {
            *reinterpret_cast<BYTE*>(address) = _breakpoint_int3;
            m_bp_list[entry].emplace_back(callback);
            return true;
        }

        return false;
    }

    bool Debugger::clrBreakPoint(std::uint64_t address)
    {
        if (!address) return false;
        bp_entry entry;
        entry.address = address;
        auto& find = m_bp_list.find(entry);
        if (find != m_bp_list.end()) {
            if (find->first.type != bp_int3)
                return false;
            memset(reinterpret_cast<BYTE*>(address), find->first.restore_bytes[0], find->first.restore_bytes.size());
            //NativeObject::native_protect((void*)address, find->first.restore_bytes.size(), entry.old_protection, &entry.old_protection);
            m_bp_list.erase(entry);
        }
        return true;
    }

    bool Debugger::enableBreakPoint(std::uint64_t address)
    {
        if (!address) return false;
        bp_entry entry;
        entry.address = address;
        auto& find = m_bp_list.find(entry);
        if (find != m_bp_list.end()) {
            if (find->first.type != bp_int3)
                return false;
            find->first.bactive = true;
        }
        return true;
    }

    bool Debugger::disableBreakPoint(std::uint64_t address)
    {
        if (!address) return false;
        bp_entry entry;
        entry.address = address;
        auto& find = m_bp_list.find(entry);
        if (find != m_bp_list.end()) {
            if (find->first.type != bp_int3)
                return false;
            find->first.bactive = false;
        }
        return true;
    }

    bool Debugger::setLongBreakPoint(std::uint64_t address, de_callback callback)
    {
        if (!address) return false;
        bp_entry entry;
        entry.address = address;
        auto& find = m_bp_list.find(entry);
        if (find != m_bp_list.end())
            return false;
        entry.bactive = 1;
        entry.type = bp_long_int3;
        entry.res_size = 2;

        tls_check_struct *tls = check_get_tls();
        if (tls) {
            tls->active = 1;
            if (setjmp(tls->jb) == 0) {
                entry.restore_bytes.resize(2);
                memcpy(&entry.restore_bytes[0], reinterpret_cast<BYTE*>(address), 2);
                tls->active = 0;
            }
            else {
                //execption occur
                tls->active = 0;
                return false;
            }
        }

        if (NativeObject::native_protect((void*)address, 2, PAGE_EXECUTE_READWRITE, &entry.old_protection)) {
            memcpy(reinterpret_cast<BYTE*>(address), _breakpoint_long_int3, 2);
            m_bp_list[entry].emplace_back(callback);
            return true;
        }

        return false;
    }

    bool Debugger::clrLongBreakPoint(std::uint64_t address)
    {
        if (!address) return false;
        bp_entry entry;
        entry.address = address;
        auto& find = m_bp_list.find(entry);
        if (find != m_bp_list.end()) {
            if (find->first.type != bp_long_int3)
                return false;
            memset(reinterpret_cast<BYTE*>(address), find->first.restore_bytes[0], find->first.restore_bytes.size());
            //NativeObject::native_protect((void*)address, find->first.restore_bytes.size(), entry.old_protection, &entry.old_protection);
            m_bp_list.erase(entry);
        }
        return true;
    }

    bool Debugger::enableLongBreakPoint(std::uint64_t address)
    {
        if (!address) return false;
        bp_entry entry;
        entry.address = address;
        auto& find = m_bp_list.find(entry);
        if (find != m_bp_list.end()) {
            if (find->first.type != bp_long_int3)
                return false;
            find->first.bactive = true;
        }
        return true;
    }

    bool Debugger::disableLongBreakPoint(std::uint64_t address)
    {
        if (!address) return false;
        bp_entry entry;
        entry.address = address;
        auto& find = m_bp_list.find(entry);
        if (find != m_bp_list.end()) {
            if (find->first.type != bp_long_int3)
                return false;
            find->first.bactive = false;
        }
        return true;
    }

    bool Debugger::setUd2(std::uint64_t address, de_callback callback)
    {
        return true;
    }

    bool Debugger::clrUd2(std::uint64_t address)
    {
        return true;
    }

    bool Debugger::enableUd2(std::uint64_t address)
    {
        return true;
    }

    bool Debugger::disableUd2(std::uint64_t address)
    {
        return true;
    }
    bool Debugger::setSingleStep(std::uint64_t address, de_callback callback)
    {
        return true;
    }

    bool Debugger::clrSingleStep(std::uint64_t address)
    {
        return true;
    }
    bool Debugger::enableSingleStep(std::uint64_t address)
    {
        return true;
    }

    bool Debugger::disableSingleStep(std::uint64_t address)
    {
        return true;
    }

    bool Debugger::setMemBpx(std::uint64_t address, de_callback callback)
    {
        return true;
    }

    bool Debugger::clrMemBpx(std::uint64_t address)
    {
        return true;
    }

    bool Debugger::enableMemBpx(std::uint64_t address)
    {
        return true;
    }

    bool Debugger::disableMemBpx(std::uint64_t address)
    {
        return true;
    }

    bool Debugger::setCustomHandler(de_type type, de_callback callback)
    {
        if (type == de_unknown || type >= de_max)
            return false;
        if (callback) {
            m_de_list[type].emplace_back(callback);
        }
        else {
            m_de_list[type].clear();
        }
        return true;
    }

    long Debugger::bp_accessviolation_cb(std::shared_ptr<Modifier> ep)
    {
        check_return();
        return EXCEPTION_CONTINUE_SEARCH;
    }

    long Debugger::bp_singlestep_cb(std::shared_ptr<Modifier> ep)
    {
        return EXCEPTION_CONTINUE_SEARCH;
    }

    long Debugger::bp_breakpoint_cb(std::shared_ptr<Modifier> ep)
    {
        if (!ep) return EXCEPTION_CONTINUE_SEARCH;
        std::uint64_t address = ep->getXip();
        if (!address) return EXCEPTION_CONTINUE_SEARCH;
        bp_entry entry;
        entry.address = address;
        auto& find = m_bp_list.find(entry);
        if (find != m_bp_list.end()) {
            do {
                if (!find->first.bactive)
                    break;
                if (find->first.type != bp_int3 &&
                    find->first.type != bp_long_int3)
                    break;
                for (auto callback : find->second) {
                    callback(ep);
                }
            } while (0);
            //for (auto& byte : find->first.restore_bytes) {
            //    *reinterpret_cast<BYTE*>(address) = byte;
            //    address++;
            //}
            memcpy(reinterpret_cast<BYTE*>(address), &find->first.restore_bytes[0], find->first.restore_bytes.size());
            //std::cout << "Xip: " << std::hex << entry.address << " protect : " << entry.old_protection << std::endl;
            //NativeObject::native_protect((void*)entry.address, find->first.restore_bytes.size(), PAGE_EXECUTE_READ, &entry.old_protection);
            m_bp_list.erase(entry);
            return EXCEPTION_CONTINUE_EXECUTION;
        }
        return EXCEPTION_CONTINUE_SEARCH;
    }

    long Debugger::bp_ud2_cb(std::shared_ptr<Modifier> ep)
    {
        if (!ep) return EXCEPTION_CONTINUE_SEARCH;
        std::uint64_t address = ep->getXip();
        if (!address) return EXCEPTION_CONTINUE_SEARCH;
        bp_entry entry;
        entry.address = address;
        auto& find = m_bp_list.find(entry);
        if (find != m_bp_list.end()) {
            do {
                if (!find->first.bactive)
                    break;
                if (find->first.type != bp_ud2)
                    break;
                for (auto callback : find->second) {
                    callback(ep);
                }
            } while (0);
            memcpy(reinterpret_cast<BYTE*>(address), &find->first.restore_bytes[0], find->first.restore_bytes.size());
            //NativeObject::native_protect((void*)address, find->first.restore_bytes.size(), entry.old_protection, &entry.old_protection);
            m_bp_list.erase(entry);
            return EXCEPTION_CONTINUE_EXECUTION;
        }
        return EXCEPTION_CONTINUE_SEARCH;
    }
} // namespace cchips
