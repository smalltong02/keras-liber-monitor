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
extern "C" {
#include "bson\bson.h"
}

namespace cchips {

    using LOGPAIR = std::pair<std::string, std::string>;

#define warning()
    class CBsonWrapper
    {
    public:
        enum _verifier_type {
            verifier_invalid = -1,
            verifier_crc16 = 0,
            verifier_crc32,
            verifier_md5,
            verifier_sha1,
        };
        CBsonWrapper() : m_bverifier(false), m_verifier_type(verifier_crc32) { ; }
        ~CBsonWrapper() = default;

        std::unique_ptr<bson> Pack(const std::unique_ptr<LOGPAIR>& data) {
            if (!data) return nullptr;
            std::string time_string;
            auto func_get_timestamp = [](std::string& time_buffer) {
#define time_buffer_size 100
                time_buffer.resize(time_buffer_size);
                auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                /*std::*/strftime(&time_buffer[0],
                    time_buffer_size,
                    "%Y-%m-%d %H:%M:%S",
                    /*std::*/localtime(&now));
                return;
            };
            std::unique_ptr<bson> bson_ptr = std::make_unique<bson>();
            assert(bson_ptr != nullptr);
            if (!bson_ptr) return nullptr;
            bson_init(&(*bson_ptr));
            func_get_timestamp(time_string);
            if (IsVerifier())
            {
                bson_append_int(&(*bson_ptr), "VerifierType", (int)m_verifier_type);
                bson_append_int(&(*bson_ptr), "Verifier", GetVerifier(data, m_verifier_type));
            }
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
                _verifier_type verifier_type = verifier_invalid;
                ULONG verifier = 0;
                std::string action;
                std::string r3log;
                while (bson_iterator_next(&i)) {
                    bson_type t = bson_iterator_type(&i);
                    if (t == 0)
                        break;
                    const char* key = bson_iterator_key(&i);
                    if (key[0] == '\0') continue;
                    switch (t) {
                    case BSON_INT:
                    {
                        if (stricmp(key, "VerifierType") == 0)
                        {
                            verifier_type = (_verifier_type)bson_iterator_int(&i);
                        }
                        else if (stricmp(key, "Verifier") == 0)
                        {
                            verifier = (_verifier_type)bson_iterator_int(&i);
                        }
                        if (!(*ss_ptr).str().length())
                            (*ss_ptr) << "{ ";
                        else
                            (*ss_ptr) << "; ";
                        (*ss_ptr) << "\"" << key << "\"" << ": " << "\"" << bson_iterator_int(&i) << "\"";
                    }
                    break;
                    case BSON_STRING:
                    {
                        if (stricmp(key, "Action") == 0)
                        {
                            action = bson_iterator_string(&i);
                        }
                        else if (stricmp(key, "R3Log") == 0)
                        {
                            r3log = bson_iterator_string(&i);
                        }
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
                if (verifier_type != verifier_invalid && verifier != 0 && action.length() != 0 && r3log.length() != 0)
                {
                    if (GetVerifier(std::make_unique<LOGPAIR>(action, r3log), verifier_type) == verifier)
                        (*ss_ptr) << ",\"" << "verifier_result" << "\"" << ": " << "\"" << "success" << "\"";
                    else
                        (*ss_ptr) << ",\"" << "verifier_result" << "\"" << ": " << "\"" << "failed" << "\"";
                }
                if ((*ss_ptr).str().length())
                {
                    (*ss_ptr) << " }";
                    return std::move(ss_ptr);
                }
            }
            return nullptr;
        }
        bool IsVerifier() { return m_bverifier; }
        void EnableVerifier() { m_bverifier = true; }
        void DisableVerifier() { m_bverifier = false; }
    private:
        ULONG GetVerifier(const std::unique_ptr<LOGPAIR>& data, _verifier_type type);

        bool m_bverifier;
        _verifier_type m_verifier_type;
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
            if (m_sync_event)
                CloseHandle(m_sync_event);
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
                {
                    warning("The log pipe handler has failed, last error %d.", error);
                    break;
                }
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
            assert(m_sync_event);
            if (!m_sync_event) return;
            HANDLE hpipe = INVALID_HANDLE_VALUE;
            auto func_open = [&]() {
                int count = 0;
                while (count < CSocketObject::lpc_connect_try_times)
                {
                    hpipe = CreateFile(lpc_pipe_name, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
                    if (ValidHandle(hpipe))
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
                if (ValidHandle(hpipe))
                    return true;
                return false;
            };
            auto func_write = [&](const char* buffer, int len) {
                while (len)
                {
                    DWORD byte_writes = 0;
                    bool bsuccess = WriteFile(hpipe, buffer, len, &byte_writes, nullptr);
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

            if (!func_open()) return;
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
            if (ValidHandle(hpipe)) CloseHandle(hpipe);
            return;
        }
        bool ValidHandle(HANDLE hpipe) const { if (hpipe == INVALID_HANDLE_VALUE || hpipe == nullptr) return false; else return true; }
        HANDLE m_sync_event = nullptr;
        std::atomic_bool m_brunning;
        std::unique_ptr<std::thread> m_listen_thread;
        std::unique_ptr<std::thread> m_connect_thread;
        //for test
        std::atomic_int m_logs_total_count = 0;
    };

    //
    // Author: Colin Mahoney (cmahoney@readysoft.es)
    //
    // C++ implementation of CRC error checking
    //
    // Based on Ross Williams' Rocksoft^tm Model CRC Algorithm,
    // available as part of the document "A Painless Guide to
    // CRC Error Detection Algorithms", also by Ross Williams.
    //
    //    Ref 1:  C-Users Journal, June 1999
    //    Ref 2:  www.ross.net/crc/.
    //

    static const int CRCMaxBits = 32;

    template<int Width, unsigned long Poly, unsigned long Init,
        unsigned long XOrOut, bool Ref, bool Direct = true>

        class CRCGenerator
    {
    public:
        class CRCTable
        {
            unsigned long Table[256];
            // Calculate the table entry at 'index'
            //
            unsigned long CalcTableEntry(int index)
            {
                unsigned long inbyte = (unsigned long)index;
                if (Ref)
                {
                    inbyte = Reflect(inbyte, 8);
                }
                unsigned long topbit = Bitmask(Width - 1);
                unsigned long reg = inbyte << (Width - 8);
                for (int i = 0; i < 8; i++)
                {
                    if (reg & topbit)
                    {
                        reg = (reg << 1) ^ Poly;
                    }
                    else
                    {
                        reg <<= 1;
                    }
                }
                if (Ref)
                {
                    reg = Reflect(reg, Width);
                }
                return reg & WidthMask(Width);
            }

        public:

            CRCTable()
            {
                for (int i = 0; i < 256; i++)
                {
                    Table[i] = CalcTableEntry(i);
                }
            }
            unsigned long operator[](int i) const { return Table[i]; }

        };

    private:

        static const CRCTable Table;

        // Register holds the current value of the CRC calculation
        unsigned long Register;

        // Return an unsigned long with i'th bit set to one
        static unsigned long Bitmask(int i) { return 1UL << i; }

        // Reflect the bottom b bits of val

        static unsigned long Reflect(unsigned long val, int b)
        {
            unsigned long t = val;
            for (int i = 0; i < b; i++)
            {
                if (t & 1L)
                {
                    val |= Bitmask((b - 1) - i);
                }
                else
                {
                    val &= ~Bitmask((b - 1) - i);
                }
                t >>= 1;
            }
            return val;
        }

        // Return an unsigned long with value ( 2^width )-1

        static unsigned long WidthMask(int width)
        {
            return (((1UL << (width - 1)) - 1UL) << 1) | 1UL;
        }

    public:

        CRCGenerator() :
            Register(Init) {}
        unsigned long GetCRC() const
        {
            return (XOrOut ^ Register) & WidthMask(Width);
        }
        unsigned long GetNormalCRC() const
        {
            unsigned long normCRC;
            if (!Ref)
            {
                normCRC = GetCRC();
                normCRC <<= (CRCMaxBits - Width);
            }
            else
            {
                normCRC = 0;
                unsigned long crc = GetCRC();
                for (int i = 8; i <= CRCMaxBits; i += 8)
                {
                    normCRC |= (crc & 0xFF) << (CRCMaxBits - i);
                    crc >>= 8;
                }
            }
            return normCRC;
        }

        bool GetDirect() const { return Direct; }

        bool GetReflect() const { return Ref; }

        int  GetWidth() const { return Width; }

        void LoadRegister(unsigned long val)
        {
            if (Ref)
            {
                unsigned long v = 0;
                int i;
                for (i = 0; i < Width - 8; i += 8)
                {
                    v <<= 8;
                    v |= (val & 0xFF);
                    val >>= 8;
                }
                int extraBits = Width & 0x07;
                v <<= extraBits;
                v |= (val & 0xFF) >> (8 - extraBits);
                val = v;
            }
            Register ^= val;
        }

        void Process(unsigned char ch)
        {
            if (!Ref)
            {
                if (Direct)
                {
                    Register = Table[((Register >> (Width - 8)) ^ ch) & 0xFFL]
                        ^ (Register << 8);
                }
                else
                {
                    Register = Table[(Register >> (Width - 8)) & 0xFFL] ^
                        (Register << 8);
                    Register ^= ch;
                }
            }
            else
            {
                if (Direct)
                {
                    Register = Table[(Register ^ ch) & 0xFFL] ^
                        (Register >> 8);
                }
                else
                {
                    Register = Table[Register & 0xFFL] ^ (Register >> 8);
                    Register ^= ch << (Width - 8);
                }
            }
        }

        void Process(unsigned char* block, int block_length)
        {
            for (int i = 0; i < block_length; i++)
            {
                Process(*block++);
            }
        }

        // Un-comment the following version if no member templates
        ///  void Process( unsigned char* first, unsigned char* last )
        ///   {
        ///   while( first != last )
        ///    {
        ///    Process( *first++ );
        ///    }
        ///   }
        // Comment out the following version if no member templates

        template<class InIter>

        void Process(InIter first, InIter last)
        {
            while (first != last)
            {
                Process(*first);
                ++first;
            }
        }

        // Process 'count' bits from 'bits'. if 'Ref' is false reads
        // starting from MSB of 'bits'. If 'Ref' is true starts from
        // LSB

        void ProcessBits(unsigned char bits, int count)
        {
            if (!Ref)
            {
                if (Direct)
                {
                    Register = Table[((Register >> (Width - count)) ^
                        (bits >> (8 - count))) & (0xFF >> (8 - count))] ^
                        (Register << count);
                }
                else
                {
                    Register = Table[(Register >> (Width - count)) &
                        (0xFF >> (8 - count))] ^ (Register << count);
                    Register ^= bits >> (8 - count);
                }
            }
            else
            {
                if (Direct)
                {
                    Register = Table[((Register ^ bits) & (0xFF >> (8 - count)))
                        << (8 - count)] ^ (Register >> count);
                }
                else
                {
                    Register = Table[(Register & (0xFF >> (8 - count))) <<
                        (8 - count)] ^ (Register >> count);
                    Register ^= bits << (Width - count);
                }
            }
        }

        void Reset()
        {
            Register = Init;
        }

        void Write(std::ostream& os) const
        {
            unsigned long ncrc = GetNormalCRC();
            for (int i = 0; i < Width; i += 8)
            {
                unsigned char byte =
                    (unsigned char)(ncrc >> (CRCMaxBits - i - 8));
                os << byte;
            }
        }

    };
    template<int Width, unsigned long Poly, unsigned long Init,
        unsigned long XOrOut, bool Ref, bool Direct>
        const typename CRCGenerator<Width, Poly, Init, XOrOut, Ref,
        Direct>::CRCTable
        CRCGenerator<Width, Poly, Init, XOrOut, Ref, Direct>::Table;

    template<int Width, unsigned long Poly, unsigned long Init,
        unsigned long XOrOut, bool Ref, bool Direct>
        std::ostream& operator<<(std::ostream& os,
            CRCGenerator<Width, Poly, Init, XOrOut, Ref, Direct>& crc)
    {
        crc.Write(os);
        return os;
    }

    typedef CRCGenerator<16, 0x8005, 0, 0, true> CRC16;
    typedef CRCGenerator<32, 0x80000005, -1, 0, true> CRC32;

} // namespace cchips