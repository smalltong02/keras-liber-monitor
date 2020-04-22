#pragma once
#include <stdio.h>
#include <windows.h>
#include <guiddef.h>
#include <WbemCli.h>
#include <WMIUtils.h>
#include <OCIdl.h>
#include <atlbase.h>
#include <winioctl.h>

typedef enum _processing_status {
	processing_continue,
	processing_skip,
}processing_status;

//typedef HMODULE(WINAPI *PDEFLOADLIBRARYW)(LPCWSTR lpLibFileName);
//typedef HRESULT(WINAPI *PCOCREATEINSTANCEEX)(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, COSERVERINFO* pServerInfo, ULONG cmq, MULTI_QI* pResults);
//typedef HRESULT(WINAPI *PCOCREATEINSTANCE)(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID iid, LPVOID *ppv);
//typedef HRESULT(WINAPI *PCOGETCLASSOBJECT)(REFCLSID rclsid, DWORD dwClsContext, COSERVERINFO *pServerInfo, REFIID iid, LPVOID *ppv);
//typedef HRESULT(WINAPI *PCOINITIALIZEEX)(LPVOID lpReserved, DWORD dwCoInit);
//typedef HRESULT(WINAPI *PCOINITIALIZE)(LPVOID lpReserved);
//typedef void(WINAPI *PCOUNINITIALIZE)(void);

//extern PDEFLOADLIBRARYW g_orgloadLibraryW;
//extern PCOCREATEINSTANCEEX g_CoCreateInstanceEx;
//extern PCOCREATEINSTANCE g_CoCreateInstance;
//extern PCOGETCLASSOBJECT g_CoGetClassObject;
//extern PCOINITIALIZEEX g_CoInitializeEx;
//extern PCOINITIALIZE g_CoInitialize;
//extern PCOUNINITIALIZE g_CoUninitialize;

//HMODULE WINAPI detour_loadLibraryW(LPCWSTR lpLibFileName);
//HRESULT WINAPI detour_CoCreateInstanceEx(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, COSERVERINFO* pServerInfo, ULONG cmq, MULTI_QI* pResults);
//HRESULT WINAPI detour_CoCreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID iid, LPVOID *ppv);
//HRESULT WINAPI detour_CoGetClassObject(REFCLSID rclsid, DWORD dwClsContext, COSERVERINFO *pServerInfo, REFIID iid, LPVOID *ppv);
//HRESULT WINAPI detour_CoInitializeEx(LPVOID lpReserved, DWORD dwCoInit);
//HRESULT WINAPI detour_CoInitialize(LPVOID lpReserved);
//void WINAPI detour_CoUninitialize(void);



