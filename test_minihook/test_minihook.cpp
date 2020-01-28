// test_minihook.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "test_minihook.h"

int main()
{
	HMODULE h = LoadLibraryW(L"HipsHook.dll");
	DWORD c = 0;
	FARPROC p = nullptr;
	bool bret = false;
	ULARGE_INTEGER free_bytes;
	ULARGE_INTEGER total_bytes;
	ULARGE_INTEGER total_free;

	// test
#if _DEBUG
	c = GetTickCount();
	h = GetModuleHandleW(L"Kernel32.dll");
	p = GetProcAddress(h, "GetDiskFreeSpaceExW");
	bret = GetDiskFreeSpaceExW(L"C:\\Windows", &free_bytes, &total_bytes, &total_free);
	getchar();
#endif

    return 0;
}

