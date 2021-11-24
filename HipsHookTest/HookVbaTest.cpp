#include "stdafx.h"
#include <stdio.h>
#include <windows.h>
#include <winsock.h>
#include <string>
#include <vector>
#include <functional>
#include <future>
#include <atomic>
#include <random>
#include "utils.h"
#include "commutils.h"
#include "LogObject.h"
#include "HipsHookTest.h"

#ifdef _FUNCTION_TEST
class HookVbaTest : public testing::Test
{
protected:
    HookVbaTest() {
        ;
    }
    ~HookVbaTest() override {}

    virtual void SetUp() override {
        ASSERT_TRUE(g_hook_test_object->EnableAllApis());
    }

    void TearDown() override {
        ASSERT_TRUE(g_hook_test_object->DisableAllApis());
    }
};

TEST_F(HookVbaTest, FindWindowWTest)
{
    HWND hwnd = FindWindowW(L"test", L"test");
    ASSERT_EQ(hwnd, nullptr);
}
#endif
