#pragma once
#include "PipeMessager.h"
#include <string>
#include <Windows.h>
#include <functional>
#include <thread>
#include <atomic>
#include <assert.h>

#ifdef CCHIPS_EXTERNAL_USE
#include <base/command_line.h>
#include <base/logging.h>
#endif

typedef std::function<void(const std::string&)> string_callback_function;

class PipeServer : public std::enable_shared_from_this<PipeServer> {
public:
    PipeServer(const std::wstring& pipe_name) : pipe_name_(pipe_name), server_thread_(nullptr), message_count_(0) {
        client_connected_event_ = CreateEvent(NULL, FALSE, FALSE, NULL);
        server_stop_event_ = CreateEvent(NULL, FALSE, FALSE, NULL);
    }
    ~PipeServer() {
        Stop();
        if (client_connected_event_)
            CloseHandle(client_connected_event_);
        if (server_stop_event_)
            CloseHandle(server_stop_event_);
        client_connected_event_ = nullptr;
        server_stop_event_ = nullptr;
        return;
    }
    bool Start(string_callback_function callback) {
        assert(server_thread_ == nullptr);
        if (server_thread_) return false;

        server_thread_ = std::make_unique<std::thread>(std::bind(&PipeServer::WorkThread, this));

        string_callback_ = callback;

        if (!string_callback_) return false;
        if (!server_thread_) return false;

        return true;
    }
    void Stop() {
        SetEvent(server_stop_event_);
        if (server_thread_ && server_thread_->joinable())
            server_thread_->join();
        server_thread_ = nullptr;
        return;
    }
    int GetMessageCount() { return message_count_; }
    void ClearMessageCount() { message_count_ = 10; }

protected:
    typedef struct _PIPE_INSTANCE {
        OVERLAPPED overlap;
        HANDLE pipe_handle;
        CHAR message[BUFFER_SIZE];
        DWORD message_length;
        std::shared_ptr<PipeServer> this_pipe_server;
    } PIPE_INSTANCE, *LPPIPE_INSTANCE;

    std::wstring pipe_name_;
    std::unique_ptr<std::thread> server_thread_;
    string_callback_function string_callback_;
    std::atomic_int message_count_;
    HANDLE client_connected_event_;
    HANDLE server_stop_event_;
    std::string message_;

    bool CreateAndConnectPipeInstance(LPHANDLE pipe_handle, LPOVERLAPPED overlap) {
        *pipe_handle = CreateNamedPipeW(pipe_name_.c_str(),
            PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES,
            BUFFER_SIZE, BUFFER_SIZE, PIPE_TIMEOUT, NULL);

        if (*pipe_handle == INVALID_HANDLE_VALUE) {
#ifdef CCHIPS_EXTERNAL_USE
            LOG(ERROR) << "CreateNamedPipe() failed with error code " << GetLastError();
#endif
            return 0;
        }
        return ConnectWithNewClient(pipe_handle, overlap);
    }
    bool ConnectWithNewClient(LPHANDLE pipe_handle, LPOVERLAPPED overlap) {
        bool is_connected, is_io_pending = FALSE;
        is_connected = ConnectNamedPipe(*pipe_handle, overlap);

        if (is_connected) {
#ifdef CCHIPS_EXTERNAL_USE
            LOG(ERROR) << "ConnectNamedPipe() failed with error code " << GetLastError();
#endif
            return 0;
        }
        switch (GetLastError()) {
        case ERROR_IO_PENDING:
            // The overlapped connection in progress
            is_io_pending = TRUE;
            break;
        case ERROR_PIPE_CONNECTED:
            // Pipe is already connected with a client, so signal an event
            SetEvent(overlap->hEvent); 
            break;
        default: {
#ifdef CCHIPS_EXTERNAL_USE
            LOG(ERROR) << "ConnectNamedPipe() failed with error code " << GetLastError();
#endif
            return 0;
        }
        }
        return is_io_pending;
    }
    bool IsConnectedWithClient(LPHANDLE pipe_handle, LPOVERLAPPED overlap, bool is_io_pending) {
        // make sure a client is succesfully connected if was io pending

        if (!is_io_pending) return true;

        bool is_connect;
        DWORD dw_bytes_transferred;
        if (is_io_pending) {
            is_connect = GetOverlappedResult(pipe_handle, overlap, &dw_bytes_transferred, FALSE);
            if (!is_connect) {
#ifdef CCHIPS_EXTERNAL_USE
                LOG(ERROR) << "ConnectNamedPipe() failed with error code " << GetLastError();
#endif
                return false;
            }
        }
        return true;
    }
    static VOID WINAPI ReadMessage(DWORD dw_error, DWORD dw_number_of_bytes, LPOVERLAPPED overlap) {
        LPPIPE_INSTANCE pipe;
        BOOL read_success = FALSE;
        pipe = (LPPIPE_INSTANCE)overlap;

        //TODO: get a proper if-condition here
        if ((dw_number_of_bytes == pipe->message_length) || (pipe->message_length == 0))
            read_success =
            ReadFileEx(pipe->pipe_handle, pipe->message, BUFFER_SIZE, (LPOVERLAPPED)pipe,
            (LPOVERLAPPED_COMPLETION_ROUTINE)ReadMessageComplete);

        if (!read_success)
            DisconnectAndClose(pipe);
    }
    static VOID WINAPI ReadMessageComplete(DWORD dw_error, DWORD bytes_read, LPOVERLAPPED overlap) {
        if (bytes_read == 0)
            return;
        LPPIPE_INSTANCE pipe = (LPPIPE_INSTANCE)overlap;
        //TODO: check if this is a valid condition
        bool is_message_ended = overlap->Internal == ERROR_SUCCESS;
        std::shared_ptr<PipeServer> this_pipe_server = pipe->this_pipe_server;

        this_pipe_server->message_ += std::string(&pipe->message[0], bytes_read);

        ReadMessage(0, 0, (LPOVERLAPPED)pipe);
        if (is_message_ended) {
            this_pipe_server->message_count_++;
            if (this_pipe_server->string_callback_)
                this_pipe_server->string_callback_(this_pipe_server->message_);
            this_pipe_server->message_.clear();
        }
    }
    static VOID WINAPI DisconnectAndClose(LPPIPE_INSTANCE pipe) {
        if (!DisconnectNamedPipe(pipe->pipe_handle)) {
#ifdef CCHIPS_EXTERNAL_USE
            LOG(ERROR) << "DisconnectNamedPipe() failed with error code " << GetLastError();
#endif
        }
        else {
#ifdef CCHIPS_EXTERNAL_USE
            LOG(INFO) << "DisconnectNamedPipe() OK.";
#endif
        }

        CloseHandle(pipe->pipe_handle);
        if (pipe != NULL) GlobalFree(pipe);
    }
    void WorkThread() {
    assert(client_connected_event_);
    assert(server_stop_event_);

    OVERLAPPED client_connection;
    LPPIPE_INSTANCE pipe;
    DWORD wait_result;
    BOOL is_io_pending;
    HANDLE pipe_handle;

    // client_connected_event_ will be set to a signaled state once a client is connected
    client_connection.hEvent = client_connected_event_;
    is_io_pending = CreateAndConnectPipeInstance(&pipe_handle, &client_connection);

    HANDLE server_events[2] = { client_connected_event_, server_stop_event_ };

    while (1) {
        wait_result = WaitForMultipleObjectsEx(2, server_events, FALSE, INFINITE, TRUE);

        switch (wait_result) {
        case WAIT_OBJECT_0 + 0:
            if (!IsConnectedWithClient(&pipe_handle, &client_connection, is_io_pending)) return;

            pipe = (LPPIPE_INSTANCE)GlobalAlloc(GPTR, sizeof(PIPE_INSTANCE));
            if (pipe == NULL) {
#ifdef CCHIPS_EXTERNAL_USE
                LOG(ERROR) << "GlobalAlloc() for buffer failed with error code " << GetLastError();
#endif
                return;
            }

            pipe->pipe_handle = pipe_handle;
            pipe->message_length = 0;
            pipe->this_pipe_server = shared_from_this();

            ReadMessage(0, 0, (LPOVERLAPPED)pipe);

            is_io_pending = CreateAndConnectPipeInstance(&pipe_handle, &client_connection);

            break;
        case WAIT_OBJECT_0 + 1:
#ifdef CCHIPS_EXTERNAL_USE
            LOG(INFO) << "Disconnecting Tracer Log Server...";
#endif
            if (pipe_handle) {
                CloseHandle(pipe_handle);
            }
            return;
        case WAIT_IO_COMPLETION:
            break;
        default: {
#ifdef CCHIPS_EXTERNAL_USE
            LOG(ERROR) << "WaitForSingleObjectEx() failed with error code %d\n" << GetLastError();
#endif
            return;
        }
        }
    }
    return;
}
};

