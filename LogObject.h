#pragma once
#include <io.h>
#include <stdio.h>
#include <windows.h>
#include <string>
#include <vector>
#include "ExceptionThrow.h"
#include "SpecialLog.h"
#include "deffeature.h"
#include "SafePtr.h"
#include "R3LogObject.h"
#ifdef USING_PIPE_MESSAGE
#include "PipeJSONClient.h"
#endif

namespace cchips {

    class CLogEntry;
    class CLogObject;
    extern std::unique_ptr<CLogObject> g_log_object;
    
    class CLpcLocalObject : public CSocketObject
    {
    public:
#define filename_template "log_file.XXXXXX"
        CLpcLocalObject() : m_brunning(false) {
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
                m_tempfp = mkstemp((char*)filename_template);
            if (!m_tempfp)
                return false;
            m_getdata = getdata;
            m_brunning = true;
            return true;
        }
        void StopListen() override { return; }
        void StopConnect() override {
            if (ValidHandle())
                fclose(m_tempfp);
        }
        void Activated() override {
            auto func_write = [&](const char* buffer, int len) {
                if (!buffer || !len) return false;
                std::string unpack_buffer(buffer, len);
                if (unpack_buffer.length() == len)
                {
                    std::unique_ptr<CRapidJsonWrapper> wrapper_ptr(Unpack(unpack_buffer));
                    if (!wrapper_ptr || !wrapper_ptr->IsValid()) return false;
                    if (CRapidJsonWrapper::optstring opt_string = wrapper_ptr->Serialize(); opt_string)
                    {
                        if ((*opt_string).length())
                        {
                            std::lock_guard lock(m_mutex);
                            fputs(std::string(*opt_string).c_str(), m_tempfp);
                            fflush(m_tempfp);
                            return true;
                        }
                    }
                }
                return false;
            };
            do {
                std::unique_ptr<bson> data = Pack(m_getdata());
                if (data)
                {
                    m_logs_total_count++;
                    func_write(bson_data(&(*data)), bson_size(&(*data)));
                }
                else
                    break;
            } while (1);
        }
        void ClearLogsCount() override { m_logs_total_count = 0; }
        int GetTotalLogs() const override { return m_logs_total_count; }
    private:
        bool ValidHandle() const { if (m_tempfp == nullptr) return false; else return true; }

        // mkstemp extracted from libc/sysdeps/posix/tempname.c.
        FILE* mkstemp(char *tmpl);
        HANDLE m_sync_event;
        std::atomic_bool m_brunning;
        CSocketObject::func_getdata m_getdata;
        FILE* m_tempfp = nullptr;
        std::mutex m_mutex;
        int m_logs_total_count = 0;
    };
    
    class CLogObject
    {
    public:
#define MAX_CACHE_LOGS 100
        using logtype = enum _logtype {
            log_invalid = -1,
            log_event = 0,
            log_debug,
            log_error,
            log_exception,
        };
        using outputtype = enum _outputtype {
            output_invalid = -1,
            output_pipe = 1,
            output_local = 2,
        };

        ~CLogObject() {
#ifdef USING_PIPE_MESSAGE
            if (m_pipe_client)
                m_pipe_client->Stop();
#else
            if (m_socket_object)
                m_socket_object->StopConnect();
#endif
        }

        bool Initialize(const int mode = output_local);
#ifdef USING_PIPE_MESSAGE
        bool AddLog(RAPID_DOC_PAIR& log_pair, bool bhead = false) {
            if (!m_valid) return false;
            if (m_pipe_client != nullptr) {
                return m_pipe_client->SendMsg(std::move(log_pair.first), bhead);
            }
            return false;
        }
#else
        bool AddLog(std::shared_ptr<CLogEntry>& log, bool bhead = false) {
            if (!m_valid) return false;
            if (!log) return false;
            {
                std::lock_guard<std::recursive_mutex> lock_guard(m_recursive_mutex);
                if (m_cache_logs.size() >= MAX_CACHE_LOGS) return false;
                if (!bhead) m_cache_logs.push_back(log);
                else m_cache_logs.push_front(log);
            }
            if (m_socket_object)
                m_socket_object->Activated();
            return true;
        }
#endif
        RAPID_DOC_PAIR GetData();
        int GetTotalLogs() const { 
#ifdef USING_PIPE_MESSAGE
            return 0;
#else
            return m_socket_object->GetTotalLogs(); 
#endif
        }
        void EnableLog() { m_valid = true; }
        void DisableLog() { m_valid = false; }
        void EnableDupFlt() { 
#ifndef USING_PIPE_MESSAGE
            m_socket_object->EnableDuplicateFlt(); 
#endif
        }
        void DisableDupFlt() { 
#ifndef USING_PIPE_MESSAGE
            m_socket_object->DisableDuplicateFlt(); 
#endif
        }
        bool IsLogsNull() { 
            std::lock_guard<std::recursive_mutex> lock_guard(m_recursive_mutex);
            return m_cache_logs.empty(); 
        }
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
#ifdef USING_PIPE_MESSAGE
        std::unique_ptr<PipeJSONClient> m_pipe_client = nullptr;
#else
        std::unique_ptr<CSocketObject> m_socket_object = nullptr;
#endif   
        std::recursive_mutex m_recursive_mutex;
        std::list<std::shared_ptr<CLogEntry>> m_cache_logs;
    };

    class CLogEntry
    {
    public:
        CLogEntry(const std::string& name, CLogObject::logtype type = CLogObject::logtype::log_invalid) : log_name(name), m_log_type(type) {
            m_log_value.SetObject();
            m_checker.Update(name);
        }
        ~CLogEntry() = default;
        const std::string& GetName() const { return log_name; }
        const CLogObject::logtype GetLogType() const { return m_log_type; }
        void SetLogType(CLogObject::logtype type) { m_log_type = type; }
        const int GetLogSize() const { return (int)m_log_value.MemberCount(); }
        RapidValue& GetElements() { return m_log_value; }
        CChecker& GetChecker() { return m_checker; }
        bool AddLog(const LOGPAIR& log_pair) {
            ASSERT(log_pair.first.length());
            ASSERT(log_pair.second.length());
            if (!log_pair.first.length()) return false;
            if (!log_pair.second.length()) return false;
            if (GetLogSize())
            {
                const auto& it = m_log_value.FindMember(log_pair.first.c_str());
                if (it != m_log_value.MemberEnd()) return false;
            }
            ADD_JSON_LOG(m_log_document, m_log_value, log_pair.first.c_str(), log_pair.second.c_str());
            m_checker.Update(log_pair.first);
            m_checker.Update(log_pair.second);
            return true;
        }
        bool AddRapidValue(const LOGPAIR& log_pair) {
            ASSERT(log_pair.first.length());
            ASSERT(log_pair.second.length());
            if (!log_pair.first.length()) return false;
            if (!log_pair.second.length()) return false;
            if (GetLogSize())
            {
                const auto& it = m_log_value.FindMember(log_pair.first.c_str());
                if (it != m_log_value.MemberEnd()) return false;
            }
            std::unique_ptr<RapidDocument> log_document = std::make_unique<RapidDocument>();
            log_document->Parse(log_pair.second.data(), log_pair.second.length());
            if (!log_document->IsObject() || log_document->IsNull()) {
                //config data is incorrect.
                return false;
            }
            RapidValue* rapid_value = log_document.get();
            if (!rapid_value) return false;
            ADD_JSON_LOG(m_log_document, m_log_value, log_pair.first.c_str(), std::move(*rapid_value));
            m_checker.Update(log_pair.first);
            m_checker.Update(log_pair.second);
            return true;
        }
        bool AddLog(const std::shared_ptr<CLogEntry>& log_entry) {
            ASSERT(log_entry);
            if (!log_entry) return false;
            if (!log_entry->GetName().length()) return false;
            if (!log_entry->GetLogSize()) return false;
            RapidValue key(log_entry->GetName().c_str(), m_log_document.GetAllocator());
            RapidValue value(log_entry->GetElements(), m_log_document.GetAllocator());
            ADD_JSON_LOG(m_log_document, m_log_value, std::move(key), std::move(value));
            m_checker.Update(log_entry->GetChecker());
            return true;
        }
        RAPID_DOC_PAIR Serialize() {
            std::unique_ptr<RapidDocument> log_document = std::make_unique<RapidDocument>();
            std::unique_ptr<CChecker> checker = std::make_unique<CChecker>(m_checker);
            if (!log_document || !checker) return RAPID_DOC_PAIR(nullptr, nullptr);
            log_document->SetObject();
            log_document->CopyFrom(GetElements(), log_document->GetAllocator());
            //log_document->Swap(GetElements());
            RapidValue value(log_name.c_str(), log_document->GetAllocator());
            ADD_JSON_DOC(*log_document, "Action", std::move(value));
            return RAPID_DOC_PAIR(std::move(log_document), std::move(checker));
        }
    private:
        std::string log_name;
        CLogObject::logtype m_log_type;
        RapidValue m_log_value;
        RapidDocument m_log_document;
        CChecker m_checker;
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
#ifdef USING_PIPE_MESSAGE
                    g_log_object->AddLog(m_handle->Serialize(), m_bhead);
#else
                    g_log_object->AddLog(m_handle, m_bhead);
#endif
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
        bool AddRapidValue(const LOGPAIR& log_pair) {
            if (m_valid)
            {
                ASSERT(m_handle != nullptr);
                return m_handle->AddRapidValue(log_pair);
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
    template<typename ...Args>
    void info_output(const char* format, const char* file, int line, Args...args)
    {
        std::stringstream os;
        cchips::special_log::sm_log(os, file, line, format, args...);
        ASSERT(os.str().length());
        if (!os.str().length()) return;
        std::unique_ptr<CLogHandle> info_handle = std::make_unique<CLogHandle>(INFO_FEATURE, CLogObject::logtype::log_error);
        if (info_handle) (*info_handle).AddLog(LOGPAIR("info", os.str()));
        return;
    }
#define info_log(format,...) info_output(format, __FILE__,__LINE__, ##__VA_ARGS__);
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
    template<typename ...Args>
    void exploit_output(const char* format, Args...args)
    {
        std::stringstream os;
        cchips::special_log::sm_simple_log(os, format, args...);
        ASSERT(os.str().length());
        if (!os.str().length()) return;
        std::unique_ptr<CLogHandle> exploit_handle = std::make_unique<CLogHandle>(EXPLOIT_FEATURE, CLogObject::logtype::log_event);
        if (exploit_handle) (*exploit_handle).AddRapidValue(LOGPAIR("exploit", os.str()));
        return;
    }
#define exploit_log(format, ...) exploit_output(format, ##__VA_ARGS__);

} // namespace cchips