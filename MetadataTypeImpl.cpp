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

    const std::vector<CBaseDef::_standard_type> CReferenceObject::_str_ref_types = {
        CBaseDef::type_char, CBaseDef::type_uchar, CBaseDef::type_wchar };

    int CTypeSymbolTableObject::m_reference_count = 0;
    std::unique_ptr<CTypeSymbolTableObject> g_typesymbolobject(CTypeSymbolTableObject::GetInstance());

    bool CMetadataTypeObject::FreshTraits(std::unique_ptr<void>& value, int op)
    {
        ASSERT(value != nullptr);
        if (!value) return false;

        std::shared_ptr<CBaseDef> newdef = m_pmetadatadef->Clone();
        if (newdef)
        {
            if ((*newdef).FreshTraits(value, op))
                m_pmetadatadef = newdef;
        }
        return false;
    }

    void CTypeSymbolTableObject::InitializeTypeSymbolTable() {
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
        m_typesymboltable[CBaseDef::GetBaseStr(CBaseDef::type_int64)] = make_metadata_s_ptr<INT64>(CBaseDef::type_int64);
        m_typesymboltable[CBaseDef::GetBaseStr(CBaseDef::type_uint64)] = make_metadata_s_ptr<UINT64>(CBaseDef::type_uint64);
        m_typesymboltable[CBaseDef::GetBaseStr(CBaseDef::type_longlong)] = make_metadata_s_ptr<LONGLONG>(CBaseDef::type_longlong);
        m_typesymboltable[CBaseDef::GetBaseStr(CBaseDef::type_ulonglong)] = make_metadata_s_ptr<ULONGLONG>(CBaseDef::type_ulonglong);
        m_typesymboltable[CBaseDef::GetBaseStr(CBaseDef::type_qword)] = make_metadata_s_ptr<ULONGLONG>(CBaseDef::type_qword);
        m_typesymboltable[CBaseDef::GetBaseStr(CBaseDef::type_large_integer)] = make_metadata_j_ptr<LARGE_INTEGER>(CBaseDef::type_large_integer, CTraits<LARGE_INTEGER>({}, CTraits<LARGE_INTEGER>::op_n_equal));
        m_typesymboltable[CBaseDef::GetBaseStr(CBaseDef::type_ularge_integer)] = make_metadata_j_ptr<ULARGE_INTEGER>(CBaseDef::type_ularge_integer, CTraits<ULARGE_INTEGER>({}, CTraits<ULARGE_INTEGER>::op_n_equal));
        //	m_typesymboltable[CBaseDef::GetBaseStr(CBaseDef::type_lpvoid)] = make_metadata_j_ptr<LPVOID>(CBaseDef::type_lpvoid, CTraits<LPVOID>(nullptr, CTraits<LPVOID>::op_n_equal));
        m_typesymboltable[CBaseDef::GetBaseStr(CBaseDef::type_handle)] = make_metadata_j_ptr<HANDLE>(CBaseDef::type_handle, CTraits<HANDLE>(nullptr, CTraits<HANDLE>::op_n_equal));
        m_typesymboltable[CBaseDef::GetBaseStr(CBaseDef::type_hmodule)] = make_metadata_j_ptr<HMODULE>(CBaseDef::type_hmodule, CTraits<HMODULE>(nullptr, CTraits<HMODULE>::op_n_equal));
        m_typesymboltable[CBaseDef::GetBaseStr(CBaseDef::type_hresult)] = make_metadata_j_ptr<HRESULT>(CBaseDef::type_hresult, CTraits<HRESULT>(0, CTraits<HRESULT>::op_greater));
        m_typesymboltable[CBaseDef::GetBaseStr(CBaseDef::type_schandle)] = make_metadata_j_ptr<SC_HANDLE>(CBaseDef::type_schandle, CTraits<SC_HANDLE>(nullptr, CTraits<SC_HANDLE>::op_n_equal));
        m_typesymboltable[CBaseDef::GetBaseStr(CBaseDef::type_ntstatus)] = make_metadata_j_ptr<NTSTATUS>(CBaseDef::type_ntstatus, CTraits<NTSTATUS>(0, CTraits<NTSTATUS>::op_greater));
        m_typesymboltable[CBaseDef::GetBaseStr(CBaseDef::type_string)] = make_metadata_j_ptr<std::string>(CBaseDef::type_string, CTraits<std::string>("\0", CTraits<std::string>::op_n_equal));
        m_typesymboltable[CBaseDef::GetBaseStr(CBaseDef::type_wstring)] = make_metadata_j_ptr<std::wstring>(CBaseDef::type_wstring, CTraits<std::wstring>(L"\0", CTraits<std::wstring>::op_n_equal));
        m_typesymboltable[CBaseDef::GetBaseStr(CBaseDef::type_guid)] = make_metadata_j_ptr<GUID>(CBaseDef::type_guid, CTraits<GUID>({}, CTraits<GUID>::op_n_equal));
        return;
    }

    template <typename T>
    bool CTraits<T>::SetValue(char* pdata, const std::stringstream& ss)
    {
        if (IsValidValue(pdata))
        {
            T* p = reinterpret_cast<T*>(pdata);
            std::stringstream tmp_ss;
            tmp_ss << ss.str();
            vir_sstream_copy<T>((char*)p, tmp_ss);
            return true;
        }
        return false;
    }

    bool CReferenceObject::SetValue(char* pdata, const std::stringstream& ss)
    {
        if (*reinterpret_cast<wchar_t**>(pdata) == nullptr)
        {
            *reinterpret_cast<wchar_t**>(pdata) = nullptr;
            return true;
        }
        if (GetObType() == ob_stringref)
        {
            if (m_pelement->GetMetadataDef()->GetBaseDef() == CBaseDef::type_wchar)
            {
                return vir_wcscpy_s(*reinterpret_cast<wchar_t**>(pdata), wcslen(*reinterpret_cast<wchar_t**>(pdata)) + 1, A2WString(ss.str().c_str()).c_str());
            }
            else
            {
                return vir_strcpy_s(*reinterpret_cast<char**>(pdata), strlen(*reinterpret_cast<char**>(pdata)) + 1, ss.str().c_str());
            }
        }
        else
        {
            std::stringstream tmp_ss;
            if (ss.str()[0] == '0' && ss.str()[0] == 'x')
                tmp_ss << ss.str().substr(2);
            else
                tmp_ss << ss.str();
            vir_sstream_copy<ULONG_PTR>(pdata, tmp_ss);
        }
        return false;
    }

    bool CWmiObject::SetMidlInterface(const std::stringstream& ss) {
        if (m_midl_interface == nullptr)
            m_midl_interface = make_metadata_j_ptr<GUID>(CBaseDef::type_guid, CTraits<GUID>({}, CTraits<GUID>::op_n_equal));
        ASSERT(m_midl_interface != nullptr);
        return m_midl_interface->GetMetadataDef()->SetValue(ss);
    }

} // namespace cchips

