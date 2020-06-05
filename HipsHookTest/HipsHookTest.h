#pragma once
#include <stdio.h>
#include <windows.h>
#include <string>
#include <vector>
#include <regex>
#include "gtest\gtest.h"
#include "resource.h"
#include "HipsCfgObject.h"
#include "HookImplementObject.h"
#include "utils.h"
#include "commutils.h"

class CServerObject
{
public:
    CServerObject() {
        m_tempfp = tmpfile();
        std::function<void(const std::unique_ptr<std::stringstream>)> callback(std::bind(&CServerObject::LogCallBack, this, std::placeholders::_1));
        m_pipe_object = std::make_unique<CLpcPipeObject>();
        if (m_pipe_object)
            m_pipe_object->Listen(std::move(callback));
    }
    ~CServerObject() {
        if (m_pipe_object)
            m_pipe_object->StopListen();
        if (m_tempfp)
            fclose(m_tempfp);
    }
    void ClearLogsCount() { return m_pipe_object->ClearLogsCount(); }
    int GetTotalLogs() const { return m_pipe_object->GetTotalLogs(); }
    std::unordered_map<std::string, int> GetLogCountMap() const { return log_count_map; }
    void ClearLogCountMap() { log_count_map.clear(); }
    void EnableServiceTest() { m_enable_service = true; }
    void DisableServiceTest() { m_enable_service = false; }
    void EnableDebugPid() { m_enable_debugpid = true; }
    void DisableDebugPid() { m_enable_debugpid = false; }
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
    bool CheckVerifier(const std::unique_ptr<std::stringstream>& log) const
    {
        if (!log) return true;
        if (log->str().find("\"verifier_result\": \"failed\"") != std::string::npos)
            return false;
        return true;
    }

    rapidjson::Document logToJson(const std::unique_ptr<std::stringstream>& log) {
        rapidjson::Document log_json;
        std::string log_string = log->str();
        std::vector<int> insert;

        for (int i = 0; i < log_string.length(); ++i) {
            if (log_string[i] == ';')
                log_string[i] = ',';
            if (i > 0 && i < log_string.length() - 1) {
                if (log_string[i - 1] == '\"' && log_string[i] == '{') {
                    log_string[i - 1] = ' ';
                }
                else if (log_string[i] == '}' && log_string[i + 1] == '\"') {
                    log_string[i + 1] = ' ';
                }
                else if (log_string[i] == '\\' && log_string[i - 1] == ':') {
                    insert.push_back(i);
                }
            }
        }
        log_json.Parse(log_string.c_str());
        return log_json;
    }

    void addToLogCountMap(std::string key) {
        if (log_count_map.find(key) != log_count_map.end()) {
            log_count_map[key]++;
        }
        else {
            log_count_map[key] = 1;
        }
    }

    void CheckServices(const std::unique_ptr<std::stringstream>& log)
    {
        if (!log) return;
        if (m_enable_service)
        {
            if (log->str().find("\"Action\": \"S0\"") != std::string::npos) {
                addToLogCountMap("S0");
            }
            else if (log->str().find("\"Action\": \"S1\"") != std::string::npos) {
                addToLogCountMap("S1");
            }
            else if (log->str().find("\"Action\": \"S2\"") != std::string::npos) {
                addToLogCountMap("S2");
            }
            else if (log->str().find("\"Action\": \"S3\"") != std::string::npos) {
                addToLogCountMap("S3");
            }
            else if (log->str().find("\"Action\": \"S4\"") != std::string::npos) {
                addToLogCountMap("S4");
            }
            else if (log->str().find("\"Action\": \"S5\"") != std::string::npos) {
                addToLogCountMap("S5");
            }
        }
        return;
    }

    void CheckDebugPid(const std::unique_ptr<std::stringstream>& log)
    {
        if (!log) return;
        if (m_enable_debugpid)
        {
            size_t pos;
            if ((pos = log->str().find("\"Pid\": ")) != std::string::npos)
            {
                std::string regular_ex = "^\"[0-9]*\"";
                std::regex reg_ex(regular_ex);
                std::smatch match_result;
                std::string search_string = log->str().substr(pos + 7);
                if (std::regex_search(search_string, match_result, reg_ex))
                {
                    char* nodig = nullptr;
                    std::string result_string = match_result[0];
                    result_string = result_string.substr(1, result_string.length() - 2);
                    DWORD_PTR pid = (DWORD_PTR)std::strtoll(result_string.c_str(), &nodig, 10);
                    {
                        std::lock_guard lock(m_debugpid_mutex);
                        auto& it = m_debugpid_list.find(pid);
                        if (it != m_debugpid_list.end())
                        {
                            it->second++;
                        }
                    }
                }
            }
        }
        return;
    }

    void LogCallBack(const std::unique_ptr<std::stringstream> log) {
        ASSERT_TRUE(log);
        if (!log) return;
        ASSERT_TRUE(CheckVerifier(log));
        CheckServices(log);
        CheckDebugPid(log);
        m_logs_total_count++;
        {
            std::lock_guard lock(m_log_mutex);
            if (m_tempfp)
            {
                *log << std::endl;
                fputs(log->str().c_str(), m_tempfp);
            }
            else
                std::cout << log->str() << std::endl;
        }
        return;
    }
    std::atomic_int m_logs_total_count = 0;
    std::mutex m_log_mutex;
    FILE* m_tempfp = nullptr;
    std::unique_ptr<CLpcPipeObject> m_pipe_object;
    std::unordered_map<std::string, int> log_count_map;
    std::atomic_bool m_enable_service = false;
    std::atomic_bool m_enable_debugpid = false;
    std::mutex m_debugpid_mutex;
    std::map<DWORD_PTR, int> m_debugpid_list;
};

class HipsHookTest
{
public:
    HipsHookTest() : m_valid(false), m_hipsCfgObject(nullptr), m_hookImplObject(nullptr) {}
    ~HipsHookTest() {}

    bool Initialize() {
        m_hookImplObject = g_impl_object;
        m_hipsCfgObject = InitializeConfig();
        if (!m_hookImplObject) return false;
        if (!m_hipsCfgObject) return false;
        if (!m_hookImplObject->Initialize(m_hipsCfgObject))
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




