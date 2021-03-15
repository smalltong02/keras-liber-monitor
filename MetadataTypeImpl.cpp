//===- MetadataTypeImpl.cpp - metadata type implement code -------------------*- C++ -*-===//
// 
// This file implements the standrad type, additional type and metadata data class code.
//
//===-----------------------------------------------------------------------------------===//

#include "stdafx.h"
#include "CBaseType.h"
#include "MetadataTypeImpl.h"
#include "utils.h"
#include "ParsingImpl.h"

namespace cchips {

    const std::map<CBaseDef::_standard_type, size_t> CBaseDef::_base_to_size_def = {
        {type_invalid, 0},{type_void, 0},{type_byte, sizeof(BYTE)},{type_bool, sizeof(BOOL)},{type_char, sizeof(CHAR)},{type_uchar, sizeof(UCHAR)},{type_int16, sizeof(INT16)},{type_uint16, sizeof(UINT16)},{type_short, sizeof(SHORT)},
        {type_ushort, sizeof(USHORT)},{type_word, sizeof(WORD)},{type_wchar, sizeof(WCHAR)},{type_int, sizeof(INT)},{type_uint, sizeof(UINT)},{type_long, sizeof(LONG)},{type_long_ptr, sizeof(LONG_PTR)},{type_ulong, sizeof(ULONG)},{type_ulong_ptr,sizeof(ULONG_PTR)},
        {type_dword, sizeof(DWORD)},{type_dword_ptr, sizeof(DWORD_PTR)},{type_float, sizeof(FLOAT)},{type_int64, sizeof(INT64)},{type_uint64, sizeof(UINT64)},{type_longlong, sizeof(LONGLONG)},{type_ulonglong, sizeof(ULONGLONG)},{type_qword, sizeof(ULONGLONG)},
        {type_double, sizeof(double)},{type_large_integer, sizeof(LARGE_INTEGER)},{type_ularge_integer, sizeof(ULARGE_INTEGER)},{type_lpvoid, sizeof(LPVOID)},{type_hmodule, sizeof(HMODULE)},{type_hinstance, sizeof(HINSTANCE)},{type_hresult, sizeof(HRESULT)},
        {type_schandle, sizeof(SC_HANDLE)},{type_ntstatus, sizeof(NTSTATUS)},{type_string, sizeof(std::string)},{type_wstring, sizeof(std::wstring) }, {type_handle, sizeof(HANDLE)}, {type_guid, sizeof(GUID)},
    };

    const std::map<CBaseDef::_standard_type, std::string> CBaseDef::_base_to_str_def = {
            {type_invalid, "\0"},{type_void, "VOID"},{type_byte, "BYTE"},{type_bool, "BOOL"},{type_char, "CHAR"},{type_uchar, "UCHAR"},{type_int16, "INT16"},{type_uint16, "UINT16"},{type_short, "SHORT"},
            {type_ushort, "USHORT"},{type_word, "WORD"},{type_wchar, "WCHAR"},{type_int, "INT"},{type_uint, "UINT"},{type_long, "LONG"},{type_long_ptr, "LONG_PTR"},{type_ulong, "ULONG"},{type_ulong_ptr, "ULONG_PTR"},
            {type_dword, "DWORD"},{type_dword_ptr, "DWORD_PTR"},{type_float, "FLOAT"},{type_int64, "INT64"},{type_uint64, "UINT64"},{type_longlong, "LONGLONG"},{type_ulonglong, "ULONGLONG"},{type_qword, "QWORD"},
            {type_double, "DOUBLE"},{type_large_integer, "LARGE_INTEGER"},{type_ularge_integer, "ULARGE_INTEGER"},{type_lpvoid, "LPVOID"},{type_hmodule, "HMODULE"},{type_hinstance, "HINSTANCE"},{type_hresult, "HRESULT"},
            {type_schandle, "SC_HANDLE"},{type_ntstatus, "NTSTATUS"},{type_string, "std::string"},{type_wstring, "std::wstring"},{type_handle, "HANDLE"},{type_guid, "GUID"},
    };
    const std::vector<CBaseDef::_standard_type> CReferenceObject::_str_ref_types = {
        CBaseDef::type_char, CBaseDef::type_uchar, CBaseDef::type_wchar };

    void CTypeSymbolTableObject::InitializeTypeSymbolTable() {
        tls_check_index_init();
        m_typesymboltable[CBaseDef::GetBaseStr(CBaseDef::type_void)] = make_metadata_s_ptr<ULONG>(CBaseDef::type_void);
        m_typesymboltable[CBaseDef::GetBaseStr(CBaseDef::type_byte)] = make_metadata_s_ptr<BYTE>(CBaseDef::type_byte);
        m_typesymboltable[CBaseDef::GetBaseStr(CBaseDef::type_bool)] = make_metadata_s_ptr<BOOL>(CBaseDef::type_bool);
        m_typesymboltable[CBaseDef::GetBaseStr(CBaseDef::type_char)] = make_metadata_s_ptr<CHAR>(CBaseDef::type_char);
        m_typesymboltable[CBaseDef::GetBaseStr(CBaseDef::type_uchar)] = make_metadata_s_ptr<UCHAR>(CBaseDef::type_uchar);
        m_typesymboltable[CBaseDef::GetBaseStr(CBaseDef::type_int16)] = make_metadata_s_ptr<INT16>(CBaseDef::type_int16);
        m_typesymboltable[CBaseDef::GetBaseStr(CBaseDef::type_uint16)] = make_metadata_s_ptr<UINT16>(CBaseDef::type_uint16);
        m_typesymboltable[CBaseDef::GetBaseStr(CBaseDef::type_short)] = make_metadata_s_ptr<SHORT>(CBaseDef::type_short);
        m_typesymboltable[CBaseDef::GetBaseStr(CBaseDef::type_ushort)] = make_metadata_s_ptr<USHORT>(CBaseDef::type_ushort);
        m_typesymboltable[CBaseDef::GetBaseStr(CBaseDef::type_word)] = make_metadata_s_ptr<WORD>(CBaseDef::type_word);
        m_typesymboltable[CBaseDef::GetBaseStr(CBaseDef::type_wchar)] = make_metadata_s_ptr<WCHAR>(CBaseDef::type_wchar);
        m_typesymboltable[CBaseDef::GetBaseStr(CBaseDef::type_int)] = make_metadata_s_ptr<INT>(CBaseDef::type_int);
        m_typesymboltable[CBaseDef::GetBaseStr(CBaseDef::type_uint)] = make_metadata_s_ptr<UINT>(CBaseDef::type_uint);
        m_typesymboltable[CBaseDef::GetBaseStr(CBaseDef::type_long)] = make_metadata_s_ptr<LONG>(CBaseDef::type_long);
        m_typesymboltable[CBaseDef::GetBaseStr(CBaseDef::type_long_ptr)] = make_metadata_s_ptr<LONG_PTR>(CBaseDef::type_long_ptr);
        m_typesymboltable[CBaseDef::GetBaseStr(CBaseDef::type_ulong)] = make_metadata_s_ptr<ULONG>(CBaseDef::type_ulong);
        m_typesymboltable[CBaseDef::GetBaseStr(CBaseDef::type_ulong_ptr)] = make_metadata_s_ptr<ULONG_PTR>(CBaseDef::type_ulong_ptr);
        m_typesymboltable[CBaseDef::GetBaseStr(CBaseDef::type_dword)] = make_metadata_s_ptr<DWORD>(CBaseDef::type_dword);
        m_typesymboltable[CBaseDef::GetBaseStr(CBaseDef::type_dword_ptr)] = make_metadata_s_ptr<DWORD_PTR>(CBaseDef::type_dword_ptr);
        m_typesymboltable[CBaseDef::GetBaseStr(CBaseDef::type_float)] = make_metadata_s_ptr<FLOAT>(CBaseDef::type_float);
        m_typesymboltable[CBaseDef::GetBaseStr(CBaseDef::type_double)] = make_metadata_s_ptr<DOUBLE>(CBaseDef::type_double);
        m_typesymboltable[CBaseDef::GetBaseStr(CBaseDef::type_int64)] = make_metadata_s_ptr<INT64>(CBaseDef::type_int64);
        m_typesymboltable[CBaseDef::GetBaseStr(CBaseDef::type_uint64)] = make_metadata_s_ptr<UINT64>(CBaseDef::type_uint64);
        m_typesymboltable[CBaseDef::GetBaseStr(CBaseDef::type_longlong)] = make_metadata_s_ptr<LONGLONG>(CBaseDef::type_longlong);
        m_typesymboltable[CBaseDef::GetBaseStr(CBaseDef::type_ulonglong)] = make_metadata_s_ptr<ULONGLONG>(CBaseDef::type_ulonglong);
        m_typesymboltable[CBaseDef::GetBaseStr(CBaseDef::type_qword)] = make_metadata_s_ptr<ULONGLONG>(CBaseDef::type_qword);
        m_typesymboltable[CBaseDef::GetBaseStr(CBaseDef::type_large_integer)] = make_metadata_j_ptr<LARGE_INTEGER>(CBaseDef::type_large_integer, {}, CObObject::op_n_equal);
        m_typesymboltable[CBaseDef::GetBaseStr(CBaseDef::type_ularge_integer)] = make_metadata_j_ptr<ULARGE_INTEGER>(CBaseDef::type_ularge_integer, {}, CObObject::op_n_equal);
        //m_typesymboltable[CBaseDef::GetBaseStr(CBaseDef::type_lpvoid)] = make_metadata_j_ptr<LPVOID>(CBaseDef::type_lpvoid, nullptr, CObObject::op_n_equal);
        m_typesymboltable[CBaseDef::GetBaseStr(CBaseDef::type_handle)] = make_metadata_j_ptr<HANDLE>(CBaseDef::type_handle, nullptr, CObObject::op_n_equal);
        m_typesymboltable[CBaseDef::GetBaseStr(CBaseDef::type_hmodule)] = make_metadata_j_ptr<HMODULE>(CBaseDef::type_hmodule, nullptr, CObObject::op_n_equal);
        m_typesymboltable[CBaseDef::GetBaseStr(CBaseDef::type_hinstance)] = make_metadata_j_ptr<HINSTANCE>(CBaseDef::type_hinstance, nullptr, CObObject::op_n_equal);
        m_typesymboltable[CBaseDef::GetBaseStr(CBaseDef::type_hresult)] = make_metadata_j_ptr<HRESULT>(CBaseDef::type_hresult, S_OK, CObObject::op_greater_e);
        m_typesymboltable[CBaseDef::GetBaseStr(CBaseDef::type_schandle)] = make_metadata_j_ptr<SC_HANDLE>(CBaseDef::type_schandle, nullptr, CObObject::op_n_equal);
        m_typesymboltable[CBaseDef::GetBaseStr(CBaseDef::type_ntstatus)] = make_metadata_j_ptr<NTSTATUS>(CBaseDef::type_ntstatus, 0, CObObject::op_greater_e);
        m_typesymboltable[CBaseDef::GetBaseStr(CBaseDef::type_string)] = make_metadata_j_ptr<std::string>(CBaseDef::type_string, {}, CObObject::op_n_equal);
        m_typesymboltable[CBaseDef::GetBaseStr(CBaseDef::type_wstring)] = make_metadata_j_ptr<std::wstring>(CBaseDef::type_wstring, {}, CObObject::op_n_equal);
        m_typesymboltable[CBaseDef::GetBaseStr(CBaseDef::type_guid)] = make_metadata_j_ptr<GUID>(CBaseDef::type_guid, {}, CObObject::op_n_equal);
        return;
    }

    template <typename T>
    bool CBaseType<T>::SetValue(char* pdata, const std::any& anyvalue) const {
        if (GetBaseDef() == CBaseDef::type_void)
            return false;
        if (IsValidValue(pdata))
        {
            return vir_sstream_copy<T>(pdata, anyvalue);
        }
        return false;
    }

    bool CReferenceObject::SetValue(char* pdata, const std::any& anyvalue) const
    {
        if (IsValidValue(pdata))
        {
            if (*reinterpret_cast<wchar_t**>(pdata) == nullptr)
            {
                return false;
            }

            if (GetObType() == ob_stringref)
            {
                if (anyvalue.has_value() && m_pelement)
                {
                    if (std::static_pointer_cast<CBaseType<BYTE>>(m_pelement->GetMetadataDef())->GetBaseDef() == CBaseDef::type_wchar)
                    {
                        if(anyvalue.type() == typeid(PVOID))
                            return vir_wcscpy_s(*reinterpret_cast<wchar_t**>(pdata), wcslen(*reinterpret_cast<wchar_t**>(pdata)) + 1, std::any_cast<PVOID>(anyvalue));
                        else if (anyvalue.type() == typeid(std::wstring))
                            return vir_wcscpy_s(*reinterpret_cast<wchar_t**>(pdata), wcslen(*reinterpret_cast<wchar_t**>(pdata)) + 1, std::any_cast<std::wstring>(anyvalue).c_str());
                        else if (anyvalue.type() == typeid(std::string))
                            return vir_wcscpy_s(*reinterpret_cast<wchar_t**>(pdata), wcslen(*reinterpret_cast<wchar_t**>(pdata)) + 1, ConvertAnyType<std::wstring>(anyvalue).c_str());
                    }
                    else
                    {
                        if(anyvalue.type() == typeid(PVOID))
                            return vir_strcpy_s(*reinterpret_cast<char**>(pdata), strlen(*reinterpret_cast<char**>(pdata)) + 1, std::any_cast<PVOID>(anyvalue));
                        else if (anyvalue.type() == typeid(std::string))
                            return vir_wcscpy_s(*reinterpret_cast<wchar_t**>(pdata), wcslen(*reinterpret_cast<wchar_t**>(pdata)) + 1, std::any_cast<std::string>(anyvalue).c_str());
                        else if (anyvalue.type() == typeid(std::wstring))
                            return vir_wcscpy_s(*reinterpret_cast<wchar_t**>(pdata), wcslen(*reinterpret_cast<wchar_t**>(pdata)) + 1, ConvertAnyType<std::string>(anyvalue).c_str());
                    }
                }
            }
            else
            {
                return vir_sstream_copy<LPVOID>(pdata, anyvalue);
            }
        }
        return false;
    }

    std::any GuidToAnyValue(const std::stringstream& ss) {
        GUID guid = { };
        if (ss.str().length()) {
            std::string pattern_str = "^\\{([0-9a-fA-F]{8})-([0-9a-fA-F]{4})-([0-9a-fA-F]{4})-([0-9a-fA-F]{4})-([0-9a-fA-F]{12})\\}$";
            RE2 pattern_guid(pattern_str);
            std::string data1, data2, data3, data4, data5;
            if (RE2::FullMatch(ss.str(), pattern_guid, &data1, &data2, &data3, &data4, &data5))
            {
                auto sstrtoul = [](const std::string& in) ->unsigned long {char* nodig = 0; unsigned long digit = strtoul(in.c_str(), &nodig, 16); return digit; };
                auto sstrtous = [](const std::string& in) ->unsigned short {char* nodig = 0; unsigned short digit = (unsigned short)strtoul(in.c_str(), &nodig, 16); return digit; };
                auto re_strtoss = [](const std::string& in, int count, int n) ->std::string { std::stringstream ss; int m = n; do { ss << in[count + n]; ss << in[count + n + 1]; n = n - 2; } while (n >= 0); count += 2 * m; return ss.str(); };
                guid.Data1 = sstrtoul(data1);
                guid.Data2 = sstrtous(data2);
                guid.Data3 = sstrtous(data3);
                *((unsigned short*)guid.Data4) = sstrtous(re_strtoss(data4, 0, 2));
                *((unsigned short*)&guid.Data4[2]) = sstrtous(re_strtoss(data5, 0, 2));
                *((unsigned long*)&guid.Data4[4]) = sstrtoul(re_strtoss(data5, 4, 6));
                return guid;
            }
        }
        return {};
    }

    bool CWmiObject::SetMidlInterface(const std::stringstream& ss) {
        if (m_midl_interface == nullptr)
            m_midl_interface = make_metadata_j_ptr<GUID>(CBaseDef::type_guid, {}, CObObject::op_n_equal);
        ASSERT(m_midl_interface != nullptr);
        return m_midl_interface->GetMetadataDef()->SetCurValue(GuidToAnyValue(ss));
    }

} // namespace cchips

