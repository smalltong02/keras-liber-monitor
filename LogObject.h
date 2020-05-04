#pragma once
#include <io.h>
#include <stdio.h>
#include <windows.h>
#include <string>
#include <vector>
#include <random>
#include <mutex>
#include <atomic>
#include <chrono>
#include <sddl.h>
#include "ExceptionThrow.h"
#include "SpecialLog.h"
#include "deffeature.h"
#include "SafePtr.h"
extern "C" {
#include "bson\bson.h"
}

namespace cchips {

    using LOGPAIR = std::pair<std::string, std::string>;
    class CLogEntry;
    class CLogObject;
    extern std::unique_ptr<CLogObject> g_log_object;

#define warning()
    class CBsonWrapper
    {
    public:
        CBsonWrapper() = default;
        ~CBsonWrapper() = default;

        std::unique_ptr<bson> Pack(const std::unique_ptr<LOGPAIR>& data) {
            if (!data) return nullptr;
            std::string time_string;
            auto func_get_timestamp = [](std::string& time_buffer) {
#define time_buffer_size 100
                time_buffer.resize(time_buffer_size);
                auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                std::strftime(&time_buffer[0],
                    time_buffer_size,
                    "%Y-%m-%d %H:%M:%S",
                    std::localtime(&now));
                return;
            };
            std::unique_ptr<bson> bson_ptr = std::make_unique<bson>();
            assert(bson_ptr != nullptr);
            if (!bson_ptr) return nullptr;
            bson_init(&(*bson_ptr));
            func_get_timestamp(time_string);
            bson_append_string(&(*bson_ptr), "TraceTime", time_string.c_str());
            bson_append_string(&(*bson_ptr), "Action", data->first.c_str());
            bson_append_string(&(*bson_ptr), "R3Log", data->second.c_str());
            bson_append_finish_array(&(*bson_ptr));
            bson_finish(&(*bson_ptr));
            return bson_ptr;
        }
        std::unique_ptr<std::stringstream> Unpack(const std::string& decode_buffer) {
            assert(decode_buffer.length());
            if (!decode_buffer.length()) return nullptr;
            bson bson_unpack;
            if (bson_init_finished_data(&bson_unpack, (char*)&decode_buffer[0], false) == BSON_OK)
            {
                bson_iterator i;
                bson_iterator_from_buffer(&i, bson_data(&bson_unpack));
                std::unique_ptr<std::stringstream> ss_ptr = std::make_unique<std::stringstream>();
                if (!ss_ptr) return nullptr;
                while (bson_iterator_next(&i)) {
                    bson_type t = bson_iterator_type(&i);
                    if (t == 0)
                        break;
                    const char* key = bson_iterator_key(&i);
                    if (key[0] == '\0') continue;
                    switch (t) {
                    case BSON_STRING:
                    {
                        if (!(*ss_ptr).str().length())
                            (*ss_ptr) << "{ ";
                        else
                            (*ss_ptr) << "; ";
                        (*ss_ptr) << "\"" << key << "\"" << ": " << "\"" << bson_iterator_string(&i) << "\"";
                    }
                    break;
                    default:
                        warning("can't get bson type : %d\n", t);
                    }
                }
                if ((*ss_ptr).str().length())
                {
                    (*ss_ptr) << " }";
                    return std::move(ss_ptr);
                }
            }
            return nullptr;
        }

    private:
    };

    class CSocketObject : public CBsonWrapper
    {
    public:
        using outputtype = enum _outputtype {
            output_invalid = -1,
            output_pipe = 0,
            output_local,
        };
        using func_callback = std::function<void(const std::unique_ptr<std::stringstream>)>;
        using func_getdata = std::function<std::unique_ptr<LOGPAIR>()>;
        CSocketObject() : m_output_type(output_invalid) { ; }
        ~CSocketObject() = default;

        virtual bool Listen(func_callback& callback) = 0;
        virtual bool Connect(func_getdata& getdata) = 0;
        virtual void StopListen() = 0;
        virtual void StopConnect() = 0;
        virtual void Activated() = 0;

        static const size_t lpc_buffer_kb;
        static const DWORD lpc_server_wait_timeout;
        static const DWORD lpc_client_wait_timeout;
        static const DWORD lpc_connect_wait_timeout;
        static const int lpc_connect_try_times;
        // for test
        virtual int GetTotalLogs() const = 0;
    protected:
        void SetOutputType(outputtype type) { m_output_type = type; }
    private:
        outputtype m_output_type;
    };

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

    class CLpcPipeObject : public CSocketObject
    {
    public:
#define lpc_pipe_name "\\\\.\\pipe\\hipshook_IPC_services"
#define lpc_security_sddl "S:(ML;;NW;;;LW)"
        CLpcPipeObject() : m_brunning(false), m_listen_thread(nullptr), m_connect_thread(nullptr) {
            m_sync_event = CreateEvent(NULL, FALSE, FALSE, NULL);
            SetOutputType(CSocketObject::output_pipe);
        }
        ~CLpcPipeObject() {
            StopLpcService();
        }

        const std::string& GetPipeName() const { return lpc_pipe_name; }
        bool Listen(CSocketObject::func_callback& callback) override {
            assert(m_listen_thread == nullptr);
            if (m_listen_thread) return false;
            m_listen_thread = std::make_unique<std::thread>(std::bind(&CLpcPipeObject::ListenThread, this, callback));
            assert(m_listen_thread != nullptr);
            if (!m_listen_thread) return false;
            return true;
        }
        void StopListen() override {
            m_brunning = false;
            if (m_listen_thread && m_listen_thread->joinable())
                m_listen_thread->join();
            m_listen_thread = nullptr;
            return;
        }
        bool Connect(CSocketObject::func_getdata& getdata) override {
            assert(m_connect_thread == nullptr);
            if (m_connect_thread) return false;
            m_connect_thread = std::make_unique<std::thread>(std::bind(&CLpcPipeObject::ConnectThread, this, getdata));
            assert(m_connect_thread != nullptr);
            if (!m_connect_thread) return false;
            return true;
        }
        void StopConnect() override {
            m_brunning = false;
            if (m_connect_thread && m_connect_thread->joinable())
                m_connect_thread->join();
            m_connect_thread = nullptr;
            return;
        }
        void Activated() override { if (m_sync_event) SetEvent(m_sync_event); }
        //for test
        int GetTotalLogs() const override { return m_logs_total_count; }

    private:

        HANDLE StartLPCServer(bool bfirst = true) {
            HANDLE hPipe = INVALID_HANDLE_VALUE;
            SECURITY_ATTRIBUTES sa;
            PSECURITY_ATTRIBUTES psa = NULL;
            PSECURITY_DESCRIPTOR psd = NULL;
            ULONG size;
            // reference: http://stackoverflow.com/questions/3282365/opening-a-named-pipe-in-low-integrity-level
            // for ensure all of the be injected samples could write log to tracer by named pipe, the server need low integrity level
            memset(&sa, 0, sizeof(SECURITY_ATTRIBUTES));
            if (ConvertStringSecurityDescriptorToSecurityDescriptor(lpc_security_sddl, SDDL_REVISION_1, &psd, &size))
            {
                sa.nLength = sizeof(SECURITY_ATTRIBUTES);
                sa.bInheritHandle = FALSE;
                sa.lpSecurityDescriptor = psd;
                psa = &sa;
            }
            else {
                psd = NULL;
            }

            DWORD open_mode = PIPE_ACCESS_INBOUND | FILE_FLAG_FIRST_PIPE_INSTANCE | FILE_FLAG_OVERLAPPED;
            if (!bfirst) open_mode = PIPE_ACCESS_INBOUND | FILE_FLAG_OVERLAPPED;
            hPipe = CreateNamedPipe(
                lpc_pipe_name,
                // This mode gives the server the equivalent of GENERIC_READ access to the pipe. The client must specify GENERIC_WRITE access when connecting to the pipe.
                // Overlapped mode is enabled.
                open_mode,
                PIPE_TYPE_BYTE | PIPE_WAIT,
                PIPE_UNLIMITED_INSTANCES,
                (DWORD)(CSocketObject::lpc_buffer_kb * 1024),
                (DWORD)(CSocketObject::lpc_buffer_kb * 1024),
                NMPWAIT_USE_DEFAULT_WAIT,
                psa);

            if (psd) LocalFree(psd);

            if (hPipe == nullptr) hPipe = INVALID_HANDLE_VALUE;
            if (hPipe == INVALID_HANDLE_VALUE)
                return INVALID_HANDLE_VALUE;

            if (!m_brunning)
            {
                m_brunning = true;
            }
            return hPipe;
        }
        void StopLpcService() {

            if (ValidHandle())
                CloseHandle(m_pipe_handle);
            if (m_sync_event)
                CloseHandle(m_sync_event);
            m_pipe_handle = INVALID_HANDLE_VALUE;
            m_sync_event = nullptr;
            return;
        }
        DWORD WINAPI ReceiveThread(HANDLE hpipe, func_callback callback) {
            bool bsuccess = false;
            DWORD bytes_read = 0;
            DWORD error;
            std::vector<char> recv_buffer(CSocketObject::lpc_buffer_kb * 1024);
            std::string decode_buffer;
            size_t decode_len = 0;

            while (m_brunning)
            {
                bsuccess = ReadFile(hpipe, &recv_buffer[0], (DWORD)recv_buffer.size(), &bytes_read, nullptr);
                if (bsuccess || (error = GetLastError()) == ERROR_MORE_DATA)
                {
                    decode_buffer += std::string(&recv_buffer[0], bytes_read);
                    assert(decode_buffer.length());
                    if (decode_buffer.length() < sizeof(ULONG))
                        continue;
                    decode_len = *reinterpret_cast<ULONG*>(&decode_buffer[0]);
                    while (decode_len <= decode_buffer.length())
                    {
                        m_logs_total_count++;
                        callback(Unpack(decode_buffer));
                        decode_buffer = decode_buffer.substr(decode_len);
                        if (decode_buffer.length() < sizeof(ULONG))
                            break;
                        decode_len = *reinterpret_cast<ULONG*>(&decode_buffer[0]);
                        if (decode_len == 0)
                        {
                            decode_buffer.clear();
                            break;
                        }
                    }
                }
                else if (error == ERROR_BROKEN_PIPE)
                {
                    // If we get the broken pipe error then this pipe connection has
                    // been terminated for one reason or another.So break from the
                    // loop and make the socket "inactive", that is, another pipe
                    // connection can in theory pick it up. (This will only happen in
                    // cases where malware for some reason broke our pipe connection).
                    FlushFileBuffers(hpipe);
                    CloseHandle(hpipe);
                    hpipe = INVALID_HANDLE_VALUE;
                    break;
                }
                else
                    warning("The log pipe handler has failed, last error %d.", error);
            }
            if (hpipe != INVALID_HANDLE_VALUE)
            {
                FlushFileBuffers(hpipe);
                DisconnectNamedPipe(hpipe);
                CloseHandle(hpipe);
            }
            return 0;
        }
        VOID WINAPI ListenThread(func_callback callback) {
            HANDLE hpipe = StartLPCServer();
            assert(m_sync_event);
            if (hpipe == INVALID_HANDLE_VALUE) return;
            if (!m_sync_event) return;

            OVERLAPPED overlapped;
            overlapped.hEvent = m_sync_event;
            std::vector<std::unique_ptr<std::thread>> receive_threads;
            while (m_brunning)
            {
                DWORD error = 0;
                if (ConnectNamedPipe(hpipe, &overlapped) ?
                    TRUE : ((error = GetLastError()) == ERROR_PIPE_CONNECTED))
                {
                    std::unique_ptr<std::thread> receive_thread = std::make_unique<std::thread>(&CLpcPipeObject::ReceiveThread, this, hpipe, callback);
                    receive_threads.push_back(std::move(receive_thread));
                    hpipe = StartLPCServer(false);
                }
                else if (error == ERROR_IO_PENDING)
                {
                    while (m_brunning) {
                        DWORD ret = WaitForSingleObject(overlapped.hEvent, CSocketObject::lpc_server_wait_timeout);
                        if (ret == WAIT_OBJECT_0) {
                            break;
                        }
                        else if (ret == WAIT_TIMEOUT) {
                            continue;
                        }
                        else {
                            // other error
                            m_brunning = false;
                            break;
                        }
                    }
                }
            }
            if (hpipe != INVALID_HANDLE_VALUE)
                CloseHandle(hpipe);
            for (auto& thread : receive_threads)
                if (thread->joinable())
                    thread->join();
            return;
        }
        VOID WINAPI ConnectThread(CSocketObject::func_getdata& getdata) {
            assert(m_pipe_handle == INVALID_HANDLE_VALUE);
            assert(m_sync_event);
            if (m_pipe_handle != INVALID_HANDLE_VALUE)
                return;
            if (!m_sync_event) return;
            auto func_open = [&]() {
                int count = 0;
                while (count < CSocketObject::lpc_connect_try_times)
                {
                    m_pipe_handle = CreateFile(lpc_pipe_name, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
                    if (ValidHandle())
                        break;
                    if (DWORD error = GetLastError() != ERROR_PIPE_BUSY)
                    {
                        warning("The log pipe handler connect failed, last error %d.", error);
                        break;
                    }
                    if (WaitNamedPipe(lpc_pipe_name, lpc_connect_wait_timeout))
                    {
                        warning("Could not open pipe: 0.1 second wait timed out.");
                        break;
                    }
                }
                if (ValidHandle())
                    return true;
                return false;
            };
            auto func_write = [&](const char* buffer, int len) {
                while (len)
                {
                    DWORD byte_writes = 0;
                    bool bsuccess = WriteFile(m_pipe_handle, buffer, len, &byte_writes, nullptr);
                    if (!bsuccess)
                    {
                        if (GetLastError() == ERROR_INVALID_HANDLE)
                        {
                            // It is possible that malware closes our pipe handle. In that
                            // case we'll get an invalid handle error. Let's just open a new
                            // pipe handle.
                            if (!func_open())
                                break;
                        }
                        else
                        {
                            warning("CRITICAL:send log by pipe failed. error %d", GetLastError());
                            break;
                        }
                    }
                    buffer += byte_writes; len -= byte_writes;
                }
                if (len)
                    return false;
                return true;
            };

            if (!func_open())
            {
                // The optional is to write log to a local temp directory.
                return;
            }
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
        bool ValidHandle() const { if (m_pipe_handle == INVALID_HANDLE_VALUE || m_pipe_handle == nullptr) return false; else return true; }
        HANDLE m_pipe_handle = INVALID_HANDLE_VALUE;
        HANDLE m_sync_event = nullptr;
        std::atomic_bool m_brunning;
        std::unique_ptr<std::thread> m_listen_thread;
        std::unique_ptr<std::thread> m_connect_thread;
        //for test
        std::atomic_int m_logs_total_count = 0;
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
            std::function<std::unique_ptr<LOGPAIR>()> getdata(std::bind(&CLogObject::GetData, this));
            std::unique_ptr<CLpcPipeObject> pipe_ptr = std::make_unique<CLpcPipeObject>();
            if (pipe_ptr->Connect(getdata))
                m_socket_object = std::move(pipe_ptr);
            else
            {
                std::unique_ptr<CLpcLocalObject> local_ptr = std::make_unique<CLpcLocalObject>();
                if (local_ptr->Connect(getdata))
                    m_socket_object = std::move(local_ptr);
            }
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
                ss << entry.first << ": " << entry.second;
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
                ss << elem.first << ": " << elem.second;
            }
            if (ss.str().length())
            {
                ss << " }";
                return std::make_unique<LOGPAIR>(log_name, ss.str());
            }
            return nullptr;
        }

        iterator		begin() { return log_elements.begin(); }
        const_iterator	begin() const { return log_elements.begin(); }
        iterator		end() { return log_elements.end(); }
        const_iterator	end() const { return log_elements.end(); }
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
        bool FreeHandle() {
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