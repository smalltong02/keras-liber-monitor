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

#ifdef _DEBUG 
#ifdef _AMD64_
#define BreakPoint BreakInt3();

#else
#define BreakPoint __asm int 3
#endif
#else
#define BreakPoint
#endif



