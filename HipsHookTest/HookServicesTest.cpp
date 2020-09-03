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
#include "LogObject.h"
#include "HipsHookTest.h"
#include "benchmark\benchmark.h"
#include "gtest\gtest.h"
#include "gmock\gmock.h"
#include "ServicesBase.h"

#ifdef _FUNCTION_TEST
class HookServicesTest : public testing::Test
{
protected:
    HookServicesTest() {
        ;
    }
    ~HookServicesTest() override {}

    virtual void SetUp() override {
        g_server_object->EnableServiceTest();
        ASSERT_TRUE(g_hook_test_object->EnableAllApis());
    }

    void TearDown() override {
        ServicesTestObject service;
        ServiceInstaller::Uninstall(service);
        g_server_object->ClearLogMap();
        g_server_object->DisableServiceTest();
        ASSERT_TRUE(g_hook_test_object->DisableAllApis());
    }
};


TEST_F(HookServicesTest, ServicesTestA)
{
    ServicesTestObject service;
    ServiceInstaller::Install(service);
    ServiceInstaller::Start(service);
    ServiceInstaller::Uninstall(service);
    Sleep(100); // waiting 100 milliseconds for receive log.
    std::unordered_map<std::string, int> log_count_map = g_server_object->GetLogCountMap();
    // OpenService be filter by sender because it is a duplicate log.
    ASSERT_TRUE(log_count_map["S0"] == 3); // OpenSCManager
    ASSERT_TRUE(log_count_map["S1"] == 1); // CreateService
    ASSERT_TRUE(log_count_map["S2"] == 2); // OpenService
    ASSERT_TRUE(log_count_map["S3"] == 1); // DeleteService
    ASSERT_TRUE(log_count_map["S4"] == 1); // StartService
    ASSERT_TRUE(log_count_map["S5"] == 2); // QueryServiceStatus
}

TEST_F(HookServicesTest, ServicesTestW)
{
    ServicesTestObject service;
    ServiceInstaller::Install(service, false);
    ServiceInstaller::Start(service, false);
    ServiceInstaller::Uninstall(service, false);
    Sleep(100); // waiting 100 milliseconds for receive log.
    std::unordered_map<std::string, int> log_count_map = g_server_object->GetLogCountMap();
    // QueryServiceStatus be filter by sender because it is a duplicate log.
    ASSERT_TRUE(log_count_map["S0"] == 3);
    ASSERT_TRUE(log_count_map["S1"] == 1);
    ASSERT_TRUE(log_count_map["S2"] == 2);
    ASSERT_TRUE(log_count_map["S3"] == 1);
    ASSERT_TRUE(log_count_map["S4"] == 1);
    ASSERT_TRUE(log_count_map["S5"] == 2);
}
#endif

