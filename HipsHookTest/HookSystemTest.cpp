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
#include "CBaseType.h"
#include "benchmark\benchmark.h"
#include "gtest\gtest.h"
#include "gmock\gmock.h"

#ifdef _FUNCTION_TEST
class ApiHookSystem
{
public:
    ApiHookSystem() = default;
    ~ApiHookSystem() = default;

    virtual BOOL ExitWindowsEx(UINT uFlags, DWORD dwReason) = 0;
    virtual BOOL GetDiskFreeSpaceExW(LPCWSTR lpDirectoryName, ULARGE_INTEGER* lpFreeBytesAvailableToCaller, ULARGE_INTEGER* lpTotalNumberOfBytes, ULARGE_INTEGER* lpTotalNumberOfFreeBytes) = 0;
    virtual HMODULE GetModuleHandleW(LPCWSTR lpModuleName) = 0;
    virtual DWORD GetTickCount() = 0;
    virtual void GetSystemTime(LPSYSTEMTIME lpSystemTime) = 0;
    virtual FARPROC GetProcAddress(HMODULE hModule, LPCSTR lpProcName) = 0;
};

class ApiHookSystemMock : public ApiHookSystem
{
public:
    MOCK_METHOD0(GetTickCount, DWORD());
    MOCK_METHOD1(GetModuleHandleW, HMODULE(LPCWSTR));
    MOCK_METHOD1(GetSystemTime, void(LPSYSTEMTIME));
    MOCK_METHOD2(ExitWindowsEx, BOOL(UINT, DWORD));
    MOCK_METHOD2(GetProcAddress, FARPROC(HMODULE, LPCSTR));
    MOCK_METHOD4(GetDiskFreeSpaceExW, BOOL(LPCWSTR, ULARGE_INTEGER*, ULARGE_INTEGER*, ULARGE_INTEGER*));
};

class HookSystemTest : public testing::Test
{
protected:
    HookSystemTest() {
        HANDLE hToken;
        TOKEN_PRIVILEGES tkp;
        if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
        {
            LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
            tkp.PrivilegeCount = 1;
            tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
            AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);
        }
    }
    ~HookSystemTest() override {}

    virtual void SetUp() override {
        ASSERT_TRUE(g_hook_test_object->EnableAllApis());
    }

    void TearDown() override {
        ASSERT_TRUE(g_hook_test_object->DisableAllApis());
    }
};


TEST_F(HookSystemTest, ExitWindowsEx_Prelog_uFlags_Test)
{
    BOOL bexit_invalid_flgs, bexit_sin_flgs, bexit_mul_flgs, bexit_inv_mul_flg;
    DWORD error_invalid_flgs, error_sin_flgs, error_mul_flgs, error_inv_mul_flg;
    ApiHookSystemMock hook_system_mock;
    //initialize
    std::vector<std::string> action_list;
    action_list.push_back("P41");
    g_server_object->AddLogCountMap(action_list);
    // test when uFlags is invalid flag
    SetLastError(0); EXPECT_CALL(hook_system_mock, ExitWindowsEx(testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::ExitWindowsEx(0x80000000, SHTDN_REASON_FLAG_PLANNED)));
    bexit_invalid_flgs = hook_system_mock.ExitWindowsEx(0x80000000, SHTDN_REASON_FLAG_PLANNED);
    error_invalid_flgs = GetLastError();
    EXPECT_EQ(bexit_invalid_flgs, FALSE);
    // test when uFlags is single flag
    SetLastError(0); EXPECT_CALL(hook_system_mock, ExitWindowsEx(testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::ExitWindowsEx(EWX_SHUTDOWN, SHTDN_REASON_FLAG_PLANNED)));
    bexit_sin_flgs = hook_system_mock.ExitWindowsEx(EWX_SHUTDOWN, SHTDN_REASON_FLAG_PLANNED);
    error_sin_flgs = GetLastError();
    EXPECT_EQ(bexit_sin_flgs, TRUE);
    EXPECT_EQ(error_sin_flgs, ERROR_SUCCESS);
    // test when uFlags is multiple flag
    SetLastError(0); EXPECT_CALL(hook_system_mock, ExitWindowsEx(testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::ExitWindowsEx(EWX_LOGOFF | EWX_REBOOT | EWX_FORCE, SHTDN_REASON_FLAG_PLANNED)));
    bexit_mul_flgs = hook_system_mock.ExitWindowsEx(EWX_LOGOFF | EWX_REBOOT | EWX_FORCE, SHTDN_REASON_FLAG_PLANNED);
    error_mul_flgs = GetLastError();
    EXPECT_EQ(bexit_mul_flgs, TRUE);
    EXPECT_EQ(error_mul_flgs, ERROR_SUCCESS);
    // test when uFlags is multiple invalid flag
    SetLastError(0); EXPECT_CALL(hook_system_mock, ExitWindowsEx(testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::ExitWindowsEx(EWX_LOGOFF | EWX_SHUTDOWN | 0x80000000, SHTDN_REASON_FLAG_PLANNED)));
    bexit_inv_mul_flg = hook_system_mock.ExitWindowsEx(EWX_LOGOFF | EWX_SHUTDOWN | 0x80000000, SHTDN_REASON_FLAG_PLANNED);
    error_inv_mul_flg = GetLastError();
    EXPECT_EQ(bexit_inv_mul_flg, TRUE);
    EXPECT_EQ(error_inv_mul_flg, ERROR_SUCCESS);
    // wait for all logs received.
    std::vector<int> count_list;
    count_list.push_back(4);
    EXPECT_EQ(g_server_object->WaitLogCountMap(count_list, 5), TRUE);
}

TEST_F(HookSystemTest, ExitWindowsEx_Prelog_dwReason_Test)
{
    BOOL bexit_0_flgs, bexit_invalid_flgs, bexit_sin_flgs, bexit_mul_flgs, bexit_inv_mul_flg;
    DWORD berror_0_flgs, error_invalid_flgs, error_sin_flgs, error_mul_flgs, error_inv_mul_flg;
    ApiHookSystemMock hook_system_mock;
    //initialize
    std::vector<std::string> action_list;
    action_list.push_back("P41");
    g_server_object->AddLogCountMap(action_list);
    // test when dwReason is 0 flag
    SetLastError(0); EXPECT_CALL(hook_system_mock, ExitWindowsEx(testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::ExitWindowsEx(EWX_LOGOFF | EWX_POWEROFF | EWX_FORCEIFHUNG, 0)));
    bexit_0_flgs = hook_system_mock.ExitWindowsEx(EWX_LOGOFF | EWX_POWEROFF | EWX_FORCEIFHUNG, 0);
    berror_0_flgs = GetLastError();
    EXPECT_EQ(bexit_0_flgs, TRUE);
    EXPECT_EQ(berror_0_flgs, ERROR_SUCCESS);
    // test when dwReason is invalid flag
    SetLastError(0); EXPECT_CALL(hook_system_mock, ExitWindowsEx(testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::ExitWindowsEx(EWX_LOGOFF | EWX_POWEROFF | EWX_FORCEIFHUNG, 0x87654321)));
    bexit_invalid_flgs = hook_system_mock.ExitWindowsEx(EWX_LOGOFF | EWX_POWEROFF | EWX_FORCEIFHUNG, 0x87654321);
    error_invalid_flgs = GetLastError();
    EXPECT_EQ(bexit_invalid_flgs, TRUE);
    EXPECT_EQ(error_invalid_flgs, ERROR_SUCCESS);
    // test when dwReason is single flag
    SetLastError(0); EXPECT_CALL(hook_system_mock, ExitWindowsEx(testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::ExitWindowsEx(EWX_LOGOFF | EWX_POWEROFF | EWX_FORCEIFHUNG, SHTDN_REASON_MAJOR_POWER)));
    bexit_sin_flgs = hook_system_mock.ExitWindowsEx(EWX_LOGOFF | EWX_POWEROFF | EWX_FORCEIFHUNG, SHTDN_REASON_MAJOR_POWER);
    error_sin_flgs = GetLastError();
    EXPECT_EQ(bexit_sin_flgs, TRUE);
    EXPECT_EQ(error_sin_flgs, ERROR_SUCCESS);
    // test when dwReason is multiple flag
    SetLastError(0); EXPECT_CALL(hook_system_mock, ExitWindowsEx(testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::ExitWindowsEx(EWX_LOGOFF | EWX_POWEROFF | EWX_FORCEIFHUNG, SHTDN_REASON_MAJOR_POWER | SHTDN_REASON_MINOR_DISK)));
    bexit_mul_flgs = hook_system_mock.ExitWindowsEx(EWX_LOGOFF | EWX_POWEROFF | EWX_FORCEIFHUNG, SHTDN_REASON_MAJOR_POWER | SHTDN_REASON_MINOR_DISK);
    error_mul_flgs = GetLastError();
    EXPECT_EQ(bexit_mul_flgs, TRUE);
    EXPECT_EQ(error_mul_flgs, ERROR_SUCCESS);
    // test when dwReason is multiple invalid flag
    SetLastError(0); EXPECT_CALL(hook_system_mock, ExitWindowsEx(testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::ExitWindowsEx(EWX_LOGOFF | EWX_POWEROFF | EWX_FORCEIFHUNG, SHTDN_REASON_MAJOR_SYSTEM | SHTDN_REASON_MINOR_SECURITY | 0x3928)));
    bexit_inv_mul_flg = hook_system_mock.ExitWindowsEx(EWX_LOGOFF | EWX_POWEROFF | EWX_FORCEIFHUNG, SHTDN_REASON_MAJOR_SYSTEM | SHTDN_REASON_MINOR_SECURITY | 0x3928);
    error_inv_mul_flg = GetLastError();
    EXPECT_EQ(bexit_inv_mul_flg, TRUE);
    EXPECT_EQ(error_inv_mul_flg, ERROR_SUCCESS);
    // wait for all logs received.
    std::vector<int> count_list;
    count_list.push_back(5);
    EXPECT_EQ(g_server_object->WaitLogCountMap(count_list, 5), TRUE);
}

TEST_F(HookSystemTest, GetDiskFreeSpaceExW_PostCheck_lpDirectoryName_Test)
{
    BOOL pre_bexit_invalid_str, pre_bexit_null_str, pre_bexit_short_str, pre_bexit_long_str, pre_bexit_long_data;
    DWORD pre_error_invalid_str, pre_berror_null_str, pre_error_short_str, pre_error_long_str, pre_error_long_data;
    BOOL hooked_bexit_invalid_str, hooked_bexit_null_str, hooked_bexit_short_str, hooked_bexit_long_str, hooked_bexit_long_data;
    DWORD hooked_error_invalid_str, hooked_berror_null_str, hooked_error_short_str, hooked_error_long_str, hooked_error_long_data;
    ApiHookSystemMock hook_system_mock;
    // test variable
    ULARGE_INTEGER free_bytes, total_bytes, total_free_bytes;
    CStringCommonTestI str_testI_object(true);
    // first call test API when DisableAllApis().
    ASSERT_TRUE(g_hook_test_object->DisableAllApis());
    // test when lpDirectoryName is nullptr
    SetLastError(0); EXPECT_CALL(hook_system_mock, GetDiskFreeSpaceExW(testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::GetDiskFreeSpaceExW(nullptr, &free_bytes, &total_bytes, &total_free_bytes)));
    pre_bexit_invalid_str = hook_system_mock.GetDiskFreeSpaceExW(nullptr, &free_bytes, &total_bytes, &total_free_bytes);
    pre_error_invalid_str = GetLastError();
    // test when lpDirectoryName is "\0"
    SetLastError(0); EXPECT_CALL(hook_system_mock, GetDiskFreeSpaceExW(testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::GetDiskFreeSpaceExW(std::get<const std::wstring>(str_testI_object.NullString(1)).c_str(), &free_bytes, &total_bytes, &total_free_bytes)));
    pre_bexit_null_str = hook_system_mock.GetDiskFreeSpaceExW(nullptr, &free_bytes, &total_bytes, &total_free_bytes);
    pre_berror_null_str = GetLastError();
    // test when lpDirectoryName is normal dll name "C:\\windows\\notepad.exe"
    SetLastError(0); EXPECT_CALL(hook_system_mock, GetDiskFreeSpaceExW(testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::GetDiskFreeSpaceExW(std::get<const std::wstring>(str_testI_object.NormalString(std::wstring(L"C:\\windows"))).c_str(), &free_bytes, &total_bytes, &total_free_bytes)));
    pre_bexit_short_str = hook_system_mock.GetDiskFreeSpaceExW(nullptr, &free_bytes, &total_bytes, &total_free_bytes);
    pre_error_short_str = GetLastError();
    // test when lpDirectoryName is invalid dll name and super long name.
    SetLastError(0); EXPECT_CALL(hook_system_mock, GetDiskFreeSpaceExW(testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::GetDiskFreeSpaceExW(std::get<const std::wstring>(str_testI_object.SuperLongString(std::wstring(L"C:\\windows"))).c_str(), &free_bytes, &total_bytes, &total_free_bytes)));
    pre_bexit_long_str = hook_system_mock.GetDiskFreeSpaceExW(nullptr, &free_bytes, &total_bytes, &total_free_bytes);
    pre_error_long_str = GetLastError();
    // test when lpDirectoryName is incorrect data block.
    SetLastError(0); EXPECT_CALL(hook_system_mock, GetDiskFreeSpaceExW(testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::GetDiskFreeSpaceExW((LPCWSTR)&str_testI_object.SuperLongData()[0], &free_bytes, &total_bytes, &total_free_bytes)));
    pre_bexit_long_data = hook_system_mock.GetDiskFreeSpaceExW(nullptr, &free_bytes, &total_bytes, &total_free_bytes);
    pre_error_long_data = GetLastError();

    // initialize
    std::vector<std::string> action_list;
    action_list.push_back("H11");
    g_server_object->AddLogCountMap(action_list);
    // second call test API when EnableAllApis().
    ASSERT_TRUE(g_hook_test_object->EnableAllApis());
    // test when lpDirectoryName is nullptr
    SetLastError(0); EXPECT_CALL(hook_system_mock, GetDiskFreeSpaceExW(testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::GetDiskFreeSpaceExW(nullptr, &free_bytes, &total_bytes, &total_free_bytes)));
    hooked_bexit_invalid_str = hook_system_mock.GetDiskFreeSpaceExW(nullptr, &free_bytes, &total_bytes, &total_free_bytes);
    hooked_error_invalid_str = GetLastError();
    // test when lpDirectoryName is "\0"
    SetLastError(0); EXPECT_CALL(hook_system_mock, GetDiskFreeSpaceExW(testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::GetDiskFreeSpaceExW(std::get<const std::wstring>(str_testI_object.NullString(1)).c_str(), &free_bytes, &total_bytes, &total_free_bytes)));
    hooked_bexit_null_str = hook_system_mock.GetDiskFreeSpaceExW(nullptr, &free_bytes, &total_bytes, &total_free_bytes);
    hooked_berror_null_str = GetLastError();
    // test when lpDirectoryName is normal dll name "C:\\windows"
    SetLastError(0); EXPECT_CALL(hook_system_mock, GetDiskFreeSpaceExW(testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::GetDiskFreeSpaceExW(std::get<const std::wstring>(str_testI_object.NormalString(std::wstring(L"C:\\windows"))).c_str(), &free_bytes, &total_bytes, &total_free_bytes)));
    hooked_bexit_short_str = hook_system_mock.GetDiskFreeSpaceExW(nullptr, &free_bytes, &total_bytes, &total_free_bytes);
    hooked_error_short_str = GetLastError();
    // test when lpDirectoryName is invalid dll name and super long name.
    SetLastError(0); EXPECT_CALL(hook_system_mock, GetDiskFreeSpaceExW(testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::GetDiskFreeSpaceExW(std::get<const std::wstring>(str_testI_object.SuperLongString(std::wstring(L"C:\\windows"))).c_str(), &free_bytes, &total_bytes, &total_free_bytes)));
    hooked_bexit_long_str = hook_system_mock.GetDiskFreeSpaceExW(nullptr, &free_bytes, &total_bytes, &total_free_bytes);
    hooked_error_long_str = GetLastError();
    // test when lpDirectoryName is incorrect data block.
    SetLastError(0); EXPECT_CALL(hook_system_mock, GetDiskFreeSpaceExW(testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::GetDiskFreeSpaceExW((LPCWSTR)&str_testI_object.SuperLongData()[0], &free_bytes, &total_bytes, &total_free_bytes)));
    hooked_bexit_long_data = hook_system_mock.GetDiskFreeSpaceExW(nullptr, &free_bytes, &total_bytes, &total_free_bytes);
    hooked_error_long_data = GetLastError();
    // wait for all logs received.
    std::vector<int> count_list;
    count_list.push_back(2); // because only 2 times success.
    EXPECT_EQ(g_server_object->WaitLogCountMap(count_list, 5), TRUE);

    // compare return result and error code.
    EXPECT_EQ(pre_bexit_invalid_str, hooked_bexit_invalid_str);
    EXPECT_EQ(pre_error_invalid_str, hooked_error_invalid_str);
    EXPECT_EQ(pre_bexit_null_str, hooked_bexit_null_str);
    EXPECT_EQ(pre_berror_null_str, hooked_berror_null_str);
    EXPECT_EQ(pre_bexit_short_str, hooked_bexit_short_str);
    EXPECT_EQ(pre_error_short_str, hooked_error_short_str);
    EXPECT_EQ(pre_bexit_long_str, hooked_bexit_long_str);
    EXPECT_EQ(pre_error_long_str, hooked_error_long_str);
    EXPECT_EQ(pre_bexit_long_data, hooked_bexit_long_data);
    EXPECT_EQ(pre_error_long_data, hooked_error_long_data);
}

TEST_F(HookSystemTest, GetDiskFreeSpaceExW_PostCheck_lpTotalNumberOfBytes_Test)
{
    BOOL pre_bexit_null_addr, /*pre_bexit_invalid_addr,*/ pre_bexit_normal_addr;
    DWORD pre_berror_null_addr, /*pre_error_invalid_addr,*/ pre_error_normal_addr;
    BOOL hooked_bexit_null_addr, /*hooked_bexit_invalid_addr,*/ hooked_bexit_normal_addr;
    DWORD hooked_berror_null_addr, /*hooked_error_invalid_addr,*/ hooked_error_normal_addr;
    ApiHookSystemMock hook_system_mock;
    // test variable
    ULARGE_INTEGER pre_free_bytes_1, pre_total_free_bytes_1;
    //ULARGE_INTEGER pre_free_bytes_2, pre_total_free_bytes_2;
    ULARGE_INTEGER pre_free_bytes_3, pre_total_bytes_3, pre_total_free_bytes_3;
    ULARGE_INTEGER hooked_free_bytes_1, hooked_total_free_bytes_1;
    //ULARGE_INTEGER hooked_free_bytes_2, hooked_total_free_bytes_2;
    ULARGE_INTEGER hooked_free_bytes_3, hooked_total_bytes_3, hooked_total_free_bytes_3;
    // first call test API when DisableAllApis().
    ASSERT_TRUE(g_hook_test_object->DisableAllApis());
    // test when lpTotalNumberOfBytes is nullptr
    SetLastError(0); EXPECT_CALL(hook_system_mock, GetDiskFreeSpaceExW(testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::GetDiskFreeSpaceExW(L"C:\\windows", &pre_free_bytes_1, nullptr, &pre_total_free_bytes_1)));
    pre_bexit_null_addr = hook_system_mock.GetDiskFreeSpaceExW(L"C:\\windows", &pre_free_bytes_1, nullptr, &pre_total_free_bytes_1);
    pre_berror_null_addr = GetLastError();
    // test when lpTotalNumberOfBytes is invalid addr
    //SetLastError(0); EXPECT_CALL(hook_system_mock, GetDiskFreeSpaceExW(testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::GetDiskFreeSpaceExW(L"C:\\windows", &pre_free_bytes_2, (ULARGE_INTEGER*)0x10000, &pre_total_free_bytes_2)));
    //pre_bexit_invalid_addr = hook_system_mock.GetDiskFreeSpaceExW(L"C:\\windows", &pre_free_bytes_2, (ULARGE_INTEGER*)0x1000, &pre_total_free_bytes_2);
    //pre_error_invalid_addr = GetLastError();
    // test when lpTotalNumberOfBytes is normal addr
    SetLastError(0); EXPECT_CALL(hook_system_mock, GetDiskFreeSpaceExW(testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::GetDiskFreeSpaceExW(L"C:\\windows", &pre_free_bytes_3, &pre_total_bytes_3, &pre_total_free_bytes_3)));
    pre_bexit_normal_addr = hook_system_mock.GetDiskFreeSpaceExW(L"C:\\windows", &pre_free_bytes_3, &pre_total_bytes_3, &pre_total_free_bytes_3);
    pre_error_normal_addr = GetLastError();

    // initialize
    std::vector<std::string> action_list;
    action_list.push_back("H11");
    g_server_object->AddLogCountMap(action_list);
    // second call test API when EnableAllApis().
    ASSERT_TRUE(g_hook_test_object->EnableAllApis());
    // test when lpTotalNumberOfBytes is nullptr
    SetLastError(0); EXPECT_CALL(hook_system_mock, GetDiskFreeSpaceExW(testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::GetDiskFreeSpaceExW(L"C:\\windows", &hooked_free_bytes_1, nullptr, &hooked_total_free_bytes_1)));
    hooked_bexit_null_addr = hook_system_mock.GetDiskFreeSpaceExW(L"C:\\windows", &hooked_free_bytes_1, nullptr, &hooked_total_free_bytes_1);
    hooked_berror_null_addr = GetLastError();
    // test when lpTotalNumberOfBytes is invalid addr
    //SetLastError(0); EXPECT_CALL(hook_system_mock, GetDiskFreeSpaceExW(testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::GetDiskFreeSpaceExW(L"C:\\windows", &hooked_free_bytes_2, (ULARGE_INTEGER*)0x10000, &hooked_total_free_bytes_2)));
    //hooked_bexit_invalid_addr = hook_system_mock.GetDiskFreeSpaceExW(L"C:\\windows", &hooked_free_bytes_2, (ULARGE_INTEGER*)0x1000, &hooked_total_free_bytes_2);
    //hooked_error_invalid_addr = GetLastError();
    // test when lpTotalNumberOfBytes is normal addr
    SetLastError(0); EXPECT_CALL(hook_system_mock, GetDiskFreeSpaceExW(testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::GetDiskFreeSpaceExW(L"C:\\windows", &hooked_free_bytes_3, &hooked_total_bytes_3, &hooked_total_free_bytes_3)));
    hooked_bexit_normal_addr = hook_system_mock.GetDiskFreeSpaceExW(L"C:\\windows", &hooked_free_bytes_3, &hooked_total_bytes_3, &hooked_total_free_bytes_3);
    hooked_error_normal_addr = GetLastError();
    // wait for all logs received.
    std::vector<int> count_list;
    count_list.push_back(1);
    EXPECT_EQ(g_server_object->WaitLogCountMap(count_list, 5), TRUE);

    // compare return result and error code.
    EXPECT_EQ(pre_bexit_null_addr, hooked_bexit_null_addr);
    EXPECT_EQ(pre_berror_null_addr, hooked_berror_null_addr);
    EXPECT_EQ(pre_free_bytes_1.QuadPart, hooked_free_bytes_1.QuadPart);
    EXPECT_EQ(pre_total_free_bytes_1.QuadPart, hooked_total_free_bytes_1.QuadPart);
    //EXPECT_EQ(pre_bexit_invalid_addr, hooked_bexit_invalid_addr);
    //EXPECT_EQ(pre_error_invalid_addr, hooked_error_invalid_addr);
    //EXPECT_EQ(pre_free_bytes_2.QuadPart, hooked_free_bytes_2.QuadPart);
    //EXPECT_EQ(pre_total_free_bytes_2.QuadPart, hooked_total_free_bytes_2.QuadPart);
    EXPECT_EQ(pre_bexit_normal_addr, hooked_bexit_normal_addr);
    EXPECT_EQ(pre_error_normal_addr, hooked_error_normal_addr);
    EXPECT_EQ(pre_free_bytes_3.QuadPart, hooked_free_bytes_3.QuadPart);
    EXPECT_EQ(pre_total_free_bytes_3.QuadPart, hooked_total_free_bytes_3.QuadPart);
    if (hooked_bexit_normal_addr == TRUE) {
        if (pre_total_bytes_3.QuadPart < ((ULONGLONG)64 * 1024 * 1024 * 1024))
            EXPECT_EQ(hooked_total_bytes_3.QuadPart, ((ULONGLONG)1000 * 1024 * 1024 * 1024));
        else
            EXPECT_EQ(pre_total_bytes_3.QuadPart, hooked_total_bytes_3.QuadPart);
    }
    else
        EXPECT_EQ(pre_total_bytes_3.QuadPart, hooked_total_bytes_3.QuadPart);
}

TEST_F(HookSystemTest, GetTickCount_Modify_Return_Test)
{
    DWORD pre_tickcount;
    DWORD pre_berror_tickcount;
    DWORD hooked_tickcount;
    DWORD hooked_berror_tickcount;
    ApiHookSystemMock hook_system_mock;

    // first call test API when DisableAllApis().
    ASSERT_TRUE(g_hook_test_object->DisableAllApis());
    SetLastError(0); EXPECT_CALL(hook_system_mock, GetTickCount()).Times(1).WillRepeatedly(testing::Return(::GetTickCount()));
    pre_tickcount = hook_system_mock.GetTickCount();
    pre_berror_tickcount = GetLastError();

    // initialize
    std::vector<std::string> action_list;
    action_list.push_back("H12");
    g_server_object->AddLogCountMap(action_list);
    // second call test API when EnableAllApis().
    ASSERT_TRUE(g_hook_test_object->EnableAllApis());
    SetLastError(0); EXPECT_CALL(hook_system_mock, GetTickCount()).Times(1).WillRepeatedly(testing::Return(::GetTickCount()));
    hooked_tickcount = hook_system_mock.GetTickCount();
    hooked_berror_tickcount = GetLastError();
    // wait for all logs received.
    std::vector<int> count_list;
    count_list.push_back(1);
    EXPECT_EQ(g_server_object->WaitLogCountMap(count_list, 5), TRUE);

    EXPECT_NE(pre_tickcount, 0);
    EXPECT_EQ(hooked_tickcount, 1234567);
    EXPECT_EQ(pre_berror_tickcount, hooked_berror_tickcount);
}

TEST_F(HookSystemTest, GetSystemTime_Modify_lpSystemTime_Test)
{
    DWORD /*pre_berror_null_time, */pre_error_normal_time;
    DWORD /*hooked_error_null_time, */hooked_error_normal_time;
    ApiHookSystemMock hook_system_mock;

    // test variable
    SYSTEMTIME pre_lpSystemTime = {};
    SYSTEMTIME hooked_lpSystemTime = {};
    // first call test API when DisableAllApis().
    ASSERT_TRUE(g_hook_test_object->DisableAllApis());
    //SetLastError(0);
    //GetSystemTime(nullptr);
    //pre_berror_null_time = GetLastError();
    SetLastError(0);
    GetSystemTime(&pre_lpSystemTime);
    pre_error_normal_time = GetLastError();

    // initialize
    std::vector<std::string> action_list;
    action_list.push_back("H13");
    g_server_object->AddLogCountMap(action_list);
    // second call test API when EnableAllApis().
    ASSERT_TRUE(g_hook_test_object->EnableAllApis());
    //SetLastError(0);
    //GetSystemTime(nullptr);
    //hooked_error_null_time = GetLastError();
    SetLastError(0);
    GetSystemTime(&hooked_lpSystemTime);
    hooked_error_normal_time = GetLastError();
    // wait for all logs received.
    std::vector<int> count_list;
    count_list.push_back(1);
    EXPECT_EQ(g_server_object->WaitLogCountMap(count_list, 5), TRUE);

    EXPECT_EQ(pre_error_normal_time, 0);
    EXPECT_EQ(hooked_error_normal_time, 0);
    EXPECT_EQ(hooked_lpSystemTime.wYear, 2010);
    EXPECT_EQ(hooked_lpSystemTime.wMonth, 12);
    EXPECT_EQ(hooked_lpSystemTime.wDay, 31);
}

TEST_F(HookSystemTest, GetModuleHandleW_PreLog_lpModuleName_Test)
{
    HMODULE pre_bexit_invalid_str, pre_bexit_null_str, pre_bexit_short_str, pre_bexit_long_str, pre_bexit_long_data;
    DWORD pre_error_invalid_str, pre_berror_null_str, pre_error_short_str, pre_error_long_str, pre_error_long_data;
    HMODULE hooked_bexit_invalid_str, hooked_bexit_null_str, hooked_bexit_short_str, hooked_bexit_long_str, hooked_bexit_long_data;
    DWORD hooked_error_invalid_str, hooked_berror_null_str, hooked_error_short_str, hooked_error_long_str, hooked_error_long_data;
    ApiHookSystemMock hook_system_mock;
    // test variable
    CStringCommonTestI str_testI_object(true);
    // first call test API when DisableAllApis().
    ASSERT_TRUE(g_hook_test_object->DisableAllApis());
    // test when lpModuleName is nullptr
    SetLastError(0); EXPECT_CALL(hook_system_mock, GetModuleHandleW(testing::_)).Times(1).WillRepeatedly(testing::Return(::GetModuleHandleW(nullptr)));
    pre_bexit_invalid_str = hook_system_mock.GetModuleHandleW(nullptr);
    pre_error_invalid_str = GetLastError();
    // test when lpModuleName is "\0"
    SetLastError(0); EXPECT_CALL(hook_system_mock, GetModuleHandleW(testing::_)).Times(1).WillRepeatedly(testing::Return(::GetModuleHandleW(std::get<const std::wstring>(str_testI_object.NullString(1)).c_str())));
    pre_bexit_null_str = hook_system_mock.GetModuleHandleW(nullptr);
    pre_berror_null_str = GetLastError();
    // test when lpModuleName is normal dll name "kernel32.dll"
    SetLastError(0); EXPECT_CALL(hook_system_mock, GetModuleHandleW(testing::_)).Times(1).WillRepeatedly(testing::Return(::GetModuleHandleW(std::get<const std::wstring>(str_testI_object.NormalString(std::wstring(L"kernel32.dll"))).c_str())));
    pre_bexit_short_str = hook_system_mock.GetModuleHandleW(nullptr);
    pre_error_short_str = GetLastError();
    // test when lpModuleName is invalid dll name and super long name.
    SetLastError(0); EXPECT_CALL(hook_system_mock, GetModuleHandleW(testing::_)).Times(1).WillRepeatedly(testing::Return(::GetModuleHandleW(std::get<const std::wstring>(str_testI_object.SuperLongString(std::wstring(L"kernel32.dll"))).c_str())));
    pre_bexit_long_str = hook_system_mock.GetModuleHandleW(nullptr);
    pre_error_long_str = GetLastError();
    // test when lpModuleName is incorrect data block.
    SetLastError(0); EXPECT_CALL(hook_system_mock, GetModuleHandleW(testing::_)).Times(1).WillRepeatedly(testing::Return(::GetModuleHandleW((LPCWSTR)&str_testI_object.SuperLongData()[0])));
    pre_bexit_long_data = hook_system_mock.GetModuleHandleW(nullptr);
    pre_error_long_data = GetLastError();

    // initialize
    std::vector<std::string> action_list;
    action_list.push_back("P60");
    g_server_object->AddLogCountMap(action_list);
    // second call test API when EnableAllApis().
    ASSERT_TRUE(g_hook_test_object->EnableAllApis());
    // test when lpModuleName is nullptr
    SetLastError(0); EXPECT_CALL(hook_system_mock, GetModuleHandleW(testing::_)).Times(1).WillRepeatedly(testing::Return(::GetModuleHandleW(nullptr)));
    hooked_bexit_invalid_str = hook_system_mock.GetModuleHandleW(nullptr);
    hooked_error_invalid_str = GetLastError();
    // test when lpModuleName is "\0"
    SetLastError(0); EXPECT_CALL(hook_system_mock, GetModuleHandleW(testing::_)).Times(1).WillRepeatedly(testing::Return(::GetModuleHandleW(std::get<const std::wstring>(str_testI_object.NullString(1)).c_str())));
    hooked_bexit_null_str = hook_system_mock.GetModuleHandleW(nullptr);
    hooked_berror_null_str = GetLastError();
    // test when lpModuleName is normal dll name "kernel32.dll"
    SetLastError(0); EXPECT_CALL(hook_system_mock, GetModuleHandleW(testing::_)).Times(1).WillRepeatedly(testing::Return(::GetModuleHandleW(std::get<const std::wstring>(str_testI_object.NormalString(std::wstring(L"kernel32.dll"))).c_str())));
    hooked_bexit_short_str = hook_system_mock.GetModuleHandleW(nullptr);
    hooked_error_short_str = GetLastError();
    // test when lpModuleName is invalid dll name and super long name.
    SetLastError(0); EXPECT_CALL(hook_system_mock, GetModuleHandleW(testing::_)).Times(1).WillRepeatedly(testing::Return(::GetModuleHandleW(std::get<const std::wstring>(str_testI_object.SuperLongString(std::wstring(L"kernel32.dll"))).c_str())));
    hooked_bexit_long_str = hook_system_mock.GetModuleHandleW(nullptr);
    hooked_error_long_str = GetLastError();
    // test when lpModuleName is incorrect data block.
    SetLastError(0); EXPECT_CALL(hook_system_mock, GetModuleHandleW(testing::_)).Times(1).WillRepeatedly(testing::Return(::GetModuleHandleW((LPCWSTR)&str_testI_object.SuperLongData()[0])));
    hooked_bexit_long_data = hook_system_mock.GetModuleHandleW(nullptr);
    hooked_error_long_data = GetLastError();
    // wait for all logs received.
    std::vector<int> count_list;
    count_list.push_back(4);
    EXPECT_EQ(g_server_object->WaitLogCountMap(count_list, 5), TRUE);

    // compare return result and error code.
    EXPECT_EQ(pre_bexit_invalid_str, hooked_bexit_invalid_str);
    EXPECT_EQ(pre_error_invalid_str, hooked_error_invalid_str);
    EXPECT_EQ(pre_bexit_null_str, hooked_bexit_null_str);
    EXPECT_EQ(pre_berror_null_str, hooked_berror_null_str);
    EXPECT_EQ(pre_bexit_short_str, hooked_bexit_short_str);
    EXPECT_EQ(pre_error_short_str, hooked_error_short_str);
    EXPECT_EQ(pre_bexit_long_str, hooked_bexit_long_str);
    EXPECT_EQ(pre_error_long_str, hooked_error_long_str);
    EXPECT_EQ(pre_bexit_long_data, hooked_bexit_long_data);
    EXPECT_EQ(pre_error_long_data, hooked_error_long_data);
}

TEST_F(HookSystemTest, GetProcAddress_PreLog_hModule_Test)
{
    FARPROC pre_bexit_invalid_proc, pre_bexit_null_proc, pre_bexit_normal_proc;
    DWORD pre_error_invalid_proc, pre_berror_null_proc, pre_error_normal_proc;
    FARPROC hooked_bexit_invalid_proc, hooked_bexit_null_proc, hooked_bexit_normal_proc;
    DWORD hooked_error_invalid_proc, hooked_berror_null_proc, hooked_error_normal_proc;
    ApiHookSystemMock hook_system_mock;

    // first call test API when DisableAllApis().
    ASSERT_TRUE(g_hook_test_object->DisableAllApis());
    // test when hModule is nullptr
    SetLastError(0); EXPECT_CALL(hook_system_mock, GetProcAddress(testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::GetProcAddress(nullptr, "CopyFileExW")));
    pre_bexit_null_proc = hook_system_mock.GetProcAddress(nullptr, "CopyFileExW");
    pre_berror_null_proc = GetLastError();
    // test when hModule is invalid
    SetLastError(0); EXPECT_CALL(hook_system_mock, GetProcAddress(testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::GetProcAddress((HMODULE)0x10000, "CopyFileExW")));
    pre_bexit_invalid_proc = hook_system_mock.GetProcAddress((HMODULE)0x10000, "CopyFileExW");
    pre_error_invalid_proc = GetLastError();
    // test when hModule is normal
    SetLastError(0); EXPECT_CALL(hook_system_mock, GetProcAddress(testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::GetProcAddress(GetModuleHandle("kernel32"), "CopyFileExW")));
    pre_bexit_normal_proc = hook_system_mock.GetProcAddress(nullptr, "CopyFileExW");
    pre_error_normal_proc = GetLastError();

    // initialize
    std::vector<std::string> action_list;
    action_list.push_back("P61");
    g_server_object->AddLogCountMap(action_list);
    // second call test API when EnableAllApis().
    ASSERT_TRUE(g_hook_test_object->EnableAllApis());
    // test when hModule is nullptr
    SetLastError(0); EXPECT_CALL(hook_system_mock, GetProcAddress(testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::GetProcAddress(nullptr, "CopyFileExW")));
    hooked_bexit_null_proc = hook_system_mock.GetProcAddress(nullptr, "CopyFileExW");
    hooked_berror_null_proc = GetLastError();
    // test when hModule is invalid
    SetLastError(0); EXPECT_CALL(hook_system_mock, GetProcAddress(testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::GetProcAddress((HMODULE)0x10000, "CopyFileExW")));
    hooked_bexit_invalid_proc = hook_system_mock.GetProcAddress((HMODULE)0x10000, "CopyFileExW");
    hooked_error_invalid_proc = GetLastError();
    // test when hModule is normal
    SetLastError(0); EXPECT_CALL(hook_system_mock, GetProcAddress(testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::GetProcAddress(GetModuleHandle("kernel32"), "CopyFileExW")));
    hooked_bexit_normal_proc = hook_system_mock.GetProcAddress(nullptr, "CopyFileExW");
    hooked_error_normal_proc = GetLastError();
    // wait for all logs received.
    std::vector<int> count_list;
    count_list.push_back(1);
    EXPECT_EQ(g_server_object->WaitLogCountMap(count_list, 5), TRUE);

    // compare return result and error code.
    EXPECT_EQ(pre_bexit_null_proc, hooked_bexit_null_proc);
    EXPECT_EQ(pre_berror_null_proc, hooked_berror_null_proc);
    EXPECT_EQ(pre_bexit_invalid_proc, hooked_bexit_invalid_proc);
    EXPECT_EQ(pre_error_invalid_proc, hooked_error_invalid_proc);
    EXPECT_EQ(pre_bexit_normal_proc, hooked_bexit_normal_proc);
    EXPECT_EQ(pre_error_normal_proc, hooked_error_normal_proc);
}

TEST_F(HookSystemTest, GetProcAddress_PreLog_lpProcName_Test)
{
    FARPROC pre_bexit_invalid_str, pre_bexit_null_str, pre_bexit_short_str, pre_bexit_long_str, pre_bexit_long_data, pre_bexit_ordinal;
    DWORD pre_error_invalid_str, pre_berror_null_str, pre_error_short_str, pre_error_long_str, pre_error_long_data, pre_error_ordinal;
    FARPROC hooked_bexit_invalid_str, hooked_bexit_null_str, hooked_bexit_short_str, hooked_bexit_long_str, hooked_bexit_long_data, hooked_bexit_ordinal;
    DWORD hooked_error_invalid_str, hooked_berror_null_str, hooked_error_short_str, hooked_error_long_str, hooked_error_long_data, hooked_error_ordinal;
    ApiHookSystemMock hook_system_mock;

    // test variable
    CStringCommonTestI str_testI_object;
    // first call test API when DisableAllApis().
    ASSERT_TRUE(g_hook_test_object->DisableAllApis());
    // test when lpProcName is nullptr
    SetLastError(0); EXPECT_CALL(hook_system_mock, GetProcAddress(testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::GetProcAddress(GetModuleHandle("kernel32"), nullptr)));
    pre_bexit_invalid_str = hook_system_mock.GetProcAddress(nullptr, nullptr);
    pre_error_invalid_str = GetLastError();
    // test when lpProcName is invalid
    SetLastError(0); EXPECT_CALL(hook_system_mock, GetProcAddress(testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::GetProcAddress(GetModuleHandle("kernel32"), std::get<const std::string>(str_testI_object.NullString(1)).c_str())));
    pre_bexit_null_str = hook_system_mock.GetProcAddress(nullptr, nullptr);
    pre_berror_null_str = GetLastError();
    // test when lpProcName is normal
    SetLastError(0); EXPECT_CALL(hook_system_mock, GetProcAddress(testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::GetProcAddress(GetModuleHandle("kernel32"), std::get<const std::string>(str_testI_object.NormalString("CopyFileExW")).c_str())));
    pre_bexit_short_str = hook_system_mock.GetProcAddress(nullptr, nullptr);
    pre_error_short_str = GetLastError();
    // test when lpProcName is invalid name and super long name.
    SetLastError(0); EXPECT_CALL(hook_system_mock, GetProcAddress(testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::GetProcAddress(GetModuleHandle("kernel32"), std::get<const std::string>(str_testI_object.SuperLongString("CopyFileExW")).c_str())));
    pre_bexit_long_str = hook_system_mock.GetProcAddress(nullptr, nullptr);
    pre_error_long_str = GetLastError();
    // test when lpProcName is incorrect data block.
    SetLastError(0); EXPECT_CALL(hook_system_mock, GetProcAddress(testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::GetProcAddress(GetModuleHandle("kernel32"), (LPCSTR)&str_testI_object.SuperLongData()[0])));
    pre_bexit_long_data = hook_system_mock.GetProcAddress(nullptr, nullptr);
    pre_error_long_data = GetLastError();
    // test when lpProcName is ordinal.
    SetLastError(0); EXPECT_CALL(hook_system_mock, GetProcAddress(testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::GetProcAddress(GetModuleHandle("kernel32"), (LPCSTR)500)));
    pre_bexit_ordinal = hook_system_mock.GetProcAddress(nullptr, nullptr);
    pre_error_ordinal = GetLastError();

    // initialize
    std::vector<std::string> action_list;
    action_list.push_back("P61");
    g_server_object->AddLogCountMap(action_list);
    // second call test API when EnableAllApis().
    ASSERT_TRUE(g_hook_test_object->EnableAllApis());
    // test when lpProcName is nullptr
    SetLastError(0); EXPECT_CALL(hook_system_mock, GetProcAddress(testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::GetProcAddress(GetModuleHandle("kernel32"), nullptr)));
    hooked_bexit_invalid_str = hook_system_mock.GetProcAddress(nullptr, nullptr);
    hooked_error_invalid_str = GetLastError();
    // test when lpProcName is invalid
    SetLastError(0); EXPECT_CALL(hook_system_mock, GetProcAddress(testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::GetProcAddress(GetModuleHandle("kernel32"), std::get<const std::string>(str_testI_object.NullString(1)).c_str())));
    hooked_bexit_null_str = hook_system_mock.GetProcAddress(nullptr, nullptr);
    hooked_berror_null_str = GetLastError();
    // test when lpProcName is normal
    SetLastError(0); EXPECT_CALL(hook_system_mock, GetProcAddress(testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::GetProcAddress(GetModuleHandle("kernel32"), std::get<const std::string>(str_testI_object.NormalString("CopyFileExW")).c_str())));
    hooked_bexit_short_str = hook_system_mock.GetProcAddress(nullptr, nullptr);
    hooked_error_short_str = GetLastError();
    // test when lpProcName is invalid name and super long name.
    SetLastError(0); EXPECT_CALL(hook_system_mock, GetProcAddress(testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::GetProcAddress(GetModuleHandle("kernel32"), std::get<const std::string>(str_testI_object.SuperLongString("CopyFileExW")).c_str())));
    hooked_bexit_long_str = hook_system_mock.GetProcAddress(nullptr, nullptr);
    hooked_error_long_str = GetLastError();
    // test when lpProcName is incorrect data block.
    SetLastError(0); EXPECT_CALL(hook_system_mock, GetProcAddress(testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::GetProcAddress(GetModuleHandle("kernel32"), (LPCSTR)&str_testI_object.SuperLongData()[0])));
    hooked_bexit_long_data = hook_system_mock.GetProcAddress(nullptr, nullptr);
    hooked_error_long_data = GetLastError();
    // test when lpProcName is ordinal.
    SetLastError(0); EXPECT_CALL(hook_system_mock, GetProcAddress(testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::GetProcAddress(GetModuleHandle("kernel32"), (LPCSTR)500)));
    hooked_bexit_ordinal = hook_system_mock.GetProcAddress(nullptr, nullptr);
    hooked_error_ordinal = GetLastError();
    // wait for all logs received.
    std::vector<int> count_list;
    count_list.push_back(2);
    EXPECT_EQ(g_server_object->WaitLogCountMap(count_list, 5), TRUE);

    // compare return result and error code.
    EXPECT_EQ(pre_bexit_invalid_str, hooked_bexit_invalid_str);
    EXPECT_EQ(pre_error_invalid_str, hooked_error_invalid_str);
    EXPECT_EQ(pre_bexit_null_str, hooked_bexit_null_str);
    EXPECT_EQ(pre_berror_null_str, hooked_berror_null_str);
    EXPECT_EQ(pre_bexit_short_str, hooked_bexit_short_str);
    EXPECT_EQ(pre_error_short_str, hooked_error_short_str);
    EXPECT_EQ(pre_bexit_long_str, hooked_bexit_long_str);
    EXPECT_EQ(pre_error_long_str, hooked_error_long_str);
    EXPECT_EQ(pre_bexit_long_data, hooked_bexit_long_data);
    EXPECT_EQ(pre_error_long_data, hooked_error_long_data);
    EXPECT_EQ(pre_bexit_ordinal, hooked_bexit_ordinal);
    EXPECT_EQ(pre_error_ordinal, hooked_error_ordinal);
}

#endif

