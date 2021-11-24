#pragma once
#include "asmfunction.h"
#include <WbemCli.h>
#include <WMIUtils.h>
#include <OCIdl.h>
#include <vector>
#include <codecvt>
#include <variant>
#include <any>
#include <setjmp.h>

#define ASM_CALL_RELATIVE   0xE8
#define ASM_JMP_RELATIVE    0xE9
#define ASM_CALL_ABSOLUTE   0x15FF
#define ASM_JMP_ABSOLUTE    0x25FF
#define ASM_CALL_REGISTRY   0xFF

using tls_check_struct = struct {
    jmp_buf jb;
    bool active;
};

bool ExtractResource(HMODULE ModuleHandle, TCHAR const * ResourceName,
    TCHAR const * ResourceId, std::vector<BYTE>& ResoureBuffer);
bool vir_strcpy_s(char* dst, size_t dst_len, const void* src);
bool vir_wcscpy_s(wchar_t* dst, size_t dst_len, const void* src);
std::wstring A2WString(const std::string& str);
std::string W2AString(const std::wstring& str);
std::string StringToLower(const std::string& str);
VARTYPE ConvertObTypeToVarintType(const std::string& type_name);
std::any GetVariantValue(const VARIANT& Value);
bool SetVariantValue(VARIANT& Value, std::any& anyvalue);
void tls_check_index_init();
tls_check_struct* check_get_tls();
void check_return();
void WalkFrameCurrentChain(std::vector<PVOID>& frame_chain);
VOID GetStackRange(ULONG_PTR* base, ULONG_PTR* limit);
int GetCallOpSize(const void* addr);
void ClearThreadTls();
char hexbyte(char hex);
bool hexfromstring(BYTE* bytes, int bytes_len, const char* str, int hex_len);
std::string hexstring(BYTE byte);
void *memmem(const void *buf, size_t buf_len, const void *byte_sequence, size_t byte_sequence_len);

inline std::wstring to_wide_string(const std::string& input)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    tls_check_struct *tls = check_get_tls();
    if (tls) {
        tls->active = 1;
        if (setjmp(tls->jb) == 0) {
            std::wstring tmp = converter.from_bytes(input);
            tls->active = 0;
            return tmp;
        }
        //execption occur
        tls->active = 0;
    }
    return {};
}
inline std::string to_byte_string(const std::wstring& input)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    tls_check_struct *tls = check_get_tls();
    if (tls) {
        tls->active = 1;
        if (setjmp(tls->jb) == 0) {
            std::string tmp;
            tmp = converter.to_bytes(input);
            tls->active = 0;
            return tmp;
        }
        //execption occur
        tls->active = 0;
    }
    return {};
}

template<typename Char> struct CaseInsensitiveCompare {
public:
    bool operator()(Char x, Char y) const {
        return tolower(x) == tolower(y);
    }
};

template <typename STR>
bool StartsWithT(const STR& str, const STR& search, bool case_sensitive) {
    if (case_sensitive) {
        return str.compare(0, search.length(), search) == 0;
    }
    else {
        if (search.size() > str.size())
            return false;
        return std::equal(search.begin(), search.end(), str.begin(),
            CaseInsensitiveCompare<typename STR::value_type>());
    }
}

bool StartsWith(const std::string& str, const std::string& search,
    bool case_sensitive);
bool StartsWith(const std::wstring& str, const std::wstring& search,
    bool case_sensitive);

template <typename STR>
bool EndsWithT(const STR& str, const STR& search, bool case_sensitive) {
    typename STR::size_type str_length = str.length();
    typename STR::size_type search_length = search.length();
    if (search_length > str_length)
        return false;
    if (case_sensitive) {
        return str.compare(str_length - search_length, search_length, search) == 0;
    }
    else {
        return std::equal(search.begin(), search.end(),
            str.begin() + (str_length - search_length),
            CaseInsensitiveCompare<typename STR::value_type>());
    }
}

bool EndsWith(const std::string& str, const std::string& search,
    bool case_sensitive);
bool EndsWith(const std::wstring& str, const std::wstring& search,
    bool case_sensitive);

template <typename Func>
inline void getExecutionTime(const std::string& title, Func func) {
    const auto sta = std::chrono::steady_clock::now();
    func();
    const std::chrono::duration<double> dur = std::chrono::steady_clock::now() - sta;
    std::cout << title << ": " << dur.count() << " sec." << std::endl;
}

template <class F> struct y_combinator {
    F f; // the lambda will be stored here

    // a forwarding operator():
    template <class... Args> decltype(auto) operator()(Args &&... args) const {
        // we pass ourselves to f, then the arguments.
        // the lambda should take the first argument as `auto&& recurse` or similar.
        return f(*this, std::forward<Args>(args)...);
    }
};
// helper function that deduces the type of the lambda:
template <class F> y_combinator<std::decay_t<F>> make_y_combinator(F &&f) {
    return { std::forward<F>(f) };
}

inline std::stringstream OutputAnyValue(const std::any& anyvalue) {
    std::stringstream ss;
    if (!anyvalue.has_value()) return ss;
    if (anyvalue.type() == typeid(DWORD))
        ss << std::any_cast<DWORD>(anyvalue);
    else if (anyvalue.type() == typeid(UINT))
        ss << std::any_cast<UINT>(anyvalue);
    else if (anyvalue.type() == typeid(INT16))
        ss << std::any_cast<INT16>(anyvalue);
    else if (anyvalue.type() == typeid(UINT16))
        ss << std::any_cast<UINT16>(anyvalue);
    else if (anyvalue.type() == typeid(SHORT))
        ss << std::any_cast<SHORT>(anyvalue);
    else if (anyvalue.type() == typeid(USHORT))
        ss << std::any_cast<USHORT>(anyvalue);
    else if (anyvalue.type() == typeid(WORD))
        ss << std::any_cast<WORD>(anyvalue);
    else if (anyvalue.type() == typeid(INT))
        ss << std::any_cast<INT>(anyvalue);
    else if (anyvalue.type() == typeid(LONG))
        ss << std::any_cast<LONG>(anyvalue);
    else if (anyvalue.type() == typeid(BOOL))
        ss << std::boolalpha << std::any_cast<BOOL>(anyvalue);
    else if (anyvalue.type() == typeid(BYTE))
        ss << std::any_cast<BYTE>(anyvalue);
    else if (anyvalue.type() == typeid(CHAR))
        ss << std::any_cast<CHAR>(anyvalue);
    else if (anyvalue.type() == typeid(UCHAR))
        ss << std::any_cast<UCHAR>(anyvalue);
    else if (anyvalue.type() == typeid(WCHAR))
        ss << std::any_cast<WCHAR>(anyvalue);
    else if (anyvalue.type() == typeid(LONG_PTR))
        ss << std::any_cast<LONG_PTR>(anyvalue);
    else if (anyvalue.type() == typeid(ULONG))
        ss << std::any_cast<ULONG>(anyvalue);
    else if (anyvalue.type() == typeid(ULONG_PTR))
        ss << std::any_cast<ULONG_PTR>(anyvalue);
    else if (anyvalue.type() == typeid(DWORD_PTR))
        ss << std::any_cast<DWORD_PTR>(anyvalue);
    else if (anyvalue.type() == typeid(INT64))
        ss << std::any_cast<INT64>(anyvalue);
    else if (anyvalue.type() == typeid(UINT64))
        ss << std::any_cast<UINT64>(anyvalue);
    else if (anyvalue.type() == typeid(LONGLONG))
        ss << std::any_cast<LONGLONG>(anyvalue);
    else if (anyvalue.type() == typeid(ULONGLONG))
        ss << std::any_cast<ULONGLONG>(anyvalue);
    else if (anyvalue.type() == typeid(HRESULT))
        ss << std::any_cast<HRESULT>(anyvalue);
    else if (anyvalue.type() == typeid(NTSTATUS))
        ss << std::any_cast<NTSTATUS>(anyvalue);
    else if (anyvalue.type() == typeid(PVOID)) {
        PVOID ptr = std::any_cast<PVOID>(anyvalue);
        if (ptr == nullptr)
            ss << "nullptr";
        else
            ss << std::hex << "0x" << ptr;
    }
    else if (anyvalue.type() == typeid(HANDLE)) {
        HANDLE ptr = std::any_cast<HANDLE>(anyvalue);
        if (ptr == nullptr)
            ss << "nullptr";
        else
            ss << std::hex << "0x" << ptr;
    }
    else if (anyvalue.type() == typeid(HMODULE)) {
        HMODULE ptr = std::any_cast<HMODULE>(anyvalue);
        if (ptr == nullptr)
            ss << "nullptr";
        else
            ss << std::hex << "0x" << ptr;
    }
    else if (anyvalue.type() == typeid(SC_HANDLE)) {
        SC_HANDLE ptr = std::any_cast<SC_HANDLE>(anyvalue);
        if (ptr == nullptr)
            ss << "nullptr";
        else
            ss << std::hex << "0x" << ptr;
    }
    else if (anyvalue.type() == typeid(FLOAT))
        ss << std::showpoint << std::any_cast<FLOAT>(anyvalue);
    else if (anyvalue.type() == typeid(DOUBLE))
        ss << std::showpoint << std::any_cast<DOUBLE>(anyvalue);
    else if (anyvalue.type() == typeid(LARGE_INTEGER))
        ss << std::any_cast<LARGE_INTEGER>(anyvalue).QuadPart;
    else if (anyvalue.type() == typeid(ULARGE_INTEGER))
        ss << std::any_cast<ULARGE_INTEGER>(anyvalue).QuadPart;
    else if (anyvalue.type() == typeid(std::string))
        ss << std::any_cast<std::string>(anyvalue);
    else if (anyvalue.type() == typeid(std::wstring)) {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        tls_check_struct *tls = check_get_tls();
        if (tls) {
            tls->active = 1;
            if (setjmp(tls->jb) == 0) {
                ss << converter.to_bytes(std::any_cast<std::wstring>(anyvalue));
                tls->active = 0;
                return ss;
            }
            //execption occur
            tls->active = 0;
            ss.clear();
            ss.str("");
            return ss;
        }
    }
    else if (anyvalue.type() == typeid(GUID)) {
        WCHAR *clsid_str = nullptr;
        GUID guid = std::any_cast<GUID>(anyvalue);
        HRESULT hr = StringFromCLSID(guid, &clsid_str);
        if (!FAILED(hr)) {
            if (clsid_str) {
                ss << to_byte_string(clsid_str);
                CoTaskMemFree(clsid_str);
            }
        }
    }
    return ss;
}
inline std::any GetAnyValue(const std::any& anyvalue) {
    if (!anyvalue.has_value()) return {};
    if (anyvalue.type() == typeid(DWORD))
        return static_cast<ULONGLONG>(std::any_cast<DWORD>(anyvalue));
    else if (anyvalue.type() == typeid(UINT))
        return static_cast<ULONGLONG>(std::any_cast<UINT>(anyvalue));
    else if (anyvalue.type() == typeid(INT16))
        return static_cast<ULONGLONG>(std::any_cast<INT16>(anyvalue));
    else if (anyvalue.type() == typeid(UINT16))
        return static_cast<ULONGLONG>(std::any_cast<UINT16>(anyvalue));
    else if (anyvalue.type() == typeid(SHORT))
        return static_cast<ULONGLONG>(std::any_cast<SHORT>(anyvalue));
    else if (anyvalue.type() == typeid(USHORT))
        return static_cast<ULONGLONG>(std::any_cast<USHORT>(anyvalue));
    else if (anyvalue.type() == typeid(WORD))
        return static_cast<ULONGLONG>(std::any_cast<WORD>(anyvalue));
    else if (anyvalue.type() == typeid(INT))
        return static_cast<ULONGLONG>(std::any_cast<INT>(anyvalue));
    else if (anyvalue.type() == typeid(LONG))
        return static_cast<ULONGLONG>(std::any_cast<LONG>(anyvalue));
    else if (anyvalue.type() == typeid(BOOL))
        return static_cast<ULONGLONG>(std::any_cast<BOOL>(anyvalue));
    else if (anyvalue.type() == typeid(BYTE))
        return static_cast<ULONGLONG>(std::any_cast<BYTE>(anyvalue));
    else if (anyvalue.type() == typeid(CHAR))
        return static_cast<ULONGLONG>(std::any_cast<CHAR>(anyvalue));
    else if (anyvalue.type() == typeid(UCHAR))
        return static_cast<ULONGLONG>(std::any_cast<UCHAR>(anyvalue));
    else if (anyvalue.type() == typeid(WCHAR))
        return static_cast<ULONGLONG>(std::any_cast<WCHAR>(anyvalue));
    else if (anyvalue.type() == typeid(LONG_PTR))
        return static_cast<ULONGLONG>(std::any_cast<LONG_PTR>(anyvalue));
    else if (anyvalue.type() == typeid(ULONG))
        return static_cast<ULONGLONG>(std::any_cast<ULONG>(anyvalue));
    else if (anyvalue.type() == typeid(ULONG_PTR))
        return static_cast<ULONGLONG>(std::any_cast<ULONG_PTR>(anyvalue));
    else if (anyvalue.type() == typeid(DWORD_PTR))
        return static_cast<ULONGLONG>(std::any_cast<DWORD_PTR>(anyvalue));
    else if (anyvalue.type() == typeid(INT64))
        return static_cast<ULONGLONG>(std::any_cast<INT64>(anyvalue));
    else if (anyvalue.type() == typeid(UINT64))
        return static_cast<ULONGLONG>(std::any_cast<UINT64>(anyvalue));
    else if (anyvalue.type() == typeid(LONGLONG))
        return static_cast<ULONGLONG>(std::any_cast<LONGLONG>(anyvalue));
    else if (anyvalue.type() == typeid(ULONGLONG))
        return static_cast<ULONGLONG>(std::any_cast<ULONGLONG>(anyvalue));
    else if (anyvalue.type() == typeid(HRESULT))
        return static_cast<ULONGLONG>(std::any_cast<HRESULT>(anyvalue));
    else if (anyvalue.type() == typeid(NTSTATUS))
        return static_cast<ULONGLONG>(std::any_cast<NTSTATUS>(anyvalue));
    else if (anyvalue.type() == typeid(PVOID))
        return ULONGLONG(std::any_cast<PVOID>(anyvalue));
    else if (anyvalue.type() == typeid(HANDLE))
        return reinterpret_cast<ULONGLONG>(std::any_cast<HANDLE>(anyvalue));
    else if (anyvalue.type() == typeid(HMODULE))
        return reinterpret_cast<ULONGLONG>(std::any_cast<HMODULE>(anyvalue));
    else if (anyvalue.type() == typeid(SC_HANDLE))
        return reinterpret_cast<ULONGLONG>(std::any_cast<SC_HANDLE>(anyvalue));
    if (anyvalue.type() == typeid(FLOAT))
        return static_cast<DOUBLE>(std::any_cast<FLOAT>(anyvalue));
    else if (anyvalue.type() == typeid(DOUBLE))
        return static_cast<DOUBLE>(std::any_cast<DOUBLE>(anyvalue));
    if (anyvalue.type() == typeid(LARGE_INTEGER))
        return static_cast<ULONGLONG>(std::any_cast<LARGE_INTEGER>(anyvalue).QuadPart);
    if (anyvalue.type() == typeid(ULARGE_INTEGER))
        return static_cast<ULONGLONG>(std::any_cast<ULARGE_INTEGER>(anyvalue).QuadPart);
    if (anyvalue.type() == typeid(std::string))
        return std::any_cast<std::string>(anyvalue);
    if (anyvalue.type() == typeid(std::wstring))
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        tls_check_struct *tls = check_get_tls();
        if (tls) {
            tls->active = 1;
            if (setjmp(tls->jb) == 0) {
                std::string tmp;
                tmp = converter.to_bytes(std::any_cast<std::wstring>(anyvalue));
                tls->active = 0;
                return tmp;
            }
            //execption occur
            tls->active = 0;
        }
    }
    return {};
}
template <typename Ty>
inline Ty ConvertAnyType(const std::any& anyvalue) {
    if constexpr (std::is_integral_v<Ty>) {
        if (!anyvalue.has_value()) return static_cast<Ty>(-1);
        if (anyvalue.type() == typeid(DWORD))
            return static_cast<Ty>(std::any_cast<DWORD>(anyvalue));
        else if (anyvalue.type() == typeid(UINT))
            return static_cast<Ty>(std::any_cast<UINT>(anyvalue));
        else if (anyvalue.type() == typeid(INT16))
            return static_cast<Ty>(std::any_cast<INT16>(anyvalue));
        else if (anyvalue.type() == typeid(UINT16))
            return static_cast<Ty>(std::any_cast<UINT16>(anyvalue));
        else if (anyvalue.type() == typeid(SHORT))
            return static_cast<Ty>(std::any_cast<SHORT>(anyvalue));
        else if (anyvalue.type() == typeid(USHORT))
            return static_cast<Ty>(std::any_cast<USHORT>(anyvalue));
        else if (anyvalue.type() == typeid(WORD))
            return static_cast<Ty>(std::any_cast<WORD>(anyvalue));
        else if (anyvalue.type() == typeid(INT))
            return static_cast<Ty>(std::any_cast<INT>(anyvalue));
        else if (anyvalue.type() == typeid(LONG))
            return static_cast<Ty>(std::any_cast<LONG>(anyvalue));
        else if (anyvalue.type() == typeid(BOOL))
            return static_cast<Ty>(std::any_cast<BOOL>(anyvalue));
        else if (anyvalue.type() == typeid(BYTE))
            return static_cast<Ty>(std::any_cast<BYTE>(anyvalue));
        else if (anyvalue.type() == typeid(CHAR))
            return static_cast<Ty>(std::any_cast<CHAR>(anyvalue));
        else if (anyvalue.type() == typeid(UCHAR))
            return static_cast<Ty>(std::any_cast<UCHAR>(anyvalue));
        else if (anyvalue.type() == typeid(WCHAR))
            return static_cast<Ty>(std::any_cast<WCHAR>(anyvalue));
        else if (anyvalue.type() == typeid(LONG_PTR))
            return static_cast<Ty>(std::any_cast<LONG_PTR>(anyvalue));
        else if (anyvalue.type() == typeid(ULONG))
            return static_cast<Ty>(std::any_cast<ULONG>(anyvalue));
        else if (anyvalue.type() == typeid(ULONG_PTR))
            return static_cast<Ty>(std::any_cast<ULONG_PTR>(anyvalue));
        else if (anyvalue.type() == typeid(DWORD_PTR))
            return static_cast<Ty>(std::any_cast<DWORD_PTR>(anyvalue));
        else if (anyvalue.type() == typeid(INT64))
            return static_cast<Ty>(std::any_cast<INT64>(anyvalue));
        else if (anyvalue.type() == typeid(UINT64))
            return static_cast<Ty>(std::any_cast<UINT64>(anyvalue));
        else if (anyvalue.type() == typeid(LONGLONG))
            return static_cast<Ty>(std::any_cast<LONGLONG>(anyvalue));
        else if (anyvalue.type() == typeid(ULONGLONG))
            return static_cast<Ty>(std::any_cast<ULONGLONG>(anyvalue));
        else if (anyvalue.type() == typeid(HRESULT))
            return static_cast<Ty>(std::any_cast<HRESULT>(anyvalue));
        else if (anyvalue.type() == typeid(NTSTATUS))
            return static_cast<Ty>(std::any_cast<NTSTATUS>(anyvalue));
        return static_cast<Ty>(-1);
    } 
    else if constexpr (std::is_pointer_v<Ty>) {
        if (!anyvalue.has_value()) return static_cast<Ty>(nullptr);
        if (anyvalue.type() == typeid(PVOID))
            return static_cast<Ty>(std::any_cast<PVOID>(anyvalue));
        else if (anyvalue.type() == typeid(HANDLE))
            return reinterpret_cast<Ty>(std::any_cast<HANDLE>(anyvalue));
        else if (anyvalue.type() == typeid(HMODULE))
            return reinterpret_cast<Ty>(std::any_cast<HMODULE>(anyvalue));
        else if (anyvalue.type() == typeid(SC_HANDLE))
            return reinterpret_cast<Ty>(std::any_cast<SC_HANDLE>(anyvalue));
        return static_cast<Ty>(nullptr);
    }
    else if constexpr (std::is_floating_point_v<Ty>) {
        if (!anyvalue.has_value()) return static_cast<Ty>(-1.0);
        if (anyvalue.type() == typeid(FLOAT))
            return static_cast<Ty>(std::any_cast<FLOAT>(anyvalue));
        else if (anyvalue.type() == typeid(DOUBLE))
            return static_cast<Ty>(std::any_cast<DOUBLE>(anyvalue));
        return static_cast<Ty>(-1.0);
    }
}
template <>
inline LARGE_INTEGER ConvertAnyType(const std::any& anyvalue) {
    if (!anyvalue.has_value()) return LARGE_INTEGER{};
    if (anyvalue.type() == typeid(LARGE_INTEGER))
        return std::any_cast<LARGE_INTEGER>(anyvalue);
    return LARGE_INTEGER{};
}
template <>
inline ULARGE_INTEGER ConvertAnyType(const std::any& anyvalue) {
    if (!anyvalue.has_value()) return ULARGE_INTEGER{};
    if (anyvalue.type() == typeid(ULARGE_INTEGER))
        return std::any_cast<ULARGE_INTEGER>(anyvalue);
    return ULARGE_INTEGER{};
}
template <>
inline std::string ConvertAnyType(const std::any& anyvalue) {
    if (!anyvalue.has_value()) return static_cast<char*>(nullptr);
    if (anyvalue.type() == typeid(std::string))
        return std::any_cast<std::string>(anyvalue);
    else if (anyvalue.type() == typeid(std::wstring))
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        tls_check_struct *tls = check_get_tls();
        if (tls) {
            tls->active = 1;
            if (setjmp(tls->jb) == 0) {
                std::string tmp;
                tmp = converter.to_bytes(std::any_cast<std::wstring>(anyvalue));
                tls->active = 0;
                return tmp;
            }
            //execption occur
            tls->active = 0;
        }
    }
    return static_cast<char*>(nullptr);
}
template <>
inline std::wstring ConvertAnyType(const std::any& anyvalue) {
    if (!anyvalue.has_value()) return static_cast<wchar_t*>(nullptr);
    if (anyvalue.type() == typeid(std::wstring))
        return std::any_cast<std::wstring>(anyvalue);
    else if (anyvalue.type() == typeid(std::string))
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        tls_check_struct *tls = check_get_tls();
        if (tls) {
            tls->active = 1;
            if (setjmp(tls->jb) == 0) {
                std::wstring tmp;
                tmp = converter.from_bytes(std::any_cast<std::string>(anyvalue));
                tls->active = 0;
                return tmp;
            }
            //execption occur
            tls->active = 0;
        }
        return {};
    }
    return static_cast<wchar_t*>(nullptr);
}
template <typename Ty>
inline bool AssignAnyType(std::any& anyvalue, Ty value) {
    if constexpr (std::is_integral_v<Ty>) {
        if (anyvalue.type() == typeid(DWORD))
        {
            anyvalue = static_cast<DWORD>(value); 
            return true;
        }
        else if (anyvalue.type() == typeid(UINT))
        {
            anyvalue = static_cast<UINT>(value);
            return true;
        }
        else if (anyvalue.type() == typeid(INT16))
        {
            anyvalue = static_cast<INT16>(value);
            return true;
        }
        else if (anyvalue.type() == typeid(UINT16))
        {
            anyvalue = static_cast<UINT16>(value);
            return true;
        }
        else if (anyvalue.type() == typeid(SHORT))
        {
            anyvalue = static_cast<SHORT>(value);
            return true;
        }
        else if (anyvalue.type() == typeid(USHORT))
        {
            anyvalue = static_cast<USHORT>(value);
            return true;
        }
        else if (anyvalue.type() == typeid(WORD))
        {
            anyvalue = static_cast<WORD>(value);
            return true;
        }
        else if (anyvalue.type() == typeid(INT))
        {
            anyvalue = static_cast<INT>(value);
            return true;
        }
        else if (anyvalue.type() == typeid(LONG))
        {
            anyvalue = static_cast<LONG>(value);
            return true;
        }
        else if (anyvalue.type() == typeid(BOOL))
        {
            anyvalue = static_cast<BOOL>(value);
            return true;
        }
        else if (anyvalue.type() == typeid(BYTE))
        {
            anyvalue = static_cast<BYTE>(value);
            return true;
        }
        else if (anyvalue.type() == typeid(CHAR))
        {
            anyvalue = static_cast<CHAR>(value);
            return true;
        }
        else if (anyvalue.type() == typeid(UCHAR))
        {
            anyvalue = static_cast<UCHAR>(value);
            return true;
        }
        else if (anyvalue.type() == typeid(WCHAR))
        {
            anyvalue = static_cast<WCHAR>(value);
            return true;
        }
        else if (anyvalue.type() == typeid(LONG_PTR))
        {
            anyvalue = static_cast<LONG_PTR>(value);
            return true;
        }
        else if (anyvalue.type() == typeid(ULONG))
        {
            anyvalue = static_cast<ULONG>(value);
            return true;
        }
        else if (anyvalue.type() == typeid(ULONG_PTR))
        {
            anyvalue = static_cast<ULONG_PTR>(value);
            return true;
        }
        else if (anyvalue.type() == typeid(DWORD_PTR))
        {
            anyvalue = static_cast<DWORD_PTR>(value);
            return true;
        }
        else if (anyvalue.type() == typeid(INT64))
        {
            anyvalue = static_cast<INT64>(value);
            return true;
        }
        else if (anyvalue.type() == typeid(UINT64))
        {
            anyvalue = static_cast<UINT64>(value);
            return true;
        }
        else if (anyvalue.type() == typeid(LONGLONG))
        {
            anyvalue = static_cast<LONGLONG>(value);
            return true;
        }
        else if (anyvalue.type() == typeid(ULONGLONG))
        {
            anyvalue = static_cast<ULONGLONG>(value);
            return true;
        }
        else if (anyvalue.type() == typeid(ULARGE_INTEGER))
        {
            ULARGE_INTEGER tmp;
            tmp.QuadPart = static_cast<ULONGLONG>(value);
            anyvalue = tmp;
            return true;
        }
        else if (anyvalue.type() == typeid(LARGE_INTEGER))
        {
            LARGE_INTEGER tmp;
            tmp.QuadPart = static_cast<LONGLONG>(value);
            anyvalue = tmp;
            return true;
        }
        else if (anyvalue.type() == typeid(HRESULT))
        {
            anyvalue = static_cast<HRESULT>(value);
            return true;
        }
        else if (anyvalue.type() == typeid(NTSTATUS))
        {
            anyvalue = static_cast<NTSTATUS>(value);
            return true;
        }
        else if (anyvalue.type() == typeid(std::string))
        {
            std::stringstream ss;
            ss << value;
            anyvalue = std::string(ss.str());
            return true;
        }
        else if (anyvalue.type() == typeid(std::wstring))
        {
            std::stringstream ss;
            ss << value;
            std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
            tls_check_struct *tls = check_get_tls();
            if (tls) {
                tls->active = 1;
                if (setjmp(tls->jb) == 0) {
                    anyvalue = converter.from_bytes(std::string(ss.str()));
                    tls->active = 0;
                    return true;
                }
                //execption occur
                tls->active = 0;
            }
            return false;
        }
    }
    else if constexpr (std::is_pointer_v<Ty>) {
        if (anyvalue.type() == typeid(PVOID))
        {
            anyvalue = reinterpret_cast<PVOID>(value);
            return true;
        }
        else if(anyvalue.type() == typeid(HANDLE))
        {
            anyvalue = reinterpret_cast<HANDLE>(value);
            return true;
        }
        else if (anyvalue.type() == typeid(HMODULE))
        {
            anyvalue = reinterpret_cast<HMODULE>(value);
            return true;
        }
        else if (anyvalue.type() == typeid(SC_HANDLE))
        {
            anyvalue = reinterpret_cast<SC_HANDLE>(value);
            return true;
        }
        else if (anyvalue.type() == typeid(std::string))
        {
            std::stringstream ss;
            ss << std::hex << value;
            anyvalue = std::string(ss.str());
            return true;
        }
        else if (anyvalue.type() == typeid(std::wstring))
        {
            std::stringstream ss;
            ss << std::hex << value;
            std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
            tls_check_struct *tls = check_get_tls();
            if (tls) {
                tls->active = 1;
                if (setjmp(tls->jb) == 0) {
                    anyvalue = converter.from_bytes(std::string(ss.str()));
                    tls->active = 0;
                    return true;
                }
                //execption occur
                tls->active = 0;
            }
            return false;
        }
    }
    else if constexpr (std::is_floating_point_v<Ty>) {
        if (anyvalue.type() == typeid(FLOAT))
        {
            anyvalue = static_cast<FLOAT>(value);
            return true;
        }
        else if (anyvalue.type() == typeid(DOUBLE))
        {
            anyvalue = static_cast<DOUBLE>(value);
            return true;
        }
        else if (anyvalue.type() == typeid(std::string))
        {
            std::stringstream ss;
            ss << value;
            anyvalue = std::string(ss.str());
            return true;
        }
        else if (anyvalue.type() == typeid(std::wstring))
        {
            std::stringstream ss;
            ss << value;
            std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
            tls_check_struct *tls = check_get_tls();
            if (tls) {
                tls->active = 1;
                if (setjmp(tls->jb) == 0) {
                    anyvalue = converter.from_bytes(std::string(ss.str()));
                    tls->active = 0;
                    return true;
                }
                //execption occur
                tls->active = 0;
            }
            return false;
        }
    }
    return false;
}
template <>
inline bool AssignAnyType(std::any& anyvalue, char* value) {
    if (anyvalue.type() == typeid(std::string))
    {
        anyvalue = std::string(value);
        return true;
    }
    return false;
}
template <>
inline bool AssignAnyType(std::any& anyvalue, const char* value) {
    if (anyvalue.type() == typeid(std::string))
    {
        anyvalue = std::string(value);
        return true;
    }
    else if (anyvalue.type() == typeid(std::wstring))
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        tls_check_struct *tls = check_get_tls();
        if (tls) {
            tls->active = 1;
            if (setjmp(tls->jb) == 0) {
                anyvalue = converter.from_bytes(std::string(value));
                tls->active = 0;
                return true;
            }
            //execption occur
            tls->active = 0;
        }
        return false;
    }
    else if(anyvalue.type() == typeid(FLOAT) || anyvalue.type() == typeid(DOUBLE))
    {
        char* nodig = 0;
        return AssignAnyType(anyvalue, strtod(value, &nodig));
    }
    else
    {
        char* nodig = 0;
        return AssignAnyType(anyvalue, strtoull(value, &nodig, 10));
    }
    return false;
}
template <>
inline bool AssignAnyType(std::any& anyvalue, wchar_t* value) {
    if (anyvalue.type() == typeid(std::wstring))
    {
        anyvalue = std::wstring(static_cast<wchar_t*>(value));
        return true;
    }
    else
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        tls_check_struct *tls = check_get_tls();
        if (tls) {
            tls->active = 1;
            if (setjmp(tls->jb) == 0) {
                bool tmp = AssignAnyType(anyvalue, converter.to_bytes(value).c_str());
                tls->active = 0;
                return tmp;
            }
            //execption occur
            tls->active = 0;
        }
    }
    return false;
}
template <>
inline bool AssignAnyType(std::any& anyvalue, const wchar_t* value) {
    if (anyvalue.type() == typeid(std::wstring))
    {
        anyvalue = std::wstring(value);
        return true;
    }
    return false;
}
template <>
inline bool AssignAnyType(std::any& anyvalue, std::any value) {
    if (!value.has_value()) return false;
    if (value.type() == typeid(ULONGLONG)) {
        return AssignAnyType(anyvalue, std::any_cast<ULONGLONG>(value));
    }
    else if (value.type() == typeid(DOUBLE)) {
        return AssignAnyType(anyvalue, std::any_cast<DOUBLE>(value));
    }
    else if (value.type() == typeid(std::string)) {
        return AssignAnyType(anyvalue, std::any_cast<std::string>(value).c_str());
    }
    else if (value.type() == typeid(DWORD)) {
        return AssignAnyType(anyvalue, std::any_cast<DWORD>(value));
    }
    else if (value.type() == typeid(UINT)) {
        return AssignAnyType(anyvalue, std::any_cast<UINT>(value));
    }
    else if (value.type() == typeid(INT16)) {
        return AssignAnyType(anyvalue, std::any_cast<INT16>(value));
    }
    else if (value.type() == typeid(UINT16)) {
        return AssignAnyType(anyvalue, std::any_cast<UINT16>(value));
    }
    else if (value.type() == typeid(SHORT)) {
        return AssignAnyType(anyvalue, std::any_cast<SHORT>(value));
    }
    else if (value.type() == typeid(USHORT)) {
        return AssignAnyType(anyvalue, std::any_cast<USHORT>(value));
    }
    else if (value.type() == typeid(WORD)) {
        return AssignAnyType(anyvalue, std::any_cast<WORD>(value));
    }
    else if (value.type() == typeid(INT)) {
        return AssignAnyType(anyvalue, std::any_cast<INT>(value));
    }
    else if (value.type() == typeid(LONG)) {
        return AssignAnyType(anyvalue, std::any_cast<LONG>(value));
    }
    else if (value.type() == typeid(BOOL)) {
        return AssignAnyType(anyvalue, std::any_cast<BOOL>(value));
    }
    else if (value.type() == typeid(BYTE)) {
        return AssignAnyType(anyvalue, std::any_cast<BYTE>(value));
    }
    else if (value.type() == typeid(CHAR)) {
        return AssignAnyType(anyvalue, std::any_cast<CHAR>(value));
    }
    else if (value.type() == typeid(UCHAR)) {
        return AssignAnyType(anyvalue, std::any_cast<UCHAR>(value));
    }
    else if (value.type() == typeid(WCHAR)) {
        return AssignAnyType(anyvalue, std::any_cast<WCHAR>(value));
    }
    else if (value.type() == typeid(LONG_PTR)) {
        return AssignAnyType(anyvalue, std::any_cast<LONG_PTR>(value));
    }
    else if (value.type() == typeid(ULONG)) {
        return AssignAnyType(anyvalue, std::any_cast<ULONG>(value));
    }
    else if (value.type() == typeid(ULONG_PTR)) {
        return AssignAnyType(anyvalue, std::any_cast<ULONG_PTR>(value));
    }
    else if (value.type() == typeid(DWORD_PTR)) {
        return AssignAnyType(anyvalue, std::any_cast<DWORD_PTR>(value));
    }
    else if (value.type() == typeid(INT64)) {
        return AssignAnyType(anyvalue, std::any_cast<INT64>(value));
    }
    else if (value.type() == typeid(UINT64)) {
        return AssignAnyType(anyvalue, std::any_cast<UINT64>(value));
    }
    else if (value.type() == typeid(LONGLONG)) {
        return AssignAnyType(anyvalue, std::any_cast<LONGLONG>(value));
    }
    else if (value.type() == typeid(HRESULT)) {
        return AssignAnyType(anyvalue, std::any_cast<HRESULT>(value));
    }
    else if (value.type() == typeid(NTSTATUS)) {
        return AssignAnyType(anyvalue, std::any_cast<NTSTATUS>(value));
    }
    else if (value.type() == typeid(PVOID)) {
        return AssignAnyType(anyvalue, std::any_cast<PVOID>(value));
    }
    else if (value.type() == typeid(HANDLE)) {
        return AssignAnyType(anyvalue, std::any_cast<HANDLE>(value));
    }
    else if (value.type() == typeid(HMODULE)) {
        return AssignAnyType(anyvalue, std::any_cast<HMODULE>(value));
    }
    else if (value.type() == typeid(SC_HANDLE)) {
        return AssignAnyType(anyvalue, std::any_cast<SC_HANDLE>(value));
    }
    else if (value.type() == typeid(FLOAT)) {
        return AssignAnyType(anyvalue, std::any_cast<FLOAT>(value));
    }
    else if (value.type() == typeid(LARGE_INTEGER)) {
        return AssignAnyType(anyvalue, std::any_cast<LARGE_INTEGER>(value));
    }
    else if (value.type() == typeid(ULARGE_INTEGER)) {
        return AssignAnyType(anyvalue, std::any_cast<ULARGE_INTEGER>(value));
    }
    else if (value.type() == typeid(std::wstring)) {
        return AssignAnyType(anyvalue, std::any_cast<std::wstring>(value).c_str());
    }
    else if (value.type() == typeid(GUID)) {
        return AssignAnyType(anyvalue, std::any_cast<GUID>(value));
    }
    return false;
}

template <typename In, typename Out, typename T, typename F>
inline In split_algorithm(In it, In end_it, Out out_it, T split_val,
    F bin_func)
{
    while (it != end_it) {
        auto slice_end(find(it, end_it, split_val));
        *out_it++ = bin_func(it, slice_end);

        if (slice_end == end_it) { return end_it; }
        it = next(slice_end);
    }
    return it;
}

#ifdef _X86_
#define TLS_TEB 0x18
#define TLS_TIB 0x2C
#define TLS_PEB 0x30
#endif
#ifdef _AMD64_
#define TLS_TEB 0x30
#define TLS_TIB 0x58
#define TLS_PEB 0x60
#endif

static inline DWORD_PTR readtls(DWORD index)
{
#ifdef _X86_
    return (DWORD)__readfsdword(index);
#endif
#ifdef _AMD64_
    return (DWORD_PTR)__readgsqword(index);
#endif
    return 0;
}

static inline void writetls(DWORD index, DWORD_PTR value)
{
#ifdef _X86_
    __writefsdword(index, value);
#endif
#ifdef _AMD64_
    __writegsqword(index, value);
#endif
    return;
}

DWORD NativeFetchMovEaxImmOffset(const char* func);

#ifdef _DEBUGGER 
#ifdef _AMD64_
#define BreakPoint BreakInt3();
#else
#define BreakPoint __asm int 3
#endif
#else
#define BreakPoint
#endif



