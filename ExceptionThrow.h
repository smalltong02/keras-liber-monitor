#pragma once
#include <Windows.h>
#include <string>
#include <sstream>
#include <vector>
#include <atomic>
#include <exception>
#include <assert.h>
#include <system_error>
#include "SpecialLog.h"

enum class _hips_error_code {
    hec_success = 0,
    hec_failed,
    hec_init_error,
    hec_get_mod_error,
    hec_extract_error,
};

namespace std {
    template<>
    struct is_error_code_enum<_hips_error_code> : true_type {};
}

class HipsErrorCategory : public std::error_category
{
public:
    static HipsErrorCategory const& instance() {
        static HipsErrorCategory instance;
        return instance;
    }

    char const* name() const noexcept override {
        return "HipsErrorCategory";
    }

    std::string message(int code) const override {
        return "HipsErrorCategoryMessage";
    }

    bool equivalent(std::error_code const& code, int condition) const noexcept override {
        //auto const& category = std::error_code(_hips_error_code{}).category();
        return false;
    }
};

inline std::error_code make_error_code(_hips_error_code code)
{
    return {
        static_cast<int>(code),
        HipsErrorCategory::instance(),
    };
}

namespace cchips {

    class exception_throw : std::exception
    {
    public:
        exception_throw(std::string throw_msg, _hips_error_code err_code = _hips_error_code::hec_failed) : m_error_code(err_code), m_throw_msg(throw_msg) { ; }
        virtual ~exception_throw() throw() { ; }
        virtual const char* what() const throw() { return m_throw_msg.c_str(); }
        const std::error_code& GetErrCode() const { return m_error_code; }
    private:
        std::error_code m_error_code;
        std::string m_throw_msg;
    };

    class CExceptionObject
    {
    public:
        ~CExceptionObject() { if (m_veh_handle) RemoveVectoredExceptionHandler(m_veh_handle); }

        static std::unique_ptr<CExceptionObject> GetInstance()
        {
            if (m_reference_count == 0)
            {
                CExceptionObject* p = new CExceptionObject();
                if (p)
                {
                    m_reference_count++;
                    return std::unique_ptr<CExceptionObject>(p);
                }
            }
            return nullptr;
        }
        static LONG WINAPI VehHandler(struct _EXCEPTION_POINTERS *ep);
        static __forceinline bool IsCplusplusException(struct _EXCEPTION_POINTERS *ep) {
            if (!ep) return false;
            if (!ep->ExceptionRecord) return false;
            if (ep->ExceptionRecord->ExceptionCode == cplusplus_exception_code &&
                ep->ExceptionRecord->NumberParameters >= 3 &&
                ep->ExceptionRecord->ExceptionInformation[0] == ms_eh_magic_number)
                return true;
            return false;
        }
        static __forceinline bool IsAccessViolation(DWORD e_code) {
            if (e_code == STATUS_ACCESS_VIOLATION)
                return true;
            return false;
        }
        static __forceinline bool IsPageGuardViolation(DWORD e_code) {
            if (e_code == STATUS_GUARD_PAGE_VIOLATION)
                return true;
            return false;
        }
        static __forceinline bool IsPageError(DWORD e_code) {
            if (e_code == EXCEPTION_IN_PAGE_ERROR)
                return true;
            return false;
        }
        static __forceinline bool IsSingleStep(DWORD e_code) {
            if (e_code == STATUS_SINGLE_STEP)
                return true;
            return false;
        }
        static __forceinline bool IsBreakPoint(DWORD e_code) {
            if (e_code == EXCEPTION_BREAKPOINT)
                return true;
            return false;
        }
        static const ULONG cplusplus_exception_code = 0xe06d7363;
        static const ULONG ms_eh_magic_number = 0x19930520;
        static const int exception_sleep_wait_timeout = 100;
        static const int exception_try_loop_count = 10;
    private:
        CExceptionObject() { m_veh_handle = AddVectoredExceptionHandler(1, VehHandler); }
        PVOID m_veh_handle;
        static int m_reference_count;
    };

#ifdef _DEBUG
#define ASSERT(condition) assert(condition)
#else
#define ASSERT(condition) /*if(!(condition)) {\
    std::stringstream os; \
    cchips::special_log::sm_log(os, __FILE__, __LINE__, "{}", #condition); \
    cchips::exception_throw exception_msg(os.str().c_str()); \
    throw exception_msg;}*/
#endif
} // namespace cchips

