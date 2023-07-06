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
class HookWmiMethodsTest : public testing::Test
{
protected:
    HookWmiMethodsTest() { }
    ~HookWmiMethodsTest() override {}

    virtual void SetUp() override {
        HRESULT hr;
        ASSERT_TRUE(g_hook_test_object->EnableAllApis());
        hr = CoInitialize(nullptr);
        ASSERT_TRUE(SUCCEEDED(hr));
        hr = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&m_wbemLoc);
        ASSERT_TRUE(SUCCEEDED(hr));
        hr = m_wbemLoc->ConnectServer(CComBSTR(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &m_wbemSvc);
        ASSERT_TRUE(SUCCEEDED(hr));
        hr = CoSetProxyBlanket(m_wbemSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
        ASSERT_TRUE(SUCCEEDED(hr));
    }

    void TearDown() override {
        ASSERT_TRUE(g_hook_test_object->DisableAllApis());
        if (m_wbemSvc)
        {
            m_wbemSvc->Release();
            m_wbemSvc = nullptr;
        }
        if (m_wbemLoc)
        {
            m_wbemLoc->Release();
            m_wbemLoc = nullptr;
        }
    }

    bool GetValueString(VARIANT& Value, std::wstring& ValueString)
    {
        switch (Value.vt)
        {
        case VT_BSTR:
            ValueString = Value.bstrVal;
            return true;
        case VT_BOOL:
            if (Value.boolVal)
                ValueString = L"true";
            else
                ValueString = L"false";
            return true;
        case VT_UI1:
        case VT_I1:
            ValueString = std::to_wstring(Value.bVal);
            return true;
        case VT_I2:
        case VT_INT:
            ValueString = std::to_wstring(Value.iVal);
            return true;
        case VT_I4:
            ValueString = std::to_wstring(Value.intVal);
            return true;
        case VT_I8:
            ValueString = std::to_wstring(Value.llVal);
            return true;
        case VT_UI2:
        case VT_UINT:
            ValueString = std::to_wstring(Value.uiVal);
            return true;
        case VT_UI4:
            ValueString = std::to_wstring(Value.ulVal);
            return true;
        case VT_UI8:
            ValueString = std::to_wstring(Value.ullVal);
            return true;
        case VT_DATE:
            ValueString = std::to_wstring(Value.date);
            return true;
        case VT_VOID:
            ValueString = std::to_wstring((ULONG_PTR)Value.byref);
            return true;
        default:
            ValueString = L"";
            break;
        }

        return false;
    }

    IWbemLocator* m_wbemLoc = nullptr;
    IEnumWbemClassObject* m_enumClsObj = nullptr;
    IWbemClassObject* m_wbemClsObj = nullptr;
    IWbemServices* m_wbemSvc = nullptr;
};

TEST_F(HookWmiMethodsTest, RemoteTest)
{
#define service_ip L"192.168.0.1 "
const CLSID CLSID_Account = { 0x5939F232,0x7B5F,0x11D4,{0x8B,0xEC,0x34,0x8F,0xBB,0x00,0x00,0x00} };

    COSERVERINFO si;
    memset(&si, 0, sizeof(COSERVERINFO));
    si.pwszName = service_ip;
    si.pAuthInfo = nullptr;
    si.dwReserved1 = 0;
    si.dwReserved2 = 0;
    MULTI_QI qi[1];
    qi[0].pIID = &IID_IUnknown;
    qi[0].pItf = NULL;
    qi[0].hr = S_OK;

    HRESULT hr = CoCreateInstanceEx(CLSID_WbemLocator, NULL, CLSCTX_REMOTE_SERVER, &si, 1, nullptr);
}

TEST_F(HookWmiMethodsTest, Win32DiskDriveTest)
{
    CComBSTR query("SELECT * FROM ");
    VARIANT vtProp;
    ULONG uReturn = 0;
    HRESULT hr;
    BOOL bRet = FALSE;
    std::wstring chRetValue;
    IEnumWbemClassObject* pEnumClsObj = nullptr;
    IWbemClassObject* pWbemClsObj = nullptr;

    //initialize
    std::vector<std::string> action_list;
    action_list.push_back("W1");
    action_list.push_back("W2");
    action_list.push_back("W6");
    g_server_object->AddLogCountMap(action_list);
    query += CComBSTR("Win32_DiskDrive");
    hr = m_wbemSvc->ExecQuery(CComBSTR("WQL"), query, WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        0, &pEnumClsObj);
    ASSERT_TRUE(SUCCEEDED(hr));
    VariantInit(&vtProp);
    hr = pEnumClsObj->Next(WBEM_INFINITE, 1, &pWbemClsObj, &uReturn);
    ASSERT_TRUE(SUCCEEDED(hr) && uReturn > 0);
    hr = pWbemClsObj->Get(CComBSTR("Size"), 0, &vtProp, 0, 0);
    ASSERT_TRUE(SUCCEEDED(hr));
    ASSERT_TRUE(GetValueString(vtProp, chRetValue));
    // check for modify size success, size = 3TG if successful.
    ASSERT_TRUE(_wcsicmp(chRetValue.c_str(), L"3298534883328") == 0);
    VariantClear(&vtProp);
    pWbemClsObj->Release();
    pEnumClsObj->Release();
    // wait for all logs received.
    std::vector<int> count_list;
    count_list.push_back(1);
    count_list.push_back(1);
    count_list.push_back(1);
    EXPECT_EQ(g_server_object->WaitLogCountMap(count_list, 5), TRUE);
}

TEST_F(HookWmiMethodsTest, Win32PhysicalMemoryArrayTest)
{
    CComBSTR query("SELECT * FROM ");
    VARIANT vtProp;
    ULONG uReturn = 0;
    HRESULT hr;
    BOOL bRet = FALSE;
    std::wstring chRetValue;
    IEnumWbemClassObject* pEnumClsObj = nullptr;
    IWbemClassObject* pWbemClsObj = nullptr;

    //initialize
    std::vector<std::string> action_list;
    action_list.push_back("W1");
    action_list.push_back("W2");
    action_list.push_back("W6");
    g_server_object->AddLogCountMap(action_list);
    query += CComBSTR("Win32_PhysicalMemoryArray");
    hr = m_wbemSvc->ExecQuery(CComBSTR("WQL"), query, WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        0, &pEnumClsObj);
    ASSERT_TRUE(SUCCEEDED(hr));
    VariantInit(&vtProp);
    hr = pEnumClsObj->Next(WBEM_INFINITE, 1, &pWbemClsObj, &uReturn);
    ASSERT_TRUE(SUCCEEDED(hr) && uReturn > 0);
    hr = pWbemClsObj->Get(CComBSTR("MemoryDevices"), 0, &vtProp, 0, 0);
    if (hr == S_FALSE) return;
    ASSERT_TRUE(GetValueString(vtProp, chRetValue));
    // check for MemoryDevices >= 1.
    ASSERT_TRUE(_wcsicmp(chRetValue.c_str(), L"0") != 0);
    VariantClear(&vtProp);
    pWbemClsObj->Release();
    pEnumClsObj->Release();
    // wait for all logs received.
    std::vector<int> count_list;
    count_list.push_back(1);
    count_list.push_back(1);
    count_list.push_back(1);
    EXPECT_EQ(g_server_object->WaitLogCountMap(count_list, 5), TRUE);
}

TEST_F(HookWmiMethodsTest, Win32PhysicalMemoryTest)
{
    CComBSTR query("SELECT * FROM ");
    VARIANT vtProp;
    ULONG uReturn = 0;
    HRESULT hr;
    BOOL bRet = FALSE;
    std::wstring chRetValue;
    IEnumWbemClassObject* pEnumClsObj = nullptr;
    IWbemClassObject* pWbemClsObj = nullptr;

    //initialize
    std::vector<std::string> action_list;
    action_list.push_back("W1");
    action_list.push_back("W2");
    action_list.push_back("W6");
    g_server_object->AddLogCountMap(action_list);
    query += CComBSTR("Win32_PhysicalMemory");
    hr = m_wbemSvc->ExecQuery(CComBSTR("WQL"), query, WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        0, &pEnumClsObj);
    ASSERT_TRUE(SUCCEEDED(hr));
    VariantInit(&vtProp);
    hr = pEnumClsObj->Next(WBEM_INFINITE, 1, &pWbemClsObj, &uReturn);
    ASSERT_TRUE(SUCCEEDED(hr) && uReturn > 0);
    hr = pWbemClsObj->Get(CComBSTR("Capacity"), 0, &vtProp, 0, 0);
    ASSERT_TRUE(SUCCEEDED(hr));
    ASSERT_TRUE(GetValueString(vtProp, chRetValue));
    // check for modify size success, size = 3TG if successful.
    //ASSERT_TRUE(_wcsicmp(chRetValue.c_str(), L"17179869184") == 0);
    VariantClear(&vtProp);
    pWbemClsObj->Release();
    pEnumClsObj->Release();
    // wait for all logs received.
    std::vector<int> count_list;
    count_list.push_back(1);
    count_list.push_back(1);
    count_list.push_back(1);
    EXPECT_EQ(g_server_object->WaitLogCountMap(count_list, 5), TRUE);
}

TEST_F(HookWmiMethodsTest, CimMemoryTest)
{
    CComBSTR query("SELECT * FROM ");
    VARIANT vtProp;
    ULONG uReturn = 0;
    HRESULT hr;
    BOOL bRet = FALSE;
    std::wstring chRetValue;
    IEnumWbemClassObject* pEnumClsObj = nullptr;
    IWbemClassObject* pWbemClsObj = nullptr;

    //initialize
    std::vector<std::string> action_list;
    action_list.push_back("W1");
    action_list.push_back("W2");
    action_list.push_back("W6");
    g_server_object->AddLogCountMap(action_list);
    query += CComBSTR("CIM_Memory");
    hr = m_wbemSvc->ExecQuery(CComBSTR("WQL"), query, WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        0, &pEnumClsObj);
    ASSERT_TRUE(SUCCEEDED(hr));
    VariantInit(&vtProp);
    hr = pEnumClsObj->Next(WBEM_INFINITE, 1, &pWbemClsObj, &uReturn);
    ASSERT_TRUE(SUCCEEDED(hr) && uReturn > 0);
    hr = pWbemClsObj->Get(CComBSTR("__CLASS"), 0, &vtProp, 0, 0);
    if (hr == S_FALSE) return;
    ASSERT_TRUE(GetValueString(vtProp, chRetValue));
    VariantClear(&vtProp);
    hr = pWbemClsObj->Get(CComBSTR("Name"), 0, &vtProp, 0, 0);
    ASSERT_TRUE(SUCCEEDED(hr));
    ASSERT_TRUE(GetValueString(vtProp, chRetValue));
    VariantClear(&vtProp);
    hr = pWbemClsObj->Get(CComBSTR("Caption"), 0, &vtProp, 0, 0);
    ASSERT_TRUE(SUCCEEDED(hr));
    ASSERT_TRUE(GetValueString(vtProp, chRetValue));
    VariantClear(&vtProp);
    hr = pWbemClsObj->Get(CComBSTR("BlockSize"), 0, &vtProp, 0, 0);
    ASSERT_TRUE(SUCCEEDED(hr));
    ASSERT_TRUE(GetValueString(vtProp, chRetValue));
    VariantClear(&vtProp);
    hr = pWbemClsObj->Get(CComBSTR("NumberOfBlocks"), 0, &vtProp, 0, 0);
    ASSERT_TRUE(SUCCEEDED(hr));
    ASSERT_TRUE(GetValueString(vtProp, chRetValue));
    VariantClear(&vtProp);
    pWbemClsObj->Release();
    pEnumClsObj->Release();
    // wait for all logs received.
    std::vector<int> count_list;
    count_list.push_back(0);
    count_list.push_back(5);
    count_list.push_back(1);
    EXPECT_EQ(g_server_object->WaitLogCountMap(count_list, 5), TRUE);
}

TEST_F(HookWmiMethodsTest, MSAcpiThermalZoneTemperatureTest)
{
    CComBSTR query("SELECT * FROM ");
    VARIANT vtProp;
    ULONG uReturn = 0;
    HRESULT hr;
    BOOL bRet = FALSE;
    std::wstring chRetValue;
    IEnumWbemClassObject* pEnumClsObj = nullptr;
    IWbemClassObject* pWbemClsObj = nullptr;

    //initialize
    std::vector<std::string> action_list;
    action_list.push_back("W1");
    action_list.push_back("W2");
    action_list.push_back("W6");
    g_server_object->AddLogCountMap(action_list);
    hr = m_wbemLoc->ConnectServer(CComBSTR(L"ROOT\\WMI"), NULL, NULL, 0, NULL, 0, 0, &m_wbemSvc);
    ASSERT_TRUE(SUCCEEDED(hr));
    query += CComBSTR("MSAcpi_ThermalZoneTemperature");
    hr = m_wbemSvc->ExecQuery(CComBSTR("WQL"), query, WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        0, &pEnumClsObj);
    ASSERT_TRUE(SUCCEEDED(hr));
    VariantInit(&vtProp);
    hr = pEnumClsObj->Next(WBEM_INFINITE, 1, &pWbemClsObj, &uReturn);
    ASSERT_TRUE(SUCCEEDED(hr) && uReturn > 0);
    hr = pWbemClsObj->Get(CComBSTR("__PATH"), 0, &vtProp, 0, 0);
    if (hr == S_FALSE) return;
    ASSERT_TRUE(GetValueString(vtProp, chRetValue));
    VariantClear(&vtProp);
    hr = pWbemClsObj->Get(CComBSTR("CurrentTemperature"), 0, &vtProp, 0, 0);
    ASSERT_TRUE(SUCCEEDED(hr));
    ASSERT_TRUE(GetValueString(vtProp, chRetValue));
    VariantClear(&vtProp);
    pWbemClsObj->Release();
    pEnumClsObj->Release();
    // wait for all logs received.
    std::vector<int> count_list;
    count_list.push_back(0);
    count_list.push_back(2);
    count_list.push_back(1);
    EXPECT_EQ(g_server_object->WaitLogCountMap(count_list, 5), TRUE);
}

TEST_F(HookWmiMethodsTest, Win32TemperatureProbeTest)
{
    CComBSTR query("SELECT * FROM ");
    VARIANT vtProp;
    ULONG uReturn = 0;
    HRESULT hr;
    BOOL bRet = FALSE;
    std::wstring chRetValue;
    IEnumWbemClassObject* pEnumClsObj = nullptr;
    IWbemClassObject* pWbemClsObj = nullptr;

    //initialize
    std::vector<std::string> action_list;
    action_list.push_back("W1");
    action_list.push_back("W2");
    action_list.push_back("W6");
    g_server_object->AddLogCountMap(action_list);
    query += CComBSTR("Win32_TemperatureProbe");
    hr = m_wbemSvc->ExecQuery(CComBSTR("WQL"), query, WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        0, &pEnumClsObj);
    ASSERT_TRUE(SUCCEEDED(hr));
    VariantInit(&vtProp);
    hr = pEnumClsObj->Next(WBEM_INFINITE, 1, &pWbemClsObj, &uReturn);
    ASSERT_TRUE(SUCCEEDED(hr) && uReturn > 0);
    hr = pWbemClsObj->Get(CComBSTR("__CLASS"), 0, &vtProp, 0, 0);
    if (hr == S_FALSE) return;
    ASSERT_TRUE(GetValueString(vtProp, chRetValue));
    VariantClear(&vtProp);
    hr = pWbemClsObj->Get(CComBSTR("Name"), 0, &vtProp, 0, 0);
    ASSERT_TRUE(SUCCEEDED(hr));
    ASSERT_TRUE(GetValueString(vtProp, chRetValue));
    VariantClear(&vtProp);
    hr = pWbemClsObj->Get(CComBSTR("Caption"), 0, &vtProp, 0, 0);
    ASSERT_TRUE(SUCCEEDED(hr));
    ASSERT_TRUE(GetValueString(vtProp, chRetValue));
    VariantClear(&vtProp);
    hr = pWbemClsObj->Get(CComBSTR("Accuracy"), 0, &vtProp, 0, 0);
    ASSERT_TRUE(SUCCEEDED(hr));
    ASSERT_TRUE(GetValueString(vtProp, chRetValue));
    VariantClear(&vtProp);
    pWbemClsObj->Release();
    pEnumClsObj->Release();
    // wait for all logs received.
    std::vector<int> count_list;
    count_list.push_back(0);
    count_list.push_back(4);
    count_list.push_back(1);
    EXPECT_EQ(g_server_object->WaitLogCountMap(count_list, 5), TRUE);
}

TEST_F(HookWmiMethodsTest, Win32BiosTest)
{
    CComBSTR query("SELECT * FROM ");
    VARIANT vtProp;
    ULONG uReturn = 0;
    HRESULT hr;
    BOOL bRet = FALSE;
    std::wstring chRetValue;
    IEnumWbemClassObject* pEnumClsObj = nullptr;
    IWbemClassObject* pWbemClsObj = nullptr;

    //initialize
    std::vector<std::string> action_list;
    action_list.push_back("W1");
    action_list.push_back("W2");
    action_list.push_back("W6");
    g_server_object->AddLogCountMap(action_list);
    query += CComBSTR("Win32_Bios");
    hr = m_wbemSvc->ExecQuery(CComBSTR("WQL"), query, WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        0, &pEnumClsObj);
    ASSERT_TRUE(SUCCEEDED(hr));
    VariantInit(&vtProp);
    hr = pEnumClsObj->Next(WBEM_INFINITE, 1, &pWbemClsObj, &uReturn);
    ASSERT_TRUE(SUCCEEDED(hr) && uReturn > 0);
    hr = pWbemClsObj->Get(CComBSTR("__CLASS"), 0, &vtProp, 0, 0);
    ASSERT_TRUE(SUCCEEDED(hr));
    ASSERT_TRUE(GetValueString(vtProp, chRetValue));
    VariantClear(&vtProp);
    hr = pWbemClsObj->Get(CComBSTR("SerialNumber"), 0, &vtProp, 0, 0);
    ASSERT_TRUE(SUCCEEDED(hr));
    ASSERT_TRUE(GetValueString(vtProp, chRetValue));
    VariantClear(&vtProp);
    hr = pWbemClsObj->Get(CComBSTR("Caption"), 0, &vtProp, 0, 0);
    ASSERT_TRUE(SUCCEEDED(hr));
    ASSERT_TRUE(GetValueString(vtProp, chRetValue));
    VariantClear(&vtProp);
    hr = pWbemClsObj->Get(CComBSTR("Manufacturer"), 0, &vtProp, 0, 0);
    ASSERT_TRUE(SUCCEEDED(hr));
    ASSERT_TRUE(GetValueString(vtProp, chRetValue));
    ASSERT_FALSE(wcsstr(chRetValue.c_str(), L"vbox"));
    VariantClear(&vtProp);
    pWbemClsObj->Release();
    pEnumClsObj->Release();
    // wait for all logs received.
    std::vector<int> count_list;
    count_list.push_back(0);
    count_list.push_back(4);
    count_list.push_back(1);
    EXPECT_EQ(g_server_object->WaitLogCountMap(count_list, 5), TRUE);
}

TEST_F(HookWmiMethodsTest, Win32ComputerSystemTest)
{
    CComBSTR query("SELECT * FROM ");
    VARIANT vtProp;
    ULONG uReturn = 0;
    HRESULT hr;
    BOOL bRet = FALSE;
    std::wstring chRetValue;
    IEnumWbemClassObject* pEnumClsObj = nullptr;
    IWbemClassObject* pWbemClsObj = nullptr;

    //initialize
    std::vector<std::string> action_list;
    action_list.push_back("W1");
    action_list.push_back("W2");
    action_list.push_back("W6");
    g_server_object->AddLogCountMap(action_list);
    query += CComBSTR("Win32_ComputerSystem");
    hr = m_wbemSvc->ExecQuery(CComBSTR("WQL"), query, WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        0, &pEnumClsObj);
    ASSERT_TRUE(SUCCEEDED(hr));
    VariantInit(&vtProp);
    hr = pEnumClsObj->Next(WBEM_INFINITE, 1, &pWbemClsObj, &uReturn);
    ASSERT_TRUE(SUCCEEDED(hr) && uReturn > 0);
    hr = pWbemClsObj->Get(CComBSTR("__CLASS"), 0, &vtProp, 0, 0);
    ASSERT_TRUE(SUCCEEDED(hr));
    ASSERT_TRUE(GetValueString(vtProp, chRetValue));
    VariantClear(&vtProp);
    hr = pWbemClsObj->Get(CComBSTR("UserName"), 0, &vtProp, 0, 0);
    if (SUCCEEDED(hr)) {
        if (GetValueString(vtProp, chRetValue)) {
            ASSERT_FALSE(wcsstr(chRetValue.c_str(), L"abc") || wcsstr(chRetValue.c_str(), L"123"));
        }
        VariantClear(&vtProp);
    }
    hr = pWbemClsObj->Get(CComBSTR("Domain"), 0, &vtProp, 0, 0);
    ASSERT_TRUE(SUCCEEDED(hr));
    ASSERT_TRUE(GetValueString(vtProp, chRetValue));
    VariantClear(&vtProp);
    hr = pWbemClsObj->Get(CComBSTR("Model"), 0, &vtProp, 0, 0);
    ASSERT_TRUE(SUCCEEDED(hr));
    ASSERT_TRUE(GetValueString(vtProp, chRetValue));
    VariantClear(&vtProp);
    hr = pWbemClsObj->Get(CComBSTR("Manufacturer"), 0, &vtProp, 0, 0);
    ASSERT_TRUE(SUCCEEDED(hr));
    ASSERT_TRUE(GetValueString(vtProp, chRetValue));
    VariantClear(&vtProp);
    pWbemClsObj->Release();
    pEnumClsObj->Release();
    // wait for all logs received.
    std::vector<int> count_list;
    count_list.push_back(1);
    count_list.push_back(4);
    count_list.push_back(1);
    EXPECT_EQ(g_server_object->WaitLogCountMap(count_list, 5), TRUE);
}

TEST_F(HookWmiMethodsTest, Win32FanTest)
{
    CComBSTR query("SELECT * FROM ");
    VARIANT vtProp;
    ULONG uReturn = 0;
    HRESULT hr;
    BOOL bRet = FALSE;
    std::wstring chRetValue;
    IEnumWbemClassObject* pEnumClsObj = nullptr;
    IWbemClassObject* pWbemClsObj = nullptr;

    //initialize
    std::vector<std::string> action_list;
    action_list.push_back("W1");
    action_list.push_back("W2");
    action_list.push_back("W6");
    g_server_object->AddLogCountMap(action_list);
    query += CComBSTR("Win32_Fan");
    hr = m_wbemSvc->ExecQuery(CComBSTR("WQL"), query, WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        0, &pEnumClsObj);
    ASSERT_TRUE(SUCCEEDED(hr));
    VariantInit(&vtProp);
    hr = pEnumClsObj->Next(WBEM_INFINITE, 1, &pWbemClsObj, &uReturn);
    ASSERT_TRUE(SUCCEEDED(hr) && uReturn > 0);
    hr = pWbemClsObj->Get(CComBSTR("__CLASS"), 0, &vtProp, 0, 0);
    if (hr == S_FALSE) return;
    ASSERT_TRUE(GetValueString(vtProp, chRetValue));
    VariantClear(&vtProp);
    pWbemClsObj->Release();
    pEnumClsObj->Release();
    // wait for all logs received.
    std::vector<int> count_list;
    count_list.push_back(0);
    count_list.push_back(1);
    count_list.push_back(1);
    EXPECT_EQ(g_server_object->WaitLogCountMap(count_list, 5), TRUE);
}

TEST_F(HookWmiMethodsTest, Win32LogicalDiskTest)
{
    CComBSTR query("SELECT * FROM ");
    VARIANT vtProp;
    ULONG uReturn = 0;
    HRESULT hr;
    BOOL bRet = FALSE;
    std::wstring chRetValue;
    IEnumWbemClassObject* pEnumClsObj = nullptr;
    IWbemClassObject* pWbemClsObj = nullptr;

    //initialize
    std::vector<std::string> action_list;
    action_list.push_back("W1");
    action_list.push_back("W2");
    action_list.push_back("W6");
    g_server_object->AddLogCountMap(action_list);
    query += CComBSTR("Win32_LogicalDisk");
    hr = m_wbemSvc->ExecQuery(CComBSTR("WQL"), query, WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        0, &pEnumClsObj);
    ASSERT_TRUE(SUCCEEDED(hr));
    VariantInit(&vtProp);
    hr = pEnumClsObj->Next(WBEM_INFINITE, 1, &pWbemClsObj, &uReturn);
    ASSERT_TRUE(SUCCEEDED(hr) && uReturn > 0);
    hr = pWbemClsObj->Get(CComBSTR("__PATH"), 0, &vtProp, 0, 0);
    ASSERT_TRUE(SUCCEEDED(hr));
    ASSERT_TRUE(GetValueString(vtProp, chRetValue));
    VariantClear(&vtProp);
    hr = pWbemClsObj->Get(CComBSTR("MediaType"), 0, &vtProp, 0, 0);
    ASSERT_TRUE(SUCCEEDED(hr));
    ASSERT_TRUE(GetValueString(vtProp, chRetValue));
    VariantClear(&vtProp);
    hr = pWbemClsObj->Get(CComBSTR("Name"), 0, &vtProp, 0, 0);
    ASSERT_TRUE(SUCCEEDED(hr));
    ASSERT_TRUE(GetValueString(vtProp, chRetValue));
    VariantClear(&vtProp);
    hr = pWbemClsObj->Get(CComBSTR("Size"), 0, &vtProp, 0, 0);
    ASSERT_TRUE(SUCCEEDED(hr));
    ASSERT_TRUE(GetValueString(vtProp, chRetValue));
    VariantClear(&vtProp);
    pWbemClsObj->Release();
    pEnumClsObj->Release();
    // wait for all logs received.
    std::vector<int> count_list;
    count_list.push_back(0);
    count_list.push_back(4);
    count_list.push_back(1);
    EXPECT_EQ(g_server_object->WaitLogCountMap(count_list, 5), TRUE);
}

TEST_F(HookWmiMethodsTest, Win32NetworkAdapterConfigurationTest)
{
    DWORD get_count = 0;
    CComBSTR query("SELECT * FROM ");
    VARIANT vtProp;
    ULONG uReturn = 0;
    HRESULT hr;
    BOOL bRet = FALSE;
    std::wstring chRetValue;
    IEnumWbemClassObject* pEnumClsObj = nullptr;
    IWbemClassObject* pWbemClsObj = nullptr;

    //initialize
    std::vector<std::string> action_list;
    action_list.push_back("W1");
    action_list.push_back("W2");
    action_list.push_back("W6");
    g_server_object->AddLogCountMap(action_list);
    query += CComBSTR("Win32_NetworkAdapterConfiguration");
    hr = m_wbemSvc->ExecQuery(CComBSTR("WQL"), query, WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        0, &pEnumClsObj);
    ASSERT_TRUE(SUCCEEDED(hr));
    VariantInit(&vtProp);
    hr = pEnumClsObj->Next(WBEM_INFINITE, 1, &pWbemClsObj, &uReturn);
    while (SUCCEEDED(hr) && uReturn > 0) {
        hr = pWbemClsObj->Get(CComBSTR("MACAddress"), 0, &vtProp, 0, 0);
        if (SUCCEEDED(hr)) {
            if (GetValueString(vtProp, chRetValue)) {
                get_count++;
                ASSERT_FALSE(wcsstr(chRetValue.c_str(), L"08:00:27"));
            }
            VariantClear(&vtProp);
        }
        hr = pEnumClsObj->Next(WBEM_INFINITE, 1, &pWbemClsObj, &uReturn);
    }
    pWbemClsObj->Release();
    pEnumClsObj->Release();
    // wait for all logs received.
    std::vector<int> count_list;
    count_list.push_back(0);
    count_list.push_back(get_count);
    count_list.push_back(1);
    EXPECT_EQ(g_server_object->WaitLogCountMap(count_list, 5), TRUE);
}

TEST_F(HookWmiMethodsTest, Win32PnPEntityTest)
{
    DWORD get_count = 0;
    CComBSTR query("SELECT * FROM ");
    VARIANT vtProp;
    ULONG uReturn = 0;
    HRESULT hr;
    BOOL bRet = FALSE;
    std::wstring chRetValue;
    IEnumWbemClassObject* pEnumClsObj = nullptr;
    IWbemClassObject* pWbemClsObj = nullptr;

    //initialize
    std::vector<std::string> action_list;
    action_list.push_back("W1");
    action_list.push_back("W2");
    action_list.push_back("W6");
    g_server_object->AddLogCountMap(action_list);
    query += CComBSTR("Win32_PnPEntity");
    hr = m_wbemSvc->ExecQuery(CComBSTR("WQL"), query, WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        0, &pEnumClsObj);
    ASSERT_TRUE(SUCCEEDED(hr));
    VariantInit(&vtProp);
    hr = pEnumClsObj->Next(WBEM_INFINITE, 1, &pWbemClsObj, &uReturn);
    while (SUCCEEDED(hr) && uReturn > 0) {
        hr = pWbemClsObj->Get(CComBSTR("Name"), 0, &vtProp, 0, 0);
        if (SUCCEEDED(hr)) {
            if(GetValueString(vtProp, chRetValue))
                get_count++;
            VariantClear(&vtProp);
        }
        hr = pWbemClsObj->Get(CComBSTR("DeviceId"), 0, &vtProp, 0, 0);
        if (SUCCEEDED(hr)) {
            if (GetValueString(vtProp, chRetValue)) {
                get_count++;
                ASSERT_FALSE(wcsstr(chRetValue.c_str(), L"PCI\\VEN_80EE&DEV_CAFE"));
            }
            VariantClear(&vtProp);
        }
        hr = pEnumClsObj->Next(WBEM_INFINITE, 1, &pWbemClsObj, &uReturn);
    }
    pWbemClsObj->Release();
    pEnumClsObj->Release();
    // wait for all logs received.
    std::vector<int> count_list;
    count_list.push_back(0);
    count_list.push_back(get_count);
    count_list.push_back(1);
    g_server_object->WaitLogCountMap(count_list, 1);
}

TEST_F(HookWmiMethodsTest, Win32ProcessorTest)
{
    DWORD get_count = 0;
    CComBSTR query("SELECT * FROM ");
    VARIANT vtProp;
    ULONG uReturn = 0;
    HRESULT hr;
    BOOL bRet = FALSE;
    std::wstring chRetValue;
    IEnumWbemClassObject* pEnumClsObj = nullptr;
    IWbemClassObject* pWbemClsObj = nullptr;

    //initialize
    std::vector<std::string> action_list;
    action_list.push_back("W1");
    action_list.push_back("W2");
    action_list.push_back("W6");
    g_server_object->AddLogCountMap(action_list);
    query += CComBSTR("Win32_Processor");
    hr = m_wbemSvc->ExecQuery(CComBSTR("WQL"), query, WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        0, &pEnumClsObj);
    ASSERT_TRUE(SUCCEEDED(hr));
    VariantInit(&vtProp);
    hr = pEnumClsObj->Next(WBEM_INFINITE, 1, &pWbemClsObj, &uReturn);
    while (SUCCEEDED(hr) && uReturn > 0) {
        hr = pWbemClsObj->Get(CComBSTR("ProcessorId"), 0, &vtProp, 0, 0);
        if (SUCCEEDED(hr)) {
            if (GetValueString(vtProp, chRetValue))
                get_count++;
            VariantClear(&vtProp);
        }
        hr = pWbemClsObj->Get(CComBSTR("NumberOfCores"), 0, &vtProp, 0, 0);
        if (SUCCEEDED(hr)) {
            if (GetValueString(vtProp, chRetValue)) {
                get_count++;
                ASSERT_TRUE(vtProp.intVal >= 2);
            }
            VariantClear(&vtProp);
        }
        hr = pEnumClsObj->Next(WBEM_INFINITE, 1, &pWbemClsObj, &uReturn);
    }
    pWbemClsObj->Release();
    pEnumClsObj->Release();
    // wait for all logs received.
    std::vector<int> count_list;
    count_list.push_back(0);
    count_list.push_back(get_count); 
    count_list.push_back(1);
    EXPECT_EQ(g_server_object->WaitLogCountMap(count_list, 5), TRUE);
}

TEST_F(HookWmiMethodsTest, Win32BaseBoardTest)
{
    CComBSTR query("SELECT * FROM ");
    VARIANT vtProp;
    ULONG uReturn = 0;
    HRESULT hr;
    BOOL bRet = FALSE;
    std::wstring chRetValue;
    IEnumWbemClassObject* pEnumClsObj = nullptr;
    IWbemClassObject* pWbemClsObj = nullptr;

    //initialize
    std::vector<std::string> action_list;
    action_list.push_back("W1");
    action_list.push_back("W2");
    action_list.push_back("W6");
    g_server_object->AddLogCountMap(action_list);
    query += CComBSTR("Win32_BaseBoard");
    hr = m_wbemSvc->ExecQuery(CComBSTR("WQL"), query, WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        0, &pEnumClsObj);
    ASSERT_TRUE(SUCCEEDED(hr));
    VariantInit(&vtProp);
    hr = pEnumClsObj->Next(WBEM_INFINITE, 1, &pWbemClsObj, &uReturn);
    ASSERT_TRUE(SUCCEEDED(hr) && uReturn > 0);
    hr = pWbemClsObj->Get(CComBSTR("__PATH"), 0, &vtProp, 0, 0);
    ASSERT_TRUE(SUCCEEDED(hr));
    ASSERT_TRUE(GetValueString(vtProp, chRetValue));
    VariantClear(&vtProp);
    hr = pWbemClsObj->Get(CComBSTR("Manufacturer"), 0, &vtProp, 0, 0);
    ASSERT_TRUE(SUCCEEDED(hr));
    ASSERT_TRUE(GetValueString(vtProp, chRetValue));
    ASSERT_FALSE(wcsstr(chRetValue.c_str(), L"vbox"));
    VariantClear(&vtProp);
    pWbemClsObj->Release();
    pEnumClsObj->Release();
    // wait for all logs received.
    std::vector<int> count_list;
    count_list.push_back(0);
    count_list.push_back(2);
    count_list.push_back(1);
    EXPECT_EQ(g_server_object->WaitLogCountMap(count_list, 5), TRUE);
}

TEST_F(HookWmiMethodsTest, Win32CDROMDriveTest)
{
    CComBSTR query("SELECT * FROM ");
    VARIANT vtProp;
    ULONG uReturn = 0;
    HRESULT hr;
    BOOL bRet = FALSE;
    std::wstring chRetValue;
    IEnumWbemClassObject* pEnumClsObj = nullptr;
    IWbemClassObject* pWbemClsObj = nullptr;

    query += CComBSTR("Win32_CDROMDrive");
    hr = m_wbemSvc->ExecQuery(CComBSTR("WQL"), query, WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        0, &pEnumClsObj);
    ASSERT_TRUE(SUCCEEDED(hr));
    VariantInit(&vtProp);
    hr = pEnumClsObj->Next(WBEM_INFINITE, 1, &pWbemClsObj, &uReturn);
    if(SUCCEEDED(hr) && uReturn > 0) {
        hr = pWbemClsObj->Get(CComBSTR("__PATH"), 0, &vtProp, 0, 0);
        if (SUCCEEDED(hr))
            GetValueString(vtProp, chRetValue);
        VariantClear(&vtProp);
        hr = pWbemClsObj->Get(CComBSTR("Name"), 0, &vtProp, 0, 0);
        if (SUCCEEDED(hr))
            GetValueString(vtProp, chRetValue);
        VariantClear(&vtProp);
        hr = pWbemClsObj->Get(CComBSTR("Manufacturer"), 0, &vtProp, 0, 0);
        if (SUCCEEDED(hr)) {
            if(GetValueString(vtProp, chRetValue))
                ASSERT_FALSE(wcsstr(chRetValue.c_str(), L"vbox"));
        }
        VariantClear(&vtProp);
        pWbemClsObj->Release();
        pEnumClsObj->Release();
    }
}

TEST_F(HookWmiMethodsTest, AntiVirusProductTest)
{
    CComBSTR query("SELECT * FROM ");
    VARIANT vtProp;
    ULONG uReturn = 0;
    HRESULT hr;
    BOOL bRet = FALSE;
    std::wstring chRetValue;
    IEnumWbemClassObject* pEnumClsObj = nullptr;
    IWbemClassObject* pWbemClsObj = nullptr;

    hr = m_wbemLoc->ConnectServer(CComBSTR(L"ROOT\\SecurityCenter2"), NULL, NULL, 0, NULL, 0, 0, &m_wbemSvc);
    ASSERT_TRUE(SUCCEEDED(hr));
    query += CComBSTR("AntiVirusProduct");
    hr = m_wbemSvc->ExecQuery(CComBSTR("WQL"), query, WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        0, &pEnumClsObj);
    ASSERT_TRUE(SUCCEEDED(hr));
    VariantInit(&vtProp);
    hr = pEnumClsObj->Next(WBEM_INFINITE, 1, &pWbemClsObj, &uReturn);
    if (SUCCEEDED(hr) && uReturn > 0) {
        hr = pWbemClsObj->Get(CComBSTR("__CLASS"), 0, &vtProp, 0, 0);
        ASSERT_TRUE(SUCCEEDED(hr));
        ASSERT_TRUE(GetValueString(vtProp, chRetValue));
        VariantClear(&vtProp);
        hr = pWbemClsObj->Get(CComBSTR("displayName"), 0, &vtProp, 0, 0);
        ASSERT_TRUE(SUCCEEDED(hr));
        ASSERT_TRUE(GetValueString(vtProp, chRetValue));
        ASSERT_FALSE(wcsstr(chRetValue.c_str(), L"Windows Defender"));
        VariantClear(&vtProp);
        pWbemClsObj->Release();
        pEnumClsObj->Release();
    }
}

TEST_F(HookWmiMethodsTest, Win32LoggedOnUserTest)
{
    CComBSTR query("SELECT * FROM ");
    VARIANT vtProp;
    ULONG uReturn = 0;
    HRESULT hr;
    BOOL bRet = FALSE;
    std::wstring chRetValue;
    IEnumWbemClassObject* pEnumClsObj = nullptr;
    IWbemClassObject* pWbemClsObj = nullptr;

    //initialize
    std::vector<std::string> action_list;
    action_list.push_back("W1");
    action_list.push_back("W2");
    action_list.push_back("W6");
    g_server_object->AddLogCountMap(action_list);
    query += CComBSTR("Win32_LoggedOnUser");
    hr = m_wbemSvc->ExecQuery(CComBSTR("WQL"), query, WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        0, &pEnumClsObj);
    ASSERT_TRUE(SUCCEEDED(hr));
    VariantInit(&vtProp);
    hr = pEnumClsObj->Next(WBEM_INFINITE, 1, &pWbemClsObj, &uReturn);
    ASSERT_TRUE(SUCCEEDED(hr) && uReturn > 0);
    hr = pWbemClsObj->Get(CComBSTR("__PATH"), 0, &vtProp, 0, 0);
    ASSERT_TRUE(SUCCEEDED(hr));
    ASSERT_TRUE(GetValueString(vtProp, chRetValue));
    VariantClear(&vtProp);
    hr = pWbemClsObj->Get(CComBSTR("Dependent"), 0, &vtProp, 0, 0);
    ASSERT_TRUE(SUCCEEDED(hr));
    ASSERT_TRUE(GetValueString(vtProp, chRetValue));
    VariantClear(&vtProp);
    hr = pWbemClsObj->Get(CComBSTR("Antecedent"), 0, &vtProp, 0, 0);
    ASSERT_TRUE(SUCCEEDED(hr));
    ASSERT_TRUE(GetValueString(vtProp, chRetValue));
    VariantClear(&vtProp);
    pWbemClsObj->Release();
    pEnumClsObj->Release();
    // wait for all logs received.
    std::vector<int> count_list;
    count_list.push_back(0);
    count_list.push_back(2);
    count_list.push_back(1);
    EXPECT_EQ(g_server_object->WaitLogCountMap(count_list, 5), TRUE);
}

TEST_F(HookWmiMethodsTest, Win32OperatingSystemTest)
{
    CComBSTR query("SELECT * FROM ");
    VARIANT vtProp;
    ULONG uReturn = 0;
    HRESULT hr;
    BOOL bRet = FALSE;
    std::wstring chRetValue;
    IEnumWbemClassObject* pEnumClsObj = nullptr;
    IWbemClassObject* pWbemClsObj = nullptr;

    //initialize
    std::vector<std::string> action_list;
    action_list.push_back("W1");
    action_list.push_back("W2");
    action_list.push_back("W6");
    g_server_object->AddLogCountMap(action_list);
    query += CComBSTR("Win32_OperatingSystem");
    hr = m_wbemSvc->ExecQuery(CComBSTR("WQL"), query, WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        0, &pEnumClsObj);
    ASSERT_TRUE(SUCCEEDED(hr));
    VariantInit(&vtProp);
    hr = pEnumClsObj->Next(WBEM_INFINITE, 1, &pWbemClsObj, &uReturn);
    ASSERT_TRUE(SUCCEEDED(hr) && uReturn > 0);
    hr = pWbemClsObj->Get(CComBSTR("__PATH"), 0, &vtProp, 0, 0);
    ASSERT_TRUE(SUCCEEDED(hr));
    ASSERT_TRUE(GetValueString(vtProp, chRetValue));
    VariantClear(&vtProp);
    hr = pWbemClsObj->Get(CComBSTR("BuildNumber"), 0, &vtProp, 0, 0);
    ASSERT_TRUE(SUCCEEDED(hr));
    ASSERT_TRUE(GetValueString(vtProp, chRetValue));
    VariantClear(&vtProp);
    hr = pWbemClsObj->Get(CComBSTR("Name"), 0, &vtProp, 0, 0);
    ASSERT_TRUE(SUCCEEDED(hr));
    ASSERT_TRUE(GetValueString(vtProp, chRetValue));
    VariantClear(&vtProp);
    hr = pWbemClsObj->Get(CComBSTR("NumberOfProcesses"), 0, &vtProp, 0, 0);
    ASSERT_TRUE(SUCCEEDED(hr));
    ASSERT_TRUE(GetValueString(vtProp, chRetValue));
    VariantClear(&vtProp);
    pWbemClsObj->Release();
    pEnumClsObj->Release();
    // wait for all logs received.
    std::vector<int> count_list;
    count_list.push_back(0);
    count_list.push_back(4);
    count_list.push_back(1);
    EXPECT_EQ(g_server_object->WaitLogCountMap(count_list, 5), TRUE);
}

TEST_F(HookWmiMethodsTest, Win32ProcessTest)
{
    CComBSTR query("SELECT * FROM ");
    VARIANT vtProp;
    ULONG uReturn = 0;
    HRESULT hr;
    BOOL bRet = FALSE;
    std::wstring chRetValue;
    IEnumWbemClassObject* pEnumClsObj = nullptr;
    IWbemClassObject* pWbemClsObj = nullptr;

    query += CComBSTR("Win32_Process");
    hr = m_wbemSvc->ExecQuery(CComBSTR("WQL"), query, WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        0, &pEnumClsObj);
    ASSERT_TRUE(SUCCEEDED(hr));
    VariantInit(&vtProp);
    hr = pEnumClsObj->Next(WBEM_INFINITE, 1, &pWbemClsObj, &uReturn);
    ASSERT_TRUE(SUCCEEDED(hr) && uReturn > 0);
    hr = pWbemClsObj->Get(CComBSTR("Caption"), 0, &vtProp, 0, 0);
    ASSERT_TRUE(SUCCEEDED(hr));
    ASSERT_TRUE(GetValueString(vtProp, chRetValue));
    VariantClear(&vtProp);
    hr = pWbemClsObj->Get(CComBSTR("Name"), 0, &vtProp, 0, 0);
    ASSERT_TRUE(SUCCEEDED(hr));
    ASSERT_TRUE(GetValueString(vtProp, chRetValue));
    VariantClear(&vtProp);
    hr = pWbemClsObj->Get(CComBSTR("ProcessId"), 0, &vtProp, 0, 0);
    ASSERT_TRUE(SUCCEEDED(hr));
    ASSERT_TRUE(GetValueString(vtProp, chRetValue));
    VariantClear(&vtProp);
    hr = pWbemClsObj->Get(CComBSTR("ThreadCount"), 0, &vtProp, 0, 0);
    ASSERT_TRUE(SUCCEEDED(hr));
    ASSERT_TRUE(GetValueString(vtProp, chRetValue));
    VariantClear(&vtProp);

    // call create method
    IWbemClassObject* pMethod = nullptr;
    IWbemClassObject* pInParams = nullptr;
    IWbemClassObject* pOutParams = nullptr;
    IWbemClassObject* pClass = nullptr;
    hr = m_wbemSvc->GetObject(L"Win32_Process", 0, NULL, &pClass, NULL);
    ASSERT_TRUE(SUCCEEDED(hr));
    hr = pClass->GetMethod(L"Create", 0, &pMethod, NULL);
    ASSERT_TRUE(SUCCEEDED(hr));
    hr = pMethod->SpawnInstance(0, &pInParams);
    ASSERT_TRUE(SUCCEEDED(hr));
    vtProp.vt = VT_BSTR;
    vtProp.bstrVal = L"notepad.exe";
    hr = pInParams->Put(L"CommandLine", 0, &vtProp, 0);
    ASSERT_TRUE(SUCCEEDED(hr));
    hr = m_wbemSvc->ExecMethod(L"Win32_Process", L"Create", 0, NULL, pInParams, &pOutParams, NULL);
    ASSERT_TRUE(SUCCEEDED(hr));
    ::Sleep(1000);
    VARIANT varReturnValue;
    hr = pOutParams->Get(L"ReturnValue", 0, &varReturnValue, NULL, 0);
    ASSERT_TRUE(SUCCEEDED(hr));
    ASSERT_TRUE(GetValueString(varReturnValue, chRetValue));
    if (varReturnValue.intVal == 0) {
        VariantClear(&varReturnValue);
        hr = pOutParams->Get(L"ProcessId", 0, &varReturnValue, NULL, 0);
        ASSERT_TRUE(SUCCEEDED(hr));
        ASSERT_TRUE(GetValueString(varReturnValue, chRetValue));
    }
    pWbemClsObj->Release();
    pEnumClsObj->Release();
}

// Excel COM Object test.
TEST_F(HookWmiMethodsTest, Excel_Application)
{
    CLSID clsid;
    HRESULT hr;
    
    std::vector<std::string> action_list;
    action_list.push_back("O0");
    g_server_object->AddLogCountMap(action_list);

    LPCOLESTR prog_id = L"Excel.Application";
    hr = CLSIDFromProgID(prog_id, &clsid);
    if (FAILED(hr))
        return;
    IDispatch *excel_app = nullptr;
    hr = CoCreateInstance(
        clsid,                    // CLSID of the server
        nullptr,
        CLSCTX_LOCAL_SERVER,    // Outlook.Application is a local server
        IID_IDispatch,            // Query the IDispatch interface
        (void **)&excel_app);    // Output
    ASSERT_EQ(hr, S_OK);
    ASSERT_NE(excel_app, nullptr);
    excel_app->Release();
    // wait for all logs received.
    std::vector<int> count_list;
    count_list.push_back(1);
    EXPECT_EQ(g_server_object->WaitLogCountMap(count_list, 5), TRUE);
}

//// IExplore COM Object test.
//TEST_F(HookWmiMethodsTest, IExplore_Application)
//{
//    CLSID clsid;
//    HRESULT hr;
//
//    std::vector<std::string> action_list;
//    action_list.push_back("O0");
//    g_server_object->AddLogCountMap(action_list);
//
//    LPCOLESTR prog_id = L"InternetExplorer.Application";
//    hr = CLSIDFromProgID(prog_id, &clsid);
//    if (FAILED(hr))
//        return;
//    IDispatch *ie_app = nullptr;
//    hr = CoCreateInstance(
//        clsid,                    // CLSID of the server
//        nullptr,
//        CLSCTX_LOCAL_SERVER,    // Outlook.Application is a local server
//        IID_IDispatch,            // Query the IDispatch interface
//        (void **)&ie_app);    // Output
//    ASSERT_EQ(hr, S_OK);
//    ASSERT_NE(ie_app, nullptr);
//
//    VARIANT x;
//    x.vt = VT_BSTR;
//    x.bstrVal = SysAllocString(L"www.google.com");
//    LPOLESTR func_name = L"Navigate";
//    VARIANT result;
//    VariantInit(&result);
//    DISPPARAMS dp = {};
//    DISPID dispidNamed = DISPID_PROPERTYPUT;
//    DISPID dispID;
//    // Get DISPID for name passed
//    hr = ie_app->GetIDsOfNames(IID_NULL, &func_name, 1, LOCALE_USER_DEFAULT, &dispID);
//    ASSERT_EQ(hr, S_OK);
//    // Build DISPPARAMS
//    dp.cArgs = 1;
//    dp.rgvarg = &x;
//    // Make the call
//    EXCEPINFO excepInfo;
//    memset(&excepInfo, 0, sizeof excepInfo);
//    hr = ie_app->Invoke(dispID, IID_NULL, LOCALE_SYSTEM_DEFAULT,
//        DISPATCH_METHOD, &dp, &result, &excepInfo, NULL);
//    ASSERT_EQ(hr, S_OK);
//    VariantClear(&result);
//    VariantClear(&x);
//    ie_app->Release();
//    // wait for all logs received.
//    std::vector<int> count_list;
//    count_list.push_back(1);
//    EXPECT_EQ(g_server_object->WaitLogCountMap(count_list, 5), TRUE);
//}

// Outlook COM Object test.
TEST_F(HookWmiMethodsTest, Outlook_Application)
{
    CLSID clsid;
    HRESULT hr;

    std::vector<std::string> action_list;
    action_list.push_back("O0");
    g_server_object->AddLogCountMap(action_list);
    
    LPCOLESTR prog_id = L"Outlook.Application";
    hr = CLSIDFromProgID(prog_id, &clsid);
    if (FAILED(hr))
        return;
    IDispatch *outlook_app = nullptr;
    hr = CoCreateInstance(
        clsid,                    // CLSID of the server
        nullptr,
        CLSCTX_LOCAL_SERVER,    // Outlook.Application is a local server
        IID_IDispatch,            // Query the IDispatch interface
        (void **)&outlook_app);    // Output
    ASSERT_EQ(hr, S_OK);
    ASSERT_NE(outlook_app, nullptr);
    IDispatch *p_gns_ptr = nullptr;
    VARIANT x;
    x.vt = VT_BSTR;
    x.bstrVal = SysAllocString(L"MAPI");
    LPOLESTR func_name = L"GetNamespace";
    VARIANT result;
    VariantInit(&result);
    DISPPARAMS dp = {};
    DISPID dispidNamed = DISPID_PROPERTYPUT;
    DISPID dispID;
    // Get DISPID for name passed
    hr = outlook_app->GetIDsOfNames(IID_NULL, &func_name, 1, LOCALE_USER_DEFAULT, &dispID);
    ASSERT_EQ(hr, S_OK);
    // Build DISPPARAMS
    dp.cArgs = 1;
    dp.rgvarg = &x;
    // Make the call
    EXCEPINFO excepInfo;
    memset(&excepInfo, 0, sizeof excepInfo);
    hr = outlook_app->Invoke(dispID, IID_NULL, LOCALE_SYSTEM_DEFAULT,
        DISPATCH_METHOD, &dp, &result, &excepInfo, NULL);
    ASSERT_EQ(hr, S_OK);
    p_gns_ptr = result.pdispVal;
    ASSERT_NE(p_gns_ptr, nullptr);
    outlook_app->Release();
    p_gns_ptr->Release();
    VariantClear(&result);
    VariantClear(&x);
    // wait for all logs received.
    std::vector<int> count_list;
    count_list.push_back(1);
    EXPECT_EQ(g_server_object->WaitLogCountMap(count_list, 5), TRUE);
}

#endif