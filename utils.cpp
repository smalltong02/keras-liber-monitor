#include "stdafx.h"
#include "utils.h"
#include "MetadataTypeImpl.h"
#include "capstone\include\capstone.h"

static csh g_capstone_handle = 0;

#ifdef _X86_
static const cs_err g_cap_error = cs_open(CS_ARCH_X86, CS_MODE_32, &g_capstone_handle);
#endif
#ifdef _AMD64_
static const cs_err g_cap_error = cs_open(CS_ARCH_X86, CS_MODE_64, &g_capstone_handle);
#endif

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

VARTYPE ConvertObTypeToVarintType(const std::string& type_name)
{
    VARTYPE var_type = VT_EMPTY;
    if (_stricmp(type_name.c_str(), "UINT") == 0)
        var_type = VT_I4;
    else if (_stricmp(type_name.c_str(), "OLECHAR*") == 0)
        var_type = VT_BSTR;
    return var_type;
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

int lde(const void *addr)
{
    if (!addr) return 0;
    if (g_cap_error != CS_ERR_OK ||
        g_capstone_handle == 0) return 0;

    cs_insn *insn = nullptr;
    size_t count =
        cs_disasm_ex(g_capstone_handle, reinterpret_cast<const uint8_t*>(addr), 16, reinterpret_cast<uintptr_t>(addr), 1, &insn);
    if (count == 0) return 0;
    if (!insn) return 0;

    int size = insn->size;

    cs_free(insn, count);
    return size;
}

DWORD NativeFetchMovEaxImmOffset(const char* func)
{
    if (!func) return 0;
    if (g_cap_error != CS_ERR_OK ||
        g_capstone_handle == 0) return 0;

    for (uint32_t idx = 0; idx < 32; idx++) {
        if (memcmp(func, "\x8b\x80", 2) == 0) {
            return *(uint32_t *)(func + 2);
        }
        if (memcmp(func, "\x8b\x40", 2) == 0) {
            return func[2];
        }
        func += lde(func);
    }
    return 0;
}
