#include "ExceptionThrow.h"
#include "LogObject.h"

namespace cchips {

    int CExceptionObject::m_reference_count = 0;

    LONG WINAPI CExceptionObject::VehHandler(struct _EXCEPTION_POINTERS *ep)
    {
        if (ep->ExceptionRecord->ExceptionCode == cplusplus_exception_code)
        {
            if (ep->ExceptionRecord->NumberParameters >= 2)
            {
                exception_throw* p = reinterpret_cast<exception_throw*>(ep->ExceptionRecord->ExceptionInformation[1]);
                if (p)
                {
                    std::unique_ptr<CLogHandle> assert_handle = std::make_unique<CLogHandle>(ASSERT_FEATURE, CLogObject::logtype::log_exception, true);
                    if (assert_handle) (*assert_handle).AddLog(LOGPAIR("assert failed", p->what()));
                    DISABLE_LOG();
                    // waiting all log output and terminate self.

                }
            }
            return EXCEPTION_CONTINUE_EXECUTION;
        }
        return EXCEPTION_CONTINUE_SEARCH;
    }

} // namespace cchips