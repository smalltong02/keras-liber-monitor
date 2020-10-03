#include "stdafx.h"
#include <stdio.h>
#include <windows.h>
#include <string>
#include <vector>
#include "commutils.h"
#include "HipsCfgObject.h"
#include "HookImplementObject.h"
#include "HipsHookTest.h"
#include "benchmark\benchmark.h"
#include "gtest\gtest.h"
#include "gmock\gmock.h"

#ifdef _FUNCTION_TEST
class ApiHookImplement
{
public:
    ApiHookImplement() = default;
    ~ApiHookImplement() = default;
};

class ApiHookMock : public ApiHookImplement
{
public:
};

class HookImplementTest : public testing::Test
{
protected:
    HookImplementTest() {}
    ~HookImplementTest() override {}

    virtual void SetUp() override {
        ASSERT_TRUE(g_hook_test_object->EnableAllApis());
    }

    void TearDown() override {
        ASSERT_TRUE(g_hook_test_object->DisableAllApis());
    }
};

#endif

#ifdef _BENCHMARK_TEST
#include "HipsHookTest.h"

static void benchmark_nohookapi_performance(benchmark::State& state)
{
    g_hook_test_object->DisableAllApis();
    bool bret;
    ULARGE_INTEGER free_bytes;
    ULARGE_INTEGER total_bytes;
    ULARGE_INTEGER total_free;

    for (auto _ : state) {
        bret = GetDiskFreeSpaceExW(L"C:\\Windows", &free_bytes, &total_bytes, &total_free);
    }
}

static void benchmark_hookapi_performance(benchmark::State& state)
{
    g_hook_test_object->EnableAllApis();
    bool bret;
    ULARGE_INTEGER free_bytes;
    ULARGE_INTEGER total_bytes;
    ULARGE_INTEGER total_free;

    for (auto _ : state) {
        bret = GetDiskFreeSpaceExW(L"C:\\Windows", &free_bytes, &total_bytes, &total_free);
    }
    g_hook_test_object->DisableAllApis();
}

BENCHMARK(benchmark_nohookapi_performance)->ThreadPerCpu()->Arg(100)->Arg(1000)->Arg(10000)->Arg(100000);
BENCHMARK(benchmark_hookapi_performance)->ThreadPerCpu()->Arg(100)->Arg(1000)->Arg(10000)->Arg(100000);


#endif
