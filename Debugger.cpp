#include "stdafx.h"
#include "Debugger.h"
#include "NativeObject.h"
#include "utils.h"
#include "ExceptionThrow.h"

namespace cchips {

    BYTE Debugger::_breakpoint_int3 = 0xCC;
    BYTE Debugger::_breakpoint_long_int3[2] = { 0xCD, 0x03 };
    BYTE Debugger::_breakpoint_ud2[2] = { 0 };

    bool Debugger::Initialize()
    {
        de_callback step_callback(std::bind(&Debugger::bp_breakpoint_cb, this, std::placeholders::_1));
        if (!setCustomHandler(de_breakpoint, step_callback))
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
        if (!ep) return ret;
        if (!ep->ExceptionRecord) return ret;
        if (!ep->ContextRecord) return ret;
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

        auto& find = m_de_list.find(detype);
        if (find == m_de_list.end())
            return ret;
        for (auto& callback : find->second) {
            std::cout << "Dispatch call!" << std::endl;
            ret |= callback(ep);
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
            NativeObject::native_protect((void*)address, find->first.restore_bytes.size(), entry.old_protection, &entry.old_protection);
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
            NativeObject::native_protect((void*)address, find->first.restore_bytes.size(), entry.old_protection, &entry.old_protection);
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
        m_de_list[type].emplace_back(callback);
        return true;
    }

    long Debugger::bp_breakpoint_cb(PVOID data)
    {
        struct _EXCEPTION_POINTERS *ep = reinterpret_cast<struct _EXCEPTION_POINTERS*>(data);
#ifdef _X86_
        std::uint64_t address = (std::uint64_t)ep->ContextRecord->Eip;
#endif
#ifdef _AMD64_
        std::uint64_t address = (std::uint64_t)ep->ContextRecord->Rip;
#endif
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
                    callback(data);
                }
            } while (0);
            memset(reinterpret_cast<BYTE*>(address), find->first.restore_bytes[0], find->first.restore_bytes.size());
            NativeObject::native_protect((void*)address, find->first.restore_bytes.size(), entry.old_protection, &entry.old_protection);
            m_bp_list.erase(entry);
            return EXCEPTION_CONTINUE_EXECUTION;
        }
        return EXCEPTION_CONTINUE_SEARCH;
    }

    long Debugger::bp_ud2_cb(PVOID data)
    {
        struct _EXCEPTION_POINTERS *ep = reinterpret_cast<struct _EXCEPTION_POINTERS*>(data);
#ifdef _X86_
        std::uint64_t address = (std::uint64_t)ep->ContextRecord->Eip;
#endif
#ifdef _AMD64_
        std::uint64_t address = (std::uint64_t)ep->ContextRecord->Rip;
#endif
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
                    callback(data);
                }
            } while (0);
            memset(reinterpret_cast<BYTE*>(address), find->first.restore_bytes[0], find->first.restore_bytes.size());
            NativeObject::native_protect((void*)address, find->first.restore_bytes.size(), entry.old_protection, &entry.old_protection);
            m_bp_list.erase(entry);
            return EXCEPTION_CONTINUE_EXECUTION;
        }
        return EXCEPTION_CONTINUE_SEARCH;
    }
} // namespace cchips
