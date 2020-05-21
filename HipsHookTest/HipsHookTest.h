#pragma once
#include <stdio.h>
#include <windows.h>
#include <string>
#include <vector>
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
    int GetTotalLogs() const { return m_pipe_object->GetTotalLogs(); }
    int GetOpenSCManagerCount() const { return m_nopenscmanager_count; }
    int GetCreateServiceCount() const { return m_ncreateservice_count; }
    int GetOpenServiceCount() const { return m_nopenservice_count; }
    int GetDeleteServiceCount() const { return m_ndeleteservice_count; }
    int GetStartServiceCount() const { return m_nstartservice_count; }
    void EnableServiceTest() { m_enable_service = true; }
    void DisableServiceTest() { m_enable_service = false; }
private:
    bool CheckVerifier(const std::unique_ptr<std::stringstream>& log) const
    {
        if (!log) return true;
        if (log->str().find("\"verifier_result\": \"failed\"") != std::string::npos)
            return false;
        return true;
    }
    void CheckServices(const std::unique_ptr<std::stringstream>& log)
    {
        if (!log) return;
        if (m_enable_service)
        {
            if (log->str().find("\"Action\": \"S0\"") != std::string::npos)
                m_nopenscmanager_count++;
            else if (log->str().find("\"Action\": \"S1\"") != std::string::npos)
                m_ncreateservice_count++;
            else if (log->str().find("\"Action\": \"S2\"") != std::string::npos)
                m_nopenservice_count++;
            else if (log->str().find("\"Action\": \"S3\"") != std::string::npos)
                m_ndeleteservice_count++;
            else if (log->str().find("\"Action\": \"S4\"") != std::string::npos)
                m_nstartservice_count++;
        }
        return;
    }
    void LogCallBack(const std::unique_ptr<std::stringstream> log) {
        ASSERT_TRUE(log);
        if (!log) return;
        ASSERT_TRUE(CheckVerifier(log));
        CheckServices(log);
        m_logs_total_count++;
        {
            std::lock_guard lock(m_mutex);
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
    std::mutex m_mutex;
    FILE* m_tempfp = nullptr;
    std::unique_ptr<CLpcPipeObject> m_pipe_object;
    bool m_enable_service = false;
    int m_nopenscmanager_count = 0;
    int m_ncreateservice_count = 0;
    int m_nopenservice_count = 0;
    int m_ndeleteservice_count = 0;
    int m_nstartservice_count = 0;
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




