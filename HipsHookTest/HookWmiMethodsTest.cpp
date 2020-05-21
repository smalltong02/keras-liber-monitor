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
    HookWmiMethodsTest() {}
    ~HookWmiMethodsTest() override {}

    virtual void SetUp() override {
        HRESULT hr;
        hr = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&m_wbemLoc);
        ASSERT_TRUE(SUCCEEDED(hr));
        hr = m_wbemLoc->ConnectServer(CComBSTR(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &m_wbemSvc);
        ASSERT_TRUE(SUCCEEDED(hr));
        hr = CoSetProxyBlanket(m_wbemSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
        ASSERT_TRUE(SUCCEEDED(hr));
        ASSERT_TRUE(g_hook_test_object->EnableAllApis());
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
            break;
        }

        return false;
    }

    IWbemLocator* m_wbemLoc = nullptr;
    IEnumWbemClassObject* m_enumClsObj = nullptr;
    IWbemClassObject* m_wbemClsObj = nullptr;
    IWbemServices* m_wbemSvc = nullptr;
};

//TEST_F(HookWmiMethodsTest, RemoteTest)
//{
//#define service_ip L"192.168.0.1 "
//const CLSID CLSID_Account = { 0x5939F232,0x7B5F,0x11D4,{0x8B,0xEC,0x34,0x8F,0xBB,0x00,0x00,0x00} };
//
//    COSERVERINFO si;
//    memset(&si, 0, sizeof(COSERVERINFO));
//    si.pwszName = service_ip;
//    si.pAuthInfo = nullptr;
//    si.dwReserved1 = 0;
//    si.dwReserved2 = 0;
//    MULTI_QI qi[1];
//    qi[0].pIID = &IID_IUnknown;
//    qi[0].pItf = NULL;
//    qi[0].hr = S_OK;
//
//    HRESULT hr = CoCreateInstanceEx(CLSID_WbemLocator, NULL, CLSCTX_REMOTE_SERVER, &si, 1, qi);
//}

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
}

TEST_F(HookWmiMethodsTest, Win32RamSlotsTest)
{
    CComBSTR query("SELECT * FROM ");
    VARIANT vtProp;
    ULONG uReturn = 0;
    HRESULT hr;
    BOOL bRet = FALSE;
    std::wstring chRetValue;
    IEnumWbemClassObject* pEnumClsObj = nullptr;
    IWbemClassObject* pWbemClsObj = nullptr;

    query += CComBSTR("Win32_PhysicalMemoryArray");
    hr = m_wbemSvc->ExecQuery(CComBSTR("WQL"), query, WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        0, &pEnumClsObj);
    ASSERT_TRUE(SUCCEEDED(hr));
    VariantInit(&vtProp);
    hr = pEnumClsObj->Next(WBEM_INFINITE, 1, &pWbemClsObj, &uReturn);
    ASSERT_TRUE(SUCCEEDED(hr) && uReturn > 0);
    hr = pWbemClsObj->Get(CComBSTR("MemoryDevices"), 0, &vtProp, 0, 0);
    ASSERT_TRUE(SUCCEEDED(hr));
    ASSERT_TRUE(GetValueString(vtProp, chRetValue));
    // check for modify size success, size = 3TG if successful.
    //ASSERT_TRUE(_wcsicmp(chRetValue.c_str(), L"1") == 0);
    VariantClear(&vtProp);
    pWbemClsObj->Release();
    pEnumClsObj->Release();
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
    VARIANT varReturnValue;
    hr = pOutParams->Get(L"ReturnValue", 0, &varReturnValue, NULL, 0);
    ASSERT_TRUE(SUCCEEDED(hr));
    ASSERT_TRUE(GetValueString(varReturnValue, chRetValue));
    hr = pOutParams->Get(L"ProcessId", 0, &varReturnValue, NULL, 0);
    ASSERT_TRUE(SUCCEEDED(hr));
    ASSERT_TRUE(GetValueString(varReturnValue, chRetValue));
    pWbemClsObj->Release();
    pEnumClsObj->Release();
}

#endif