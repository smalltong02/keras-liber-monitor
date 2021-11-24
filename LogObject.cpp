#include "stdafx.h"
#include "LogObject.h"
#include "HookImplementObject.h"

namespace cchips {

    int CLogObject::m_reference_count = 0;
    std::unique_ptr<CLogObject> g_log_object(CLogObject::GetInstance());

    /* Generate a temporary file name based on TMPL.  TMPL must match the
       rules for mk[s]temp (i.e. end in "XXXXXX").  The name constructed
       does not exist at the time of the call to mkstemp.  TMPL is
       overwritten with the result.  */
    FILE* CLpcLocalObject::mkstemp(char *tmpl)
    {
        int len;
        char *XXXXXX;
        static unsigned long long value;
        unsigned long long random_time_bits;
        unsigned int count;
        FILE* fd = nullptr;
        int save_errno = errno;
        char tmpl_name[MAX_PATH] = { };
        static const char letters[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

        /* A lower bound on the number of temporary files to attempt to
           generate.  The maximum total number of temporary file names that
           can exist for a given template is 62**6.  It should never be
           necessary to try all these combinations.  Instead if a reasonable
           number of names is tried (we define reasonable as 62**3) fail to
           give the system administrator the chance to remove the problems.  */
#define ATTEMPTS_MIN (62 * 62 * 62)

           /* The number of times to attempt to generate a temporary file.  To
              conform to POSIX, this must be no smaller than TMP_MAX.  */
#if ATTEMPTS_MIN < TMP_MAX
        unsigned int attempts = TMP_MAX;
#else
        unsigned int attempts = ATTEMPTS_MIN;
#endif

        int dwRetVal = GetTempPath(MAX_PATH,          // length of the buffer
            tmpl_name); // buffer for path 
        if (dwRetVal > MAX_PATH || (dwRetVal == 0))
        {
            errno = EINVAL;
            return nullptr;
        }

        len = (int)strlen(tmpl);
        if (len < 6 || len >= (MAX_PATH - dwRetVal) || strcmp(&tmpl[len - 6], "XXXXXX"))
        {
            errno = EINVAL;
            return nullptr;
        }
        if (!strcat(tmpl_name, tmpl)) return nullptr;
        /* This is where the Xs start.  */
        len += dwRetVal;
        XXXXXX = &tmpl_name[len - 6];

        /* Get some more or less random data.  */
        {
            SYSTEMTIME      stNow;
            FILETIME ftNow;

            // get system time
            GetSystemTime(&stNow);
            stNow.wMilliseconds = 500;
            if (!SystemTimeToFileTime(&stNow, &ftNow))
            {
                errno = -1;
                return nullptr;
            }

            random_time_bits = (((unsigned long long)ftNow.dwHighDateTime << 32)
                | (unsigned long long)ftNow.dwLowDateTime);
        }
        value += random_time_bits ^ (unsigned long long)GetCurrentThreadId();

        for (count = 0; count < attempts; value += 7777, ++count)
        {
            unsigned long long v = value;

            /* Fill in the random bits.  */
            XXXXXX[0] = letters[v % 62];
            v /= 62;
            XXXXXX[1] = letters[v % 62];
            v /= 62;
            XXXXXX[2] = letters[v % 62];
            v /= 62;
            XXXXXX[3] = letters[v % 62];
            v /= 62;
            XXXXXX[4] = letters[v % 62];
            v /= 62;
            XXXXXX[5] = letters[v % 62];

            fd = fopen(tmpl_name, "w+x");
            if (fd >= 0)
            {
                errno = save_errno;
                return fd;
            }
            else if (errno != EEXIST)
                return nullptr;
        }

        /* We got out of the loop because we ran out of combinations to try.  */
        errno = EEXIST;
        return nullptr;
    }

    bool CLogObject::Initialize()
    {
        std::function<RAPID_DOC_PAIR()> getdata(std::bind(&CLogObject::GetData, this));
#ifdef USING_PIPE_MESSAGE
        std::wstring pipe_name;
        if (g_impl_object->IsPipeMode()) {
            pipe_name = L"\\\\.\\pipe\\hips_hook";
        }
        if (g_impl_object->IsLocalMode()) {
            wchar_t tmp_buffer[MAX_PATH] = {};
            GetTempPathW(MAX_PATH * sizeof(wchar_t), tmp_buffer);
            pipe_name = std::wstring(L"LocalFile\\") + std::wstring(tmp_buffer) + L"hipshook.log";
        }
        if (!pipe_name.length()) return false;
        std::unique_ptr<PipeJSONClient> pipe_ptr = std::make_unique<PipeJSONClient>(pipe_name, PipeJSONClient::cfg_serial);
        if (pipe_ptr && pipe_ptr->Start())
            m_pipe_client = std::move(pipe_ptr);
        if (m_pipe_client)
            return true;
#else
        std::unique_ptr<CLpcPipeObject> pipe_ptr = std::make_unique<CLpcPipeObject>();
        if (pipe_ptr->Connect(getdata))
            m_socket_object = std::move(pipe_ptr);
        else
        {
            std::unique_ptr<CLpcLocalObject> local_ptr = std::make_unique<CLpcLocalObject>();
            if (local_ptr->Connect(getdata))
                m_socket_object = std::move(local_ptr);
        }
        if(m_socket_object)
            return true;
#endif
        return false;
    }
    RAPID_DOC_PAIR CLogObject::GetData() {
        static std::once_flag log_flag;
        std::shared_ptr<CLogEntry> log = nullptr;
        do {
            std::lock_guard<std::recursive_mutex> lockGuard(m_recursive_mutex);
            if (m_cache_logs.empty()) break;
            log = m_cache_logs.front();
            m_cache_logs.pop_front();
        } while (0);
        if (log)
            return log->Serialize();
        return RAPID_DOC_PAIR(nullptr,nullptr);
    }

} // namespace cchips