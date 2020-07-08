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

    virtual HMODULE LoadLibraryA(LPCSTR lpLibFileName) = 0;
    virtual HMODULE LoadLibraryW(LPCWSTR lpLibFileName) = 0;
    virtual HMODULE LoadLibraryExA(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) = 0;
    virtual HMODULE LoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) = 0;
};

class ApiHookMock : public ApiHookImplement
{
public:
    MOCK_METHOD1(LoadLibraryA, HMODULE(LPCSTR));
    MOCK_METHOD1(LoadLibraryW, HMODULE(LPCWSTR));
    MOCK_METHOD3(LoadLibraryExA, HMODULE(LPCSTR, HANDLE, DWORD));
    MOCK_METHOD3(LoadLibraryExW, HMODULE(LPCWSTR, HANDLE, DWORD));
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

// test Win32 API LoadLibraryA
TEST_F(HookImplementTest, LoadLibraryA_Prelog_lpLibFileName_Test)
{
    HMODULE pre_hmod_nul, pre_hmod_nul_str, pre_hmod_normal, pre_hmod_long, pre_hmod_invalid_data;
    DWORD pre_error_nul, pre_error_nul_str, pre_error_normal, pre_error_long, pre_error_invalid_data;
    HMODULE hooked_hmod_nul, hooked_hmod_nul_str, hooked_hmod_normal, hooked_hmod_long, hooked_hmod_invalid_data;
    DWORD hooked_error_nul, hooked_error_nul_str, hooked_error_normal, hooked_error_long, hooked_error_invalid_data;
    ApiHookMock hook_mock;
    // test variable
    CStringCommonTestI str_testI_object;

    // first call test API when DisableAllApis().
    ASSERT_TRUE(g_hook_test_object->DisableAllApis());
    // test when lpLibFileName is nullptr
    SetLastError(0); EXPECT_CALL(hook_mock, LoadLibraryA(testing::_)).Times(1).WillRepeatedly(testing::Return(::LoadLibraryA(nullptr)));
    pre_hmod_nul = hook_mock.LoadLibraryA(nullptr);
    pre_error_nul = GetLastError();
    // test when lpLibFileName is "\0"
    SetLastError(0); EXPECT_CALL(hook_mock, LoadLibraryA(testing::_)).Times(1).WillRepeatedly(testing::Return(::LoadLibraryA(std::get<const std::string>(str_testI_object.NullString(1)).c_str())));
    pre_hmod_nul_str = hook_mock.LoadLibraryA(nullptr);
    pre_error_nul_str = GetLastError();
    // test when lpLibFileName is normal dll name "ole32.dll"
    SetLastError(0); EXPECT_CALL(hook_mock, LoadLibraryA(testing::_)).Times(1).WillRepeatedly(testing::Return(::LoadLibraryA(std::get<const std::string>(str_testI_object.NormalString(std::string("ole32.dll"))).c_str())));
    pre_hmod_normal = hook_mock.LoadLibraryA(nullptr);
    pre_error_normal = GetLastError();
    // test when lpLibFileName is invalid dll name and super long name.
    SetLastError(0); EXPECT_CALL(hook_mock, LoadLibraryA(testing::_)).Times(1).WillRepeatedly(testing::Return(::LoadLibraryA(std::get<const std::string>(str_testI_object.SuperLongString(std::string("kernel32.dll"))).c_str())));
    pre_hmod_long = hook_mock.LoadLibraryA(nullptr);
    pre_error_long = GetLastError();
    // test when lpLibFileName is incorrect data block.
    SetLastError(0); EXPECT_CALL(hook_mock, LoadLibraryA(testing::_)).Times(1).WillRepeatedly(testing::Return(::LoadLibraryA(&str_testI_object.SuperLongData()[0])));
    pre_hmod_invalid_data = hook_mock.LoadLibraryA(nullptr);
    pre_error_invalid_data = GetLastError();

    // second call test API when EnableAllApis().
    ASSERT_TRUE(g_hook_test_object->EnableAllApis());
    // test when lpLibFileName is nullptr
    SetLastError(0); EXPECT_CALL(hook_mock, LoadLibraryA(testing::_)).Times(1).WillRepeatedly(testing::Return(::LoadLibraryA(nullptr)));
    hooked_hmod_nul = hook_mock.LoadLibraryA(nullptr);
    hooked_error_nul = GetLastError();
    // test when lpLibFileName is "\0"
    SetLastError(0); EXPECT_CALL(hook_mock, LoadLibraryA(testing::_)).Times(1).WillRepeatedly(testing::Return(::LoadLibraryA(std::get<const std::string>(str_testI_object.NullString(1)).c_str())));
    hooked_hmod_nul_str = hook_mock.LoadLibraryA(nullptr);
    hooked_error_nul_str = GetLastError();
    // test when lpLibFileName is normal dll name "ole32.dll"
    SetLastError(0); EXPECT_CALL(hook_mock, LoadLibraryA(testing::_)).Times(1).WillRepeatedly(testing::Return(::LoadLibraryA(std::get<const std::string>(str_testI_object.NormalString(std::string("ole32.dll"))).c_str())));
    hooked_hmod_normal = hook_mock.LoadLibraryA(nullptr);
    hooked_error_normal = GetLastError();
    // test when lpLibFileName is invalid dll name and super long name.
    SetLastError(0); EXPECT_CALL(hook_mock, LoadLibraryA(testing::_)).Times(1).WillRepeatedly(testing::Return(::LoadLibraryA(std::get<const std::string>(str_testI_object.SuperLongString(std::string("kernel32.dll"))).c_str())));
    hooked_hmod_long = hook_mock.LoadLibraryA(nullptr);
    hooked_error_long = GetLastError();
    // test when lpLibFileName is incorrect data block.
    SetLastError(0); EXPECT_CALL(hook_mock, LoadLibraryA(testing::_)).Times(1).WillRepeatedly(testing::Return(::LoadLibraryA(&str_testI_object.SuperLongData()[0])));
    hooked_hmod_invalid_data = hook_mock.LoadLibraryA(nullptr);
    hooked_error_invalid_data = GetLastError();

    // compare return result and error code.
    EXPECT_EQ(pre_hmod_nul, hooked_hmod_nul);
    EXPECT_EQ(pre_error_nul, hooked_error_nul);
    EXPECT_EQ(pre_hmod_nul_str, hooked_hmod_nul_str);
    EXPECT_EQ(pre_error_nul_str, hooked_error_nul_str);
    EXPECT_EQ(pre_hmod_normal, hooked_hmod_normal);
    EXPECT_EQ(pre_error_normal, hooked_error_normal);
    EXPECT_EQ(pre_hmod_long, hooked_hmod_long);
    EXPECT_EQ(pre_error_long, hooked_error_long);
    EXPECT_EQ(pre_hmod_invalid_data, hooked_hmod_invalid_data);
    EXPECT_EQ(pre_error_invalid_data, hooked_error_invalid_data);
}

// test Win32 API LoadLibraryW
TEST_F(HookImplementTest, LoadLibraryW_Prelog_lpLibFileName_Test)
{
    HMODULE pre_hmod_nul, pre_hmod_nul_str, pre_hmod_normal, pre_hmod_long, pre_hmod_invalid_data;
    DWORD pre_error_nul, pre_error_nul_str, pre_error_normal, pre_error_long, pre_error_invalid_data;
    HMODULE hooked_hmod_nul, hooked_hmod_nul_str, hooked_hmod_normal, hooked_hmod_long, hooked_hmod_invalid_data;
    DWORD hooked_error_nul, hooked_error_nul_str, hooked_error_normal, hooked_error_long, hooked_error_invalid_data;
    ApiHookMock hook_mock;
    // test variable
    CStringCommonTestI str_testI_object(true);

    // first call test API when DisableAllApis().
    ASSERT_TRUE(g_hook_test_object->DisableAllApis());
    // test when lpLibFileName is nullptr
    SetLastError(0); EXPECT_CALL(hook_mock, LoadLibraryW(testing::_)).Times(1).WillRepeatedly(testing::Return(::LoadLibraryW(nullptr)));
    pre_hmod_nul = hook_mock.LoadLibraryW(nullptr);
    pre_error_nul = GetLastError();
    // test when lpLibFileName is "\0"
    SetLastError(0); EXPECT_CALL(hook_mock, LoadLibraryW(testing::_)).Times(1).WillRepeatedly(testing::Return(::LoadLibraryW(std::get<const std::wstring>(str_testI_object.NullString(1)).c_str())));
    pre_hmod_nul_str = hook_mock.LoadLibraryW(nullptr);
    pre_error_nul_str = GetLastError();
    // test when lpLibFileName is normal dll name "ole32.dll"
    SetLastError(0); EXPECT_CALL(hook_mock, LoadLibraryW(testing::_)).Times(1).WillRepeatedly(testing::Return(::LoadLibraryW(std::get<const std::wstring>(str_testI_object.NormalString(std::wstring(L"ole32.dll"))).c_str())));
    pre_hmod_normal = hook_mock.LoadLibraryW(nullptr);
    pre_error_normal = GetLastError();
    // test when lpLibFileName is invalid dll name and super long name.
    SetLastError(0); EXPECT_CALL(hook_mock, LoadLibraryW(testing::_)).Times(1).WillRepeatedly(testing::Return(::LoadLibraryW(std::get<const std::wstring>(str_testI_object.SuperLongString(std::wstring(L"kernel32.dll"))).c_str())));
    pre_hmod_long = hook_mock.LoadLibraryW(nullptr);
    pre_error_long = GetLastError();
    // test when lpLibFileName is incorrect data block.
    SetLastError(0); EXPECT_CALL(hook_mock, LoadLibraryW(testing::_)).Times(1).WillRepeatedly(testing::Return(::LoadLibraryW((LPCWSTR)&str_testI_object.SuperLongData()[0])));
    pre_hmod_invalid_data = hook_mock.LoadLibraryW(nullptr);
    pre_error_invalid_data = GetLastError();

    // second call test API when EnableAllApis().
    ASSERT_TRUE(g_hook_test_object->EnableAllApis());
    // test when lpLibFileName is nullptr
    SetLastError(0); EXPECT_CALL(hook_mock, LoadLibraryW(testing::_)).Times(1).WillRepeatedly(testing::Return(::LoadLibraryW(nullptr)));
    hooked_hmod_nul = hook_mock.LoadLibraryW(nullptr);
    hooked_error_nul = GetLastError();
    // test when lpLibFileName is "\0"
    SetLastError(0); EXPECT_CALL(hook_mock, LoadLibraryW(testing::_)).Times(1).WillRepeatedly(testing::Return(::LoadLibraryW(std::get<const std::wstring>(str_testI_object.NullString(1)).c_str())));
    hooked_hmod_nul_str = hook_mock.LoadLibraryW(nullptr);
    hooked_error_nul_str = GetLastError();
    // test when lpLibFileName is normal dll name "ole32.dll"
    SetLastError(0); EXPECT_CALL(hook_mock, LoadLibraryW(testing::_)).Times(1).WillRepeatedly(testing::Return(::LoadLibraryW(std::get<const std::wstring>(str_testI_object.NormalString(std::wstring(L"ole32.dll"))).c_str())));
    hooked_hmod_normal = hook_mock.LoadLibraryW(nullptr);
    hooked_error_normal = GetLastError();
    // test when lpLibFileName is invalid dll name and super long name.
    SetLastError(0); EXPECT_CALL(hook_mock, LoadLibraryW(testing::_)).Times(1).WillRepeatedly(testing::Return(::LoadLibraryW(std::get<const std::wstring>(str_testI_object.SuperLongString(std::wstring(L"kernel32.dll"))).c_str())));
    hooked_hmod_long = hook_mock.LoadLibraryW(nullptr);
    hooked_error_long = GetLastError();
    // test when lpLibFileName is incorrect data block.
    SetLastError(0); EXPECT_CALL(hook_mock, LoadLibraryW(testing::_)).Times(1).WillRepeatedly(testing::Return(::LoadLibraryW((LPCWSTR)&str_testI_object.SuperLongData()[0])));
    hooked_hmod_invalid_data = hook_mock.LoadLibraryW(nullptr);
    hooked_error_invalid_data = GetLastError();

    // compare return result and error code.
    EXPECT_EQ(pre_hmod_nul, hooked_hmod_nul);
    EXPECT_EQ(pre_error_nul, hooked_error_nul);
    EXPECT_EQ(pre_hmod_nul_str, hooked_hmod_nul_str);
    EXPECT_EQ(pre_error_nul_str, hooked_error_nul_str);
    EXPECT_EQ(pre_hmod_normal, hooked_hmod_normal);
    EXPECT_EQ(pre_error_normal, hooked_error_normal);
    EXPECT_EQ(pre_hmod_long, hooked_hmod_long);
    EXPECT_EQ(pre_error_long, hooked_error_long);
    EXPECT_EQ(pre_hmod_invalid_data, hooked_hmod_invalid_data);
    EXPECT_EQ(pre_error_invalid_data, hooked_error_invalid_data);
}

// test Win32 API LoadLibraryExA
TEST_F(HookImplementTest, LoadLibraryExA_Prelog_lpLibFileName_Test)
{
    ;
}

TEST_F(HookImplementTest, LoadLibraryExA_Prelog_dwFlags_Test)
{
    ;
}

// test Win32 API LoadLibraryExW
TEST_F(HookImplementTest, LoadLibraryExW_Prelog_lpLibFileName_Test)
{
    ;
}

TEST_F(HookImplementTest, LoadLibraryExW_Prelog_dwFlags_Test)
{
    ;
}

//TEST_F(HookImplObjTest, HookApiVerifierTest)
//{
//    HRESULT pre_hr, hooked_hr, pre_hr1, hooked_hr1;
//    HMODULE pre_hmod, hooked_hmod, pre_hmod1, hooked_hmod1;
//    FARPROC pre_proc = nullptr, hooked_proc = nullptr;
//    bool pre_bret, hooked_bret;
//
//    ULARGE_INTEGER pre_free_bytes, hooked_free_bytes;
//    ULARGE_INTEGER pre_total_bytes, hooked_total_bytes;
//    ULARGE_INTEGER pre_total_free, hooked_total_free;
//    IDispatch* pDispatch;
//    CLSID clsid;
//    DWORD dwFlags = 0;
//
//    ASSERT_TRUE(g_hook_test_object->DisableAllApis());
//    pre_hr = CLSIDFromProgID(OLESTR("se.mysoft"), &clsid);
//    pre_hmod = LoadLibraryExW(L"ole32.dll", nullptr, dwFlags);
//    pre_hr1 = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pDispatch);
//    pre_hmod1 = GetModuleHandleW(L"Kernel32.dll");
//    if (pre_hmod1 != nullptr)
//        pre_proc = GetProcAddress(pre_hmod1, "GetDiskFreeSpaceExW");
//    pre_bret = GetDiskFreeSpaceExW(L"C:\\Windows", &pre_free_bytes, &pre_total_bytes, &pre_total_free);
//
//    ASSERT_TRUE(g_hook_test_object->EnableAllApis());
//    hooked_hr = CLSIDFromProgID(OLESTR("se.mysoft"), &clsid);
//    ASSERT_EQ(pre_hr, hooked_hr);
//    hooked_hmod = LoadLibraryW(L"ole32.dll");
//    ASSERT_EQ(pre_hmod, hooked_hmod);
//    hooked_hr1 = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pDispatch);
//    ASSERT_EQ(pre_hr1, hooked_hr1);
//    hooked_hmod1 = GetModuleHandleW(L"Kernel32.dll");
//    ASSERT_EQ(pre_hmod1, hooked_hmod1);
//    if (hooked_hmod1 != nullptr)
//        hooked_proc = GetProcAddress(pre_hmod1, "GetDiskFreeSpaceExW");
//    ASSERT_EQ(pre_proc, hooked_proc);
//    hooked_bret = GetDiskFreeSpaceExW(L"C:\\Windows", &hooked_free_bytes, &hooked_total_bytes, &hooked_total_free);
//    ASSERT_EQ(pre_bret, hooked_bret);
//    ASSERT_EQ(pre_total_bytes.QuadPart, hooked_total_bytes.QuadPart);
//}
//
//TEST_F(HookImplObjTest, CheckFunctionTest)
//{
//    // test for modify return value
//    DWORD c = GetTickCount();
//    ASSERT_TRUE(c = 1234567);
//    // test for modify argument
//    SYSTEMTIME SystemTime = {};
//    GetSystemTime(&SystemTime);
//    ASSERT_TRUE(SystemTime.wYear == 2010 && SystemTime.wMonth == 12 && SystemTime.wDay == 31);
//}
//
////TEST_F(HookImplObjTest, DelayHookTest)
////{
////	return;
////}

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
