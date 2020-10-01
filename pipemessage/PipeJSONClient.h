#pragma once
#include "PipeClient.h"
#include "../../rapidjson/document.h"
#include "../../rapidjson/writer.h"

using RapidValue = rapidjson::GenericValue<rapidjson::UTF8<>>;
using RapidDocument = rapidjson::GenericDocument<rapidjson::UTF8<>>;

class PipeJSONClient : public PipeClient {
public:
    using pipe_json_cfg = enum _pipe_json_cfg {
        cfg_null = 0x0,
        cfg_serial = 0x1,
        cfg_crc32 = 0x2,
    };
    PipeJSONClient(const std::wstring& pipe_name, pipe_json_cfg cfg = cfg_null) : PipeClient(pipe_name), pipe_cfg_(cfg), message_count_(0) {}
    ~PipeJSONClient() = default;
    bool SendMsg(std::unique_ptr<RapidDocument> json, bool insert_to_front = false) {
        if (!json) return false;
        if (pipe_cfg_ & cfg_serial) {
            RapidDocument::AllocatorType& allocator = json->GetAllocator();
            json->AddMember(RapidValue("SN", allocator), message_count_++, allocator);
        }
        Message message = JSONToMessage(std::move(json));
        if (pipe_cfg_ & cfg_crc32) {
            RapidDocument::AllocatorType& allocator = json->GetAllocator();
            json->AddMember(RapidValue("Crc32", allocator), 0, allocator);
        }
        return __super::SendMsg(message, insert_to_front);
    }

private:
    pipe_json_cfg pipe_cfg_;
    std::atomic_int message_count_;
    Message JSONToMessage(const std::unique_ptr<RapidDocument> json) {
        if (json == nullptr) return "";

        rapidjson::StringBuffer string_buffer;
        string_buffer.Clear();
        rapidjson::Writer<rapidjson::StringBuffer> writer(string_buffer);

        json->Accept(writer);
        if (!string_buffer.GetSize()) return{};
        return string_buffer.GetString();
    }
};
