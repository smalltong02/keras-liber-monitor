#include "ExceptionThrow.h"
#include "LogObject.h"
#include "utils.h"
#include <ehdata.h>

namespace cchips {

    int CExceptionObject::m_reference_count = 0;
    cchips::exception_throw g_exception_msg("this is sample");

    LONG WINAPI CExceptionObject::VehHandler(struct _EXCEPTION_POINTERS *ep)
    {
        if (ep->ExceptionRecord->ExceptionCode == cplusplus_exception_code)
        {
            if (ep->ExceptionRecord->NumberParameters>= 3 && ep->ExceptionRecord->ExceptionInformation[0] == ms_eh_magic_number)
            {
                ClearThreadTls();
                static ULONG_PTR* exception_vtbl = (reinterpret_cast<ULONG_PTR*>(*reinterpret_cast<ULONG_PTR*>(&g_exception_msg)));
                std::exception* p = reinterpret_cast<std::exception*>(ep->ExceptionRecord->ExceptionInformation[1]);
                ThrowInfo* throw_info = reinterpret_cast<ThrowInfo*>(ep->ExceptionRecord->ExceptionInformation[2]);
                if (p && throw_info)
                {
                    if(throw_info->pmfnUnwind)
                    {
                        ULONG_PTR* p_vtbl = (reinterpret_cast<ULONG_PTR*>(*reinterpret_cast<ULONG_PTR*>(p)));
                        if (p_vtbl && exception_vtbl && (*p_vtbl == *exception_vtbl))
                        {
                            std::string msg = p->what();
                            exception_throw* q = reinterpret_cast<exception_throw*>(p);
                            std::error_code err_code = q->GetErrCode();
                            int code = err_code.value();
                            const std::error_category& category = err_code.category();
                            std::string msg2 = category.message(code);
                            std::string name = category.name();
                            //{
                            //    std::unique_ptr<CLogHandle> assert_handle = std::make_unique<CLogHandle>(ASSERT_FEATURE, CLogObject::logtype::log_exception, true);
                            //    if (assert_handle) (*assert_handle).AddLog(LOGPAIR("exit process", p->what()));
                            //}
                            //DISABLE_LOG();
                            // waiting all log output and terminate self.
                            // waiting for 1 second.
                            //int count = 0;
                            //do {
                            //    if (IS_LOGS_NULL()) break;
                            //    std::this_thread::sleep_for(std::chrono::milliseconds(exception_sleep_wait_timeout));
                            //    if ((count++) > exception_try_loop_count) break;
                            //} while (1);
                            return EXCEPTION_CONTINUE_EXECUTION;
                        }
                    }
                }
                check_return();
            }
        //    //std::abort();
        }
        else if(ep->ExceptionRecord->ExceptionCode == STATUS_ACCESS_VIOLATION)
        {
            check_return();
        }
        return EXCEPTION_CONTINUE_SEARCH;
    }

} // namespace cchips