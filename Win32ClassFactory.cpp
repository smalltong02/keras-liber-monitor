#include "stdafx.h"
#include "Win32ClassFactory.h"

GUID IID_idIClassFactory = {
	0x00000001,
	0x0000,
	0x0000,
	0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46
};

HRESULT STDMETHODCALLTYPE FactoryQueryInterface(
	idIClassFactory *This,
	REFIID riid,
	void **ppvObject)
{
	HRESULT hr = S_OK;

	hr = ((IClassFactory*)(This->lpVtbl->m_realFactoryObj))->QueryInterface(riid, ppvObject);
	if (hr == S_OK)
	{
		CloneComObject((GUID*)&riid, &IID_idIQueryUnknown, ppvObject);
	}

	return hr;
}

ULONG STDMETHODCALLTYPE FactoryAddRef(
	idIClassFactory *This)
{
	ULONG Ref = ((IClassFactory*)(This->lpVtbl->m_realFactoryObj))->AddRef();
	This->lpVtbl->m_ref = Ref;
	return Ref;
}

ULONG STDMETHODCALLTYPE FactoryRelease(
	idIClassFactory *This)
{
	ULONG Ref = ((IClassFactory*)(This->lpVtbl->m_realFactoryObj))->Release();
	This->lpVtbl->m_ref = Ref;
	if (!Ref)
	{
		free(This->lpVtbl);
		free(This);
	}
	return Ref;
}

HRESULT STDMETHODCALLTYPE FactoryCreateInstance(
	idIClassFactory *This,
	IUnknown *pUnkOuter,
	REFIID riid,
	void **ppvObject)
{
	HRESULT hr = S_FALSE;

	hr = ((IClassFactory*)(This->lpVtbl->m_realFactoryObj))->CreateInstance(pUnkOuter, riid, ppvObject);
	if (hr == S_OK)
	{
		CloneComObject((GUID*)&riid, &IID_idIQueryUnknown, ppvObject);
	}

	return hr;
}

HRESULT STDMETHODCALLTYPE FactoryLockServer(
	idIClassFactory *This,
	BOOL fLock)
{
	HRESULT hr = S_OK;
	hr = ((IClassFactory*)(This->lpVtbl->m_realFactoryObj))->LockServer(fLock);
	return hr;
}


