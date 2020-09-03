#include "stdafx.h"
#include "MetadataImpl.h"
#include "utils.h"
#include "ParsingImpl.h"

namespace cchips {
    //const std::map<int, size_t> CBaseDef_d::_base_to_size_def_d = {
    //            {type_invalid, 0},{type_void, 0},{type_byte, sizeof(BYTE)},{type_bool, sizeof(BOOL)},{type_char, sizeof(CHAR)},{type_uchar, sizeof(UCHAR)},{type_int16, sizeof(INT16)},{type_uint16, sizeof(UINT16)},{type_short, sizeof(SHORT)},
    //            {type_ushort, sizeof(USHORT)},{type_word, sizeof(WORD)},{type_wchar, sizeof(WCHAR)},{type_int, sizeof(INT)},{type_uint, sizeof(UINT)},{type_long, sizeof(LONG)},{type_long_ptr, sizeof(LONG_PTR)},{type_ulong, sizeof(ULONG)},{type_ulong_ptr,sizeof(ULONG_PTR)},
    //            {type_dword, sizeof(DWORD)},{type_dword_ptr, sizeof(DWORD_PTR)},{type_float, sizeof(FLOAT)},{type_int64, sizeof(INT64)},{type_uint64, sizeof(UINT64)},{type_longlong, sizeof(LONGLONG)},{type_ulonglong, sizeof(ULONGLONG)},{type_qword, sizeof(ULONGLONG)},
    //            {type_double, sizeof(double)},{type_large_integer, sizeof(LARGE_INTEGER)},{type_ularge_integer, sizeof(ULARGE_INTEGER)},{type_lpvoid, sizeof(LPVOID)},{type_hmodule, sizeof(HMODULE)},{type_hresult, sizeof(HRESULT)},{type_schandle, sizeof(SC_HANDLE)},
    //            {type_ntstatus, sizeof(NTSTATUS)},{type_string, sizeof(std::string)},{type_wstring, sizeof(std::wstring) }, {type_handle, sizeof(HANDLE)}, {type_guid, sizeof(GUID)},
    //};

    //const std::map<int, std::string> CBaseDef_d::_base_to_str_def_d = {
    //        {type_invalid, "INVALID"},{type_void, "VOID"},{type_byte, "BYTE"},{type_bool, "BOOL"},{type_char, "CHAR"},{type_uchar, "UCHAR"},{type_int16, "INT16"},{type_uint16, "UINT16"},{type_short, "SHORT"},
    //        {type_ushort, "USHORT"},{type_word, "WORD"},{type_wchar, "WCHAR"},{type_int, "INT"},{type_uint, "UINT"},{type_long, "LONG"},{type_long_ptr, "LONG_PTR"},{type_ulong, "ULONG"},{type_ulong_ptr, "ULONG_PTR"},
    //        {type_dword, "DWORD"},{type_dword_ptr, "DWORD_PTR"},{type_float, "FLOAT"},{type_int64, "INT64"},{type_uint64, "UINT64"},{type_longlong, "LONGLONG"},{type_ulonglong, "ULONGLONG"},{type_qword, "QWORD"},
    //        {type_double, "DOUBLE"},{type_large_integer, "LARGE_INTEGER"},{type_ularge_integer, "ULARGE_INTEGER"},{type_lpvoid, "LPVOID"},{type_hmodule, "HMODULE"},{type_hresult, "HRESULT"},{type_schandle, "SC_HANDLE"},
    //        {type_ntstatus, "NTSTATUS"},{type_string, "std::string"},{type_wstring, "std::wstring"},{type_handle, "HANDLE"},{type_guid, "GUID"},
    //};

} // namespace cchips