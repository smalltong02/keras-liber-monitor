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
#include "R3LogObject.h"

class CServerObject
{
public:
    CServerObject() {
        m_tempfp = tmpfile();
        std::function<void(const std::unique_ptr<CRapidJsonWrapper>)> callback(std::bind(&CServerObject::LogCallBack, this, std::placeholders::_1));
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
    bool CheckVerifier(const CRapidJsonWrapper& rapid_log) const
    {
        std::string verifier_result;
        if (rapid_log.FindTopMember("verifier_result", verifier_result))
        {
            if (!verifier_result.compare("false")) {
                return false;
            }
        }
        return true;
    }

    void addToLogCountMap(std::string key) {
        if (log_count_map.find(key) != log_count_map.end()) {
            log_count_map[key]++;
        }
        else {
            log_count_map[key] = 1;
        }
    }

    void CheckServices(const CRapidJsonWrapper& rapid_log)
    {
        if (m_enable_service)
        {
            std::string value;
            if (rapid_log.FindTopMember("Action", value))
            {
                if (!value.compare("S0")) {
                    addToLogCountMap("S0");
                }
                else if (!value.compare("S1")) {
                    addToLogCountMap("S1");
                }
                else if (!value.compare("S2")) {
                    addToLogCountMap("S2");
                }
                else if (!value.compare("S3")) {
                    addToLogCountMap("S3");
                }
                else if (!value.compare("S4")) {
                    addToLogCountMap("S4");
                }
                else if (!value.compare("S5")) {
                    addToLogCountMap("S5");
                }
            }
        }
        return;
    }

    void CheckDebugPid(const CRapidJsonWrapper& rapid_log)
    {
        if (m_enable_debugpid)
        {
            std::string pid_string;
            if (rapid_log.FindTopMember("Pid", pid_string))
            {
                char* nodig = nullptr;
                DWORD_PTR pid = (DWORD_PTR)std::strtoll(pid_string.c_str(), &nodig, 10);
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
        return;
    }

    void LogCallBack(const std::unique_ptr<CRapidJsonWrapper> rapid_log) {
        ASSERT_TRUE(rapid_log);
        if (!rapid_log) return;
        if (!rapid_log->IsValid()) return;
        ASSERT_TRUE(CheckVerifier(*rapid_log));
        CheckServices(*rapid_log);
        CheckDebugPid(*rapid_log);
        m_logs_total_count++;
        {
            std::lock_guard lock(m_log_mutex);
            if (m_tempfp)
            {
                fputs(rapid_log->Serialize().c_str(), m_tempfp);
            }
            else
                std::cout << rapid_log->Serialize() << std::endl;
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




