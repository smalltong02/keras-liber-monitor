#include "stdafx.h"
#include <stdio.h>
#include <windows.h>
#include <string>
#include <vector>
#include "CBaseType.h"
#include "ParsingImpl.h"
#include "HipsHookTest.h"
#include "benchmark\benchmark.h"
#include "gtest\gtest.h"
#include "gmock\gmock.h"

#ifdef _FUNCTION_TEST

class HookSpecialTest : public testing::Test
{
protected:
    HookSpecialTest() {}
    ~HookSpecialTest() override {}

    virtual void SetUp() override {

    }

    void TearDown() override {

    }
};

// test ConvertAnyType
TEST_F(HookSpecialTest, ConvertAnyType_Test)
{
    std::any test_int = int(5);
    std::string str("test_string");
    std::any test_str = str;
    std::wstring wstr(L"test_wstring");
    std::any test_wstr = wstr;
    std::any test_pvoid = static_cast<PVOID>(&test_int);
    std::any test_float = float(2.6);
    std::any test_integer = LARGE_INTEGER{5};
    std::any test_uinteger = ULARGE_INTEGER{ 5 };
    unsigned int ret_uint = ConvertAnyType<unsigned int>(test_int);
    EXPECT_EQ(ret_uint, 5);
    CHAR ret_char = ConvertAnyType<CHAR>(test_int);
    EXPECT_EQ(ret_char, '\5');
    UCHAR ret_uchar = ConvertAnyType<UCHAR>(test_int);
    EXPECT_EQ(ret_uchar, '\5');
    WCHAR ret_wchar = ConvertAnyType<WCHAR>(test_int);
    EXPECT_EQ(ret_wchar, L'\5');
    PVOID ret_pvoid = ConvertAnyType<PVOID>(test_pvoid);
    EXPECT_TRUE(ret_pvoid == &test_int);
    HANDLE ret_handle = ConvertAnyType<HANDLE>(test_pvoid);
    EXPECT_TRUE(ret_handle == &test_int);
    HMODULE ret_hmodule = ConvertAnyType<HMODULE>(test_pvoid);
    EXPECT_TRUE(ret_hmodule == (HMODULE)&test_int);
    SC_HANDLE ret_schandle = ConvertAnyType<SC_HANDLE>(test_pvoid);
    EXPECT_TRUE(ret_schandle == (SC_HANDLE)&test_int);
    std::string ret_str = ConvertAnyType<std::string>(test_str);
    EXPECT_EQ(ret_str, str);
    std::wstring ret_wstr = ConvertAnyType<std::wstring>(test_wstr);
    EXPECT_EQ(ret_wstr, wstr);
    FLOAT ret_float = ConvertAnyType<FLOAT>(test_float);
    EXPECT_EQ(ret_float, float(2.6));
    DOUBLE ret_double = ConvertAnyType<DOUBLE>(test_float);
    EXPECT_EQ(ret_double, double(float(2.6)));
    BYTE ret_byte = ConvertAnyType<BYTE>(test_int);
    EXPECT_EQ(ret_byte, BYTE(5));
    LARGE_INTEGER ret_integer = ConvertAnyType<LARGE_INTEGER>(test_int);
    EXPECT_EQ(ret_integer.QuadPart, 0);
    ret_integer = ConvertAnyType<LARGE_INTEGER>(test_integer);
    EXPECT_EQ(ret_integer.QuadPart, 5);
    ULARGE_INTEGER ret_uinteger = ConvertAnyType<ULARGE_INTEGER>(test_int);
    EXPECT_EQ(ret_uinteger.QuadPart, 0);
    ret_uinteger = ConvertAnyType<ULARGE_INTEGER>(test_uinteger);
    EXPECT_EQ(ret_uinteger.QuadPart, 5);
}

// test ConvertAnyType
TEST_F(HookSpecialTest, AssignAnyType_Test)
{
    std::any test_int = int(5);
    std::string str("test_string");
    std::any test_str = str;
    std::wstring wstr(L"test_wstring");
    std::any test_wstr = wstr;
    std::any test_pvoid = static_cast<PVOID>(&test_int);
    std::any test_float = float(2.6);
    std::any test_double = double(2.6);
    std::any anyhandle = HANDLE(nullptr);
    std::any anyhmodule = HMODULE(nullptr);
    std::any anyschandle = SC_HANDLE(nullptr);
    std::any anyvalue;
    anyvalue = DWORD(5);
    EXPECT_TRUE(AssignAnyType(anyvalue, INT16(12)));
    ASSERT_TRUE(anyvalue.has_value() && anyvalue.type() == typeid(DWORD));
    EXPECT_EQ(std::any_cast<DWORD>(anyvalue), 12);
    EXPECT_TRUE(AssignAnyType(anyvalue, USHORT(12)));
    ASSERT_TRUE(anyvalue.has_value() && anyvalue.type() == typeid(DWORD));
    EXPECT_EQ(std::any_cast<DWORD>(anyvalue), 12);
    EXPECT_TRUE(AssignAnyType(anyvalue, WORD(-12)));
    ASSERT_TRUE(anyvalue.has_value() && anyvalue.type() == typeid(DWORD));
    EXPECT_EQ(std::any_cast<DWORD>(anyvalue), DWORD(WORD(-12)));
    EXPECT_TRUE(AssignAnyType(anyvalue, LONG(12)));
    ASSERT_TRUE(anyvalue.has_value() && anyvalue.type() == typeid(DWORD));
    EXPECT_EQ(std::any_cast<DWORD>(anyvalue), 12);
    EXPECT_TRUE(AssignAnyType(anyvalue, BOOL(FALSE)));
    ASSERT_TRUE(anyvalue.has_value() && anyvalue.type() == typeid(DWORD));
    EXPECT_EQ(std::any_cast<DWORD>(anyvalue), 0);
    EXPECT_TRUE(AssignAnyType(anyvalue, UCHAR('\72')));
    ASSERT_TRUE(anyvalue.has_value() && anyvalue.type() == typeid(DWORD));
    EXPECT_EQ(std::any_cast<DWORD>(anyvalue), 58);
    EXPECT_TRUE(AssignAnyType(anyvalue, WCHAR(L'\xfc')));
    ASSERT_TRUE(anyvalue.has_value() && anyvalue.type() == typeid(DWORD));
    EXPECT_EQ(std::any_cast<DWORD>(anyvalue), 0xfc);
    EXPECT_TRUE(AssignAnyType(anyvalue, LONG_PTR(-82)));
    ASSERT_TRUE(anyvalue.has_value() && anyvalue.type() == typeid(DWORD));
    EXPECT_EQ(std::any_cast<DWORD>(anyvalue), DWORD(-82));
    EXPECT_TRUE(AssignAnyType(anyvalue, ULONG_PTR(651232)));
    ASSERT_TRUE(anyvalue.has_value() && anyvalue.type() == typeid(DWORD));
    EXPECT_EQ(std::any_cast<DWORD>(anyvalue), 651232);
    EXPECT_TRUE(AssignAnyType(anyvalue, UINT64(72368320)));
    ASSERT_TRUE(anyvalue.has_value() && anyvalue.type() == typeid(DWORD));
    EXPECT_EQ(std::any_cast<DWORD>(anyvalue), DWORD(72368320));
    EXPECT_TRUE(AssignAnyType(anyvalue, LONGLONG(-64448334)));
    ASSERT_TRUE(anyvalue.has_value() && anyvalue.type() == typeid(DWORD));
    EXPECT_EQ(std::any_cast<DWORD>(anyvalue), DWORD(-64448334));
    EXPECT_TRUE(AssignAnyType(test_pvoid, PVOID(0xffffffff)));
    ASSERT_TRUE(test_pvoid.has_value() && test_pvoid.type() == typeid(PVOID));
    EXPECT_EQ(std::any_cast<PVOID>(test_pvoid), PVOID(0xffffffff));
    EXPECT_TRUE(AssignAnyType(anyhandle, &test_int));
    ASSERT_TRUE(anyhandle.has_value() && anyhandle.type() == typeid(HANDLE));
    EXPECT_EQ(std::any_cast<HANDLE>(anyhandle), HANDLE(&test_int));
    EXPECT_TRUE(AssignAnyType(anyhmodule, &test_int));
    ASSERT_TRUE(anyhmodule.has_value() && anyhmodule.type() == typeid(HMODULE));
    EXPECT_EQ(std::any_cast<HMODULE>(anyhmodule), HMODULE(&test_int));
    EXPECT_TRUE(AssignAnyType(anyschandle, &test_int));
    ASSERT_TRUE(anyschandle.has_value() && anyschandle.type() == typeid(SC_HANDLE));
    EXPECT_EQ(std::any_cast<SC_HANDLE>(anyschandle), SC_HANDLE(&test_int));
    EXPECT_TRUE(AssignAnyType(test_wstr, L"replace wstring!"));
    ASSERT_TRUE(test_wstr.has_value() && test_wstr.type() == typeid(std::wstring));
    EXPECT_EQ(std::any_cast<std::wstring>(test_wstr), L"replace wstring!");
    EXPECT_TRUE(AssignAnyType(test_str, "replace wstring!"));
    ASSERT_TRUE(test_str.has_value() && test_str.type() == typeid(std::string));
    EXPECT_EQ(std::any_cast<std::string>(test_str), "replace wstring!");
    EXPECT_TRUE(AssignAnyType(test_float, float(7.0)));
    ASSERT_TRUE(test_float.has_value() && test_float.type() == typeid(FLOAT));
    EXPECT_EQ(std::any_cast<FLOAT>(test_float), float(7.0));
    EXPECT_TRUE(AssignAnyType(test_double, float(7.0)));
    ASSERT_TRUE(test_double.has_value() && test_double.type() == typeid(DOUBLE));
    EXPECT_EQ(std::any_cast<DOUBLE>(test_double), double(float(7.0)));
}

#endif
