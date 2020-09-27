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

std::string StringToLower(const std::string& str)
{
    std::string lower_str = str;
    transform(lower_str.begin(), lower_str.end(), lower_str.begin(), ::toupper);
    return lower_str;
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

bool vir_wcscpy_s(wchar_t* dst, size_t dst_len, const void* src)
{
    const wchar_t* src_cp = (const wchar_t*)src;
    if (dst_len >= wcslen(src_cp))
    {
        DWORD OldProtect = 0;
        if (VirtualProtect(reinterpret_cast<LPVOID>(dst), dst_len * sizeof(wchar_t), PAGE_READWRITE, &OldProtect))
        {
            wcscpy_s(dst, dst_len, src_cp);
            VirtualProtect(reinterpret_cast<LPVOID>(dst), dst_len * sizeof(wchar_t), OldProtect, &OldProtect);
            return true;
        }
    }
    return false;
}

bool vir_strcpy_s(char* dst, size_t dst_len, const void* src)
{
    const char* src_cp = (const char*)src;
    if (dst_len >= strlen(src_cp))
    {
        DWORD OldProtect = 0;
        if (VirtualProtect(reinterpret_cast<LPVOID>(dst), dst_len, PAGE_READWRITE, &OldProtect))
        {
            strcpy_s(dst, dst_len, src_cp);
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

std::any GetVariantValue(const VARIANT& Value)
{
    switch (Value.vt)
    {
    case VT_BSTR:
    {
        std::string str;
        str = W2AString(Value.bstrVal);
        return str;
    }
    case VT_BOOL:
    {
        if (Value.boolVal)
            return true;
        return false;
    }
    case VT_UI1:
    case VT_I1:
        return Value.bVal;
    case VT_I2:
    case VT_INT:
        return Value.iVal;
    case VT_I4:
        return Value.intVal;
    case VT_I8:
        return Value.llVal;
    case VT_UI2:
    case VT_UINT:
        return Value.uiVal;
    case VT_UI4:
        return Value.ulVal;
    case VT_UI8:
        return Value.ullVal;
    case VT_DATE:
        return Value.date;
    case VT_VOID:
        return Value.byref;
    default:
        break;
    }
    return {};
}

bool SetVariantValue(VARIANT& Value, std::any& anyvalue)
{
    if (!anyvalue.has_value())
        return false;
    switch (Value.vt)
    {
    case VT_BSTR:
    {
        VARIANT Val;
        Val.vt = Value.vt;
        std::string str = ConvertAnyType<std::string>(anyvalue);
        if (str.length()) {
            Val.bstrVal = SysAllocString(A2WString(str).c_str());
            Value = Val;
            return true;
        }
        return false;
    }
    case VT_BOOL:
    {
        Value.boolVal = ConvertAnyType<bool>(anyvalue);
        return true;
    }
    case VT_UI1:
    case VT_I1:
    {
        Value.bVal = ConvertAnyType<BYTE>(anyvalue);
        return true;
    }
    case VT_I2:
    case VT_INT:
    {
        Value.iVal = ConvertAnyType<SHORT>(anyvalue);
        return true;
    }
    case VT_I4:
    {
        Value.intVal = ConvertAnyType<INT>(anyvalue);
        return true;
    }
    case VT_I8:
    {
        Value.llVal = ConvertAnyType<LONGLONG>(anyvalue);
        return true;
    }
    case VT_UI2:
    case VT_UINT:
    {
        Value.uiVal = ConvertAnyType<USHORT>(anyvalue);
        return true;
    }
    case VT_UI4:
    {
        Value.ulVal = ConvertAnyType<ULONG>(anyvalue);
        return true;
    }
    case VT_UI8:
    {
        Value.ullVal = ConvertAnyType<ULONGLONG>(anyvalue);
        return true;
    }
    case VT_DATE:
    {
        return false;
    }
    case VT_VOID:
    {
        Value.byref = ConvertAnyType<PVOID>(anyvalue);
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

static DWORD g_tls_check_index = TLS_OUT_OF_INDEXES;

void tls_check_index_init()
{
    g_tls_check_index = TlsAlloc();
}

// don't free this memory until process exit.
tls_check_struct* check_get_tls()
{
    if (g_tls_check_index == TLS_OUT_OF_INDEXES) return nullptr;
    tls_check_struct* ret = (tls_check_struct*)TlsGetValue(g_tls_check_index);
    if (ret == nullptr) {
        ret = (tls_check_struct*)VirtualAlloc(nullptr, sizeof(tls_check_struct), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        TlsSetValue(g_tls_check_index, ret);
    }
    return ret;
}

void check_return()
{
    tls_check_struct *tls = check_get_tls();

    if (tls->active != 0) {
        longjmp(tls->jb, 1);
    }
}
