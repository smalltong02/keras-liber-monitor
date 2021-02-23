#pragma once
#include <stdio.h>
#include <windows.h>
#include <string>
#include <vector>
#include <regex>
#include <unordered_set>
#include <thread>
#include <chrono>
#include "gtest\gtest.h"
#include "resource.h"
#include "HipsCfgObject.h"
#include "HookImplementObject.h"
#include "utils.h"
#include "commutils.h"
#include "R3LogObject.h"
#ifdef USING_PIPE_MESSAGE
#include "PipeJSONServer.h"
#endif

using namespace std::chrono_literals;

class CServerObject
{
public:
    CServerObject() {
        m_tempfp = tmpfile();
#ifdef USING_PIPE_MESSAGE
        std::function<void(const std::unique_ptr<RapidDocument>)> callback(std::bind(&CServerObject::LogCallBack, this, std::placeholders::_1));
        m_pipe_mess_object = std::make_shared<PipeJSONServer>(L"\\\\.\\pipe\\hips_hook");
        if (m_pipe_mess_object)
        {
            m_pipe_mess_object->Start(callback);
        }
#else
        std::function<void(const std::unique_ptr<CRapidJsonWrapper>)> callback(std::bind(&CServerObject::LogCallBack, this, std::placeholders::_1));
        m_pipe_object = std::make_unique<CLpcPipeObject>();
        if (m_pipe_object)
        {
            m_pipe_object->Listen(std::move(callback));
        }
#endif
    }
    ~CServerObject() {
#ifdef USING_PIPE_MESSAGE
        if (m_pipe_mess_object)
            m_pipe_mess_object->Stop();
#else
        if (m_pipe_object)
            m_pipe_object->StopListen();
#endif
        if (m_tempfp)
            fclose(m_tempfp);
    }
    void ClearLogsCount() { 
#ifdef USING_PIPE_MESSAGE
        return;
#else
        return m_pipe_object->ClearLogsCount(); 
#endif
    }
    int GetTotalLogs() const { 
#ifdef USING_PIPE_MESSAGE
        return 0;
#else
        return m_pipe_object->GetTotalLogs(); 
#endif
    }
    std::unordered_map<std::string, int> GetLogCountMap() { 
        std::lock_guard lock(m_log_count_mutex);
        return m_log_count_map; 
    }
    void ClearLogMap() {
        std::lock_guard lock(m_log_count_mutex);
        m_log_count_map.clear(); 
    }
    void EnableDebugPid() { m_enable_debugpid = true; }
    void DisableDebugPid() { m_enable_debugpid = false; }
    void AddLogCountMap(const std::vector<std::string>& action_list) {
        ClearLogMap();
        std::lock_guard lock(m_log_count_mutex);
        for (auto& elem : action_list) {
            if (elem.length() && m_log_count_map.find(elem) == m_log_count_map.end()) {
                m_log_count_map[elem] = 0;
            }
        }
        return;
    }
    BOOL WaitLogCountMap(const std::vector<int>& count_list, DWORD dwMaxseconds)
    {
        if (!count_list.size())
            return FALSE;
        {
            std::lock_guard lock(m_log_count_mutex);
            if (count_list.size() != m_log_count_map.size())
                return FALSE;
        }
        BOOL bmatch = TRUE;
        std::chrono::milliseconds total_times = {};
        while (true) {
            bmatch = TRUE;
            {
                int count = 0;
                std::lock_guard lock(m_log_count_mutex);
                for (auto& elem : m_log_count_map) {
                    if (count_list[count] > elem.second)
                    {
                        bmatch = FALSE;
                        break;
                    }
                    count++;
                }
            }
            if (bmatch) break;
            std::this_thread::sleep_for(100ms);
            total_times += 100ms;
            if (total_times.count() >= dwMaxseconds*1000)
                break;
        }
        return bmatch;
    }
    
    bool AddDebugPid(DWORD_PTR pid)
    {
        std::lock_guard lock(m_debugpid_mutex);
        auto& it = m_debugpid_list.find(pid);
        if (it != m_debugpid_list.end()) return false;
        m_debugpid_list[pid] = 0;
        return true;
    }
    void DelDebugPid(DWORD_PTR pid)
    {
        std::lock_guard lock(m_debugpid_mutex);
        auto& it = m_debugpid_list.find(pid);
        if (it == m_debugpid_list.end()) return;
        m_debugpid_list.erase(pid);
        return;
    }
    int GetCountForDebugPid(DWORD_PTR pid)
    {
        std::lock_guard lock(m_debugpid_mutex);
        auto& it = m_debugpid_list.find(pid);
        if (it == m_debugpid_list.end()) return 0;
        return it->second;
    }
private:
    bool CheckVerifier(const CRapidJsonWrapper& rapid_log) const
    {
        if (auto anyvalue(rapid_log.GetMember("verifier_result"));
            anyvalue.has_value() && anyvalue.type() == typeid(std::string_view))
        {
            std::string verifier_result(std::any_cast<std::string_view>(anyvalue));
            if (!verifier_result.compare("false")) {
                return false;
            }
        }
        return true;
    }

    void CheckLogCountMap(const CRapidJsonWrapper& rapid_log) {
        std::lock_guard lock(m_log_count_mutex);
        if (!m_log_count_map.size()) return;
        if (auto anyvalue(rapid_log.GetMember("Action"));
            anyvalue.has_value() && anyvalue.type() == typeid(std::string_view))
        {
            for (auto& elem : m_log_count_map) {
                std::string action_str(std::any_cast<std::string_view>(anyvalue));
                if (action_str.compare(elem.first) == 0) {
                    elem.second++;
                }
            }
        }
        return;
    }

    void CheckDebugPid(const CRapidJsonWrapper& rapid_log)
    {
        if (m_enable_debugpid)
        {
            if (auto anyvalue(rapid_log.GetMember("Pid"));
                anyvalue.has_value() && anyvalue.type() == typeid(std::string_view))
            {
                std::string pid_str(std::any_cast<std::string_view>(anyvalue));
                if(pid_str.length())
                {
                    char* end;
                    DWORD_PTR pid = (DWORD_PTR)strtol(pid_str.c_str(), &end, 10);
                    std::lock_guard lock(m_debugpid_mutex);
                    auto& it = m_debugpid_list.find(pid);
                    if (it != m_debugpid_list.end())
                    {
                        it->second++;
                    }
                }
            }
        }
        return;
    }
#ifdef USING_PIPE_MESSAGE
    std::string JSONToString(std::unique_ptr<RapidDocument> json) {
        if (json == nullptr) return "";

        rapidjson::StringBuffer string_buffer;
        string_buffer.Clear();
        rapidjson::Writer<rapidjson::StringBuffer> writer(string_buffer);

        json->Accept(writer);
        if (!string_buffer.GetSize()) return "";
        return string_buffer.GetString();
    }

    void LogCallBack(std::unique_ptr<RapidDocument> json_log) {
        if (!json_log)
            return;
        std::string rapid_string = JSONToString(std::move(json_log));
        ASSERT_TRUE(!rapid_string.empty());
        std::unique_ptr<CRapidJsonWrapper> rapid_log = std::make_unique<CRapidJsonWrapper>(rapid_string);
        ASSERT_TRUE(rapid_log);
        if (!rapid_log) return;
        if (!rapid_log->IsValid()) return;
        ASSERT_TRUE(CheckVerifier(*rapid_log));
        CheckLogCountMap(*rapid_log);
        CheckDebugPid(*rapid_log);
        m_logs_total_count++;
        {
            auto optstring = rapid_log->Serialize();
            if (!optstring) return;
            std::lock_guard lock(m_log_mutex);
            if (m_tempfp)
            {

                fputs(optstring.value().c_str(), m_tempfp);
            }
            else
                std::cout << optstring.value() << std::endl;
        }
        return;
    }
#else
    void LogCallBack(const std::unique_ptr<CRapidJsonWrapper> rapid_log) {
        //ASSERT_TRUE(rapid_log);
        if (!rapid_log) 
            return;
        if (!rapid_log->IsValid()) return;
        ASSERT_TRUE(CheckVerifier(*rapid_log));
        CheckLogCountMap(*rapid_log);
        CheckDebugPid(*rapid_log);
        m_logs_total_count++;
        {
            auto optstring = rapid_log->Serialize();
            if (!optstring) return;
            std::lock_guard lock(m_log_mutex);
            if (m_tempfp)
            {
                fputs(optstring.value().c_str(), m_tempfp);
            }
            else
                std::cout << optstring.value() << std::endl;
        }
        return;
    }
#endif
    std::atomic_int m_logs_total_count = 0;
    std::mutex m_log_mutex;
    FILE* m_tempfp = nullptr;
#ifdef USING_PIPE_MESSAGE
    std::shared_ptr<PipeJSONServer> m_pipe_mess_object;
#else
    std::unique_ptr<CLpcPipeObject> m_pipe_object;
#endif
    std::recursive_mutex m_log_count_mutex;
    std::unordered_map<std::string, int> m_log_count_map;
    std::atomic_bool m_enable_debugpid = false;
    std::mutex m_debugpid_mutex;
    std::map<DWORD_PTR, int> m_debugpid_list;
};

class HipsHookTest
{
public:
    HipsHookTest() : m_valid(false), m_hipsCfgObject(nullptr), m_hookImplObject(nullptr) {}
    ~HipsHookTest() {
        m_hookImplObject->DisableAllApis();
    }

    bool Initialize() {
        m_hookImplObject = g_impl_object;
        m_hipsCfgObject = InitializeConfig();
        if (!m_hookImplObject) return false;
        if (!m_hipsCfgObject) return false;
        g_log_object->DisableDupFlt();
        if (!m_hookImplObject->Initialize(std::move(m_hipsCfgObject)))
            return false;
        if (!m_hookImplObject->HookAllApis())
            return false;
        m_valid = true;
        return m_valid;
    }

    bool EnableAllApis() { return m_hookImplObject->EnableAllApis(); }
    bool DisableAllApis() { return m_hookImplObject->DisableAllApis(); }

private:
    bool m_valid;
    std::shared_ptr<cchips::CHipsCfgObject> m_hipsCfgObject;
    std::shared_ptr<cchips::CHookImplementObject> m_hookImplObject;
};

extern std::unique_ptr<HipsHookTest> g_hook_test_object;
extern std::unique_ptr<CServerObject> g_server_object;




