#pragma once
#include <io.h>
#include <stdio.h>
#include <windows.h>
#include <sddl.h>
#include <string>
#include <random>
#include <chrono>
#include <atomic>
#include <sstream>
#include <cassert>
#include <functional>
#include <thread>
#include <mutex>
#include "rapidjson\document.h"
#include "rapidjson\writer.h"
#include "PackageWrapper.h"


namespace cchips {

    class CSocketObject : public CBsonWrapper
    {
    public:
        using outputtype = enum _outputtype {
            output_invalid = -1,
            output_pipe = 0,
            output_local,
        };
        using func_callback = std::function<void(const std::unique_ptr<CRapidJsonWrapper>)>;
        using func_getdata = std::function<RAPID_DOC_PAIR()>;
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
        virtual void ClearLogsCount() = 0;
        virtual int GetTotalLogs() const = 0;
    protected:
        void SetOutputType(outputtype type) { m_output_type = type; }
    private:
        outputtype m_output_type;
    };

    class CLpcPipeObject : public CSocketObject
    {
    public:
#define lpc_pipe_name "\\\\.\\pipe\\hipshook_IPC_services"
#define lpc_security_sddl "S:(ML;;NW;;;LW)"
        CLpcPipeObject() : m_brunning(false), m_listen_thread(nullptr) {
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
            SetEvent(m_sync_event);
            if (m_listen_thread && m_listen_thread->joinable())
                m_listen_thread->join();
            m_listen_thread = nullptr;
            return;
        }
        bool Reconnect() {
            auto func_open = [&]() {
                int count = 0;
                while (count < CSocketObject::lpc_connect_try_times)
                {
                    m_connect_pipe = CreateFileA(lpc_pipe_name, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
                    if (ValidHandle(m_connect_pipe))
                        break;
                    if (DWORD error = GetLastError() != ERROR_PIPE_BUSY || !m_brunning)
                    {
                        warning("The log pipe handler connect failed, last error %d.", error);
                        break;
                    }
                    if (!WaitNamedPipeA(lpc_pipe_name, lpc_connect_wait_timeout))
                    {
                        if (GetLastError() != ERROR_SEM_TIMEOUT)
                        {
                            warning("wait name pipe failed, last error %d.", error);
                            break;
                        }
                        warning("Could not open pipe: 0.1 second wait timed out.");
                        std::this_thread::sleep_for(std::chrono::microseconds(100));
                        continue;
                    }
                    count++;
                }
                if (ValidHandle(m_connect_pipe))
                    return true;
                return false;
            };
            if (!func_open()) return false;
            m_brunning = true;
            return true;
        }
        bool Connect(CSocketObject::func_getdata& getdata) override {
            m_getdata = getdata;
            return Reconnect();
        }
        void StopConnect() override {
            m_brunning = false;
            if (ValidHandle(m_connect_pipe)) CloseHandle(m_connect_pipe);
            return;
        }
        void Activated() override { 
            auto func_write = [&](const char* buffer, int len) {
                if (!buffer || !len) return false;
                std::lock_guard lock(m_hpipes_mutex);
                while (len)
                {
                    DWORD byte_writes = 0;
                    bool bsuccess = WriteFile(m_connect_pipe, buffer, len, &byte_writes, nullptr);
                    if (!bsuccess)
                    {
                        if (GetLastError() == ERROR_INVALID_HANDLE)
                        {
                            // It is possible that malware closes our pipe handle. In that
                            // case we'll get an invalid handle error. Let's just open a new
                            // pipe handle.
                            if (!Reconnect())
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

            while(m_brunning) {
                std::unique_ptr<bson> data = Pack(m_getdata());
                if (data)
                {
                    m_logs_total_count++;
                    func_write(bson_data(&(*data)), bson_size(&(*data)));
                }
                else
                    break;
            }
        }
        //for test
        void ClearLogsCount() override { m_logs_total_count = 0; }
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
            using ConvertStringSecurityDescriptorToSecurityDescriptorA_Define = NTSTATUS(WINAPI*)(LPCSTR StringSecurityDescriptor, DWORD StringSDRevision, PSECURITY_DESCRIPTOR *SecurityDescriptor, PULONG SecurityDescriptorSize);
            ConvertStringSecurityDescriptorToSecurityDescriptorA_Define tosecuritydescriptora_func = reinterpret_cast<ConvertStringSecurityDescriptorToSecurityDescriptorA_Define>(GetProcAddress(GetModuleHandleA("advapi32"), "ConvertStringSecurityDescriptorToSecurityDescriptorA"));
            if (tosecuritydescriptora_func) {
                memset(&sa, 0, sizeof(SECURITY_ATTRIBUTES));
                if (tosecuritydescriptora_func(lpc_security_sddl, SDDL_REVISION_1, &psd, &size))
                {
                    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
                    sa.bInheritHandle = FALSE;
                    sa.lpSecurityDescriptor = psd;
                    psa = &sa;
                }
                else {
                    psd = NULL;
                }
            }

            DWORD open_mode = PIPE_ACCESS_INBOUND | FILE_FLAG_FIRST_PIPE_INSTANCE | FILE_FLAG_OVERLAPPED;
            if (!bfirst) open_mode = PIPE_ACCESS_INBOUND | FILE_FLAG_OVERLAPPED;
            hPipe = CreateNamedPipeA(
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
            if (m_sync_event)
                CloseHandle(m_sync_event);
            m_sync_event = nullptr;
            return;
        }
        DWORD WINAPI ReceiveThread(std::shared_ptr<HANDLE> hpipe, func_callback callback) {
            bool bsuccess = false;
            DWORD bytes_read = 0;
            DWORD error;
            if (!hpipe) return 0;
            std::vector<char> recv_buffer(CSocketObject::lpc_buffer_kb * 1024);
            std::string decode_buffer;
            size_t decode_len = 0;
            HANDLE read_event = CreateEvent(NULL, FALSE, FALSE, NULL);
            OVERLAPPED overlapped = {};
            overlapped.hEvent = read_event;
            HANDLE handle_array[2];
            handle_array[0] = overlapped.hEvent;
            handle_array[1] = m_sync_event;

            while (m_brunning)
            {
                bsuccess = ReadFile(*hpipe, &recv_buffer[0], (DWORD)recv_buffer.size(), &bytes_read, nullptr);
                if (bsuccess || (error = GetLastError()) == ERROR_MORE_DATA)
                {
#ifdef CCHIPS_EXTERNAL_USE
                    r3_debug_log("ReceiveThread: receive a r3 log.");
#endif
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
                    {
                        std::lock_guard lock(m_hpipes_mutex);
                        if ((*hpipe) != INVALID_HANDLE_VALUE)
                        {
                            FlushFileBuffers(*hpipe);
                            CloseHandle(*hpipe);
                        }
                        *hpipe = INVALID_HANDLE_VALUE;
                    }
                    break;
                }
                else
                {
                    warning("The log pipe handler has failed, last error %d.", error);
                    break;
                }
            }
            {
                std::lock_guard lock(m_hpipes_mutex);
                if (*hpipe != INVALID_HANDLE_VALUE)
                {
                    FlushFileBuffers(*hpipe);
                    DisconnectNamedPipe(*hpipe);
                    CloseHandle(*hpipe);
                    *hpipe = INVALID_HANDLE_VALUE;
                }
            }
            return 0;
        }
        VOID WINAPI ListenThread(func_callback callback) {
            std::shared_ptr<HANDLE> hpipe =std::make_shared<HANDLE>(StartLPCServer());
            assert(m_sync_event);
            if (hpipe == nullptr) return;
            if (*hpipe == INVALID_HANDLE_VALUE) return;
            if (!m_sync_event) return;

            OVERLAPPED overlapped;
            overlapped.hEvent = m_sync_event;
            std::vector<std::pair<std::unique_ptr<std::thread>, std::shared_ptr<HANDLE>>> receive_threads;
            while (m_brunning)
            {
                DWORD error = 0;
                if (ConnectNamedPipe(*hpipe, &overlapped) ?
                    TRUE : ((error = GetLastError()) == ERROR_PIPE_CONNECTED))
                {
#ifdef CCHIPS_EXTERNAL_USE
                    r3_debug_log("ConnectNamedPipe success!");
#endif
                    std::unique_ptr<std::thread> receive_thread = std::make_unique<std::thread>(&CLpcPipeObject::ReceiveThread, this, hpipe, callback);
                    receive_threads.push_back(std::pair<std::unique_ptr<std::thread>, std::shared_ptr<HANDLE>>(std::move(receive_thread), hpipe));
                    hpipe = std::make_shared<HANDLE>(StartLPCServer(false));
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
            if (*hpipe != INVALID_HANDLE_VALUE)
                CloseHandle(*hpipe);
            for (auto& thread : receive_threads)
            {
                if (thread.first->joinable())
                {
                    {
                        std::lock_guard lock(m_hpipes_mutex);
                        if (*thread.second != INVALID_HANDLE_VALUE)
                        {
                            FlushFileBuffers(*thread.second);
                            DisconnectNamedPipe(*thread.second);
                            CloseHandle(*thread.second);
                            *thread.second = INVALID_HANDLE_VALUE;
                        }
                    }
                    thread.first->join();
                }
            }
            return;
        }
        bool ValidHandle(HANDLE hpipe) const { if (hpipe == INVALID_HANDLE_VALUE || hpipe == nullptr) return false; else return true; }
        HANDLE m_sync_event = nullptr;
        HANDLE m_connect_pipe = nullptr;
        CSocketObject::func_getdata m_getdata;
        std::atomic_bool m_brunning;
        std::unique_ptr<std::thread> m_listen_thread;
        std::mutex m_hpipes_mutex;
        //for test
        std::atomic_int m_logs_total_count = 0;
    };

} // namespace cchips
