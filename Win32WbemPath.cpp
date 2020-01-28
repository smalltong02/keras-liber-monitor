#include "stdafx.h"
#include "Win32WbemPath.h"

GUID CLSID_idWbemDefPath = {
	0xcf4cc405,
	0xe2c5,
	0x4ddd,
	0xb3,0xce,0x5e,0x75,0x82,0xd8,0xc9,0xfa
};

GUID IID_idIWbemPath = {
	0x3bc15af2,
	0x736c,
	0x477e,
	0x9e,0x51,0x23,0x8a,0xf8,0x66,0x7d,0xcc
};

HRESULT STDMETHODCALLTYPE WbemPathQueryInterface(
	idIWbemPath *This,
	REFIID riid,
	void **ppvObject)
{
	HRESULT hr = S_OK;
	hr = ((IWbemPath*)(This->lpVtbl->m_realIWbemPathObj.pItf))->QueryInterface(riid, ppvObject);
	if (hr == S_OK)
	{
		if (*ppvObject == This->lpVtbl->m_realIWbemPathObj.pItf)
			*ppvObject = This;
		else if (IsEqualIIID(&IID_idIUnknown, &riid))
			CloneComObject(&IID_idIWbemPath, &This->lpVtbl->m_realIWbemPathObj.queryIID, ppvObject);
		else
			CloneComObject((GUID*)&riid, &This->lpVtbl->m_realIWbemPathObj.queryIID, ppvObject);
	}
	return hr;
}

ULONG STDMETHODCALLTYPE WbemPathAddRef(
	idIWbemPath *This)
{
	ULONG Ref = ((IWbemPath*)(This->lpVtbl->m_realIWbemPathObj.pItf))->AddRef();
	This->lpVtbl->m_ref = Ref;
	return Ref;
}

ULONG STDMETHODCALLTYPE WbemPathRelease(
	idIWbemPath *This)
{
	ULONG Ref = ((IWbemPath*)(This->lpVtbl->m_realIWbemPathObj.pItf))->Release();
	This->lpVtbl->m_ref = Ref;
	if (!Ref)
	{
		free(This->lpVtbl);
		free(This);
	}
	return Ref;
}

/*** IWbemPath methods ***/
HRESULT STDMETHODCALLTYPE WbemPathSetText(
	idIWbemPath *This,
	ULONG uMode,
	LPCWSTR pszPath)
{
	HRESULT hr = S_OK;
	hr = ((IWbemPath*)(This->lpVtbl->m_realIWbemPathObj.pItf))->SetText(uMode, pszPath);
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemPathGetText(
	idIWbemPath *This,
	LONG lFlags,
	ULONG *puBuffLength,
	LPWSTR pszText)
{
	HRESULT hr = S_OK;
	hr = ((IWbemPath*)(This->lpVtbl->m_realIWbemPathObj.pItf))->GetText(lFlags, puBuffLength, pszText);
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemPathGetInfo(
	idIWbemPath *This,
	ULONG uRequestedInfo,
	ULONGLONG *puResponse)
{
	HRESULT hr = S_OK;
	hr = ((IWbemPath*)(This->lpVtbl->m_realIWbemPathObj.pItf))->GetInfo(uRequestedInfo, puResponse);
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemPathSetServer(
	idIWbemPath *This,
	LPCWSTR Name)
{
	HRESULT hr = S_OK;
	hr = ((IWbemPath*)(This->lpVtbl->m_realIWbemPathObj.pItf))->SetServer(Name);
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemPathGetServer(
	idIWbemPath *This,
	ULONG *puNameBufLength,
	LPWSTR pName)
{
	HRESULT hr = S_OK;
	hr = ((IWbemPath*)(This->lpVtbl->m_realIWbemPathObj.pItf))->GetServer(puNameBufLength, pName);
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemPathGetNamespaceCount(
	idIWbemPath *This,
	ULONG *puCount)
{
	HRESULT hr = S_OK;
	hr = ((IWbemPath*)(This->lpVtbl->m_realIWbemPathObj.pItf))->GetNamespaceCount(puCount);
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemPathSetNamespaceAt(
	idIWbemPath *This,
	ULONG uIndex,
	LPCWSTR pszName)
{
	HRESULT hr = S_OK;
	hr = ((IWbemPath*)(This->lpVtbl->m_realIWbemPathObj.pItf))->SetNamespaceAt(uIndex, pszName);
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemPathGetNamespaceAt(
	idIWbemPath *This,
	ULONG uIndex,
	ULONG *puNameBufLength,
	LPWSTR pName)
{
	HRESULT hr = S_OK;
	hr = ((IWbemPath*)(This->lpVtbl->m_realIWbemPathObj.pItf))->GetNamespaceAt(uIndex, puNameBufLength, pName);
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemPathRemoveNamespaceAt(
	idIWbemPath *This,
	ULONG uIndex)
{
	HRESULT hr = S_OK;
	hr = ((IWbemPath*)(This->lpVtbl->m_realIWbemPathObj.pItf))->RemoveNamespaceAt(uIndex);
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemPathRemoveAllNamespaces(
	idIWbemPath *This)
{
	HRESULT hr = S_OK;
	hr = ((IWbemPath*)(This->lpVtbl->m_realIWbemPathObj.pItf))->RemoveAllNamespaces();
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemPathGetScopeCount(
	idIWbemPath *This,
	ULONG *puCount)
{
	HRESULT hr = S_OK;
	hr = ((IWbemPath*)(This->lpVtbl->m_realIWbemPathObj.pItf))->GetScopeCount(puCount);
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemPathSetScope(
	idIWbemPath *This,
	ULONG uIndex,
	LPWSTR pszClass)
{
	HRESULT hr = S_OK;
	hr = ((IWbemPath*)(This->lpVtbl->m_realIWbemPathObj.pItf))->SetScope(uIndex, pszClass);
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemPathSetScopeFromText(
	idIWbemPath *This,
	ULONG uIndex,
	LPWSTR pszText)
{
	HRESULT hr = S_OK;
	hr = ((IWbemPath*)(This->lpVtbl->m_realIWbemPathObj.pItf))->SetScopeFromText(uIndex, pszText);
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemPathGetScope(
	idIWbemPath *This,
	ULONG uIndex,
	ULONG *puClassNameBufSize,
	LPWSTR pszClass,
	IWbemPathKeyList **pKeyList)
{
	HRESULT hr = S_OK;
	hr = ((IWbemPath*)(This->lpVtbl->m_realIWbemPathObj.pItf))->GetScope(uIndex, puClassNameBufSize, pszClass, pKeyList);
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemPathGetScopeAsText(
	idIWbemPath *This,
	ULONG uIndex,
	ULONG *puTextBufSize,
	LPWSTR pszText)
{
	HRESULT hr = S_OK;
	hr = ((IWbemPath*)(This->lpVtbl->m_realIWbemPathObj.pItf))->GetScopeAsText(uIndex, puTextBufSize, pszText);
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemPathRemoveScope(
	idIWbemPath *This,
	ULONG uIndex)
{
	HRESULT hr = S_OK;
	hr = ((IWbemPath*)(This->lpVtbl->m_realIWbemPathObj.pItf))->RemoveScope(uIndex);
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemPathRemoveAllScopes(
	idIWbemPath *This)
{
	HRESULT hr = S_OK;
	hr = ((IWbemPath*)(This->lpVtbl->m_realIWbemPathObj.pItf))->RemoveAllScopes();
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemPathSetClassName(
	idIWbemPath *This,
	LPCWSTR Name)
{
	HRESULT hr = S_OK;
	hr = ((IWbemPath*)(This->lpVtbl->m_realIWbemPathObj.pItf))->SetClassName(Name);
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemPathGetClassName(
	idIWbemPath *This,
	ULONG *puBuffLength,
	LPWSTR pszName)
{
	HRESULT hr = S_OK;
	hr = ((IWbemPath*)(This->lpVtbl->m_realIWbemPathObj.pItf))->GetClassName(puBuffLength, pszName);
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemPathGetKeyList(
	idIWbemPath *This,
	IWbemPathKeyList **pOut)
{
	HRESULT hr = S_OK;
	hr = ((IWbemPath*)(This->lpVtbl->m_realIWbemPathObj.pItf))->GetKeyList(pOut);
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemPathCreateClassPart(
	idIWbemPath *This,
	LONG lFlags,
	LPCWSTR Name)
{
	HRESULT hr = S_OK;
	hr = ((IWbemPath*)(This->lpVtbl->m_realIWbemPathObj.pItf))->CreateClassPart(lFlags, Name);
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemPathDeleteClassPart(
	idIWbemPath *This,
	LONG lFlags)
{
	HRESULT hr = S_OK;
	hr = ((IWbemPath*)(This->lpVtbl->m_realIWbemPathObj.pItf))->DeleteClassPart(lFlags);
	return hr;
}

BOOL STDMETHODCALLTYPE WbemPathIsRelative(
	idIWbemPath *This,
	LPWSTR wszMachine,
	LPWSTR wszNamespace)
{
	HRESULT hr = S_OK;
	hr = ((IWbemPath*)(This->lpVtbl->m_realIWbemPathObj.pItf))->IsRelative(wszMachine, wszNamespace);
	return hr;
}

BOOL STDMETHODCALLTYPE WbemPathIsRelativeOrChild(
	idIWbemPath *This,
	LPWSTR wszMachine,
	LPWSTR wszNamespace,
	LONG lFlags)
{
	HRESULT hr = S_OK;
	hr = ((IWbemPath*)(This->lpVtbl->m_realIWbemPathObj.pItf))->IsRelativeOrChild(wszMachine, wszNamespace, lFlags);
	return hr;
}

BOOL STDMETHODCALLTYPE WbemPathIsLocal(
	idIWbemPath *This,
	LPCWSTR wszMachine)
{
	HRESULT hr = S_OK;
	hr = ((IWbemPath*)(This->lpVtbl->m_realIWbemPathObj.pItf))->IsLocal(wszMachine);
	return hr;
}

BOOL STDMETHODCALLTYPE WbemPathIsSameClassName(
	idIWbemPath *This,
	LPCWSTR wszClass)
{
	HRESULT hr = S_OK;
	hr = ((IWbemPath*)(This->lpVtbl->m_realIWbemPathObj.pItf))->IsSameClassName(wszClass);
	return hr;
}



