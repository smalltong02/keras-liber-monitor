#pragma once
#include "PipeMessager.h"
#include <Windows.h>
#include <string>
#include <atomic>
#include <thread>
#include <list>
#include <mutex>
#include <assert.h>
#include <algorithm>

#ifdef CCHIPS_EXTERNAL_USE
#include <base/command_line.h>
#include <base/logging.h>
#endif

typedef std::string Message;

class PipeClient {
public:
    PipeClient(const std::wstring& pipe_name) : pipe_name_(pipe_name), is_local(false), brunning_(false) {}
    ~PipeClient() {
        if (pipe_handle_) {
            CloseHandle(pipe_handle_);
        }
        return;
    }
    bool Start() {
        std::wstring prefix_pipe = L"\\\\.\\pipe\\";
        std::wstring prefix_local = L"LocalFile\\";

        auto pipe = std::mismatch(prefix_pipe.begin(), prefix_pipe.end(), pipe_name_.begin());
        auto local = std::mismatch(prefix_local.begin(), prefix_local.end(), pipe_name_.begin());

        if (pipe.first == prefix_pipe.end()) {
            // write to pipe
            is_local = false;
            if (!ConnectPipe()) return false;
        }
        else if (local.first == prefix_local.end()) {
            // write to local file
            is_local = true;
            if (!CreateLocalFile(pipe_name_.substr(prefix_local.length()))) return false;
        }
        brunning_ = true;
        return true;
    }
    void Stop() {
        brunning_ = false;
        if (pipe_handle_) {
            CloseHandle(pipe_handle_);
        }
        pipe_handle_ = nullptr;
        return;
    }
    bool SendMsg(Message& message, bool insert_to_front = false) {
        assert(IsValidMessage(message));
        if (!IsValidMessage(message)) return false;
        WritePipe(message);
        return true;
    }

protected:
    std::wstring pipe_name_; // use "LocalFile\\" as prefix to save logs to local file
    HANDLE pipe_handle_; // used as a file handle if in local mode
    std::mutex message_list_mutex_;
    bool is_local;
    std::atomic_bool brunning_;

    bool ConnectPipe() {
        DWORD dw_mode;
        BOOL set_mode_success;

        while (1) {
            pipe_handle_ = CreateFileW(pipe_name_.c_str(), GENERIC_WRITE | GENERIC_READ,
                0, NULL, OPEN_EXISTING, 0, NULL);

            if (pipe_handle_ != INVALID_HANDLE_VALUE) break;

            if (GetLastError() != ERROR_PIPE_BUSY) {
#ifdef CCHIPS_EXTERNAL_USE
                LOG(ERROR) << "Could not open pipe. Please check if server is running";
#endif
                return false;
            }

            if (!WaitNamedPipeW(pipe_name_.c_str(), NMPWAIT_USE_DEFAULT_WAIT)) {
#ifdef CCHIPS_EXTERNAL_USE
                LOG(ERROR) << "Wait time out. Could not open pipe";
#endif
                return false;
            }
        }

        dw_mode = PIPE_READMODE_MESSAGE;
        set_mode_success = SetNamedPipeHandleState(pipe_handle_, &dw_mode, NULL, NULL);
        if (!set_mode_success) {
#ifdef CCHIPS_EXTERNAL_USE
            LOG(ERROR) << "SetNamedPipeHandleState() failed with error code " << GetLastError();
#endif
            return false;
        }
#ifdef CCHIPS_EXTERNAL_USE
        LOG(INFO) << "Pipe connected.";
#endif

        return true;
    }
    bool CreateLocalFile(const std::wstring& file_name) {
        pipe_handle_ = CreateFileW(file_name.c_str(), GENERIC_WRITE | GENERIC_READ,
            0, NULL, CREATE_ALWAYS, 0, NULL);

        if (pipe_handle_ != INVALID_HANDLE_VALUE) {
#ifdef CCHIPS_EXTERNAL_USE
            LOG(INFO) << "Local log file created.";
#endif
            return true;
        }

        return false;
    }
    bool IsValidMessage(const Message& message) {
        return !message.empty();
    }
    bool WritePipe(Message& message) {
        BOOL write_success = FALSE;
        if (!brunning_) return false;
        if (IsValidMessage(message)) {
            DWORD bytes_written;
            std::lock_guard<std::mutex> lockGuard(message_list_mutex_);
            if (!is_local) {
                write_success =
                    WriteFile(pipe_handle_,
                        message.c_str(),
                        message.length(),
                        &bytes_written,
                        NULL);
            }
            else {
                message.append("\n");
                write_success =
                    WriteFile(pipe_handle_,
                        message.c_str(),
                        message.length(),
                        &bytes_written,
                        NULL);
            }

            if (!write_success) {
#ifdef CCHIPS_EXTERNAL_USE
                DWORD error = GetLastError();
                if (error == ERROR_NO_DATA || error == ERROR_BROKEN_PIPE)
                    LOG(ERROR) << "WriteFile() failed because pipe has been closed. Please check if server is still running";
                else
                    LOG(ERROR) << "WriteFile() failed with error code " << error;
#endif
            }
        }
        return (bool)write_success;
    }
};



