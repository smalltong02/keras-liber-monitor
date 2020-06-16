#include "crc.h"
#include "R3LogObject.h"

namespace cchips {

    const size_t CSocketObject::lpc_buffer_kb = 4;   // 4kb
    const DWORD CSocketObject::lpc_server_wait_timeout = 2000; // 2 seconds
    const DWORD CSocketObject::lpc_client_wait_timeout = 1000; // 1 second
    const DWORD CSocketObject::lpc_connect_wait_timeout = 100;  // 0.1 second
    const int CSocketObject::lpc_connect_try_times = 50;

    ULONG CBsonWrapper::GetVerifier(const char* data, _verifier_type type)
    {
        if (!data) return 0;
        switch (type)
        {
        case verifier_crc16:
        {
            std::stringstream crc_data;
            crc_data << data;
            CRC16 crc16;
            crc16.Process((unsigned char*)crc_data.str().c_str(), crc_data.str().length());
            return crc16.GetNormalCRC();
        }
        break;
        case verifier_crc32:
        {
            std::stringstream crc_data;
            crc_data << data;
            CRC32 crc32;
            crc32.Process((unsigned char*)crc_data.str().c_str(), crc_data.str().length());
            return crc32.GetNormalCRC();
        }
        break;
        case verifier_md5:
        {
            // not support now.
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

    void CChecker::Initialize(_checker_type type)
    {
        switch (m_checker_type)
        {
        case checker_crc16:
        {
            m_crc16_calculator = std::make_unique<CRC16>();
        }
        break;
        case checker_crc32:
        {
            m_crc32_calculator = std::make_unique<CRC32>();
        }
        break;
        case checker_md5:
        {
            // not support now.
        }
        break;
        case checker_sha1:
        {
            // not support now.
        }
        break;
        case checker_sha256:
        {
            // not support now.
        }
        break;
        default:
            ;
        }
    }

    bool CChecker::Update(const std::string& data) {
        bool bret = false;
        if (!data.length()) return false;
        switch (m_checker_type)
        {
        case checker_crc16:
        {
            if (!m_crc16_calculator) return false;
            m_crc16_calculator->Process((unsigned char*)data.c_str(), data.length());
            m_checker_data = m_checker_data.to_ulong() + m_crc16_calculator->GetNormalCRC();
            m_checker_ss.str(""); m_checker_ss.clear(); m_checker_ss << std::hex << m_checker_data.to_ulong();
            bret = true;
        }
        break;
        case checker_crc32:
        {
            if (!m_crc32_calculator) return false;
            m_crc32_calculator->Process((unsigned char*)data.c_str(), data.length());
            m_checker_data = m_checker_data.to_ulong() + m_crc32_calculator->GetNormalCRC();
            m_checker_ss.str(""); m_checker_ss.clear(); m_checker_ss << std::hex << m_checker_data.to_ulong();
            bret = true;
        }
        break;
        case checker_md5:
        {
            // not support now.
        }
        break;
        case checker_sha1:
        {
            // not support now.
        }
        break;
        case checker_sha256:
        {
            // not support now.
        }
        break;
        default:
            ;
        }
        return bret;
    }

    bool CChecker::Update(const CChecker& checker)
    {
        if (checker.GetType() != m_checker_type) return false;
        if (m_checker_type != checker_crc16 && m_checker_type != checker_crc32) return false;
        m_checker_data = m_checker_data.to_ulong() + checker.GetData().to_ulong();
        m_checker_ss.str(""); m_checker_ss.clear(); m_checker_ss << std::hex << m_checker_data.to_ulong();
        return true;
    }

    ULONG CChecker::GetKey() const
    {
        switch (m_checker_type)
        {
        case checker_crc16:
        case checker_crc32:
        {
            return m_checker_data.to_ulong();
        }
        break;
        case checker_md5:
        {
            // not support now.
        }
        break;
        case checker_sha1:
        {
            // not support now.
        }
        break;
        case checker_sha256:
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