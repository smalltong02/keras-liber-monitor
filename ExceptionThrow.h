#pragma once
#include <Windows.h>
#include <string>
#include <sstream>
#include <vector>
#include <exception>
#include "SpecialLog.h"

namespace cchips {

    class exception_throw : std::exception
    {
    public:
        exception_throw(std::string throw_msg) : m_throw_msg(throw_msg) { ; }
        virtual ~exception_throw() throw() { ; }
        virtual const char* what() const throw() { return m_throw_msg.c_str(); }

    private:
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
        static const ULONG cplusplus_exception_code = 0xe06d7363;
    private:
        CExceptionObject() { m_veh_handle = AddVectoredExceptionHandler(1, VehHandler); }
        PVOID m_veh_handle;
        static int m_reference_count;
    };

#ifdef _DEBUG
#define ASSERT(condition) assert(condition)
#else
#define ASSERT(condition) if(!(condition)) {\
    std::stringstream os; \
    cchips::special_log::sm_log(os, __FILE__, __LINE__, "{}", #condition); \
	cchips::exception_throw exception_msg(os.str().c_str()); \
	throw exception_msg;}
#endif
} // namespace cchips

