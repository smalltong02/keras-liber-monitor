#pragma once
#include <io.h>
#include <stdio.h>
#include <windows.h>
#include <string>
#include <vector>
#include <mutex>
#include "ExceptionThrow.h"
#include "SpecialLog.h"
#include "deffeature.h"
#include "SafePtr.h"
#include "R3LogObject.h"

namespace cchips {

    class CLogEntry;
    class CLogObject;
    extern std::unique_ptr<CLogObject> g_log_object;

    class CLpcLocalObject : public CSocketObject
    {
    public:
#define filename_template "log_file.XXXXXX"
        CLpcLocalObject() : m_brunning(false), m_connect_thread(nullptr) {
            m_sync_event = CreateEvent(NULL, FALSE, FALSE, NULL);
            SetOutputType(CSocketObject::output_local);
        }
        ~CLpcLocalObject() {
            if (m_tempfp)
                fclose(m_tempfp);
            if (m_sync_event)
                CloseHandle(m_sync_event);
        }

        bool Listen(CSocketObject::func_callback& callback) override { return true; }
        bool Connect(CSocketObject::func_getdata& getdata) override {
            if (!m_tempfp)
                m_tempfp = mkstemp(filename_template);
            if (!m_tempfp)
                return false;
            assert(m_connect_thread == nullptr);
            if (m_connect_thread) return false;
            m_connect_thread = std::make_unique<std::thread>(std::bind(&CLpcLocalObject::ConnectThread, this, getdata));
            assert(m_connect_thread != nullptr);
            if (!m_connect_thread) return false;
            return true;
        }
        void StopListen() override { return; }
        void StopConnect() override {
            if (m_tempfp)
                fclose(m_tempfp);
        }
        void Activated() override { if (m_sync_event) SetEvent(m_sync_event); }
        int GetTotalLogs() const override { return m_logs_total_count; }
    private:
        VOID WINAPI ConnectThread(CSocketObject::func_getdata& getdata) {
            assert(m_tempfp);
            assert(m_sync_event);
            if (!m_tempfp) return;
            if (!m_sync_event) return;

            auto func_write = [&](const char* buffer, int len) {
                assert(buffer);
                if (!buffer) return false;
                std::string unpack_buffer(buffer, len);
                if (unpack_buffer.length() == len)
                {
                    std::unique_ptr<std::stringstream> ss_ptr(Unpack(unpack_buffer));
                    if (!ss_ptr) return false;
                    *ss_ptr << std::endl;
                    fputs(ss_ptr->str().c_str(), m_tempfp);
                    fflush(m_tempfp);
                    return true;
                }
                return false;
            };

            m_brunning = true;
            // write log to the pipe server
            while (m_brunning) {
                DWORD ret = WaitForSingleObject(m_sync_event, CSocketObject::lpc_client_wait_timeout);
                if (ret == WAIT_OBJECT_0) {
                    do {
                        std::unique_ptr<bson> data = Pack(getdata());
                        if (data)
                        {
                            m_logs_total_count++;
                            func_write(bson_data(&(*data)), bson_size(&(*data)));
                        }
                        else
                            break;
                    } while (1);
                }
                else if (ret == WAIT_TIMEOUT) {
                    continue;
                }
                else {
                    // other error
                    break;
                }
            }
        }
        bool ValidHandle() const { if (m_tempfp == nullptr) return false; else return true; }

        // mkstemp extracted from libc/sysdeps/posix/tempname.c.
        FILE* mkstemp(char *tmpl);
        HANDLE m_sync_event;
        std::atomic_bool m_brunning;
        std::unique_ptr<std::thread> m_connect_thread;
        FILE* m_tempfp = nullptr;
        int m_logs_total_count = 0;
    };
    
    class CLogObject
    {
    public:
        using logtype = enum _logtype {
            log_invalid = -1,
            log_event = 0,
            log_debug,
            log_error,
            log_exception,
        };
        using outputtype = enum _outputtype {
            output_invalid = -1,
            output_pipe = 0,
            output_local,
        };

        ~CLogObject() {
            if (m_socket_object)
                m_socket_object->StopConnect();
        }

        bool Initialize();

        bool AddLog(std::shared_ptr<CLogEntry>& log, bool bhead = false) {
            if (!m_socket_object) return false;
            if (!m_valid) return false;
            if (!log) return false;
            if (!bhead) m_cache_logs->push_back(log);
            else m_cache_logs->push_front(log);
            m_socket_object->Activated();
            return true;
        }
        std::unique_ptr<LOGPAIR> GetData();
        int GetTotalLogs() const { return m_socket_object->GetTotalLogs(); }
        void EnableLog() { m_valid = true; }
        void DisableLog() { m_valid = false; }
        bool IsLogsNull() { return m_cache_logs->empty(); }
        static std::unique_ptr<CLogObject> GetInstance()
        {
            if (m_reference_count == 0)
            {
                CLogObject* p = new CLogObject();
                if (p)
                {
                    m_reference_count++;
                    return std::unique_ptr<CLogObject>(p);
                }
            }
            return nullptr;
        }
    private:
        CLogObject() {
        }
        bool m_valid = true;
        static int m_reference_count;
        static std::mutex m_mutex_log;
        std::unique_ptr<CSocketObject> m_socket_object = nullptr;
        sf::contfree_safe_ptr<std::list<std::shared_ptr<CLogEntry>>> m_cache_logs;
    };

    class CLogEntry
    {
    public:
        using iterator = std::map<std::string, std::string>::iterator;
        using const_iterator = std::map<std::string, std::string>::const_iterator;

        CLogEntry(const std::string& name, CLogObject::logtype type = CLogObject::logtype::log_invalid) : log_name(name), m_log_type(type) {}
        ~CLogEntry() = default;
        const std::string& GetName() const { return log_name; }
        const CLogObject::logtype GetLogType() const { return m_log_type; }
        void SetLogType(CLogObject::logtype type) { m_log_type = type; }
        const int GetLogSize() const { return (int)log_elements.size(); }
        bool AddLog(const LOGPAIR& log_pair) {
            ASSERT(log_pair.first.length());
            ASSERT(log_pair.second.length());
            if (!log_pair.first.length()) return false;
            if (!log_pair.second.length()) return false;
            const auto& it = log_elements.find(log_pair.first);
            ASSERT(it == log_elements.end());
            if (it != log_elements.end()) return false;
            log_elements[log_pair.first] = log_pair.second;
            return true;
        }
        bool AddLog(const std::shared_ptr<CLogEntry>& log_entry) {
            ASSERT(log_entry);
            if (!log_entry) return false;
            std::stringstream ss;
            for (const auto& entry : *log_entry)
            {
                if (!ss.str().length())
                    ss << "{ ";
                else
                    ss << "; ";
                ss << "\"" << entry.first << "\"" << ": " << "\"" << entry.second << "\"";
            }
            if (ss.str().length())
            {
                ss << " }";
                return AddLog(LOGPAIR((*log_entry).GetName(), ss.str()));
            }
            return false;
        }
        std::unique_ptr<LOGPAIR> Serialize() const {
            if (!log_name.length()) return nullptr;
            if (!log_elements.size()) return nullptr;
            std::stringstream ss;
            for (const auto& elem : log_elements)
            {
                if (!ss.str().length())
                    ss << "{ ";
                else
                    ss << "; ";
                ss << "\"" << elem.first << "\"" << ": " << "\"" << elem.second << "\"";
            }
            if (ss.str().length())
            {
                ss << " }";
                return std::make_unique<LOGPAIR>(log_name, ss.str());
            }
            return nullptr;
        }

        iterator        begin() { return log_elements.begin(); }
        const_iterator  begin() const { return log_elements.begin(); }
        iterator        end() { return log_elements.end(); }
        const_iterator  end() const { return log_elements.end(); }
    private:
        std::string log_name;
        CLogObject::logtype m_log_type;
        std::map<std::string, std::string> log_elements;
    };

    class CLogHandle
    {
    public:
        CLogHandle(std::shared_ptr<PVOID> handle) {
            ASSERT(handle != nullptr);
            if (!handle) return;
            m_handle = std::reinterpret_pointer_cast<CLogEntry>(handle);
            m_valid = true;
            m_bhead = false;
        }
        CLogHandle(const std::string& name, CLogObject::logtype type = CLogObject::logtype::log_invalid, bool bhead = false) {
            m_handle = std::make_shared<CLogEntry>(name, type);
            ASSERT(m_handle != nullptr);
            m_valid = true;
            m_bhead = bhead;
        }
        ~CLogHandle() {
            if (!m_handle || !m_handle->GetLogSize()) return;
            if (m_handle.use_count() == 1 && m_handle->GetLogType() != CLogObject::logtype::log_invalid)
            {
                if (g_log_object != nullptr)
                {
                    std::stringstream tid_ss;
                    std::stringstream pid_ss;
                    tid_ss << std::this_thread::get_id();
                    pid_ss << ::GetCurrentProcessId();
                    m_handle->AddLog(LOGPAIR("Pid", pid_ss.str()));
                    m_handle->AddLog(LOGPAIR("Tid", tid_ss.str()));
                    g_log_object->AddLog(m_handle, m_bhead);
                }
            }
        }
        std::string GetName() const {
            if (m_valid)
            {
                ASSERT(m_handle != nullptr);
                return m_handle->GetName();
            }
            return std::string("");
        }
        bool AddLog(const LOGPAIR& log_pair) {
            if (m_valid)
            {
                ASSERT(m_handle != nullptr);
                return m_handle->AddLog(log_pair);
            }
            return false;
        }
        bool AddLog(const std::unique_ptr<CLogHandle>& handle) {
            ASSERT(handle != nullptr);
            if (!handle) return false;
            if (m_valid)
            {
                ASSERT(m_handle != nullptr);
                return m_handle->AddLog(std::reinterpret_pointer_cast<CLogEntry>(handle->GetHandle()));
            }
            return false;
        }
        int LogCounts() const {
            ASSERT(m_handle != nullptr);
            if (!m_handle) return false;
            return m_handle->GetLogSize();
        }
        std::shared_ptr<PVOID> GetHandle() const {
            if (m_valid)
                return std::reinterpret_pointer_cast<PVOID>(m_handle);
            return nullptr;
        }
        bool FreeHandle(CLogObject::logtype type = CLogObject::logtype::log_invalid) {
            if (type != CLogObject::logtype::log_invalid)
                m_handle->SetLogType(type);
            m_valid = false;
            return true;
        }
    private:
        bool m_valid;
        bool m_bhead;
        std::shared_ptr<CLogEntry> m_handle;
    };

#define LOGGER local_logger
#define BEGIN_LOG(NAME) \
	std::shared_ptr<CLogHandle> LOGGER = std::make_shared<CLogHandle>(NAME);
#define LOGGING(KEY, VALUE) \
	LOGGER->AddLog(LOGPAIR(KEY, VALUE));
#define END_LOG(LOGOBJ) \
	if (LOGGER->LogCounts()) \
		std::reinterpret_pointer_cast<CLogEntry>(LOGOBJ)->AddLog(std::reinterpret_pointer_cast<CLogEntry>(LOGGER->GetHandle()));
#define ENABLE_LOG() if(g_log_object != nullptr) g_log_object->EnableLog();
#define DISABLE_LOG() if(g_log_object != nullptr) g_log_object->DisableLog();
#define IS_LOGS_NULL() []() { if(g_log_object != nullptr) return g_log_object->IsLogsNull(); else return false; }()

#ifdef _OUTPUT_CONSOLE_STREAM
#define error_log(format,...) SAMPLE_LOG_STREAM(std::cerr,format, ##__VA_ARGS__)
#ifdef _DEBUG 
#define debug_log(format,...) SAMPLE_LOG_STREAM(std::cout,format, ##__VA_ARGS__)
#else
#define debug_log()
#endif
#else
    template<typename ...Args>
    void error_output(const char* format, const char* file, int line, Args...args)
    {
        std::stringstream os;
        cchips::special_log::sm_log(os, file, line, format, args...);
        ASSERT(os.str().length());
        if (!os.str().length()) return;
        std::unique_ptr<CLogHandle> error_handle = std::make_unique<CLogHandle>(ERROR_FEATURE, CLogObject::logtype::log_error);
        if (error_handle) (*error_handle).AddLog(LOGPAIR("error", os.str()));
        return;
    }
#define error_log(format,...) error_output(format, __FILE__,__LINE__, ##__VA_ARGS__);
#ifdef _DEBUG 
    template<typename ...Args>
    void debug_output(const char* format, const char* file, int line, Args...args)
    {
        std::stringstream os;
        cchips::special_log::sm_log(os, file, line, format, args...);
        ASSERT(os.str().length());
        if (!os.str().length()) return;
        std::unique_ptr<CLogHandle> debug_handle = std::make_unique<CLogHandle>(DEBUG_FEATURE, CLogObject::logtype::log_debug);
        if (debug_handle) (*debug_handle).AddLog(LOGPAIR("debug", os.str()));
        return;
    }
#define debug_log(format,...) debug_output(format, __FILE__,__LINE__, ##__VA_ARGS__);
#else
#define debug_log()
#endif
#endif

} // namespace cchips