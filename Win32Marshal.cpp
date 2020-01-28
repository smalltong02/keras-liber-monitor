#include "stdafx.h"
#include "Win32Marshal.h"

extern GUID IID_idIUnknown;
extern GUID IID_idIMarshal;

GUID CLSID_idDfMarshal = {
	0x0000030b,
	0x0,
	0x0,
	0xC0,0,0,0,0,0,0,0x46
};

void InitializeWin32IMarshal(idWin32IMarshal* p_IWin32IMarshal)
{
	p_IWin32IMarshal->lpVtbl->AddRef = MarshalAddRef;
	p_IWin32IMarshal->lpVtbl->Release = MarshalRelease;
	p_IWin32IMarshal->lpVtbl->QueryInterface = MarshalQueryInterface;
	p_IWin32IMarshal->lpVtbl->DisconnectObject = MarshalDisconnectObject;
	p_IWin32IMarshal->lpVtbl->GetMarshalSizeMax = MarshalGetMarshalSizeMax;
	p_IWin32IMarshal->lpVtbl->GetUnmarshalClass = MarshalGetUnmarshalClass;
	p_IWin32IMarshal->lpVtbl->MarshalInterface = MarshalMarshalInterface;
	p_IWin32IMarshal->lpVtbl->ReleaseMarshalData = MarshalReleaseMarshalData;
	p_IWin32IMarshal->lpVtbl->UnmarshalInterface = MarshalUnmarshalInterface;
	p_IWin32IMarshal->lpVtbl->AddRef(p_IWin32IMarshal);
	return;
}


HRESULT STDMETHODCALLTYPE MarshalQueryInterface(
	idWin32IMarshal *This,
	REFIID riid,
	void **ppvObject)
{
	HRESULT hr = E_NOINTERFACE;
	if (IsEqualIIID(&IID_idIUnknown, &riid) ||
		IsEqualIIID(&IID_idIMarshal, &riid))
	{
		*ppvObject = This;
		MarshalAddRef(This);
		return S_OK;
	}
	return hr;
}

ULONG STDMETHODCALLTYPE MarshalAddRef(
	idWin32IMarshal *This)
{
	ULONG Ref = ++This->lpVtbl->m_ref;
	return Ref;
}

ULONG STDMETHODCALLTYPE MarshalRelease(
	idWin32IMarshal *This)
{
	ULONG Ref = --This->lpVtbl->m_ref;
	if (!Ref)
	{
		free(This->lpVtbl);
		free(This);
	}
	return Ref;
}

/*** IMarshal methods ***/
HRESULT STDMETHODCALLTYPE MarshalGetUnmarshalClass(
	idWin32IMarshal *This,
	REFIID riid,
	void *pv,
	DWORD dwDestContext,
	void *pvDestContext,
	DWORD mshlflags,
	CLSID *pCid)
{
	HRESULT hr = S_OK;
	*pCid = CLSID_idDfMarshal;
	return hr;
}

HRESULT STDMETHODCALLTYPE MarshalGetMarshalSizeMax(
	idWin32IMarshal *This,
	REFIID riid,
	void *pv,
	DWORD dwDestContext,
	void *pvDestContext,
	DWORD mshlflags,
	DWORD *pSize)
{
	HRESULT hr = S_OK;
	*pSize = 0;
	return hr;
}

HRESULT STDMETHODCALLTYPE MarshalMarshalInterface(
	idWin32IMarshal *This,
	IStream *pStm,
	REFIID riid,
	void *pv,
	DWORD dwDestContext,
	void *pvDestContext,
	DWORD mshlflags)
{
	HRESULT hr = S_OK;
	return hr;
}

HRESULT STDMETHODCALLTYPE MarshalUnmarshalInterface(
	idWin32IMarshal *This,
	IStream *pStm,
	REFIID riid,
	void **ppv)
{
	HRESULT hr = S_FALSE;
	return hr;
}

HRESULT STDMETHODCALLTYPE MarshalReleaseMarshalData(
	idWin32IMarshal *This,
	IStream *pStm)
{
	HRESULT hr = S_OK;
	return hr;
}

HRESULT STDMETHODCALLTYPE MarshalDisconnectObject(
	idWin32IMarshal *This,
	DWORD dwReserved)
{
	HRESULT hr = S_OK;
	return hr;
}



