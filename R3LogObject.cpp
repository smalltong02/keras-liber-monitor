#include "R3LogObject.h"

namespace cchips {

    const size_t CSocketObject::lpc_buffer_kb = 4;   // 4kb
    const DWORD CSocketObject::lpc_server_wait_timeout = 2000; // 2 seconds
    const DWORD CSocketObject::lpc_client_wait_timeout = 1000; // 1 second
    const DWORD CSocketObject::lpc_connect_wait_timeout = 100;  // 0.1 second
    const int CSocketObject::lpc_connect_try_times = 10;

    ULONG CBsonWrapper::GetVerifier(const std::unique_ptr<LOGPAIR>& data, _verifier_type type)
    {
        switch (type)
        {
        case verifier_crc16:
        {
            std::stringstream crc_data;
            crc_data << data->first.c_str() << data->second.c_str();
            CRC16 crc16;
            crc16.Process((unsigned char*)crc_data.str().c_str(), crc_data.str().length());
            return crc16.GetNormalCRC();
        }
        break;
        case verifier_crc32:
        {
            std::stringstream crc_data;
            crc_data << data->first.c_str() << data->second.c_str();
            CRC32 crc32;
            crc32.Process((unsigned char*)crc_data.str().c_str(), crc_data.str().length());
            return crc32.GetNormalCRC();
        }
        break;
        case verifier_md5:
        {

        }
        break;
        case verifier_sha1:
        {
            // not support now.
        }
        break;
        default:
            ;
        }
        return 0;
    }
} // namespace cchips