#include "stdafx.h"
#include <stdio.h>
#include <windows.h>
#include <string>
#include <vector>
#include <functional>
#include <future>
#include <atomic>
#include <random>
#include <powerbase.h>
#include <Lm.h>
#include <lmjoin.h>
#include <lmaccess.h>
#include <wininet.h>
#include "utils.h"
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
    virtual NTSTATUS NtQueryInformationProcess(HANDLE ProcessHandle, PROCESSINFOCLASS ProcessInformationClass, PVOID ProcessInformation, ULONG ProcessInformationLength, PULONG ReturnLength) = 0;
    virtual NTSTATUS NtQueryLicenseValue(PUNICODE_STRING ValueName, PULONG Type, PVOID Data, ULONG DataSize, PULONG ResultDataSize) = 0;
    virtual BOOLEAN GetPwrCapabilities(PSYSTEM_POWER_CAPABILITIES lpspc) = 0;
    //exploit test
    virtual LPVOID VirtualAllocEx(HANDLE hProcess, LPVOID lpAddress, SIZE_T dwSize, DWORD  flAllocationType, DWORD  flProtect) = 0;
    virtual BOOL VirtualProtectEx(HANDLE hProcess, LPVOID lpAddress, SIZE_T dwSize, DWORD flNewProtect, LPDWORD lpflOldProtect) = 0;
    virtual BOOL CreateProcess(LPCSTR lpApplicationName, LPSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCSTR lpCurrentDirectory, LPSTARTUPINFOA lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation) = 0;
    virtual HANDLE CreateFile(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) = 0;
    //explore test
    virtual NET_API_STATUS NetGetJoinInformation(LPCWSTR lpServer, LPWSTR *lpNameBuffer, PNETSETUP_JOIN_STATUS BufferType) = 0;
    virtual NET_API_STATUS NetUserGetInfo(LPCWSTR servername, LPCWSTR username, DWORD level, LPBYTE *bufptr) = 0;
    virtual NET_API_STATUS NetUserGetLocalGroups(LPCWSTR servername, LPCWSTR username, DWORD level, DWORD flags, LPBYTE *bufptr, DWORD prefmaxlen, LPDWORD entriesread, LPDWORD totalentries) = 0;
    virtual NET_API_STATUS NetShareEnum(LPWSTR servername, DWORD level, LPBYTE *bufptr, DWORD prefmaxlen, LPDWORD entriesread, LPDWORD totalentries, LPDWORD resume_handle) = 0;
    //internet test
    virtual LPVOID InternetOpen(LPCSTR lpszAgent, DWORD dwAccessType, LPCSTR lpszProxy, LPCSTR lpszProxyBypass, DWORD dwFlags) = 0;
    virtual LPVOID InternetOpenUrl(HINTERNET hInternet, LPCSTR lpszUrl, LPCSTR lpszHeaders, DWORD dwHeadersLength, DWORD dwFlags, DWORD_PTR dwContext) = 0;
    virtual BOOL InternetReadFile(HINTERNET hFile, LPVOID lpBuffer, DWORD dwNumberOfBytesToRead, LPDWORD lpdwNumberOfBytesRead) = 0;
    virtual BOOL InternetCrackUrl(LPCSTR lpszUrl, DWORD dwUrlLength, DWORD dwFlags, LPURL_COMPONENTSA lpUrlComponents) = 0;
    virtual LPVOID InternetConnect(HINTERNET hInternet, LPCSTR lpszServerName, INTERNET_PORT nServerPort, LPCSTR lpszUserName, LPCSTR lpszPassword, DWORD dwService, DWORD dwFlags, DWORD_PTR dwContext) = 0;
    virtual LPVOID HttpOpenRequest(HINTERNET hConnect, LPCSTR lpszVerb, LPCSTR lpszObjectName, LPCSTR lpszVersion, LPCSTR lpszReferrer, LPCSTR *lplpszAcceptTypes, DWORD dwFlags, DWORD_PTR dwContext) = 0;
    virtual BOOL HttpSendRequest(HINTERNET hRequest, LPCSTR lpszHeaders, DWORD dwHeadersLength, LPVOID lpOptional, DWORD dwOptionalLength) = 0;
};

class ApiHookSystemMock : public ApiHookSystem
{
public:
    MOCK_METHOD0(GetTickCount, DWORD());
    MOCK_METHOD1(GetModuleHandleW, HMODULE(LPCWSTR));
    MOCK_METHOD1(GetSystemTime, void(LPSYSTEMTIME));
    MOCK_METHOD1(GetPwrCapabilities, BOOLEAN(PSYSTEM_POWER_CAPABILITIES));
    MOCK_METHOD2(ExitWindowsEx, BOOL(UINT, DWORD));
    MOCK_METHOD2(GetProcAddress, FARPROC(HMODULE, LPCSTR));
    MOCK_METHOD3(NetGetJoinInformation, NET_API_STATUS(LPCWSTR, LPWSTR*, PNETSETUP_JOIN_STATUS));
    MOCK_METHOD4(GetDiskFreeSpaceExW, BOOL(LPCWSTR, ULARGE_INTEGER*, ULARGE_INTEGER*, ULARGE_INTEGER*));
    MOCK_METHOD4(NetUserGetInfo, NET_API_STATUS(LPCWSTR, LPCWSTR, DWORD, LPBYTE*));
    MOCK_METHOD4(InternetReadFile, BOOL(HINTERNET, LPVOID, DWORD, LPDWORD));
    MOCK_METHOD4(InternetCrackUrl, BOOL(LPCSTR, DWORD, DWORD, LPURL_COMPONENTSA));
    MOCK_METHOD5(NtQueryInformationProcess, NTSTATUS(HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG));
    MOCK_METHOD5(NtQueryLicenseValue, NTSTATUS(PUNICODE_STRING, PULONG, PVOID, ULONG, PULONG));
    MOCK_METHOD5(VirtualAllocEx, LPVOID(HANDLE, LPVOID, SIZE_T, DWORD, DWORD));
    MOCK_METHOD5(VirtualProtectEx, BOOL(HANDLE, LPVOID, SIZE_T, DWORD, LPDWORD));
    MOCK_METHOD5(InternetOpen, LPVOID(LPCSTR, DWORD, LPCSTR, LPCSTR, DWORD));
    MOCK_METHOD5(HttpSendRequest, BOOL(HINTERNET, LPCSTR, DWORD, LPVOID, DWORD));
    MOCK_METHOD6(InternetOpenUrl, LPVOID(HINTERNET, LPCSTR, LPCSTR, DWORD, DWORD, DWORD_PTR));
    MOCK_METHOD7(CreateFile, HANDLE(LPCSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE));
    MOCK_METHOD7(NetShareEnum, NET_API_STATUS(LPWSTR, DWORD, LPBYTE*, DWORD, LPDWORD, LPDWORD, LPDWORD));
    MOCK_METHOD8(NetUserGetLocalGroups, NET_API_STATUS(LPCWSTR, LPCWSTR, DWORD, DWORD, LPBYTE*, DWORD, LPDWORD, LPDWORD));
    MOCK_METHOD8(InternetConnect, LPVOID(HINTERNET, LPCSTR, INTERNET_PORT, LPCSTR, LPCSTR, DWORD, DWORD, DWORD_PTR));
    MOCK_METHOD8(HttpOpenRequest, LPVOID(HINTERNET, LPCSTR, LPCSTR, LPCSTR, LPCSTR, LPCSTR*, DWORD, DWORD_PTR));
    MOCK_METHOD10(CreateProcess, BOOL(LPCSTR, LPSTR, LPSECURITY_ATTRIBUTES, LPSECURITY_ATTRIBUTES, BOOL, DWORD, LPVOID, LPCSTR, LPSTARTUPINFOA, LPPROCESS_INFORMATION));
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
        ntqueryinformationprocess_func = reinterpret_cast<NtQueryInformationProcess_Define>(GetProcAddress(GetModuleHandle("ntdll"), "NtQueryInformationProcess"));
        ntquerylicensevalue_func = reinterpret_cast<NtQueryLicenseValue_Define>(GetProcAddress(GetModuleHandle("ntdll"), "NtQueryLicenseValue"));
        rtlallocateheap_func = reinterpret_cast<RtlAllocateHeap_Define>(GetProcAddress(GetModuleHandle("ntdll"), "RtlAllocateHeap"));
        rtlfreeheap_func = reinterpret_cast<RtlFreeHeap_Define>(GetProcAddress(GetModuleHandle("ntdll"), "RtlFreeHeap"));
        rtldestroyheap_func = reinterpret_cast<RtlDestroyHeap_Define>(GetProcAddress(GetModuleHandle("ntdll"), "RtlDestroyHeap"));
    }
    ~HookSystemTest() override {}

    virtual void SetUp() override {
        ASSERT_TRUE(g_hook_test_object->EnableAllApis());
    }

    void TearDown() override {
        ASSERT_TRUE(g_hook_test_object->DisableAllApis());
    }

    NtQueryInformationProcess_Define ntqueryinformationprocess_func;
    NtQueryLicenseValue_Define ntquerylicensevalue_func;
    RtlAllocateHeap_Define rtlallocateheap_func;
    RtlFreeHeap_Define rtlfreeheap_func;
    RtlDestroyHeap_Define rtldestroyheap_func;
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
    //EXPECT_EQ(pre_free_bytes_1.QuadPart, hooked_free_bytes_1.QuadPart);
    //EXPECT_EQ(pre_total_free_bytes_1.QuadPart, hooked_total_free_bytes_1.QuadPart);
    //EXPECT_EQ(pre_bexit_invalid_addr, hooked_bexit_invalid_addr);
    //EXPECT_EQ(pre_error_invalid_addr, hooked_error_invalid_addr);
    //EXPECT_EQ(pre_free_bytes_2.QuadPart, hooked_free_bytes_2.QuadPart);
    //EXPECT_EQ(pre_total_free_bytes_2.QuadPart, hooked_total_free_bytes_2.QuadPart);
    EXPECT_EQ(pre_bexit_normal_addr, hooked_bexit_normal_addr);
    EXPECT_EQ(pre_error_normal_addr, hooked_error_normal_addr);
    //EXPECT_EQ(pre_free_bytes_3.QuadPart, hooked_free_bytes_3.QuadPart);
    //EXPECT_EQ(pre_total_free_bytes_3.QuadPart, hooked_total_free_bytes_3.QuadPart);
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

TEST_F(HookSystemTest, NtQueryInformationProcess_PreLog_ProcessHandle_Test)
{
    NTSTATUS pre_bexit_invalid_handle, pre_bexit_null_handle, pre_bexit_normal_handle, pre_bexit_other_handle1, pre_bexit_other_handle2, pre_bexit_other_handle3;
    DWORD pre_error_invalid_handle, pre_berror_null_handle, pre_error_normal_handle, pre_error_other_handle1, pre_error_other_handle2, pre_error_other_handle3;
    NTSTATUS hooked_bexit_invalid_handle, hooked_bexit_null_handle, hooked_bexit_normal_handle, hooked_bexit_other_handle1, hooked_bexit_other_handle2, hooked_bexit_other_handle3;
    DWORD hooked_error_invalid_handle, hooked_berror_null_handle, hooked_error_normal_handle, hooked_error_other_handle1, hooked_error_other_handle2, hooked_error_other_handle3;
    ApiHookSystemMock hook_system_mock;

    if (ntqueryinformationprocess_func) {
        // test variable
        PROCESS_BASIC_INFORMATION basic_info = {};
        ULONG return_len = 0;
        // first call test API when DisableAllApis().
        ASSERT_TRUE(g_hook_test_object->DisableAllApis());
        // test when ProcessHandle is invalid
        SetLastError(0); EXPECT_CALL(hook_system_mock, NtQueryInformationProcess(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(ntqueryinformationprocess_func((HANDLE)0x123, ProcessBasicInformation, &basic_info, sizeof(basic_info), &return_len)));
        pre_bexit_invalid_handle = hook_system_mock.NtQueryInformationProcess((HANDLE)0x123, ProcessBasicInformation, &basic_info, sizeof(basic_info), &return_len);
        pre_error_invalid_handle = GetLastError();
        // test when ProcessHandle is nullptr
        SetLastError(0); EXPECT_CALL(hook_system_mock, NtQueryInformationProcess(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(ntqueryinformationprocess_func(nullptr, ProcessBasicInformation, &basic_info, sizeof(basic_info), &return_len)));
        pre_bexit_null_handle = hook_system_mock.NtQueryInformationProcess(nullptr, ProcessBasicInformation, &basic_info, sizeof(basic_info), &return_len);
        pre_berror_null_handle = GetLastError();
        // test when ProcessHandle is normal
        SetLastError(0); EXPECT_CALL(hook_system_mock, NtQueryInformationProcess(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(ntqueryinformationprocess_func(GetCurrentProcess(), ProcessBasicInformation, &basic_info, sizeof(basic_info), &return_len)));
        pre_bexit_normal_handle = hook_system_mock.NtQueryInformationProcess(GetCurrentProcess(), ProcessBasicInformation, &basic_info, sizeof(basic_info), &return_len);
        pre_error_normal_handle = GetLastError();
        // test when other parameter invalid.
        //SetLastError(0); EXPECT_CALL(hook_system_mock, NtQueryInformationProcess(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(ntqueryinformationprocess_func(GetCurrentProcess(), ProcessBasicInformation, nullptr, sizeof(basic_info), &return_len)));
        //pre_bexit_other_handle1 = hook_system_mock.NtQueryInformationProcess(GetCurrentProcess(), ProcessBasicInformation, nullptr, sizeof(basic_info), &return_len);
        //pre_error_other_handle1 = GetLastError();
        SetLastError(0); EXPECT_CALL(hook_system_mock, NtQueryInformationProcess(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(ntqueryinformationprocess_func(GetCurrentProcess(), ProcessBasicInformation, &basic_info, 0, &return_len)));
        pre_bexit_other_handle2 = hook_system_mock.NtQueryInformationProcess(GetCurrentProcess(), ProcessBasicInformation, &basic_info, 0, &return_len);
        pre_error_other_handle2 = GetLastError();
        SetLastError(0); EXPECT_CALL(hook_system_mock, NtQueryInformationProcess(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(ntqueryinformationprocess_func(GetCurrentProcess(), ProcessBasicInformation, &basic_info, sizeof(basic_info), nullptr)));
        pre_bexit_other_handle3 = hook_system_mock.NtQueryInformationProcess(GetCurrentProcess(), ProcessBasicInformation, &basic_info, sizeof(basic_info), nullptr);
        pre_error_other_handle3 = GetLastError();

        // initialize
        std::vector<std::string> action_list;
        action_list.push_back("P62");
        g_server_object->AddLogCountMap(action_list);
        // second call test API when EnableAllApis().
        ASSERT_TRUE(g_hook_test_object->EnableAllApis());
        // test when ProcessHandle is invalid
        SetLastError(0); EXPECT_CALL(hook_system_mock, NtQueryInformationProcess(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(ntqueryinformationprocess_func((HANDLE)0x123, ProcessBasicInformation, &basic_info, sizeof(basic_info), &return_len)));
        hooked_bexit_invalid_handle = hook_system_mock.NtQueryInformationProcess((HANDLE)0x123, ProcessBasicInformation, &basic_info, sizeof(basic_info), &return_len);
        hooked_error_invalid_handle = GetLastError();
        // test when ProcessHandle is nullptr
        SetLastError(0); EXPECT_CALL(hook_system_mock, NtQueryInformationProcess(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(ntqueryinformationprocess_func(nullptr, ProcessBasicInformation, &basic_info, sizeof(basic_info), &return_len)));
        hooked_bexit_null_handle = hook_system_mock.NtQueryInformationProcess(nullptr, ProcessBasicInformation, &basic_info, sizeof(basic_info), &return_len);
        hooked_berror_null_handle = GetLastError();
        // test when ProcessHandle is normal
        SetLastError(0); EXPECT_CALL(hook_system_mock, NtQueryInformationProcess(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(ntqueryinformationprocess_func(GetCurrentProcess(), ProcessBasicInformation, &basic_info, sizeof(basic_info), &return_len)));
        hooked_bexit_normal_handle = hook_system_mock.NtQueryInformationProcess(GetCurrentProcess(), ProcessBasicInformation, &basic_info, sizeof(basic_info), &return_len);
        hooked_error_normal_handle = GetLastError();
        // test when other parameter invalid.
        //SetLastError(0); EXPECT_CALL(hook_system_mock, NtQueryInformationProcess(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(ntqueryinformationprocess_func(GetCurrentProcess(), ProcessBasicInformation, nullptr, sizeof(basic_info), &return_len)));
        //hooked_bexit_other_handle1 = hook_system_mock.NtQueryInformationProcess(GetCurrentProcess(), ProcessBasicInformation, nullptr, sizeof(basic_info), &return_len);
        //hooked_error_other_handle1 = GetLastError();
        SetLastError(0); EXPECT_CALL(hook_system_mock, NtQueryInformationProcess(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(ntqueryinformationprocess_func(GetCurrentProcess(), ProcessBasicInformation, &basic_info, 0, &return_len)));
        hooked_bexit_other_handle2 = hook_system_mock.NtQueryInformationProcess(GetCurrentProcess(), ProcessBasicInformation, &basic_info, 0, &return_len);
        hooked_error_other_handle2 = GetLastError();
        SetLastError(0); EXPECT_CALL(hook_system_mock, NtQueryInformationProcess(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(ntqueryinformationprocess_func(GetCurrentProcess(), ProcessBasicInformation, &basic_info, sizeof(basic_info), nullptr)));
        hooked_bexit_other_handle3 = hook_system_mock.NtQueryInformationProcess(GetCurrentProcess(), ProcessBasicInformation, &basic_info, sizeof(basic_info), nullptr);
        hooked_error_other_handle3 = GetLastError();
        // wait for all logs received.
        std::vector<int> count_list;
        count_list.push_back(5);
        EXPECT_EQ(g_server_object->WaitLogCountMap(count_list, 5), TRUE);

        // compare return result and error code.
        EXPECT_EQ(pre_bexit_invalid_handle, hooked_bexit_invalid_handle);
        EXPECT_EQ(pre_error_invalid_handle, hooked_error_invalid_handle);
        EXPECT_EQ(pre_bexit_null_handle, hooked_bexit_null_handle);
        EXPECT_EQ(pre_berror_null_handle, hooked_berror_null_handle);
        EXPECT_EQ(pre_bexit_normal_handle, hooked_bexit_normal_handle);
        EXPECT_EQ(pre_error_normal_handle, hooked_error_normal_handle);
        //EXPECT_EQ(pre_bexit_other_handle1, hooked_bexit_other_handle1);
        //EXPECT_EQ(pre_error_other_handle1, hooked_error_other_handle1);
        EXPECT_EQ(pre_bexit_other_handle2, hooked_bexit_other_handle2);
        EXPECT_EQ(pre_error_other_handle2, hooked_error_other_handle2);
        EXPECT_EQ(pre_bexit_other_handle3, hooked_bexit_other_handle3);
        EXPECT_EQ(pre_error_other_handle3, hooked_error_other_handle3);
    }
}

TEST_F(HookSystemTest, NtQueryLicenseValue_PreLog_ValueName_Test)
{
    NTSTATUS pre_bexit_invalid_name, pre_bexit_null_name, pre_bexit_normal_name, pre_bexit_other_name1, pre_bexit_other_name2, pre_bexit_other_name3;
    DWORD pre_error_invalid_name, pre_berror_null_name, pre_error_normal_name, pre_error_other_name1, pre_error_other_name2, pre_error_other_name3;
    NTSTATUS hooked_bexit_invalid_name, hooked_bexit_null_name, hooked_bexit_normal_name, hooked_bexit_other_name1, hooked_bexit_other_name2, hooked_bexit_other_name3;
    DWORD hooked_error_invalid_name, hooked_berror_null_name, hooked_error_normal_name, hooked_error_other_name1, hooked_error_other_name2, hooked_error_other_name3;
    ApiHookSystemMock hook_system_mock;

    if (ntquerylicensevalue_func) {
        // test variable
        UNICODE_STRING LicenseValue;
        LicenseValue.Buffer = L"Kernel-VMDetection-Private";
        LicenseValue.MaximumLength = sizeof(L"Kernel-VMDetection-Private");
        LicenseValue.Length = LicenseValue.MaximumLength - sizeof(wchar_t);
        ULONG Result = 0, ReturnLength;
        // first call test API when DisableAllApis().
        ASSERT_TRUE(g_hook_test_object->DisableAllApis());
        // test when ValueName is nullptr
        SetLastError(0); EXPECT_CALL(hook_system_mock, NtQueryLicenseValue(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(ntquerylicensevalue_func(nullptr, NULL, reinterpret_cast<PVOID>(&Result), sizeof(ULONG), &ReturnLength)));
        pre_bexit_null_name = hook_system_mock.NtQueryLicenseValue(nullptr, NULL, reinterpret_cast<PVOID>(&Result), sizeof(ULONG), &ReturnLength);
        pre_berror_null_name = GetLastError();
        // test when ValueName is normal
        SetLastError(0); EXPECT_CALL(hook_system_mock, NtQueryLicenseValue(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(ntquerylicensevalue_func(&LicenseValue, NULL, reinterpret_cast<PVOID>(&Result), sizeof(ULONG), &ReturnLength)));
        pre_bexit_normal_name = hook_system_mock.NtQueryLicenseValue(&LicenseValue, NULL, reinterpret_cast<PVOID>(&Result), sizeof(ULONG), &ReturnLength);
        pre_error_normal_name = GetLastError();
        // test when other parameter invalid.
        SetLastError(0); EXPECT_CALL(hook_system_mock, NtQueryLicenseValue(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(ntquerylicensevalue_func(&LicenseValue, NULL, nullptr, 0, &ReturnLength)));
        pre_bexit_other_name2 = hook_system_mock.NtQueryLicenseValue(&LicenseValue, NULL, nullptr, 0, &ReturnLength);
        pre_error_other_name2 = GetLastError();
        SetLastError(0); EXPECT_CALL(hook_system_mock, NtQueryLicenseValue(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(ntquerylicensevalue_func(&LicenseValue, NULL, reinterpret_cast<PVOID>(&Result), sizeof(ULONG), nullptr)));
        pre_bexit_other_name3 = hook_system_mock.NtQueryLicenseValue(&LicenseValue, NULL, reinterpret_cast<PVOID>(&Result), sizeof(ULONG), nullptr);
        pre_error_other_name3 = GetLastError();

        // initialize
        std::vector<std::string> action_list;
        action_list.push_back("P65");
        g_server_object->AddLogCountMap(action_list);
        // second call test API when EnableAllApis().
        ASSERT_TRUE(g_hook_test_object->EnableAllApis());
        // test when ValueName is nullptr
        SetLastError(0); EXPECT_CALL(hook_system_mock, NtQueryLicenseValue(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(ntquerylicensevalue_func(nullptr, NULL, reinterpret_cast<PVOID>(&Result), sizeof(ULONG), &ReturnLength)));
        hooked_bexit_null_name = hook_system_mock.NtQueryLicenseValue(nullptr, NULL, reinterpret_cast<PVOID>(&Result), sizeof(ULONG), &ReturnLength);
        hooked_berror_null_name = GetLastError();
        // test when ValueName is normal
        SetLastError(0); EXPECT_CALL(hook_system_mock, NtQueryLicenseValue(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(ntquerylicensevalue_func(&LicenseValue, NULL, reinterpret_cast<PVOID>(&Result), sizeof(ULONG), &ReturnLength)));
        hooked_bexit_normal_name = hook_system_mock.NtQueryLicenseValue(&LicenseValue, NULL, reinterpret_cast<PVOID>(&Result), sizeof(ULONG), &ReturnLength);
        hooked_error_normal_name = GetLastError();
        // test when other parameter invalid.
        SetLastError(0); EXPECT_CALL(hook_system_mock, NtQueryLicenseValue(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(ntquerylicensevalue_func(&LicenseValue, NULL, nullptr, 0, &ReturnLength)));
        hooked_bexit_other_name2 = hook_system_mock.NtQueryLicenseValue(&LicenseValue, NULL, nullptr, 0, &ReturnLength);
        hooked_error_other_name2 = GetLastError();
        SetLastError(0); EXPECT_CALL(hook_system_mock, NtQueryLicenseValue(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(ntquerylicensevalue_func(&LicenseValue, NULL, reinterpret_cast<PVOID>(&Result), sizeof(ULONG), nullptr)));
        hooked_bexit_other_name3 = hook_system_mock.NtQueryLicenseValue(&LicenseValue, NULL, reinterpret_cast<PVOID>(&Result), sizeof(ULONG), nullptr);
        hooked_error_other_name3 = GetLastError();
        // wait for all logs received.
        std::vector<int> count_list;
        count_list.push_back(3);
        EXPECT_EQ(g_server_object->WaitLogCountMap(count_list, 5), TRUE);

        //// compare return result and error code.
        EXPECT_EQ(pre_bexit_null_name, hooked_bexit_null_name);
        EXPECT_EQ(pre_berror_null_name, hooked_berror_null_name);
        EXPECT_EQ(hooked_bexit_normal_name, 0xC0000034);
        EXPECT_EQ(hooked_bexit_other_name2, 0xC0000034);
        EXPECT_EQ(hooked_bexit_other_name3, 0xC0000034);
    }
}

TEST_F(HookSystemTest, GetPwrCapabilities_Modify_lpspc_Test)
{
    BOOLEAN pre_bexit_null_lpspc, pre_bexit_normal_lpspc;
    DWORD pre_berror_null_lpspc, pre_error_normal_lpspc;
    BOOLEAN hooked_bexit_null_lpspc, hooked_bexit_normal_lpspc;
    DWORD hooked_berror_null_lpspc, hooked_error_normal_lpspc;
    ApiHookSystemMock hook_system_mock;

    // test variable
    SYSTEM_POWER_CAPABILITIES spc = {};
    // first call test API when DisableAllApis().
    ASSERT_TRUE(g_hook_test_object->DisableAllApis());
    // test when lpspc is nullptr
    SetLastError(0); EXPECT_CALL(hook_system_mock, GetPwrCapabilities(testing::_)).Times(1).WillRepeatedly(testing::Return(GetPwrCapabilities(nullptr)));
    pre_bexit_null_lpspc = hook_system_mock.GetPwrCapabilities(nullptr);
    pre_berror_null_lpspc = GetLastError();
    // test when lpspc is normal
    SetLastError(0); EXPECT_CALL(hook_system_mock, GetPwrCapabilities(testing::_)).Times(1).WillRepeatedly(testing::Return(GetPwrCapabilities(&spc)));
    pre_bexit_normal_lpspc = hook_system_mock.GetPwrCapabilities(&spc);
    pre_error_normal_lpspc = GetLastError();

    // initialize
    std::vector<std::string> action_list;
    action_list.push_back("P66");
    g_server_object->AddLogCountMap(action_list);
    // second call test API when EnableAllApis().
    ASSERT_TRUE(g_hook_test_object->EnableAllApis());
    // test when lpspc is nullptr
    SetLastError(0); EXPECT_CALL(hook_system_mock, GetPwrCapabilities(testing::_)).Times(1).WillRepeatedly(testing::Return(GetPwrCapabilities(nullptr)));
    hooked_bexit_null_lpspc = hook_system_mock.GetPwrCapabilities(nullptr);
    hooked_berror_null_lpspc = GetLastError();
    // test when lpspc is normal
    SetLastError(0); EXPECT_CALL(hook_system_mock, GetPwrCapabilities(testing::_)).Times(1).WillRepeatedly(testing::Return(GetPwrCapabilities(&spc)));
    hooked_bexit_normal_lpspc = hook_system_mock.GetPwrCapabilities(&spc);
    hooked_error_normal_lpspc = GetLastError();
    // wait for all logs received.
    std::vector<int> count_list;
    count_list.push_back(1);
    EXPECT_EQ(g_server_object->WaitLogCountMap(count_list, 5), TRUE);

    //// compare return result and error code.
    EXPECT_EQ(pre_bexit_null_lpspc, hooked_bexit_null_lpspc);
    EXPECT_EQ(pre_berror_null_lpspc, hooked_berror_null_lpspc);
    ASSERT_TRUE(hooked_bexit_normal_lpspc);
    EXPECT_TRUE(spc.SystemS1);
}

TEST_F(HookSystemTest, VirtualAllocEx_Postlog_flAllocationType_Test)
{
    PVOID pre_bexit_invalid_alloctype, pre_bexit_sin_alloctype, pre_bexit_mul_alloctype, pre_bexit_inv_mul_alloctype;
    DWORD pre_error_invalid_alloctype, pre_error_sin_alloctype, pre_error_mul_alloctype, pre_error_inv_mul_alloctype;
    PVOID hooked_bexit_invalid_alloctype, hooked_bexit_sin_alloctype, hooked_bexit_mul_alloctype, hooked_bexit_inv_mul_alloctype;
    DWORD hooked_error_invalid_alloctype, hooked_error_sin_alloctype, hooked_error_mul_alloctype, hooked_error_inv_mul_alloctype;
    ApiHookSystemMock hook_system_mock;

    // test variable
    // first call test API when DisableAllApis().
    ASSERT_TRUE(g_hook_test_object->DisableAllApis());
    // test when flAllocationType is 0 flag
    SetLastError(0); EXPECT_CALL(hook_system_mock, VirtualAllocEx(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(VirtualAllocEx(GetCurrentProcess(), nullptr, 0x1000, 0, PAGE_EXECUTE_READWRITE)));
    pre_bexit_invalid_alloctype = hook_system_mock.VirtualAllocEx(GetCurrentProcess(), nullptr, 0x1000, 0, PAGE_EXECUTE_READWRITE);
    pre_error_invalid_alloctype = GetLastError();
    // test when flAllocationType is single flag
    SetLastError(0); EXPECT_CALL(hook_system_mock, VirtualAllocEx(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(VirtualAllocEx(GetCurrentProcess(), nullptr, 0x1000, MEM_COMMIT, PAGE_EXECUTE_READWRITE)));
    pre_bexit_sin_alloctype = hook_system_mock.VirtualAllocEx(GetCurrentProcess(), nullptr, 0x1000, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    pre_error_sin_alloctype = GetLastError();
    // test when flAllocationType is multiple flag
    SetLastError(0); EXPECT_CALL(hook_system_mock, VirtualAllocEx(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(VirtualAllocEx(GetCurrentProcess(), nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE)));
    pre_bexit_mul_alloctype = hook_system_mock.VirtualAllocEx(GetCurrentProcess(), nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    pre_error_mul_alloctype = GetLastError();
    // test when flAllocationType is multiple invalid flag
    SetLastError(0); EXPECT_CALL(hook_system_mock, VirtualAllocEx(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(VirtualAllocEx(GetCurrentProcess(), nullptr, 0x1000, MEM_COMMIT | 0x333, PAGE_EXECUTE_READWRITE)));
    pre_bexit_inv_mul_alloctype = hook_system_mock.VirtualAllocEx(GetCurrentProcess(), nullptr, 0x1000, MEM_COMMIT | 0x333, PAGE_EXECUTE_READWRITE);
    pre_error_inv_mul_alloctype = GetLastError();

    // initialize
    std::vector<std::string> action_list;
    action_list.push_back("P68");
    g_server_object->AddLogCountMap(action_list);
    // second call test API when EnableAllApis().
    ASSERT_TRUE(g_hook_test_object->EnableAllApis());
    // test when flAllocationType is 0 flag
    SetLastError(0); EXPECT_CALL(hook_system_mock, VirtualAllocEx(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(VirtualAllocEx(GetCurrentProcess(), nullptr, 0x1000, 0, PAGE_EXECUTE_READWRITE)));
    hooked_bexit_invalid_alloctype = hook_system_mock.VirtualAllocEx(GetCurrentProcess(), nullptr, 0x1000, 0, PAGE_EXECUTE_READWRITE);
    hooked_error_invalid_alloctype = GetLastError();
    // test when flAllocationType is single flag
    SetLastError(0); EXPECT_CALL(hook_system_mock, VirtualAllocEx(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(VirtualAllocEx(GetCurrentProcess(), nullptr, 0x1000, MEM_COMMIT, PAGE_EXECUTE_READWRITE)));
    hooked_bexit_sin_alloctype = hook_system_mock.VirtualAllocEx(GetCurrentProcess(), nullptr, 0x1000, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    hooked_error_sin_alloctype = GetLastError();
    // test when flAllocationType is multiple flag
    SetLastError(0); EXPECT_CALL(hook_system_mock, VirtualAllocEx(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(VirtualAllocEx(GetCurrentProcess(), nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE)));
    hooked_bexit_mul_alloctype = hook_system_mock.VirtualAllocEx(GetCurrentProcess(), nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    hooked_error_mul_alloctype = GetLastError();
    // test when flAllocationType is multiple invalid flag
    SetLastError(0); EXPECT_CALL(hook_system_mock, VirtualAllocEx(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(VirtualAllocEx(GetCurrentProcess(), nullptr, 0x1000, MEM_COMMIT | 0x333, PAGE_EXECUTE_READWRITE)));
    hooked_bexit_inv_mul_alloctype = hook_system_mock.VirtualAllocEx(GetCurrentProcess(), nullptr, 0x1000, MEM_COMMIT | 0x333, PAGE_EXECUTE_READWRITE);
    hooked_error_inv_mul_alloctype = GetLastError();
    // wait for all logs received.
    std::vector<int> count_list;
    count_list.push_back(0);
    EXPECT_EQ(g_server_object->WaitLogCountMap(count_list, 5), TRUE);

    //// compare return result and error code.
    if(pre_bexit_invalid_alloctype) EXPECT_TRUE(hooked_bexit_invalid_alloctype);
    EXPECT_EQ(pre_error_invalid_alloctype, hooked_error_invalid_alloctype);
    if(pre_bexit_sin_alloctype) EXPECT_TRUE(hooked_bexit_sin_alloctype);
    EXPECT_EQ(pre_error_sin_alloctype, hooked_error_sin_alloctype);
    if(pre_bexit_mul_alloctype) EXPECT_TRUE(hooked_bexit_mul_alloctype);
    EXPECT_EQ(pre_error_mul_alloctype, hooked_error_mul_alloctype);
    if(pre_bexit_inv_mul_alloctype) EXPECT_TRUE(hooked_bexit_inv_mul_alloctype);
    EXPECT_EQ(pre_error_inv_mul_alloctype, hooked_error_inv_mul_alloctype);
    
    if (pre_bexit_invalid_alloctype) VirtualFree(pre_bexit_invalid_alloctype, 0 , MEM_RELEASE);
    if (hooked_bexit_invalid_alloctype) VirtualFree(hooked_bexit_invalid_alloctype, 0, MEM_RELEASE);
    if (pre_bexit_sin_alloctype) VirtualFree(pre_bexit_sin_alloctype, 0, MEM_RELEASE);
    if (hooked_bexit_sin_alloctype) VirtualFree(hooked_bexit_sin_alloctype, 0, MEM_RELEASE);
    if (pre_bexit_mul_alloctype) VirtualFree(pre_bexit_mul_alloctype, 0, MEM_RELEASE);
    if (hooked_bexit_mul_alloctype) VirtualFree(hooked_bexit_mul_alloctype, 0, MEM_RELEASE);
    if (pre_bexit_inv_mul_alloctype) VirtualFree(pre_bexit_inv_mul_alloctype, 0, MEM_RELEASE);
    if (hooked_bexit_inv_mul_alloctype) VirtualFree(hooked_bexit_inv_mul_alloctype, 0, MEM_RELEASE);
}

TEST_F(HookSystemTest, VirtualAllocEx_Postlog_flProtect_Test)
{
    PVOID pre_bexit_invalid_protect, pre_bexit_sin_protect, pre_bexit_mul_protect, pre_bexit_inv_mul_protect;
    DWORD pre_error_invalid_protect, pre_error_sin_protect, pre_error_mul_protect, pre_error_inv_mul_protect;
    PVOID hooked_bexit_invalid_protect, hooked_bexit_sin_protect, hooked_bexit_mul_protect, hooked_bexit_inv_mul_protect;
    DWORD hooked_error_invalid_protect, hooked_error_sin_protect, hooked_error_mul_protect, hooked_error_inv_mul_protect;
    ApiHookSystemMock hook_system_mock;

    // test variable
    // first call test API when DisableAllApis().
    ASSERT_TRUE(g_hook_test_object->DisableAllApis());
    // test when flAllocationType is 0 flag
    SetLastError(0); EXPECT_CALL(hook_system_mock, VirtualAllocEx(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(VirtualAllocEx(GetCurrentProcess(), nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, 0)));
    pre_bexit_invalid_protect = hook_system_mock.VirtualAllocEx(GetCurrentProcess(), nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, 0);
    pre_error_invalid_protect = GetLastError();
    // test when flAllocationType is single flag
    SetLastError(0); EXPECT_CALL(hook_system_mock, VirtualAllocEx(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(VirtualAllocEx(GetCurrentProcess(), nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE)));
    pre_bexit_sin_protect = hook_system_mock.VirtualAllocEx(GetCurrentProcess(), nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    pre_error_sin_protect = GetLastError();
    // test when flAllocationType is multiple flag
    SetLastError(0); EXPECT_CALL(hook_system_mock, VirtualAllocEx(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(VirtualAllocEx(GetCurrentProcess(), nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_WRITECOPY | PAGE_NOCACHE)));
    pre_bexit_mul_protect = hook_system_mock.VirtualAllocEx(GetCurrentProcess(), nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_WRITECOPY | PAGE_NOCACHE);
    pre_error_mul_protect = GetLastError();
    // test when flAllocationType is multiple invalid flag
    SetLastError(0); EXPECT_CALL(hook_system_mock, VirtualAllocEx(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(VirtualAllocEx(GetCurrentProcess(), nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READ | 0x333)));
    pre_bexit_inv_mul_protect = hook_system_mock.VirtualAllocEx(GetCurrentProcess(), nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READ | 0x333);
    pre_error_inv_mul_protect = GetLastError();

    // initialize
    std::vector<std::string> action_list;
    action_list.push_back("P68");
    g_server_object->AddLogCountMap(action_list);
    // second call test API when EnableAllApis().
    ASSERT_TRUE(g_hook_test_object->EnableAllApis());
    // test when flAllocationType is 0 flag
    SetLastError(0); EXPECT_CALL(hook_system_mock, VirtualAllocEx(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(VirtualAllocEx(GetCurrentProcess(), nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, 0)));
    hooked_bexit_invalid_protect = hook_system_mock.VirtualAllocEx(GetCurrentProcess(), nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, 0);
    hooked_error_invalid_protect = GetLastError();
    // test when flAllocationType is single flag
    SetLastError(0); EXPECT_CALL(hook_system_mock, VirtualAllocEx(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(VirtualAllocEx(GetCurrentProcess(), nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE)));
    hooked_bexit_sin_protect = hook_system_mock.VirtualAllocEx(GetCurrentProcess(), nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    hooked_error_sin_protect = GetLastError();
    // test when flAllocationType is multiple flag
    SetLastError(0); EXPECT_CALL(hook_system_mock, VirtualAllocEx(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(VirtualAllocEx(GetCurrentProcess(), nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_WRITECOPY | PAGE_NOCACHE)));
    hooked_bexit_mul_protect = hook_system_mock.VirtualAllocEx(GetCurrentProcess(), nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_WRITECOPY | PAGE_NOCACHE);
    hooked_error_mul_protect = GetLastError();
    // test when flAllocationType is multiple invalid flag
    SetLastError(0); EXPECT_CALL(hook_system_mock, VirtualAllocEx(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(VirtualAllocEx(GetCurrentProcess(), nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READ | 0x333)));
    hooked_bexit_inv_mul_protect = hook_system_mock.VirtualAllocEx(GetCurrentProcess(), nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READ | 0x333);
    hooked_error_inv_mul_protect = GetLastError();
    // wait for all logs received.
    std::vector<int> count_list;
    count_list.push_back(0);
    EXPECT_EQ(g_server_object->WaitLogCountMap(count_list, 5), TRUE);

    //// compare return result and error code.
    if (pre_bexit_invalid_protect) EXPECT_TRUE(hooked_bexit_invalid_protect);
    EXPECT_EQ(pre_error_invalid_protect, hooked_error_invalid_protect);
    if (pre_bexit_sin_protect) EXPECT_TRUE(hooked_bexit_sin_protect);
    EXPECT_EQ(pre_error_sin_protect, hooked_error_sin_protect);
    if (pre_bexit_mul_protect) EXPECT_TRUE(hooked_bexit_mul_protect);
    EXPECT_EQ(pre_error_mul_protect, hooked_error_mul_protect);
    if (pre_bexit_inv_mul_protect) EXPECT_TRUE(hooked_bexit_inv_mul_protect);
    EXPECT_EQ(pre_error_inv_mul_protect, hooked_error_inv_mul_protect);

    if (pre_bexit_invalid_protect) VirtualFree(pre_bexit_invalid_protect, 0, MEM_RELEASE);
    if (hooked_bexit_invalid_protect) VirtualFree(hooked_bexit_invalid_protect, 0, MEM_RELEASE);
    if (pre_bexit_sin_protect) VirtualFree(pre_bexit_sin_protect, 0, MEM_RELEASE);
    if (hooked_bexit_sin_protect) VirtualFree(hooked_bexit_sin_protect, 0, MEM_RELEASE);
    if (pre_bexit_mul_protect) VirtualFree(pre_bexit_mul_protect, 0, MEM_RELEASE);
    if (hooked_bexit_mul_protect) VirtualFree(hooked_bexit_mul_protect, 0, MEM_RELEASE);
    if (pre_bexit_inv_mul_protect) VirtualFree(pre_bexit_inv_mul_protect, 0, MEM_RELEASE);
    if (hooked_bexit_inv_mul_protect) VirtualFree(hooked_bexit_inv_mul_protect, 0, MEM_RELEASE);
}

TEST_F(HookSystemTest, VirtualProtectEx_Postlog_flNewProtect_Test)
{
    BOOL pre_bexit_invalid_protect, pre_bexit_sin_protect, pre_bexit_mul_protect, pre_bexit_inv_mul_protect;
    DWORD pre_error_invalid_protect, pre_error_sin_protect, pre_error_mul_protect, pre_error_inv_mul_protect;
    BOOL hooked_bexit_invalid_protect, hooked_bexit_sin_protect, hooked_bexit_mul_protect, hooked_bexit_inv_mul_protect;
    DWORD hooked_error_invalid_protect, hooked_error_sin_protect, hooked_error_mul_protect, hooked_error_inv_mul_protect;
    ApiHookSystemMock hook_system_mock;

    // first call test API when DisableAllApis().
    ASSERT_TRUE(g_hook_test_object->DisableAllApis());
    // test variable
    DWORD lpflOldProtect = 0;
    PVOID test_address = VirtualAllocEx(GetCurrentProcess(), nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_READONLY);
    ASSERT_NE(test_address, nullptr);
    // test when flAllocationType is 0 flag
    SetLastError(0); EXPECT_CALL(hook_system_mock, VirtualProtectEx(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(VirtualProtectEx(GetCurrentProcess(), test_address, 0x1000, 0, &lpflOldProtect)));
    pre_bexit_invalid_protect = hook_system_mock.VirtualProtectEx(GetCurrentProcess(), test_address, 0x1000, 0, &lpflOldProtect);
    pre_error_invalid_protect = GetLastError();
    // test when flAllocationType is single flag
    SetLastError(0); EXPECT_CALL(hook_system_mock, VirtualProtectEx(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(VirtualProtectEx(GetCurrentProcess(), test_address, 0x1000, PAGE_EXECUTE_READWRITE, &lpflOldProtect)));
    pre_bexit_sin_protect = hook_system_mock.VirtualProtectEx(GetCurrentProcess(), test_address, 0x1000, PAGE_EXECUTE_READWRITE, &lpflOldProtect);
    pre_error_sin_protect = GetLastError();
    // test when flAllocationType is multiple flag
    SetLastError(0); EXPECT_CALL(hook_system_mock, VirtualProtectEx(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(VirtualProtectEx(GetCurrentProcess(), test_address, 0x1000, PAGE_EXECUTE_READWRITE | PAGE_NOCACHE, &lpflOldProtect)));
    pre_bexit_mul_protect = hook_system_mock.VirtualProtectEx(GetCurrentProcess(), test_address, 0x1000, PAGE_EXECUTE_READWRITE | PAGE_NOCACHE, &lpflOldProtect);
    pre_error_mul_protect = GetLastError();
    // test when flAllocationType is multiple invalid flag
    SetLastError(0); EXPECT_CALL(hook_system_mock, VirtualProtectEx(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(VirtualProtectEx(GetCurrentProcess(), test_address, 0x1000, PAGE_EXECUTE_READ | 0x333, &lpflOldProtect)));
    pre_bexit_inv_mul_protect = hook_system_mock.VirtualProtectEx(GetCurrentProcess(), test_address, 0x1000, PAGE_EXECUTE_READ | 0x333, &lpflOldProtect);
    pre_error_inv_mul_protect = GetLastError();

    // initialize
    std::vector<std::string> action_list;
    action_list.push_back("P68");
    g_server_object->AddLogCountMap(action_list);
    // second call test API when EnableAllApis().
    ASSERT_TRUE(g_hook_test_object->EnableAllApis());
    // test when flAllocationType is 0 flag
    SetLastError(0); EXPECT_CALL(hook_system_mock, VirtualProtectEx(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(VirtualProtectEx(GetCurrentProcess(), test_address, 0x1000, 0, &lpflOldProtect)));
    hooked_bexit_invalid_protect = hook_system_mock.VirtualProtectEx(GetCurrentProcess(), test_address, 0x1000, 0, &lpflOldProtect);
    hooked_error_invalid_protect = GetLastError();
    // test when flAllocationType is single flag
    SetLastError(0); EXPECT_CALL(hook_system_mock, VirtualProtectEx(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(VirtualProtectEx(GetCurrentProcess(), test_address, 0x1000, PAGE_EXECUTE_READWRITE, &lpflOldProtect)));
    hooked_bexit_sin_protect = hook_system_mock.VirtualProtectEx(GetCurrentProcess(), test_address, 0x1000, PAGE_EXECUTE_READWRITE, &lpflOldProtect);
    hooked_error_sin_protect = GetLastError();
    // test when flAllocationType is multiple flag
    SetLastError(0); EXPECT_CALL(hook_system_mock, VirtualProtectEx(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(VirtualProtectEx(GetCurrentProcess(), test_address, 0x1000, PAGE_EXECUTE_READ | PAGE_NOCACHE, &lpflOldProtect)));
    hooked_bexit_mul_protect = hook_system_mock.VirtualProtectEx(GetCurrentProcess(), test_address, 0x1000, PAGE_EXECUTE_READ | PAGE_NOCACHE, &lpflOldProtect);
    hooked_error_mul_protect = GetLastError();
    // test when flAllocationType is multiple invalid flag
    SetLastError(0); EXPECT_CALL(hook_system_mock, VirtualProtectEx(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(VirtualProtectEx(GetCurrentProcess(), test_address, 0x1000, PAGE_EXECUTE_READ | 0x333, &lpflOldProtect)));
    hooked_bexit_inv_mul_protect = hook_system_mock.VirtualProtectEx(GetCurrentProcess(), test_address, 0x1000, PAGE_EXECUTE_READ | 0x333, &lpflOldProtect);
    hooked_error_inv_mul_protect = GetLastError();
    // wait for all logs received.
    std::vector<int> count_list;
    count_list.push_back(0);
    EXPECT_EQ(g_server_object->WaitLogCountMap(count_list, 5), TRUE);

    //// compare return result and error code.
    EXPECT_EQ(pre_bexit_invalid_protect, hooked_bexit_invalid_protect);
    EXPECT_EQ(pre_error_invalid_protect, hooked_error_invalid_protect);
    EXPECT_EQ(pre_bexit_sin_protect, hooked_bexit_sin_protect);
    EXPECT_EQ(pre_error_sin_protect, hooked_error_sin_protect);
    EXPECT_EQ(pre_bexit_mul_protect, hooked_bexit_mul_protect);
    EXPECT_EQ(pre_error_mul_protect, hooked_error_mul_protect);
    EXPECT_EQ(pre_bexit_inv_mul_protect, hooked_bexit_inv_mul_protect);
    EXPECT_EQ(pre_error_inv_mul_protect, hooked_error_inv_mul_protect);

    VirtualFree(test_address, 0, MEM_RELEASE);
}

TEST_F(HookSystemTest, CreateProcess_Prelog_lpApplicationName_Test)
{
    BOOL pre_bexit_invalid_str, pre_bexit_null_str, pre_bexit_short_str, pre_bexit_long_str, pre_bexit_long_data;
    DWORD pre_error_invalid_str, pre_berror_null_str, pre_error_short_str, pre_error_long_str, pre_error_long_data;
    PROCESS_INFORMATION proc_info_pre_invalid_str = {}; PROCESS_INFORMATION proc_info_pre_null_str = {}; PROCESS_INFORMATION proc_info_pre_short_str = {}; PROCESS_INFORMATION proc_info_pre_long_str = {}; PROCESS_INFORMATION proc_info_pre_long_data = {};
    BOOL hooked_bexit_invalid_str, hooked_bexit_null_str, hooked_bexit_short_str, hooked_bexit_long_str, hooked_bexit_long_data;
    DWORD hooked_error_invalid_str, hooked_berror_null_str, hooked_error_short_str, hooked_error_long_str, hooked_error_long_data;
    PROCESS_INFORMATION proc_info_hooked_invalid_str = {}; PROCESS_INFORMATION proc_info_hooked_null_str = {}; PROCESS_INFORMATION proc_info_hooked_short_str = {}; PROCESS_INFORMATION proc_info_hooked_long_str = {}; PROCESS_INFORMATION proc_info_hooked_long_data = {};
    ApiHookSystemMock hook_system_mock;
    // test variable
    STARTUPINFO si = {};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_FORCEOFFFEEDBACK | STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_SHOWDEFAULT;
    DWORD create_flag = CREATE_DEFAULT_ERROR_MODE | NORMAL_PRIORITY_CLASS;
    CStringCommonTestI str_testI_object;
    // first call test API when DisableAllApis().
    ASSERT_TRUE(g_hook_test_object->DisableAllApis());
    // test when lpApplicationName is nullptr
    SetLastError(0); EXPECT_CALL(hook_system_mock, CreateProcess(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::CreateProcess(nullptr, nullptr, nullptr, nullptr, false, create_flag, nullptr, nullptr, &si, &proc_info_pre_invalid_str)));
    pre_bexit_invalid_str = hook_system_mock.CreateProcess(nullptr, nullptr, nullptr, nullptr, false, create_flag, nullptr, nullptr, &si, &proc_info_pre_invalid_str);
    pre_error_invalid_str = GetLastError();
    // test when lpApplicationName is "\0"
    SetLastError(0); EXPECT_CALL(hook_system_mock, CreateProcess(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::CreateProcess(std::get<const std::string>(str_testI_object.NullString(1)).c_str(), nullptr, nullptr, nullptr, false, create_flag, nullptr, nullptr, &si, &proc_info_pre_null_str)));
    pre_bexit_null_str = hook_system_mock.CreateProcess(nullptr, nullptr, nullptr, nullptr, false, create_flag, nullptr, nullptr, &si, &proc_info_pre_null_str);
    pre_berror_null_str = GetLastError();
    // test when lpApplicationName is normal name "C:\\windows\\notepad.exe"
    SetLastError(0); EXPECT_CALL(hook_system_mock, CreateProcess(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::CreateProcess(std::get<const std::string>(str_testI_object.NormalString(std::string("C:\\windows\\notepad.exe"))).c_str(), nullptr, nullptr, nullptr, false, create_flag, nullptr, nullptr, &si, &proc_info_pre_short_str)));
    pre_bexit_short_str = hook_system_mock.CreateProcess(nullptr, nullptr, nullptr, nullptr, false, create_flag, nullptr, nullptr, &si, &proc_info_pre_short_str);
    pre_error_short_str = GetLastError();
    // test when lpApplicationName is invalid dll name and super long name.
    SetLastError(0); EXPECT_CALL(hook_system_mock, CreateProcess(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::CreateProcess(std::get<const std::string>(str_testI_object.SuperLongString(std::string("C:\\windows\\notepad.exe"))).c_str(), nullptr, nullptr, nullptr, false, create_flag, nullptr, nullptr, &si, &proc_info_pre_long_str)));
    pre_bexit_long_str = hook_system_mock.CreateProcess(nullptr, nullptr, nullptr, nullptr, false, create_flag, nullptr, nullptr, &si, &proc_info_pre_long_str);
    pre_error_long_str = GetLastError();
    // test when lpApplicationName is incorrect data block.
    SetLastError(0); EXPECT_CALL(hook_system_mock, CreateProcess(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::CreateProcess((LPCSTR)&str_testI_object.SuperLongData()[0], nullptr, nullptr, nullptr, false, create_flag, nullptr, nullptr, &si, &proc_info_pre_long_data)));
    pre_bexit_long_data = hook_system_mock.CreateProcess(nullptr, nullptr, nullptr, nullptr, false, create_flag, nullptr, nullptr, &si, &proc_info_pre_long_data);
    pre_error_long_data = GetLastError();

    // initialize
    std::vector<std::string> action_list;
    action_list.push_back("P72");
    g_server_object->AddLogCountMap(action_list);
    // second call test API when EnableAllApis().
    ASSERT_TRUE(g_hook_test_object->EnableAllApis());
    // test when lpApplicationName is nullptr
    SetLastError(0); EXPECT_CALL(hook_system_mock, CreateProcess(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::CreateProcess(nullptr, nullptr, nullptr, nullptr, false, create_flag, nullptr, nullptr, &si, &proc_info_hooked_invalid_str)));
    hooked_bexit_invalid_str = hook_system_mock.CreateProcess(nullptr, nullptr, nullptr, nullptr, false, create_flag, nullptr, nullptr, &si, &proc_info_hooked_invalid_str);
    hooked_error_invalid_str = GetLastError();
    // test when lpApplicationName is "\0"
    SetLastError(0); EXPECT_CALL(hook_system_mock, CreateProcess(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::CreateProcess(std::get<const std::string>(str_testI_object.NullString(1)).c_str(), nullptr, nullptr, nullptr, false, create_flag, nullptr, nullptr, &si, &proc_info_hooked_null_str)));
    hooked_bexit_null_str = hook_system_mock.CreateProcess(nullptr, nullptr, nullptr, nullptr, false, create_flag, nullptr, nullptr, &si, &proc_info_hooked_null_str);
    hooked_berror_null_str = GetLastError();
    // test when lpApplicationName is normal name "C:\\windows\\notepad.exe"
    ::CreateProcess(std::get<const std::string>(str_testI_object.NormalString(std::string("C:\\windows\\notepad.exe"))).c_str(), nullptr, nullptr, nullptr, false, create_flag, nullptr, nullptr, &si, &proc_info_hooked_short_str);
    SetLastError(0); EXPECT_CALL(hook_system_mock, CreateProcess(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::CreateProcess(std::get<const std::string>(str_testI_object.NormalString(std::string("C:\\windows\\notepad.exe"))).c_str(), nullptr, nullptr, nullptr, false, create_flag, nullptr, nullptr, &si, &proc_info_hooked_short_str)));
    hooked_bexit_short_str = hook_system_mock.CreateProcess(nullptr, nullptr, nullptr, nullptr, false, create_flag, nullptr, nullptr, &si, &proc_info_hooked_short_str);
    hooked_error_short_str = GetLastError();
    // test when lpApplicationName is invalid dll name and super long name.
    SetLastError(0); EXPECT_CALL(hook_system_mock, CreateProcess(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::CreateProcess(std::get<const std::string>(str_testI_object.SuperLongString(std::string("C:\\windows\\notepad.exe"))).c_str(), nullptr, nullptr, nullptr, false, create_flag, nullptr, nullptr, &si, &proc_info_hooked_long_str)));
    hooked_bexit_long_str = hook_system_mock.CreateProcess(nullptr, nullptr, nullptr, nullptr, false, create_flag, nullptr, nullptr, &si, &proc_info_hooked_long_str);
    hooked_error_long_str = GetLastError();
    // test when lpApplicationName is incorrect data block.
    SetLastError(0); EXPECT_CALL(hook_system_mock, CreateProcess(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::CreateProcess((LPCSTR)&str_testI_object.SuperLongData()[0], nullptr, nullptr, nullptr, false, create_flag, nullptr, nullptr, &si, &proc_info_hooked_long_data)));
    hooked_bexit_long_data = hook_system_mock.CreateProcess(nullptr, nullptr, nullptr, nullptr, false, create_flag, nullptr, nullptr, &si, &proc_info_hooked_long_data);
    hooked_error_long_data = GetLastError();
    // wait for all logs received.
    std::vector<int> count_list;
    count_list.push_back(0); // because only 2 times success.
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

TEST_F(HookSystemTest, CreateFile_Exploit_Test)
{
    HANDLE pre_bexit_filename;
    DWORD pre_error_filename;
    HANDLE hooked_bexit_filename;
    DWORD hooked_error_filename;
    ApiHookSystemMock hook_system_mock;
    // test variable
    DWORD dwDesiredAccess = GENERIC_READ;
    DWORD dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
    DWORD dwCreationDisposition = OPEN_EXISTING;
    DWORD dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL;
    // first call test API when DisableAllApis().
    ASSERT_TRUE(g_hook_test_object->DisableAllApis());
    // test when FileName is C:\\windows\\notepad.exe
    SetLastError(0); EXPECT_CALL(hook_system_mock, CreateFile(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::CreateFile("C:\\windows\\notepad.exe", dwDesiredAccess, dwShareMode, nullptr, dwCreationDisposition, dwFlagsAndAttributes, nullptr)));
    pre_bexit_filename = hook_system_mock.CreateFile("C:\\windows\\notepad.exe", dwDesiredAccess, dwShareMode, nullptr, dwCreationDisposition, dwFlagsAndAttributes, nullptr);
    pre_error_filename = GetLastError();
    if (pre_bexit_filename != nullptr &&
        pre_bexit_filename != INVALID_HANDLE_VALUE) 
        CloseHandle(pre_bexit_filename);

    // initialize
    std::vector<std::string> action_list;
    action_list.push_back("P72");
    g_server_object->AddLogCountMap(action_list);
    // second call test API when EnableAllApis().
    ASSERT_TRUE(g_hook_test_object->EnableAllApis());
    // test when FileName is C:\\windows\\notepad.exe
    SetLastError(0); EXPECT_CALL(hook_system_mock, CreateFile(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::CreateFile("C:\\windows\\notepad.exe", dwDesiredAccess, dwShareMode, nullptr, dwCreationDisposition, dwFlagsAndAttributes, nullptr)));
    hooked_bexit_filename = hook_system_mock.CreateFile("C:\\windows\\notepad.exe", dwDesiredAccess, dwShareMode, nullptr, dwCreationDisposition, dwFlagsAndAttributes, nullptr);
    hooked_error_filename = GetLastError();
    if (hooked_bexit_filename != nullptr &&
        hooked_bexit_filename != INVALID_HANDLE_VALUE)
        CloseHandle(hooked_bexit_filename);
    // wait for all logs received.
    std::vector<int> count_list;
    count_list.push_back(0); // because only 2 times success.
    EXPECT_EQ(g_server_object->WaitLogCountMap(count_list, 5), TRUE);

    // compare return result and error code.
    EXPECT_EQ(pre_error_filename, hooked_error_filename);
}

TEST_F(HookSystemTest, RtlAllocateHeap_Prelog_Size_Test)
{
    char* pre_bexit_0_size = nullptr; char* pre_bexit_small_size = nullptr; char* pre_bexit_big_size = nullptr;
    DWORD pre_error_0_size, pre_error_small_size, pre_error_big_size;
    char* hooked_bexit_0_size = nullptr; char* hooked_bexit_small_size = nullptr; char* hooked_bexit_big_size = nullptr;
    DWORD hooked_error_0_size, hooked_error_small_size, hooked_error_big_size;
    ApiHookSystemMock hook_system_mock;

    // test variable
    // first call test API when DisableAllApis().
    ASSERT_TRUE(g_hook_test_object->DisableAllApis());
    // test when Size is 0
    SetLastError(0);
    pre_bexit_0_size = new char[0];
    pre_error_0_size = GetLastError();
    // test when Size is 1024 bytes
    SetLastError(0);
    pre_bexit_small_size = new char[256];
    pre_error_small_size = GetLastError();
    // test when Size is 200M
    SetLastError(0);
    pre_bexit_big_size = new char[200 * 1024 * 1024];
    pre_error_big_size = GetLastError();
    if (pre_bexit_0_size) delete[] pre_bexit_0_size;
    if (pre_bexit_small_size) delete[] pre_bexit_small_size;
    if (pre_bexit_big_size) delete[] pre_bexit_big_size;

    // initialize
    std::vector<std::string> action_list;
    action_list.push_back("P85");
    g_server_object->AddLogCountMap(action_list);
    // second call test API when EnableAllApis().
    ASSERT_TRUE(g_hook_test_object->EnableAllApis());
    // test when Size is 0
    SetLastError(0);
    hooked_bexit_0_size = new char[0];
    hooked_error_0_size = GetLastError();
    // test when Size is 1024 bytes
    SetLastError(0);
    hooked_bexit_small_size = new char[256];
    hooked_error_small_size = GetLastError();
    // test when Size is 200M
    SetLastError(0);
    hooked_bexit_big_size = new char[200 * 1024 * 1024];
    hooked_error_big_size = GetLastError();
    if (hooked_bexit_0_size) delete[] hooked_bexit_0_size;
    if (hooked_bexit_small_size) delete[] hooked_bexit_small_size;
    if (hooked_bexit_big_size) delete[] hooked_bexit_big_size;
    // wait for all logs received.
    std::vector<int> count_list;
    count_list.push_back(0); // because only 2 times success.
    EXPECT_EQ(g_server_object->WaitLogCountMap(count_list, 5), TRUE);
}

TEST_F(HookSystemTest, NetGetJoinInformation_Postlog_lpServer_Test)
{
    NET_API_STATUS pre_bexit_0_lpServer;
    DWORD pre_error_0_lpServer;
    NET_API_STATUS hooked_bexit_0_lpServer;
    DWORD hooked_error_0_lpServer;
    ApiHookSystemMock hook_system_mock;

    // test variable
    LPWSTR lpNameBuffer = nullptr;
    NETSETUP_JOIN_STATUS join_status;
    // first call test API when DisableAllApis().
    ASSERT_TRUE(g_hook_test_object->DisableAllApis());
    // test when lpServer is nullptr
    SetLastError(0); EXPECT_CALL(hook_system_mock, NetGetJoinInformation(testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::NetGetJoinInformation(nullptr, &lpNameBuffer, &join_status)));
    pre_bexit_0_lpServer = hook_system_mock.NetGetJoinInformation(nullptr, &lpNameBuffer, &join_status);
    pre_error_0_lpServer = GetLastError();

    // initialize
    std::vector<std::string> action_list;
    action_list.push_back("P92");
    g_server_object->AddLogCountMap(action_list);
    // second call test API when EnableAllApis().
    ASSERT_TRUE(g_hook_test_object->EnableAllApis());
    // test when lpServer is nullptr
    SetLastError(0); EXPECT_CALL(hook_system_mock, NetGetJoinInformation(testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::NetGetJoinInformation(nullptr, &lpNameBuffer, &join_status)));
    pre_bexit_0_lpServer = hook_system_mock.NetGetJoinInformation(nullptr, &lpNameBuffer, &join_status);
    pre_error_0_lpServer = GetLastError();

    // compare return result and error code.
    EXPECT_EQ(pre_bexit_0_lpServer, pre_bexit_0_lpServer);
    EXPECT_EQ(pre_error_0_lpServer, pre_error_0_lpServer);
    // wait for all logs received.
    std::vector<int> count_list;
    count_list.push_back(1);
    EXPECT_EQ(g_server_object->WaitLogCountMap(count_list, 5), TRUE);
}

TEST_F(HookSystemTest, NetUserGetInfo_servername_Test)
{
    NET_API_STATUS pre_bexit_0_servername;
    DWORD pre_error_0_servername;
    NET_API_STATUS hooked_bexit_0_servername;
    DWORD hooked_error_0_servername;
    ApiHookSystemMock hook_system_mock;

    // test variable
    LPBYTE  bufptr = nullptr;
    // first call test API when DisableAllApis().
    ASSERT_TRUE(g_hook_test_object->DisableAllApis());
    // test when servername is nullptr
    SetLastError(0); EXPECT_CALL(hook_system_mock, NetUserGetInfo(testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::NetUserGetInfo(nullptr, L"Guest", 0, &bufptr)));
    pre_bexit_0_servername = hook_system_mock.NetUserGetInfo(nullptr, L"Guest", 0, &bufptr);
    pre_error_0_servername = GetLastError();
    if (bufptr) NetApiBufferFree(bufptr); bufptr = nullptr;

    // initialize
    std::vector<std::string> action_list;
    action_list.push_back("P93");
    g_server_object->AddLogCountMap(action_list);
    // second call test API when EnableAllApis().
    ASSERT_TRUE(g_hook_test_object->EnableAllApis());
    // test when servername is nullptr
    SetLastError(0); EXPECT_CALL(hook_system_mock, NetUserGetInfo(testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::NetUserGetInfo(nullptr, L"Guest", 0, &bufptr)));
    hooked_bexit_0_servername = hook_system_mock.NetUserGetInfo(nullptr, L"Guest", 0, &bufptr);
    hooked_error_0_servername = GetLastError();
    if (bufptr) NetApiBufferFree(bufptr); bufptr = nullptr;

    // compare return result and error code.
    EXPECT_EQ(pre_bexit_0_servername, hooked_bexit_0_servername);
    EXPECT_EQ(pre_error_0_servername, hooked_error_0_servername);
    // wait for all logs received.
    std::vector<int> count_list;
    count_list.push_back(1);
    EXPECT_EQ(g_server_object->WaitLogCountMap(count_list, 5), TRUE);
}

TEST_F(HookSystemTest, NetUserGetLocalGroups_servername_Test)
{
    NET_API_STATUS pre_bexit_0_servername;
    DWORD pre_error_0_servername;
    NET_API_STATUS hooked_bexit_0_servername;
    DWORD hooked_error_0_servername;
    ApiHookSystemMock hook_system_mock;

    // test variable
    LPBYTE  bufptr = nullptr;
    DWORD entriesread = 0;
    DWORD totalentries = 0;
    // first call test API when DisableAllApis().
    ASSERT_TRUE(g_hook_test_object->DisableAllApis());
    // test when servername is nullptr
    SetLastError(0); EXPECT_CALL(hook_system_mock, NetUserGetLocalGroups(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::NetUserGetLocalGroups(nullptr, L"Guest", 0, LG_INCLUDE_INDIRECT, &bufptr, MAX_PREFERRED_LENGTH, &entriesread, &totalentries)));
    pre_bexit_0_servername = hook_system_mock.NetUserGetLocalGroups(nullptr, L"Guest", 0, LG_INCLUDE_INDIRECT, &bufptr, MAX_PREFERRED_LENGTH, &entriesread, &totalentries);
    pre_error_0_servername = GetLastError();
    if (bufptr) NetApiBufferFree(bufptr); bufptr = nullptr;
    // initialize
    std::vector<std::string> action_list;
    action_list.push_back("P94");
    g_server_object->AddLogCountMap(action_list);
    // second call test API when EnableAllApis().
    ASSERT_TRUE(g_hook_test_object->EnableAllApis());
    // test when servername is nullptr
    SetLastError(0); EXPECT_CALL(hook_system_mock, NetUserGetLocalGroups(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::NetUserGetLocalGroups(nullptr, L"Guest", 0, LG_INCLUDE_INDIRECT, &bufptr, MAX_PREFERRED_LENGTH, &entriesread, &totalentries)));
    hooked_bexit_0_servername = hook_system_mock.NetUserGetLocalGroups(nullptr, L"Guest", 0, LG_INCLUDE_INDIRECT, &bufptr, MAX_PREFERRED_LENGTH, &entriesread, &totalentries);
    hooked_error_0_servername = GetLastError();
    if (bufptr) NetApiBufferFree(bufptr); bufptr = nullptr;

    // compare return result and error code.
    EXPECT_EQ(pre_bexit_0_servername, hooked_bexit_0_servername);
    EXPECT_EQ(pre_error_0_servername, hooked_error_0_servername);
    // wait for all logs received.
    std::vector<int> count_list;
    count_list.push_back(1);
    EXPECT_EQ(g_server_object->WaitLogCountMap(count_list, 5), TRUE);
}

TEST_F(HookSystemTest, NetShareEnum_servername_Test)
{
    NET_API_STATUS pre_bexit_0_servername;
    DWORD pre_error_0_servername;
    NET_API_STATUS hooked_bexit_0_servername;
    DWORD hooked_error_0_servername;
    ApiHookSystemMock hook_system_mock;

    // test variable
    LPBYTE  bufptr = nullptr;
    DWORD entriesread = 0;
    DWORD totalentries = 0;
    DWORD resume_handle = 0;
    // first call test API when DisableAllApis().
    ASSERT_TRUE(g_hook_test_object->DisableAllApis());
    // test when servername is nullptr
    SetLastError(0); EXPECT_CALL(hook_system_mock, NetShareEnum(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::NetShareEnum(nullptr, 0, &bufptr, MAX_PREFERRED_LENGTH, &entriesread, &totalentries, &resume_handle)));
    pre_bexit_0_servername = hook_system_mock.NetShareEnum(nullptr, 0, &bufptr, MAX_PREFERRED_LENGTH, &entriesread, &totalentries, &resume_handle);
    pre_error_0_servername = GetLastError();
    if (bufptr) NetApiBufferFree(bufptr); bufptr = nullptr;
    // initialize
    std::vector<std::string> action_list;
    action_list.push_back("P95");
    g_server_object->AddLogCountMap(action_list);
    // second call test API when EnableAllApis().
    ASSERT_TRUE(g_hook_test_object->EnableAllApis());
    // test when servername is nullptr
    SetLastError(0); EXPECT_CALL(hook_system_mock, NetShareEnum(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::NetShareEnum(nullptr, 0, &bufptr, MAX_PREFERRED_LENGTH, &entriesread, &totalentries, &resume_handle)));
    hooked_bexit_0_servername = hook_system_mock.NetShareEnum(nullptr, 0, &bufptr, MAX_PREFERRED_LENGTH, &entriesread, &totalentries, &resume_handle);
    hooked_error_0_servername = GetLastError();
    if (bufptr) NetApiBufferFree(bufptr); bufptr = nullptr;

    // compare return result and error code.
    EXPECT_EQ(pre_bexit_0_servername, hooked_bexit_0_servername);
    EXPECT_EQ(pre_error_0_servername, hooked_error_0_servername);
    // wait for all logs received.
    std::vector<int> count_list;
    count_list.push_back(1);
    EXPECT_EQ(g_server_object->WaitLogCountMap(count_list, 5), TRUE);
}

TEST_F(HookSystemTest, InternetOpen_lpszAgent_Test)
{
    LPVOID pre_bexit_0_lpszAgent;
    DWORD pre_error_0_lpszAgent;
    LPVOID hooked_bexit_0_lpszAgent;
    DWORD hooked_error_0_lpszAgent;
    ApiHookSystemMock hook_system_mock;

    // test variable
    char lpszAgent[] = {"Mozilla/4.0 (compatible)"};
    // first call test API when DisableAllApis().
    ASSERT_TRUE(g_hook_test_object->DisableAllApis());
    SetLastError(0); EXPECT_CALL(hook_system_mock, InternetOpen(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::InternetOpen(lpszAgent, INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0)));
    pre_bexit_0_lpszAgent = hook_system_mock.InternetOpen(lpszAgent, INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);
    pre_error_0_lpszAgent = GetLastError();

    // initialize
    std::vector<std::string> action_list;
    action_list.push_back("P96");
    g_server_object->AddLogCountMap(action_list);
    // second call test API when EnableAllApis().
    ASSERT_TRUE(g_hook_test_object->EnableAllApis());
    SetLastError(0); EXPECT_CALL(hook_system_mock, InternetOpen(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::InternetOpen(lpszAgent, INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0)));
    hooked_bexit_0_lpszAgent = hook_system_mock.InternetOpen(lpszAgent, INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);
    hooked_error_0_lpszAgent = GetLastError();

    // compare return result and error code.
    EXPECT_NE(pre_bexit_0_lpszAgent, nullptr);
    EXPECT_NE(hooked_bexit_0_lpszAgent, nullptr);
    if (pre_bexit_0_lpszAgent) InternetCloseHandle(pre_bexit_0_lpszAgent);
    if (hooked_bexit_0_lpszAgent) InternetCloseHandle(hooked_bexit_0_lpszAgent);
    EXPECT_EQ(pre_error_0_lpszAgent, hooked_error_0_lpszAgent);
    // wait for all logs received.
    std::vector<int> count_list;
    count_list.push_back(1);
    EXPECT_EQ(g_server_object->WaitLogCountMap(count_list, 5), TRUE);
}

TEST_F(HookSystemTest, InternetOpenUrl_hInternet_Test)
{
    LPVOID pre_bexit_0_hInternet;
    DWORD pre_error_0_hInternet;
    LPVOID hooked_bexit_0_hInternet;
    DWORD hooked_error_0_hInternet;
    ApiHookSystemMock hook_system_mock;

    // test variable
    char lpszAgent[] = { "Mozilla/4.0 (compatible)" };
    // first call test API when DisableAllApis().
    ASSERT_TRUE(g_hook_test_object->DisableAllApis());
    LPVOID hInternet = InternetOpen(lpszAgent, INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);
    if (!hInternet)
        return;
    SetLastError(0); EXPECT_CALL(hook_system_mock, InternetOpenUrl(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::InternetOpenUrl(hInternet, "http://www.google.com", 0, 0, INTERNET_FLAG_RAW_DATA, 0)));
    pre_bexit_0_hInternet = hook_system_mock.InternetOpenUrl(hInternet, "http://www.google.com", 0, 0, INTERNET_FLAG_RAW_DATA, 0);
    pre_error_0_hInternet = GetLastError();

    // initialize
    std::vector<std::string> action_list;
    action_list.push_back("P97");
    g_server_object->AddLogCountMap(action_list);
    // second call test API when EnableAllApis().
    ASSERT_TRUE(g_hook_test_object->EnableAllApis());
    SetLastError(0); EXPECT_CALL(hook_system_mock, InternetOpenUrl(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::InternetOpenUrl(hInternet, "http://www.google.com", 0, 0, INTERNET_FLAG_RAW_DATA, 0)));
    hooked_bexit_0_hInternet = hook_system_mock.InternetOpenUrl(hInternet, "http://www.google.com", 0, 0, INTERNET_FLAG_RAW_DATA, 0);
    hooked_error_0_hInternet = GetLastError();

    // compare return result and error code.
    InternetCloseHandle(hInternet);
    EXPECT_NE(pre_bexit_0_hInternet, nullptr);
    EXPECT_NE(hooked_bexit_0_hInternet, nullptr);
    if (pre_bexit_0_hInternet) InternetCloseHandle(pre_bexit_0_hInternet);
    if (hooked_bexit_0_hInternet) InternetCloseHandle(hooked_bexit_0_hInternet);
    EXPECT_EQ(pre_error_0_hInternet, hooked_error_0_hInternet);
    // wait for all logs received.
    std::vector<int> count_list;
    count_list.push_back(1);
    EXPECT_EQ(g_server_object->WaitLogCountMap(count_list, 5), TRUE);
}

TEST_F(HookSystemTest, InternetReadFile_hFile_Test)
{
    BOOL pre_bexit_0_hFile;
    DWORD pre_error_0_hFile;
    BOOL hooked_bexit_0_hFile;
    DWORD hooked_error_0_hFile;
    ApiHookSystemMock hook_system_mock;

    // test variable
    char lpszAgent[] = { "Mozilla/4.0 (compatible)" };
    DWORD nSize = 256;
    BYTE lpBuffer[256] = { 0 };
    // first call test API when DisableAllApis().
    ASSERT_TRUE(g_hook_test_object->DisableAllApis());
    LPVOID hInternet = InternetOpen(lpszAgent, INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);
    if (!hInternet)
        return;
    LPVOID hUrl = InternetOpenUrl(hInternet, "http://www.google.com", 0, 0, INTERNET_FLAG_RAW_DATA, 0);
    if (!hUrl)
        return;
    SetLastError(0); EXPECT_CALL(hook_system_mock, InternetReadFile(testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::InternetReadFile(hUrl, lpBuffer, nSize, &nSize)));
    pre_bexit_0_hFile = hook_system_mock.InternetReadFile(hUrl, lpBuffer, nSize, &nSize);
    pre_error_0_hFile = GetLastError();

    // initialize
    std::vector<std::string> action_list;
    action_list.push_back("P98");
    g_server_object->AddLogCountMap(action_list);
    // second call test API when EnableAllApis().
    ASSERT_TRUE(g_hook_test_object->EnableAllApis());
    SetLastError(0); EXPECT_CALL(hook_system_mock, InternetReadFile(testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::InternetReadFile(hUrl, lpBuffer, nSize, &nSize)));
    hooked_bexit_0_hFile = hook_system_mock.InternetReadFile(hUrl, lpBuffer, nSize, &nSize);
    hooked_error_0_hFile = GetLastError();

    // compare return result and error code.
    InternetCloseHandle(hInternet);
    InternetCloseHandle(hUrl);
    EXPECT_EQ(pre_bexit_0_hFile, hooked_bexit_0_hFile);
    EXPECT_EQ(pre_error_0_hFile, hooked_error_0_hFile);
    // wait for all logs received.
    std::vector<int> count_list;
    count_list.push_back(1);
    EXPECT_EQ(g_server_object->WaitLogCountMap(count_list, 5), TRUE);
}

TEST_F(HookSystemTest, InternetCrackUrl_lpszUrl_Test)
{
    BOOL pre_bexit_0_lpszUrl;
    DWORD pre_error_0_lpszUrl;
    BOOL hooked_bexit_0_lpszUrl;
    DWORD hooked_error_0_lpszUrl;
    ApiHookSystemMock hook_system_mock;

    // test variable
    URL_COMPONENTS url;
    memset(&url, 0, sizeof(url));
    url.dwStructSize = sizeof(url);
    url.dwHostNameLength = 1;
    url.dwUserNameLength = 1;
    url.dwPasswordLength = 1;
    url.dwUrlPathLength = 1;
    char lpszUrl[] = { "https://docs.microsoft.com/en-us/windows/win32/api/wininet/nf-wininet-internetcrackurla" };
    // first call test API when DisableAllApis().
    ASSERT_TRUE(g_hook_test_object->DisableAllApis());
    SetLastError(0); EXPECT_CALL(hook_system_mock, InternetCrackUrl(testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::InternetCrackUrl(lpszUrl, sizeof(lpszUrl), 0, &url)));
    pre_bexit_0_lpszUrl = hook_system_mock.InternetCrackUrl(lpszUrl, sizeof(lpszUrl), 0, &url);
    pre_error_0_lpszUrl = GetLastError();

    // initialize
    std::vector<std::string> action_list;
    action_list.push_back("P100");
    g_server_object->AddLogCountMap(action_list);
    // second call test API when EnableAllApis().
    memset(&url, 0, sizeof(url));
    url.dwStructSize = sizeof(url);
    url.dwHostNameLength = 1;
    url.dwUserNameLength = 1;
    url.dwPasswordLength = 1;
    url.dwUrlPathLength = 1;
    ASSERT_TRUE(g_hook_test_object->EnableAllApis());
    SetLastError(0); EXPECT_CALL(hook_system_mock, InternetCrackUrl(testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::InternetCrackUrl(lpszUrl, sizeof(lpszUrl), 0, &url)));
    hooked_bexit_0_lpszUrl = hook_system_mock.InternetCrackUrl(lpszUrl, sizeof(lpszUrl), 0, &url);
    hooked_error_0_lpszUrl = GetLastError();

    // compare return result and error code.
    EXPECT_EQ(pre_bexit_0_lpszUrl, hooked_bexit_0_lpszUrl);
    EXPECT_EQ(pre_error_0_lpszUrl, hooked_error_0_lpszUrl);
    // wait for all logs received.
    std::vector<int> count_list;
    count_list.push_back(1);
    EXPECT_EQ(g_server_object->WaitLogCountMap(count_list, 5), TRUE);
}

TEST_F(HookSystemTest, InternetConnect_hInternet_Test)
{
    LPVOID pre_bexit_0_hInternet, pre_bexit_0_request;
    DWORD pre_error_0_hInternet, pre_error_0_request;
    LPVOID hooked_bexit_0_hInternet, hooked_bexit_0_request;
    DWORD hooked_error_0_hInternet, hooked_error_0_request;
    ApiHookSystemMock hook_system_mock;

    // test variable
    char lpszAgent[] = { "Mozilla/4.0 (compatible)" };
    char lpszUrl[] = { "https://docs.microsoft.com/en-us/windows/win32/api/wininet/nf-wininet-internetcrackurla" };
    // first call test API when DisableAllApis().
    ASSERT_TRUE(g_hook_test_object->DisableAllApis());
    LPVOID hInternet = InternetOpen(lpszAgent, INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);
    if (!hInternet)
        return;
    SetLastError(0); EXPECT_CALL(hook_system_mock, InternetConnect(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::InternetConnect(hInternet, lpszUrl, 0, nullptr, nullptr, INTERNET_SERVICE_HTTP, 0, 0)));
    pre_bexit_0_hInternet = hook_system_mock.InternetConnect(hInternet, lpszUrl, 0, nullptr, nullptr, INTERNET_SERVICE_HTTP, 0, 0);
    pre_error_0_hInternet = GetLastError();

    if (pre_bexit_0_hInternet) {
        LPCSTR lplpszAcceptTypes[] = { _T("text/*"), NULL };
        SetLastError(0); EXPECT_CALL(hook_system_mock, HttpOpenRequest(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::HttpOpenRequest(pre_bexit_0_hInternet, nullptr, lpszUrl, nullptr, nullptr, lplpszAcceptTypes, INTERNET_FLAG_NO_UI, 0)));
        pre_bexit_0_request = hook_system_mock.HttpOpenRequest(pre_bexit_0_hInternet, nullptr, lpszUrl, nullptr, nullptr, lplpszAcceptTypes, INTERNET_FLAG_NO_UI, 0);
        pre_error_0_request = GetLastError();
    }

    // initialize
    std::vector<std::string> action_list;
    action_list.push_back("P101");
    g_server_object->AddLogCountMap(action_list);
    // second call test API when EnableAllApis().
    ASSERT_TRUE(g_hook_test_object->EnableAllApis());
    SetLastError(0); EXPECT_CALL(hook_system_mock, InternetConnect(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::InternetConnect(hInternet, lpszUrl, 0, nullptr, nullptr, INTERNET_SERVICE_HTTP, 0, 0)));
    hooked_bexit_0_hInternet = hook_system_mock.InternetConnect(hInternet, lpszUrl, 0, nullptr, nullptr, INTERNET_SERVICE_HTTP, 0, 0);
    hooked_error_0_hInternet = GetLastError();

    if (hooked_bexit_0_hInternet) {
        LPCSTR lplpszAcceptTypes[] = { _T("text/*"), NULL };
        SetLastError(0); EXPECT_CALL(hook_system_mock, HttpOpenRequest(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1).WillRepeatedly(testing::Return(::HttpOpenRequest(pre_bexit_0_hInternet, nullptr, lpszUrl, nullptr, nullptr, lplpszAcceptTypes, INTERNET_FLAG_NO_UI, 0)));
        hooked_bexit_0_request = hook_system_mock.HttpOpenRequest(pre_bexit_0_hInternet, nullptr, lpszUrl, nullptr, nullptr, lplpszAcceptTypes, INTERNET_FLAG_NO_UI, 0);
        hooked_error_0_request = GetLastError();
    }

    // compare return result and error code.
    InternetCloseHandle(hInternet);
    EXPECT_NE(pre_bexit_0_hInternet, nullptr);
    EXPECT_NE(hooked_bexit_0_hInternet, nullptr);
    if (pre_bexit_0_hInternet)InternetCloseHandle(pre_bexit_0_hInternet);
    if (hooked_bexit_0_hInternet)InternetCloseHandle(hooked_bexit_0_hInternet);
    EXPECT_EQ(pre_error_0_hInternet, hooked_error_0_hInternet);
    EXPECT_NE(pre_bexit_0_request, nullptr);
    EXPECT_NE(hooked_bexit_0_request, nullptr);
    if (pre_bexit_0_request)InternetCloseHandle(pre_bexit_0_request);
    if (hooked_bexit_0_request)InternetCloseHandle(hooked_bexit_0_request);
    EXPECT_EQ(pre_error_0_request, hooked_error_0_request);
    // wait for all logs received.
    std::vector<int> count_list;
    count_list.push_back(1);
    EXPECT_EQ(g_server_object->WaitLogCountMap(count_list, 5), TRUE);
}

#endif

