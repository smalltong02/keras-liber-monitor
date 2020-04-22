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
class HookImplObjTest : public testing::Test
{
protected:
	HookImplObjTest() {}
	~HookImplObjTest() override {}

	virtual void SetUp() override {
		ASSERT_TRUE(g_hook_test_object->EnableAllApis());
	}

	void TearDown() override {
		ASSERT_TRUE(g_hook_test_object->DisableAllApis());
	}
};

TEST_F(HookImplObjTest, HookApiVerifierTest)
{
	HRESULT pre_hr, hooked_hr, pre_hr1, hooked_hr1;
	HMODULE pre_hmod, hooked_hmod, pre_hmod1, hooked_hmod1;
	FARPROC pre_proc = nullptr, hooked_proc = nullptr;
	bool pre_bret, hooked_bret;
	
	ULARGE_INTEGER pre_free_bytes, hooked_free_bytes;
	ULARGE_INTEGER pre_total_bytes, hooked_total_bytes;
	ULARGE_INTEGER pre_total_free, hooked_total_free;
	IDispatch* pDispatch;
	CLSID clsid;
	DWORD dwFlags = 0;

	ASSERT_TRUE(g_hook_test_object->DisableAllApis());
	pre_hr = CLSIDFromProgID(OLESTR("se.mysoft"), &clsid);
	pre_hmod = LoadLibraryExW(L"ole32.dll", nullptr, dwFlags);
	pre_hr1 = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pDispatch);
	pre_hmod1 = GetModuleHandleW(L"Kernel32.dll");
	if(pre_hmod1 != nullptr)
		pre_proc = GetProcAddress(pre_hmod1, "GetDiskFreeSpaceExW");
	pre_bret = GetDiskFreeSpaceExW(L"C:\\Windows", &pre_free_bytes, &pre_total_bytes, &pre_total_free);

	ASSERT_TRUE(g_hook_test_object->EnableAllApis());
	hooked_hr = CLSIDFromProgID(OLESTR("se.mysoft"), &clsid);
	ASSERT_EQ(pre_hr, hooked_hr);
	hooked_hmod = LoadLibraryW(L"ole32.dll");
	ASSERT_EQ(pre_hmod, hooked_hmod);
	hooked_hr1 = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pDispatch);
	ASSERT_EQ(pre_hr1, hooked_hr1);
	hooked_hmod1 = GetModuleHandleW(L"Kernel32.dll");
	ASSERT_EQ(pre_hmod1, hooked_hmod1);
	if (hooked_hmod1 != nullptr)
		hooked_proc = GetProcAddress(pre_hmod1, "GetDiskFreeSpaceExW");
	ASSERT_EQ(pre_proc, hooked_proc);
	hooked_bret = GetDiskFreeSpaceExW(L"C:\\Windows", &hooked_free_bytes, &hooked_total_bytes, &hooked_total_free);
	ASSERT_EQ(pre_bret, hooked_bret);
	ASSERT_EQ(pre_total_bytes.QuadPart, hooked_total_bytes.QuadPart);
}

TEST_F(HookImplObjTest, CheckFunctionTest)
{
	// test for modify return value
	DWORD c = GetTickCount();
	ASSERT_TRUE(c = 1234567);
	// test for modify argument
	SYSTEMTIME SystemTime = {};
	GetSystemTime(&SystemTime);
	ASSERT_TRUE(SystemTime.wYear == 2010 && SystemTime.wMonth == 12 && SystemTime.wDay == 31);
}

//TEST_F(HookImplObjTest, DelayHookTest)
//{
//	return;
//}

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
