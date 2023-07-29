#pragma once
#include <io.h>
#include <stdio.h>
#include <windows.h>
#include <bitset>
#include <queue>
#include <any>
#include <optional>
#include <atomic>
#include <chrono>
#include <unordered_map>
#include "RapidJsonWrapper.h"
#include "crc.h"

#ifdef CCHIPS_EXTERNAL_USE
extern "C" {
#include "bson.h"
}
extern void r3_debug_log(const std::string& log_string);
#else
extern "C" {
#include "bson\bson.h"
}
#endif

namespace cchips {
#define warning()

    class CChecker;
    using RAPID_DOC_PAIR = std::pair<std::unique_ptr<RapidDocument>, std::unique_ptr<CChecker>>;
    using RAPID_VAL_PAIR = std::pair<std::unique_ptr<RapidValue>, std::unique_ptr<CChecker>>;

    class CChecker
    {
    public:
#define MAX_SUPPORT_BITS 32
        enum _checker_type {
            checker_invalid = -1,
            checker_crc16 = 0,      // 16bit
            checker_crc32,          // 32bit
            checker_md5,            // 128bit
            checker_sha1,           // 160bit
            checker_sha256,         // 256bit
        };

        CChecker(_checker_type type = checker_crc32) : m_checker_type(type) {
            Initialize(type);
        }
        CChecker(const CChecker& checker) : m_checker_type(checker.GetType()) {
            Initialize(m_checker_type);
            m_checker_data = checker.GetData();
            m_checker_ss << std::hex << m_checker_data.to_ulong();
        }
        ~CChecker() = default;

        void Reset(_checker_type type) {
            m_checker_ss.str("");
            m_checker_ss.clear();
            m_checker_data.reset();
            m_crc16_calculator = nullptr;
            m_crc32_calculator = nullptr;
            Initialize(type);
        }
        bool Update(const std::string& data);
        bool Update(const CChecker& checker);
        _checker_type GetType() const { return m_checker_type; }
        const std::bitset<MAX_SUPPORT_BITS>& GetData() const { return m_checker_data; }
        const std::stringstream& Serialize() const { return m_checker_ss; }
        bool Same(const std::unique_ptr<CChecker>& checker) const {
            if (!checker) return false;
            if (m_checker_type != checker->GetType())
                return false;
            if (m_checker_data != checker->GetData())
                return false;
            return true;
        }
        ULONG GetKey() const;

    private:
        void Initialize(_checker_type type);

        _checker_type m_checker_type;
        std::stringstream m_checker_ss;
        std::bitset<MAX_SUPPORT_BITS> m_checker_data;
        std::unique_ptr<CRC16> m_crc16_calculator = nullptr;
        std::unique_ptr<CRC32> m_crc32_calculator = nullptr;
    };

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
        CBsonWrapper() : m_bverifier(false), m_verifier_type(verifier_crc32), m_bduplicate_flt(false), m_log_serialnum(0) { ; }
        ~CBsonWrapper() = default;

        bool is_duplicated_log(std::unique_ptr<CChecker>& checker) {
#define MAX_QUEUE_SIZE 100
            if (!IsDuplicateFlt()) return false;
            if (!checker || checker->GetKey() == 0) return false;
            if (m_duplicate_map.find(checker->GetKey()) != m_duplicate_map.end()) {
                if (m_duplicate_map[checker->GetKey()])
                {
                    if (m_duplicate_map[checker->GetKey()]->Same(checker))
                        return true;
                }
            }
            else {
                m_duplicate_queue.push(checker->GetKey());
                m_duplicate_map[checker->GetKey()] = std::move(checker);
                if (m_duplicate_queue.size() > MAX_QUEUE_SIZE) {
                    unsigned long front = m_duplicate_queue.front();
                    m_duplicate_map.erase(front);
                    m_duplicate_queue.pop();
                }
            }
            return false;
        }

        std::unique_ptr<bson> Pack(RAPID_DOC_PAIR data, bool braw = false) {
            if (!data.first) return nullptr;
            if (!data.second) return nullptr;

            // keep a size 50 cache to avoid duplicated logs
            if (is_duplicated_log(data.second)) return nullptr;

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
            if (!braw) func_get_timestamp(time_string);
            if (!braw && time_string.length())
            {
                ADD_JSON_DOC(*data.first, "Time", time_string.c_str());
            }
            int serialnum = m_log_serialnum++;
            ADD_JSON_DOC_N(*data.first, "SerialN", serialnum);
            rapidjson::StringBuffer string_buffer;
            string_buffer.Clear();
            rapidjson::Writer<rapidjson::StringBuffer> writer(string_buffer);
            data.first->Accept(writer);
            if (!string_buffer.GetSize()) return nullptr;
            //if (IsVerifier())
            //{
            //    bson_append_int(&(*bson_ptr), "VerifierType", (int)m_verifier_type);
            //    bson_append_int(&(*bson_ptr), "Verifier", GetVerifier(string_buffer.GetString(), m_verifier_type));
            //}
            if (bson_append_string(&(*bson_ptr), "R3Log", string_buffer.GetString()) == BSON_OK) {
                //if(bson_append_finish_array(&(*bson_ptr)) == BSON_OK)
                    if(bson_finish(&(*bson_ptr)) == BSON_OK)
                        return bson_ptr;
            }
            return nullptr;
        }
        std::unique_ptr<CRapidJsonWrapper> Unpack(const std::string& decode_buffer) {
            assert(decode_buffer.length());
            if (!decode_buffer.length()) return nullptr;
            std::unique_ptr<CRapidJsonWrapper> json_wrapper = nullptr;
            bson bson_unpack;
            if (bson_init_finished_data(&bson_unpack, (char*)&decode_buffer[0], false) == BSON_OK)
            {
                bson_iterator i;
                bson_iterator_from_buffer(&i, bson_data(&bson_unpack));
                std::stringstream ss;
                _verifier_type verifier_type = verifier_invalid;
                ULONG verifier = 0;
                while (bson_iterator_next(&i)) {
                    bson_type t = bson_iterator_type(&i);
                    if (t == 0)
                        break;
                    const char* key = bson_iterator_key(&i);
                    if (key[0] == '\0') continue;
                    switch (t) {
                    case BSON_INT:
                    {
                        if (_stricmp(key, "VerifierType") == 0)
                        {
                            verifier_type = (_verifier_type)bson_iterator_int(&i);
                        }
                        else if (_stricmp(key, "Verifier") == 0)
                        {
                            verifier = (_verifier_type)bson_iterator_int(&i);
                        }
                        // verifier here
                        break;
                    }
                    case BSON_STRING:
                    {
                        if (_stricmp(key, "R3Log") == 0)
                        {
                            ss << bson_iterator_string(&i);
                            json_wrapper = std::make_unique<CRapidJsonWrapper>(ss.str());
                        }
                        break;
                    }
                    default:
                        warning("can't get bson type : %d\n", t);
                        break;
                    }
                }
                if (!json_wrapper) 
                    return nullptr;
                if (verifier_type != verifier_invalid && verifier != 0)
                {
                    if (GetVerifier(ss.str().c_str(), verifier_type) == verifier)
                        json_wrapper->AddTopMember("verifier_result", RapidValue("true"));
                    else
                        json_wrapper->AddTopMember("verifier_result", RapidValue("false"));
                }
                if (json_wrapper->IsValid())
                {
                    return std::move(json_wrapper);
                }
            }
            return nullptr;
        }
        bool IsVerifier() const { return m_bverifier; }
        void EnableVerifier() { m_bverifier = true; }
        void DisableVerifier() { m_bverifier = false; }
        bool IsDuplicateFlt() const { return m_bduplicate_flt; }
        void EnableDuplicateFlt() { m_bduplicate_flt = true; }
        void DisableDuplicateFlt() { m_bduplicate_flt = false; }
    private:
        ULONG GetVerifier(const char* data, _verifier_type type);

        bool m_bverifier;
        bool m_bduplicate_flt;
        _verifier_type m_verifier_type;
        std::atomic_ulong m_log_serialnum;
        std::queue<unsigned long> m_duplicate_queue;
        std::unordered_map<unsigned long, std::unique_ptr<CChecker>> m_duplicate_map;
    };

} // namespace cchips
