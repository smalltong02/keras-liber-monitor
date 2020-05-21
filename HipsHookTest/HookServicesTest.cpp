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
        g_server_object->DisableServiceTest();
        ASSERT_TRUE(g_hook_test_object->DisableAllApis());
    }
};

TEST_F(HookServicesTest, ServicesTest)
{
    ServicesTestObject service;
    ServiceInstaller::Install(service);
    ServiceInstaller::Uninstall(service);
    Sleep(100); // waiting 100 milliseconds for receive log.
    ASSERT_TRUE(g_server_object->GetOpenSCManagerCount() == 2);
}


