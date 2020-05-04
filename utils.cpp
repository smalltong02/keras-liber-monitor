#include "stdafx.h"
#include "utils.h"

std::wstring A2WString(const std::string& str)
{
    std::wstring str_w;
    str_w.resize(str.length());
    for (int x = 0; x < str.length(); ++x)
    {
        str_w[x] = str[x];
    }
    return str_w;
}

std::string W2AString(const std::wstring& str)
{
    std::string str_a;
    str_a.resize(str.length());
    for (int x = 0; x < str.length(); ++x)
    {
        str_a[x] = str[x];
    }
    return str_a;
}

bool ExtractResource(HMODULE ModuleHandle, TCHAR const * ResourceName,
    TCHAR const * ResourceId, std::vector<BYTE>& ResoureBuffer)
{
    if (ModuleHandle == NULL)
    {
        ModuleHandle = GetModuleHandle(NULL);
    }

    HRSRC resourceInfo = FindResource(ModuleHandle, ResourceId, ResourceName);
    if (resourceInfo == NULL)
    {
        DWORD const lastError = GetLastError();
        return false;
    }

    DWORD const resourceSize = SizeofResource(ModuleHandle, resourceInfo);
    if (resourceSize == 0)
    {
        DWORD const lastError = GetLastError();
        return false;
    }

    HGLOBAL resourceHandle = LoadResource(ModuleHandle, resourceInfo);
    if (resourceHandle == NULL)
    {
        DWORD const lastError = GetLastError();
        return false;
    }

    void * resourceData = LockResource(resourceHandle);
    if (resourceData == NULL)
    {
        DWORD const lastError = GetLastError();
        return false;
    }

    ResoureBuffer.resize(resourceSize);
    memcpy_s(&ResoureBuffer[0], ResoureBuffer.size(), resourceData, resourceSize);

    return true;
}

bool vir_wcscpy_s(wchar_t* dst, size_t dst_len, const wchar_t* src)
{
    if (dst_len >= wcslen(src))
    {
        DWORD OldProtect = 0;
        if (VirtualProtect(reinterpret_cast<LPVOID>(dst), dst_len * sizeof(wchar_t), PAGE_READWRITE, &OldProtect))
        {
            wcscpy_s(dst, dst_len, src);
            VirtualProtect(reinterpret_cast<LPVOID>(dst), dst_len * sizeof(wchar_t), OldProtect, &OldProtect);
            return true;
        }
    }
    return false;
}

bool vir_strcpy_s(char* dst, size_t dst_len, const char* src)
{
    if (dst_len >= strlen(src))
    {
        DWORD OldProtect = 0;
        if (VirtualProtect(reinterpret_cast<LPVOID>(dst), dst_len, PAGE_READWRITE, &OldProtect))
        {
            strcpy_s(dst, dst_len, src);
            VirtualProtect(reinterpret_cast<LPVOID>(dst), dst_len, OldProtect, &OldProtect);
            return true;
        }
    }
    return false;
}

bool GetValueString(const VARIANT& Value, std::string& ValueString)
{
    switch (Value.vt)
    {
    case VT_BSTR:
        ValueString = W2AString(Value.bstrVal);
        return true;
    case VT_BOOL:
        if (Value.boolVal)
            ValueString = "true";
        else
            ValueString = "false";
        return true;
    case VT_UI1:
    case VT_I1:
        ValueString = std::to_string(Value.bVal);
        return true;
    case VT_I2:
    case VT_INT:
        ValueString = std::to_string(Value.iVal);
        return true;
    case VT_I4:
        ValueString = std::to_string(Value.intVal);
        return true;
    case VT_I8:
        ValueString = std::to_string(Value.llVal);
        return true;
    case VT_UI2:
    case VT_UINT:
        ValueString = std::to_string(Value.uiVal);
        return true;
    case VT_UI4:
        ValueString = std::to_string(Value.ulVal);
        return true;
    case VT_UI8:
        ValueString = std::to_string(Value.ullVal);
        return true;
    case VT_DATE:
        ValueString = std::to_string(Value.date);
        return true;
    case VT_VOID:
        ValueString = std::to_string((ULONG_PTR)Value.byref);
        return true;
    default:
        break;
    }

    return false;
}

bool SetValueString(const std::string& ValueString, VARIANT& Value)
{
    if (!ValueString.length()) return false;

    switch (Value.vt)
    {
    case VT_BSTR:
    {
        VARIANT Val;
        Val.vt = Value.vt;
        Val.bstrVal = SysAllocString(A2WString(ValueString).c_str());
        Value = Val;
        return true;
    }
    case VT_BOOL:
        if (_stricmp(ValueString.c_str(), "true") == 0)
            Value.boolVal = true;
        else
            Value.boolVal = false;
        return true;
    case VT_UI1:
    case VT_I1:
    {
        char* nodig = nullptr;
        Value.bVal = (BYTE)strtol(ValueString.c_str(), &nodig, 16);
        return true;
    }
    case VT_I2:
    case VT_INT:
    {
        char* nodig = nullptr;
        Value.iVal = (SHORT)strtol(ValueString.c_str(), &nodig, 16);
        return true;
    }
    case VT_I4:
    {
        char* nodig = nullptr;
        Value.intVal = (INT)strtol(ValueString.c_str(), &nodig, 16);
        return true;
    }
    case VT_I8:
    {
        char* nodig = nullptr;
        Value.llVal = (LONGLONG)strtoll(ValueString.c_str(), &nodig, 16);
        return true;
    }
    case VT_UI2:
    case VT_UINT:
    {
        char* nodig = nullptr;
        Value.uiVal = (USHORT)strtoll(ValueString.c_str(), &nodig, 16);
        return true;
    }
    case VT_UI4:
    {
        char* nodig = nullptr;
        Value.ulVal = (ULONG)strtoll(ValueString.c_str(), &nodig, 16);
        return true;
    }
    case VT_UI8:
    {
        char* nodig = nullptr;
        Value.ullVal = (ULONGLONG)strtoll(ValueString.c_str(), &nodig, 16);
        return true;
    }
    case VT_DATE:
    {
        char* nodig = nullptr;
        Value.date = (double)strtod(ValueString.c_str(), &nodig);
        return true;
    }
    case VT_VOID:
    {
        char* nodig = nullptr;
        Value.byref = (PVOID)strtoll(ValueString.c_str(), &nodig, 16);
        return true;
    }
    default:
        break;
    }

    return false;

}
