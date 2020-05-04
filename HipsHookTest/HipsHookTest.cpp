// hipshooktest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <wincodec.h>
#include <Wbemidl.h>
#include "HipsHookTest.h"
#include "benchmark\benchmark.h"
#include "gtest\gtest.h"
#include "gmock\gmock.h"
#include "resource.h"

using namespace cchips;
using namespace testing;

#pragma comment(lib, "wbemuuid.lib")

std::unique_ptr<HipsHookTest> g_hook_test_object = std::make_unique<HipsHookTest>();

#ifdef _FUNCTION_TEST
int _tmain(int argc, _TCHAR* argv[])
{
	if (g_hook_test_object->Initialize())
	{
		testing::InitGoogleMock(&argc, argv);
		return RUN_ALL_TESTS();
	}
	return 0;
}
#endif

#ifdef _BENCHMARK_TEST
int _tmain(int argc, _TCHAR* argv[])
{
	if (g_hook_test_object->Initialize())
	{
		::benchmark::Initialize(&argc, argv);
		if (::benchmark::ReportUnrecognizedArguments(argc, argv)) return 1;
		::benchmark::RunSpecifiedBenchmarks();
	}
	return 0;
}
#endif


