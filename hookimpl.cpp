#include "stdafx.h"
#include "hookimpl.h"

PDEFLOADLIBRARYW g_orgloadLibraryW = NULL;
PCOCREATEINSTANCEEX g_CoCreateInstanceEx = NULL;
PCOCREATEINSTANCE g_CoCreateInstance = NULL;
PCOGETCLASSOBJECT g_CoGetClassObject = NULL;
PCOINITIALIZEEX g_CoInitializeEx = NULL;
PCOINITIALIZE g_CoInitialize = NULL;
PCOUNINITIALIZE g_CoUninitialize = NULL;

HMODULE WINAPI detour_loadLibraryW(LPCWSTR lpLibFileName)
{
	HMODULE hModule = NULL;

	hModule = g_orgloadLibraryW(lpLibFileName);

	return hModule;
}

HRESULT WINAPI detour_CoCreateInstanceEx(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, COSERVERINFO* pServerInfo, ULONG cmq, MULTI_QI* pResults)
{
	HRESULT hr = S_OK;
	hr = g_CoCreateInstanceEx(rclsid, pUnkOuter, dwClsContext, pServerInfo, cmq, pResults);
	return hr;
}

HRESULT WINAPI detour_CoCreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID iid, LPVOID *ppv)
{
	MULTI_QI multi_qi = { &iid };
	HRESULT hr = S_OK;

	if (ppv == 0)
		return E_POINTER;

	hr = g_CoCreateInstanceEx(rclsid, pUnkOuter, dwClsContext, NULL, 1, &multi_qi);
	*ppv = multi_qi.pItf;
	return hr;
}

HRESULT WINAPI detour_CoGetClassObject(REFCLSID rclsid, DWORD dwClsContext, COSERVERINFO *pServerInfo, REFIID iid, LPVOID *ppv)
{
	HRESULT hr = S_OK;
	hr = g_CoGetClassObject(rclsid, dwClsContext, pServerInfo, iid, ppv);
	return hr;
}

HRESULT WINAPI detour_CoInitializeEx(LPVOID lpReserved, DWORD dwCoInit)
{
	HRESULT hr = S_OK;
	hr = g_CoInitializeEx(lpReserved, dwCoInit);
	return hr;
}

HRESULT WINAPI detour_CoInitialize(LPVOID lpReserved)
{
	HRESULT hr = S_OK;
	hr = g_CoInitialize(lpReserved);
	return hr;
}

void WINAPI detour_CoUninitialize(void)
{
	g_CoUninitialize();
	return;
}

