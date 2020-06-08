#include "stdafx.h"
#include <stdio.h>
#include <windows.h>
#include <string>
#include <vector>
#include <functional>
#include <future>
#include <atomic>
#include <random>
#include "commutils.h"
#include "R3LogObject.h"
#include "HipsHookTest.h"
#include "benchmark\benchmark.h"
#include "gtest\gtest.h"
#include "gmock\gmock.h"
#include "R3LogObject.h"

#ifdef _FUNCTION_TEST
class CClientObject
{
public:
#define long_log_test_size 10*1024 // 10k
    CClientObject(int tag) : m_tag(tag) {
        std::function<cchips::RAPID_DOC_PAIR()> getdata(std::bind(&CClientObject::GetData, this));
        m_pipe_object = std::make_unique<CLpcPipeObject>();
        if (m_pipe_object)
            m_pipe_object->Connect(getdata);
    }
    ~CClientObject() { m_pipe_object->StopConnect(); }
    RAPID_DOC_PAIR GetData() {
        std::lock_guard lock(m_mutex_log);
        if (!m_log_elements) return RAPID_DOC_PAIR(nullptr, nullptr);
        if (!m_checker) return RAPID_DOC_PAIR(nullptr, nullptr);
        return RAPID_DOC_PAIR(std::move(m_log_elements), std::move(m_checker));
    }
    void EnableVerifier() { m_pipe_object->EnableVerifier(); }
    void DisableVerifier() { m_pipe_object->DisableVerifier(); }
    int GetTotalLogs() const { return m_pipe_object->GetTotalLogs(); }
    // for test
    void SendData() {
        std::stringstream send_ss;
        send_ss << "test-" << m_tag << ": " << m_count++;
        {
            std::lock_guard lock(m_mutex_log);
            if (!m_log_elements)
            {
                m_log_elements = std::make_unique<RapidDocument>();
                m_log_elements->SetObject();
            }
            else
            {
                m_count--;
                return;
            }
            if (!m_checker) m_checker = std::make_unique<CChecker>();
            else
            {
                m_count--;
                return;
            }
            ADD_JSON_DOC(*m_log_elements, "Test", send_ss.str().c_str());
            if (m_checker) m_checker->Update("Test");
            if (m_checker) m_checker->Update(send_ss.str());
        }
        m_pipe_object->Activated();
    }
    void SendLongData() {
        std::stringstream send_ss;
        std::mt19937 generator{ std::random_device{}() };
        std::uniform_int_distribution<int> distribution('a', 'z');
        int generate_len = long_log_test_size;
        std::string str(generate_len, '\0');
        for (auto& c : str)
            c = distribution(generator);
        send_ss << str << "-" << m_tag << ": " << m_count++;
        {
            std::lock_guard lock(m_mutex_log);
            if (!m_log_elements)
            {
                m_log_elements = std::make_unique<RapidDocument>();
                m_log_elements->SetObject();
            }
            else
            {
                m_count--;
                return;
            }
            if (!m_checker) m_checker = std::make_unique<CChecker>();
            else
            {
                m_count--;
                return;
            }
            ADD_JSON_DOC(*m_log_elements, "LongTest", send_ss.str().c_str());
            if (m_checker) m_checker->Update("LongTest");
            if (m_checker) m_checker->Update(send_ss.str());
        }
        m_pipe_object->Activated();
    }
private:
    int m_count = 0;
    int m_tag;
    std::unique_ptr<RapidDocument> m_log_elements = nullptr;
    std::unique_ptr<CChecker> m_checker = nullptr;
    std::unique_ptr<CLpcPipeObject> m_pipe_object;
    std::mutex m_mutex_log;
};

//class HookLogObjectTest : public testing::Test
//{
//protected:
//    HookLogObjectTest() {
//        ;
//    }
//    ~HookLogObjectTest() override {}
//
//    virtual void SetUp() override {
//        ASSERT_TRUE(g_hook_test_object->EnableAllApis());
//    }
//
//    void TearDown() override {
//        ASSERT_TRUE(g_hook_test_object->DisableAllApis());
//    }
//};
//
//TEST_F(HookLogObjectTest, PipeLogsCountTest)
//{
//    std::atomic_bool brunning = true;
//    std::vector<std::future<int>> results;
//    int send_logs_count = g_server_object->GetTotalLogs();
//    unsigned task_count = std::thread::hardware_concurrency();
//    for (unsigned int i = 1; i <= task_count; i++)
//    {
//        std::packaged_task<int(int)> task(
//            [&](int tag) ->int {
//                CClientObject client(tag);
//                while (brunning)
//                {
//                    client.SendData();
//                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
//                }
//                return client.GetTotalLogs();
//            });
//        results.push_back(task.get_future());
//        std::thread thread(std::move(task), i);
//        thread.detach();
//    }
//    std::this_thread::sleep_for(std::chrono::seconds(10));
//    brunning = false;
//    for (auto& r : results) {
//        send_logs_count += r.get();
//    }
//    int receive_logs_count = g_server_object->GetTotalLogs();
//    ASSERT_TRUE(receive_logs_count >= send_logs_count);
//}
//
//TEST_F(HookLogObjectTest, PipeLongLogTest)
//{
//    std::atomic_bool brunning = true;
//    unsigned task_count = 1;
//    std::thread thread( 
//        [&]() {
//            CClientObject client(1);
//            client.EnableVerifier();
//            while (brunning)
//            {
//                client.SendLongData();
//                std::this_thread::sleep_for(std::chrono::milliseconds(100));
//            }
//            client.DisableVerifier();
//        });
//    std::this_thread::sleep_for(std::chrono::seconds(1));
//    brunning = false;
//    thread.join();
//}

#endif