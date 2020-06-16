#include "ExceptionThrow.h"
#include "LogObject.h"

namespace cchips {

    int CExceptionObject::m_reference_count = 0;

    LONG WINAPI CExceptionObject::VehHandler(struct _EXCEPTION_POINTERS *ep)
    {
        //if (ep->ExceptionRecord->ExceptionCode == cplusplus_exception_code)
        //{
        //    if (ep->ExceptionRecord->NumberParameters >= 2)
        //    {
        //        exception_throw* p = reinterpret_cast<exception_throw*>(ep->ExceptionRecord->ExceptionInformation[1]);
        //        if (p)
        //        {
        //            {
        //                std::unique_ptr<CLogHandle> assert_handle = std::make_unique<CLogHandle>(ASSERT_FEATURE, CLogObject::logtype::log_exception, true);
        //                if (assert_handle) (*assert_handle).AddLog(LOGPAIR("exit process", p->what()));
        //            }
        //            //DISABLE_LOG();
        //            // waiting all log output and terminate self.
        //            // waiting for 1 second.
        //            //int count = 0;
        //            //do {
        //            //    if (IS_LOGS_NULL()) break;
        //            //    std::this_thread::sleep_for(std::chrono::milliseconds(exception_sleep_wait_timeout));
        //            //    if ((count++) > exception_try_loop_count) break;
        //            //} while (1);
        //        }
        //    }
        //    //std::abort();
        //}
        return EXCEPTION_CONTINUE_SEARCH;
    }

} // namespace cchips