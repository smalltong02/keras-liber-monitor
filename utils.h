#pragma once
#include "asmfunction.h"
#include <WbemCli.h>
#include <WMIUtils.h>
#include <OCIdl.h>
#include <vector>

bool ExtractResource(HMODULE ModuleHandle, TCHAR const * ResourceName,
    TCHAR const * ResourceId, std::vector<BYTE>& ResoureBuffer);
bool vir_strcpy_s(char* dst, size_t dst_len, const char* src);
bool vir_wcscpy_s(wchar_t* dst, size_t dst_len, const wchar_t* src);
std::wstring A2WString(const std::string& str);
std::string W2AString(const std::wstring& str);
bool GetValueString(const VARIANT& Value, std::string& ValueString);
bool SetValueString(const std::string& ValueString, VARIANT& Value);
VARTYPE ConvertObTypeToVarintType(const std::string& type_name);

#ifdef _X86_
#define TLS_TEB 0x18
#define TLS_PEB 0x30
#endif
#ifdef _AMD64_
#define TLS_TEB 0x30
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



