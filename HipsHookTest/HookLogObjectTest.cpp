#include "stdafx.h"
#include <stdio.h>
#include <windows.h>
#include <string>
#include <vector>
#include <functional>
#include <future>
#include <atomic>
#include "commutils.h"
#include "LogObject.h"
#include "HipsHookTest.h"
#include "benchmark\benchmark.h"
#include "gtest\gtest.h"
#include "gmock\gmock.h"

#ifdef _FUNCTION_TEST
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
private:
    void LogCallBack(const std::unique_ptr<std::stringstream> log) {
        ASSERT_TRUE(log);
        if (!log) return;
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
};

class CClientObject
{
public:
    CClientObject(int tag) : m_tag(tag) {
        std::function<std::unique_ptr<LOGPAIR>()> getdata(std::bind(&CClientObject::GetData, this));
        m_pipe_object = std::make_unique<CLpcPipeObject>();
        if (m_pipe_object)
            m_pipe_object->Connect(getdata);
    }
    ~CClientObject() { m_pipe_object->StopConnect(); }
    std::unique_ptr<LOGPAIR> GetData() {
        if (m_log)
            return std::move(m_log);
        else return nullptr;
    }
    int GetTotalLogs() const { return m_pipe_object->GetTotalLogs(); }
    // for test
    void SendData() {
        std::stringstream send_ss;
        send_ss << "test-" << m_tag << ": " << m_count++;
        m_log = std::make_unique<LOGPAIR>(LOGPAIR("CClientObject", send_ss.str()));
        m_pipe_object->Activated();
    }
private:
    int m_count = 0;
    int m_tag;
    std::unique_ptr<LOGPAIR> m_log;
    std::unique_ptr<CLpcPipeObject> m_pipe_object;
};

std::unique_ptr<CServerObject> g_server_object = std::make_unique<CServerObject>();

class HookLogObjectTest : public testing::Test
{
protected:
    HookLogObjectTest() {
        ;
    }
    ~HookLogObjectTest() override {}

    virtual void SetUp() override {
        ASSERT_TRUE(g_hook_test_object->EnableAllApis());
    }

    void TearDown() override {
        ASSERT_TRUE(g_hook_test_object->DisableAllApis());
    }
};

TEST_F(HookLogObjectTest, PipeLogsCountTest)
{
    std::atomic_bool brunning = true;
    std::vector<std::future<int>> results;
    unsigned task_count = std::thread::hardware_concurrency();
    for (unsigned int i = 1; i <= task_count; i++)
    {
        std::packaged_task<int(int)> task(
            [&](int tag) ->int {
                CClientObject client(tag);
                while (brunning)
                {
                    client.SendData();
                    Sleep(100);
                }
                return client.GetTotalLogs();
            });
        results.push_back(task.get_future());
        std::thread thread(std::move(task), i);
        thread.detach();
    }
    Sleep(10000);
    brunning = false;
    int send_logs_count = g_log_object->GetTotalLogs();
    for (auto& r : results) {
        send_logs_count += r.get();
    }
    int receive_logs_count = g_server_object->GetTotalLogs();
    ASSERT_TRUE(send_logs_count == receive_logs_count);
}

#endif