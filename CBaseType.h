#pragma once
#include <Windows.h>
#include <cstdio>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <sstream>
#include <cassert>
#include <regex>
#include "MetadataTypeImpl.h"
#include "utils.h"

namespace cchips {

const std::map<int, size_t> CBaseDef::_base_to_size_def = {
		{type_invalid, 0},{type_void, 0},{type_byte, sizeof(BYTE)},{type_bool, sizeof(BOOL)},{type_char, sizeof(CHAR)},{type_uchar, sizeof(UCHAR)},{type_int16, sizeof(INT16)},{type_uint16, sizeof(UINT16)},{type_short, sizeof(SHORT)},
		{type_ushort, sizeof(USHORT)},{type_word, sizeof(WORD)},{type_wchar, sizeof(WCHAR)},{type_int, sizeof(INT)},{type_uint, sizeof(UINT)},{type_long, sizeof(LONG)},{type_long_ptr, sizeof(LONG_PTR)},{type_ulong, sizeof(ULONG)},{type_ulong_ptr,sizeof(ULONG_PTR)},
		{type_dword, sizeof(DWORD)},{type_dword_ptr, sizeof(DWORD_PTR)},{type_float, sizeof(FLOAT)},{type_int64, sizeof(INT64)},{type_uint64, sizeof(UINT64)},{type_longlong, sizeof(LONGLONG)},{type_ulonglong, sizeof(ULONGLONG)},{type_qword, sizeof(ULONGLONG)},
		{type_double, sizeof(double)},{type_large_integer, sizeof(LARGE_INTEGER)},{type_ularge_integer, sizeof(ULARGE_INTEGER)},{type_lpvoid, sizeof(LPVOID)},{type_hmodule, sizeof(HMODULE)},{type_hresult, sizeof(HRESULT)},{type_schandle, sizeof(SC_HANDLE)},
		{type_ntstatus, sizeof(NTSTATUS)},{type_string, sizeof(std::string)},{type_wstring, sizeof(std::wstring) }, {type_handle, sizeof(HANDLE)}, {type_guid, sizeof(GUID)},
};

const std::map<int, std::string> CBaseDef::_base_to_str_def = {
		{type_invalid, "\0"},{type_void, "VOID"},{type_byte, "BYTE"},{type_bool, "BOOL"},{type_char, "CHAR"},{type_uchar, "UCHAR"},{type_int16, "INT16"},{type_uint16, "UINT16"},{type_short, "SHORT"},
		{type_ushort, "USHORT"},{type_word, "WORD"},{type_wchar, "WCHAR"},{type_int, "INT"},{type_uint, "UINT"},{type_long, "LONG"},{type_long_ptr, "LONG_PTR"},{type_ulong, "ULONG"},{type_ulong_ptr, "ULONG_PTR"},
		{type_dword, "DWORD"},{type_dword_ptr, "DWORD_PTR"},{type_float, "FLOAT"},{type_int64, "INT64"},{type_uint64, "UINT64"},{type_longlong, "LONGLONG"},{type_ulonglong, "ULONGLONG"},{type_qword, "QWORD"},
		{type_double, "DOUBLE"},{type_large_integer, "LARGE_INTEGER"},{type_ularge_integer, "ULARGE_INTEGER"},{type_lpvoid, "LPVOID"},{type_hmodule, "HMODULE"},{type_hresult, "HRESULT"},{type_schandle, "SC_HANDLE"},
		{type_ntstatus, "NTSTATUS"},{type_string, "std::string"},{type_wstring, "std::wstring"},{type_handle, "HANDLE"},{type_guid, "GUID"},
};

// overload operator for support special type.
std::stringstream& operator<<(std::stringstream& os, const std::wstring& wstr)
{
	os << W2AString(wstr);
	return os;
}
std::stringstream& operator<<(std::stringstream& os, std::wstring& wstr)
{
	os << W2AString(wstr);
	return os;
}
std::stringstream& operator<<(std::stringstream& os, const LARGE_INTEGER& large_int)
{
	os << large_int.QuadPart;
	return os;
}
std::stringstream& operator<<(std::stringstream& os, LARGE_INTEGER& large_int)
{
	os << large_int.QuadPart;
	return os;
}
std::stringstream& operator<<(std::stringstream& os, const ULARGE_INTEGER& ularge_int)
{
	os << ularge_int.QuadPart;
	return os;
}
std::stringstream& operator<<(std::stringstream& os, ULARGE_INTEGER& ularge_int)
{
	os << ularge_int.QuadPart;
	return os;
}
std::stringstream& operator<<(std::stringstream& os, const GUID& guid)
{
	const std::string hex = "0123456789abcdef";
	os << "{";
	os << std::hex << guid.Data1;
	os << "-";
	os << std::hex << guid.Data2;
	os << "-";
	os << std::hex << guid.Data3;
	os << "-";
	os << hex[guid.Data4[0] >> 4] << hex[guid.Data4[0] & 0xf] \
		<< hex[guid.Data4[1] >> 4] << hex[guid.Data4[1] & 0xf] \
		<< "-" \
		<< hex[guid.Data4[2] >> 4] << hex[guid.Data4[2] & 0xf] \
		<< hex[guid.Data4[3] >> 4] << hex[guid.Data4[3] & 0xf] \
		<< hex[guid.Data4[4] >> 4] << hex[guid.Data4[4] & 0xf] \
		<< hex[guid.Data4[5] >> 4] << hex[guid.Data4[5] & 0xf] \
		<< hex[guid.Data4[6] >> 4] << hex[guid.Data4[6] & 0xf] \
		<< hex[guid.Data4[7] >> 4] << hex[guid.Data4[7] & 0xf] \
		<< "}";
	return os;
}
std::stringstream& operator<<(std::stringstream& os, GUID& guid)
{
	const std::string hex = "0123456789abcdef";
	os << "{";
	os << std::hex << guid.Data1;
	os << "-";
	os << std::hex << guid.Data2;
	os << "-";
	os << std::hex << guid.Data3;
	os << "-";
	os << hex[guid.Data4[0] >> 4] << hex[guid.Data4[0] & 0xf] \
		<< hex[guid.Data4[1] >> 4] << hex[guid.Data4[1] & 0xf] \
		<< "-" \
		<< hex[guid.Data4[2] >> 4] << hex[guid.Data4[2] & 0xf] \
		<< hex[guid.Data4[3] >> 4] << hex[guid.Data4[3] & 0xf] \
		<< hex[guid.Data4[4] >> 4] << hex[guid.Data4[4] & 0xf] \
		<< hex[guid.Data4[5] >> 4] << hex[guid.Data4[5] & 0xf] \
		<< hex[guid.Data4[6] >> 4] << hex[guid.Data4[6] & 0xf] \
		<< hex[guid.Data4[7] >> 4] << hex[guid.Data4[7] & 0xf] \
		<< "}";
	return os;
}
std::stringstream& operator>>(std::stringstream& is, WCHAR& w)
{
	std::wstring ws = A2WString(is.str());
	if (ws.length() > 0)
		w = ws[0];
	return is;
}
std::stringstream& operator>>(std::stringstream& is, std::wstring& wstr)
{
	wstr = A2WString(is.str());
	return is;
}
std::stringstream& operator>>(std::stringstream& is, LARGE_INTEGER& large_int)
{
	is >> large_int.QuadPart;
	return is;
}
std::stringstream& operator>>(std::stringstream& is, ULARGE_INTEGER& ularge_int)
{
	is >> ularge_int.QuadPart;
	return is;
}
std::stringstream& operator>>(std::stringstream& is, GUID& guid)
{
	guid = { };
	std::string in = is.str();
	assert(in.length());
	std::string regular_ex = "^\\{[0-9A-F]{8}(-[0-9A-F]{4}){3}-[0-9A-F]{12}\\}$";
	std::regex reg_ex(regular_ex, std::regex::icase);
	bool bret = std::regex_match(in, reg_ex);
	if (bret)
	{
		auto strtoss = [](const std::string& in, int& count, int n) ->std::string { std::stringstream ss; int i = 0; while (i++ < n) { ss << in[count++]; ss << in[count++]; } count++; return ss.str(); };
		auto re_strtoss = [](const std::string& in, int& count, int n) ->std::string { std::stringstream ss; int m = n; do { ss << in[count + n]; ss << in[count + n + 1]; n = n - 2; } while (n >= 0); count += 2 * m; return ss.str(); };
		auto sstrtoul = [](const std::string& in) ->unsigned long {char* nodig = 0; unsigned long digit = strtoul(in.c_str(), &nodig, 16); return digit; };
		auto sstrtous = [](const std::string& in) ->unsigned short {char* nodig = 0; unsigned short digit = (unsigned short)strtoul(in.c_str(), &nodig, 16); return digit; };
		int count = 1;
		guid.Data1 = sstrtoul(strtoss(in, count, 4));
		guid.Data2 = sstrtous(strtoss(in, count, 2));
		guid.Data3 = sstrtous(strtoss(in, count, 2));
		*((unsigned short*)guid.Data4) = sstrtous(re_strtoss(in, count, 2)); count++;
		*((unsigned short*)&guid.Data4[2]) = sstrtous(re_strtoss(in, count, 2));
		*((unsigned long*)&guid.Data4[4]) = sstrtoul(re_strtoss(in, count, 6));
	}
	return is;
}
std::stringstream& operator>>(std::stringstream& is, HMODULE& hmodule)
{
	char* nodig = 0;
#ifdef _X86_
	unsigned long digit = strtoul(is.str().c_str(), &nodig, 16);
#else
	unsigned long long digit = strtoull(is.str().c_str(), &nodig, 16);
#endif
	hmodule = (HMODULE)digit;
	return is;
}
std::stringstream& operator>>(std::stringstream& is, SC_HANDLE& sc_handle)
{
	char* nodig = 0;
#ifdef _X86_
	unsigned long digit = strtoul(is.str().c_str(), &nodig, 16);
#else
	unsigned long long digit = strtoull(is.str().c_str(), &nodig, 16);
#endif
	sc_handle = (SC_HANDLE)digit;
	return is;
}
std::stringstream& operator>>(std::stringstream& is, HANDLE& handle)
{
	char* nodig = 0;
#ifdef _X86_
	unsigned long digit = strtoul(is.str().c_str(), &nodig, 16);
#else
	unsigned long long digit = strtoull(is.str().c_str(), &nodig, 16);
#endif
	handle = (HANDLE)digit;
	return is;
}

bool CTraits<LARGE_INTEGER>::IsValidValue(char* pdata) const
{
	assert(pdata != nullptr);
	LARGE_INTEGER* p = reinterpret_cast<LARGE_INTEGER*>(pdata);
	if (!p) return false;
	if ((*p).QuadPart == _value.QuadPart)
		return false;
	return true;
}

bool CTraits<ULARGE_INTEGER>::IsValidValue(char* pdata) const
{
	assert(pdata != nullptr);
	ULARGE_INTEGER* p = reinterpret_cast<ULARGE_INTEGER*>(pdata);
	if (!p) return false;
	if ((*p).QuadPart == _value.QuadPart)
		return false;
	return true;
}

std::stringstream CTraits<BOOL>::GetValue() const
{
	std::stringstream ss;

	const std::string hex = "0123456789abcdef";
	if (_value == 0)
		ss << "false";
	else
		ss << "true";
	return ss;
}
std::stringstream CTraits<BOOL>::GetValue(char* pdata) const
{
	std::stringstream ss;
	if (IsValidValue(pdata))
	{
		const std::string hex = "0123456789abcdef";
		BOOL* p = reinterpret_cast<BOOL*>(pdata);
		if (*p == 0)
			ss << "false";
		else
			ss << "true";
	}
	return ss;
}

std::stringstream CTraits<CHAR>::GetValue() const
{
	std::stringstream ss;
	const std::string hex = "0123456789abcdef";
	ss << hex[_value >> 4];
	return ss;
}
std::stringstream CTraits<CHAR>::GetValue(char* pdata) const
{
	std::stringstream ss;
	if (IsValidValue(pdata))
	{
		const std::string hex = "0123456789abcdef";
		CHAR* p = reinterpret_cast<CHAR*>(pdata);
		ss << hex[*p >> 4];
	}
	return ss;
}

std::stringstream CTraits<HANDLE>::GetValue() const
{
	std::stringstream ss;
	ss << std::hex << "0x" << _value;
	return ss;
}
std::stringstream CTraits<HANDLE>::GetValue(char* pdata) const
{
	std::stringstream ss;
	if (IsValidValue(pdata))
	{
		HANDLE* p = reinterpret_cast<HANDLE*>(pdata);
		ss << std::hex << "0x" << *p;
	}
	return ss;
}

std::stringstream CTraits<HMODULE>::GetValue() const
{
	std::stringstream ss;
	ss << std::hex << "0x" << _value;
	return ss;
}
std::stringstream CTraits<HMODULE>::GetValue(char* pdata) const
{
	std::stringstream ss;
	if (IsValidValue(pdata))
	{
		HMODULE* p = reinterpret_cast<HMODULE*>(pdata);
		ss << std::hex << "0x" << *p;
	}
	return ss;
}

std::stringstream CTraits<SC_HANDLE>::GetValue() const
{
	std::stringstream ss;
	ss << std::hex << "0x" << _value;
	return ss;
}
std::stringstream CTraits<SC_HANDLE>::GetValue(char* pdata) const
{
	std::stringstream ss;
	if (IsValidValue(pdata))
	{
		SC_HANDLE* p = reinterpret_cast<SC_HANDLE*>(pdata);
		ss << std::hex << "0x" << *p;
	}
	return ss;
}

bool CTraits<LARGE_INTEGER>::Success(char* pdata) const
{
	if (IsValidValue(pdata))
	{
		LARGE_INTEGER* p = reinterpret_cast<LARGE_INTEGER*>(pdata);
		if (_op == op_n_equal)
		{
			if (p->QuadPart == _value.QuadPart)
				return false;
			else
				return true;
		}
		else if (_op == op_greater)
		{
			if (p->QuadPart >= _value.QuadPart)
				return true;
			else
				return false;
		}
		else if (_op == op_less)
		{
			if (p->QuadPart <= _value.QuadPart)
				return true;
			else
				return false;
		}
		if (p->QuadPart == _value.QuadPart)
			return true;
	}
	return false;
}

bool CTraits<ULARGE_INTEGER>::Success(char* pdata) const
{
	if (IsValidValue(pdata))
	{
		ULARGE_INTEGER* p = reinterpret_cast<ULARGE_INTEGER*>(pdata);
		if (_op == op_n_equal)
		{
			if (p->QuadPart == _value.QuadPart)
				return false;
			else
				return true;
		}
		else if (_op == op_greater)
		{
			if (p->QuadPart >= _value.QuadPart)
				return true;
			else
				return false;
		}
		else if (_op == op_less)
		{
			if (p->QuadPart <= _value.QuadPart)
				return true;
			else
				return false;
		}
		if (p->QuadPart == _value.QuadPart)
			return true;
	}
	return false;
}

bool CTraits<GUID>::Success(char* pdata) const
{
	if (IsValidValue(pdata))
	{
		GUID* p = reinterpret_cast<GUID*>(pdata);
		if (_op == op_n_equal)
		{
			if (*p == _value)
				return false;
			else
				return true;
		}
		else if (_op == op_greater)
		{
			// GUID only support equal or n_equal
			return false;
		}
		else if (_op == op_less)
		{
			// GUID only support equal or n_equal
			return false;
		}
		if (*p == _value)
			return true;
	}
	return false;
}

template<typename T>
bool vir_sstream_copy(char* dst, std::stringstream& ss)
{
	DWORD OldProtect = 0;
	if (VirtualProtect((T*)dst, sizeof(T), PAGE_READWRITE, &OldProtect))
	{
		ss >> *reinterpret_cast<T*>(dst);
		VirtualProtect((T*)dst, sizeof(T), OldProtect, &OldProtect);
		return true;
	}
	return false;
}

} // namespace cchips