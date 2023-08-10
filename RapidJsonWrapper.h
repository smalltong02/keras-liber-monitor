#pragma once
#include <bitset>
#include <queue>
#include <any>
#include <optional>
#include <atomic>
#include <chrono>
#include <memory>
#include <unordered_map>
#include "rapidjson\document.h"
#include "rapidjson\writer.h"
#include "rapidjson\prettywriter.h"


namespace cchips {

    using LOGPAIR = std::pair<std::string, std::string>;
    using RapidValue = rapidjson::GenericValue<rapidjson::UTF8<>>;
    using RapidArray = rapidjson::GenericArray<false, RapidValue>;
    using ConstRapidArray = rapidjson::GenericArray<true, RapidValue>;
    using RapidObject = rapidjson::GenericObject<false, RapidValue>;
    using ConstRapidObject = rapidjson::GenericObject<true, RapidValue>;
    using RapidDocument = rapidjson::GenericDocument<rapidjson::UTF8<>>;

#define RapidAllocator g_document.GetAllocator()
#define ADD_JSON_LOG(document, rapidvalue, key, value) \
    RapidDocument::AllocatorType& allocator = (document).GetAllocator(); \
    (rapidvalue).AddMember(RapidValue(key, allocator), RapidValue(value, allocator), allocator);
#define ADD_JSON_DOC(document, key, value) \
    RapidDocument::AllocatorType& allocator = (document).GetAllocator(); \
    (document).AddMember(RapidValue(key, allocator), RapidValue(value, allocator), allocator);
#define ADD_JSON_DOC_N(document, key, value) \
    RapidDocument::AllocatorType& allocator = (document).GetAllocator(); \
    (document).AddMember(key, value, allocator);

    class CRapidJsonWrapper
    {
    public:
        using optstring = std::optional<std::string>;
        using optstring_view = std::optional<std::string_view>;
        using iterator = RapidDocument::MemberIterator;
        using const_iterator = RapidDocument::ConstMemberIterator;
        CRapidJsonWrapper(const std::string_view& json_str) : m_bValid(false) {
            m_document = std::make_unique<RapidDocument>();
            Initialize(json_str);
        }
        ~CRapidJsonWrapper() = default;
        bool IsValid() const { return m_bValid; }
        bool Initialize(const std::string_view& json_str) {
            if (!m_document) return false;
            if (json_str.length() == 0) return false;
            m_document->Parse(json_str.data(), json_str.length());
            if (!m_document->IsObject() || m_document->IsNull()) {
                //config data is incorrect.
                return false;
            }
            m_bValid = true;
            return true;
        }
        RapidDocument::AllocatorType& GetAllocator() {
            return (*m_document).GetAllocator();
        }
        std::uint64_t Capacity() const {
            if (m_document) {
                return m_document->Capacity();
            }
            return 0;
        }
        bool AddTopMember(const std::string& name, std::unique_ptr<RapidValue> value) {
            if (!IsValid()) return false;
            if (auto anyvalue{ GetMember(name) }; !anyvalue.has_value()) {
                RapidDocument::AllocatorType& allocator = (*m_document).GetAllocator();
                (*m_document).AddMember(RapidValue(name.c_str(), allocator), *value, allocator);
                return true;
            }
            return false;
        }
        bool AddTopMember(const std::string& name, const std::string& value) {
            RapidDocument::AllocatorType& allocator = (*m_document).GetAllocator();
            return AddTopMember(name, RapidValue(value.c_str(), allocator));
        }
        bool AddTopMember(const std::string& name, RapidValue value) {
            if (!IsValid()) return false;
            if (auto anyvalue{ GetMember(name) }; !anyvalue.has_value()) {
                RapidDocument::AllocatorType& allocator = (*m_document).GetAllocator();
                (*m_document).AddMember(RapidValue(name.c_str(), allocator), value, allocator);
                return true;
            }
            return false;
        }
        optstring Serialize() const {
            if (!IsValid()) return std::nullopt;
            rapidjson::StringBuffer string_buffer;
            string_buffer.Clear();
            rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(string_buffer);
            if (m_document->Accept(writer))
                return optstring(std::in_place, string_buffer.GetString());
            return std::nullopt;
        }
        static std::any GetMember(const std::string& name, const ConstRapidObject& Object) {
            const auto& it = Object.FindMember(name.c_str());
            if (it == Object.MemberEnd()) return {};
            return GetAnyValue(it->value);
        }
        std::any GetMember(const std::string& name) const {
            if (!IsValid()) return {};
            const auto& it = m_document->FindMember(name.c_str());
            if (it == m_document->MemberEnd()) return {};
            return GetAnyValue(it->value);
        }
        static std::any GetMember(const std::vector<std::string>& keys, const ConstRapidObject& Object) {
            std::any anyobject = Object;
            for (auto& key : keys)
            {
                if (!anyobject.has_value() || anyobject.type() != typeid(ConstRapidObject))
                    return {};
                anyobject = GetMember(key, std::any_cast<ConstRapidObject>(anyobject));
            }
            return anyobject;
        }
        std::any GetMember(const std::vector<std::string>& keys) const {
            if (!IsValid()) return {};
            if (auto anyvalue(GetValue(keys)); anyvalue.has_value()) {
                return GetAnyValue(*std::any_cast<RapidValue*>(anyvalue));
            }
            return {};
        }
        bool SetMember(const std::vector<std::string>& keys, std::any anyvalue, bool replace = false)
        {
            if (!keys.size()) return false;
            if (!IsValid()) return false;
            auto rapid_value(SetAnyValue(anyvalue));
            if (!rapid_value) return false;
            return SetValue(keys, *rapid_value, replace);
        }
        bool SetMember(const std::vector<std::string>& keys, RapidValue& value, bool replace = false)
        {
            if (!keys.size()) return false;
            if (!IsValid()) return false;
            return SetValue(keys, value, replace);
        }
        bool AddSoftwareMember(const std::string& name, std::unique_ptr<RapidValue> value) {
            if (!IsValid()) return false;
            RapidValue* sub_document = &(*m_document)["software"];
            if (!sub_document || !sub_document->IsObject())
                return false;
            RapidDocument::AllocatorType& allocator = (*m_document).GetAllocator();
            (*sub_document).AddMember(RapidValue(name.c_str(), allocator), *value, allocator);
            return true;
        }
        std::unique_ptr<RapidDocument> MoveDocument() {
            m_bValid = false;
            if (m_document) {
                return std::move(m_document);
            }
            return nullptr;
        }
        bool CopyRapidValue(std::unique_ptr<RapidValue> value) {
            if (!IsValid()) return false;
            if (!value) return false;
            m_document->CopyFrom(*value, m_document->GetAllocator());
            return true;
        }
        iterator        begin() { return (*m_document).GetObject().begin(); }
        const_iterator  begin() const { return (*m_document).GetObject().begin(); }
        iterator        end() { return (*m_document).GetObject().end(); }
        const_iterator  end() const { return (*m_document).GetObject().end(); }
    private:
        bool m_bValid;
        std::unique_ptr<RapidDocument> m_document = nullptr;

        static std::any GetAnyValue(const RapidValue& Value)
        {
            if (Value.IsObject())
                return Value.GetObject();
            if (Value.IsString())
                return std::string_view(Value.GetString());
            if (Value.IsInt())
                return Value.GetInt();
            if (Value.IsInt64())
                return Value.GetInt64();
            if (Value.IsUint())
                return Value.GetUint();
            if (Value.IsUint64())
                return Value.GetUint64();
            if (Value.IsArray())
                return Value.GetArray();
            if (Value.IsBool())
                return Value.GetBool();
            if (Value.IsDouble())
                return Value.GetDouble();
            if (Value.IsFloat())
                return Value.GetFloat();
            return {};
        }
        std::unique_ptr<RapidValue> SetAnyValue(const std::any& anyvalue) {
            auto& allocator = m_document->GetAllocator();

            if (anyvalue.type() == typeid(std::string))
                return std::make_unique<RapidValue>(std::any_cast<std::string>(anyvalue).c_str(), allocator);
            if (anyvalue.type() == typeid(int))
                return std::make_unique<RapidValue>(std::any_cast<int>(anyvalue));
            if (anyvalue.type() == typeid(std::uint32_t))
                return std::make_unique<RapidValue>(std::any_cast<std::uint32_t>(anyvalue));
            if (anyvalue.type() == typeid(std::int64_t))
                return std::make_unique<RapidValue>(std::any_cast<std::int64_t>(anyvalue));
            if (anyvalue.type() == typeid(std::uint64_t))
                return std::make_unique<RapidValue>(std::any_cast<std::uint64_t>(anyvalue));
            if (anyvalue.type() == typeid(bool))
                return std::make_unique<RapidValue>(std::any_cast<bool>(anyvalue));
            if (anyvalue.type() == typeid(double))
                return std::make_unique<RapidValue>(std::any_cast<double>(anyvalue));
            if (anyvalue.type() == typeid(float))
                return std::make_unique<RapidValue>(std::any_cast<float>(anyvalue));
            return nullptr;
        }
        std::any GetValue(const std::vector<std::string>& keys) const {
            if (!m_document->IsObject())
                return {};
            RapidValue* Value = m_document.get();
            for (const auto& key : keys)
            {
                if (!Value->IsObject())
                    return {};
                if (!Value->HasMember(key.c_str())) {
                    return {};
                }
                else
                    Value = &(*Value)[key.c_str()];
            }
            return Value;
        }

        bool SetValue(const std::vector<std::string>& keys, RapidValue& value, bool replace = false) {
            if (!keys.size()) return false;
            RapidDocument::AllocatorType& allocator = m_document->GetAllocator();
            RapidValue* sub_document = m_document.get();

            for (const auto& key : keys)
            {
                if (!sub_document->IsObject())
                    return false;
                if (sub_document->HasMember(key.c_str())) {
                    sub_document = &(*sub_document)[key.c_str()];
                }
                else {
                    RapidValue temp_field;
                    RapidValue str_key(key.c_str(), allocator);
                    temp_field.SetObject();
                    sub_document = &sub_document->AddMember(str_key, temp_field, allocator);
                }
            }

            if (!sub_document->IsObject()) {
                return false;
            }
            else if (sub_document->HasMember(keys.rbegin()->c_str())) {
                if (replace) {
                    (*sub_document)[keys.rbegin()->c_str()] = value;
                }
                else if ((*sub_document)[keys.rbegin()->c_str()].IsArray()) {
                    (*sub_document)[keys.rbegin()->c_str()].PushBack(value, allocator);
                }
                else {
                    RapidValue temp_val;
                    temp_val = (*sub_document)[keys.rbegin()->c_str()];
                    (*sub_document)[keys.rbegin()->c_str()].SetArray();
                    (*sub_document)[keys.rbegin()->c_str()].PushBack(temp_val, allocator);
                    (*sub_document)[keys.rbegin()->c_str()].PushBack(value, allocator);
                }
            }
            else {
                RapidValue str_key(keys.rbegin()->c_str(), allocator);
                sub_document->AddMember(str_key, value, allocator);
            }
            return true;
        }
    };
}
