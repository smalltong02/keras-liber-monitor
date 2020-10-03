#pragma once

#include "PipeServer.h"
#include "../../rapidjson/document.h"
#include "../../rapidjson/writer.h"

using RapidDocument = rapidjson::GenericDocument<rapidjson::UTF8<>>;
typedef std::function<void(const std::unique_ptr<RapidDocument>)> json_callback_function;

class PipeJSONServer : public PipeServer {
public:
    PipeJSONServer(const std::wstring& pipe_name) : PipeServer(pipe_name) {}
    ~PipeJSONServer() = default;
    bool Start(json_callback_function callback) {
        assert(server_thread_ == nullptr);
        if (server_thread_) return false;

        server_thread_ = std::make_unique<std::thread>(std::bind(&PipeJSONServer::WorkThread, this));

        string_callback_ = [this, callback](const std::string& message) {
            std::unique_ptr<RapidDocument> document = this->StringToJSON(message);
            callback(std::move(document));
        };

        if (!string_callback_) return false;
        if (!server_thread_) return false;

        return true;
    }

private:
    std::unique_ptr<RapidDocument> StringToJSON(const std::string& json_string) {
        if (json_string.empty()) return nullptr;

        std::unique_ptr<RapidDocument> document = std::make_unique<RapidDocument>();
        document->Parse(json_string.c_str());
        if (!document->IsObject() || document->IsNull()) return nullptr;
        return std::move(document);
    }
};
