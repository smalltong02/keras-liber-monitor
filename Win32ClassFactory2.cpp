#include "stdafx.h"
#include "Win32ClassFactory2.h"

GUID IID_idIClassFactory2 = {
	0xb196b28f,
	0xbab4,
	0x101a,
	0xb6,0x9c,0x00,0xaa,0x00,0x34,0x1d,0x07
};

HRESULT STDMETHODCALLTYPE Factory2QueryInterface(
	idIClassFactory2 *This,
	REFIID riid,
	void **ppvObject)
{
	HRESULT hr = S_OK;
	hr = ((IClassFactory2*)(This->lpVtbl->m_realFactory2Obj.pItf))->QueryInterface(riid, ppvObject);
	return hr;
}

ULONG STDMETHODCALLTYPE Factory2AddRef(
	idIClassFactory2 *This)
{
	ULONG Ref = ((IClassFactory2*)(This->lpVtbl->m_realFactory2Obj.pItf))->AddRef();
	This->lpVtbl->m_ref = Ref;
	return Ref;
}

ULONG STDMETHODCALLTYPE Factory2Release(
	idIClassFactory2 *This)
{
	ULONG Ref = ((IClassFactory2*)(This->lpVtbl->m_realFactory2Obj.pItf))->Release();
	This->lpVtbl->m_ref = Ref;
	if (!Ref)
	{
		free(This->lpVtbl);
		free(This);
	}
	return Ref;
}

HRESULT STDMETHODCALLTYPE Factory2CreateInstance(
	idIClassFactory2 *This,
	IUnknown *pUnkOuter,
	REFIID riid,
	void **ppvObject)
{
	HRESULT hr = S_OK;
	hr = ((IClassFactory2*)(This->lpVtbl->m_realFactory2Obj.pItf))->CreateInstance(pUnkOuter, riid, ppvObject);
	return hr;
}

HRESULT STDMETHODCALLTYPE Factory2LockServer(
	idIClassFactory2 *This,
	BOOL fLock)
{
	HRESULT hr = S_OK;
	hr = ((IClassFactory2*)(This->lpVtbl->m_realFactory2Obj.pItf))->LockServer(fLock);
	return hr;
}

HRESULT STDMETHODCALLTYPE Factory2GetLicInfo(
	idIClassFactory2 *This,
	LICINFO *pLicInfo)
{
	HRESULT hr = S_OK;
	hr = ((IClassFactory2*)(This->lpVtbl->m_realFactory2Obj.pItf))->GetLicInfo(pLicInfo);
	return hr;
}

HRESULT STDMETHODCALLTYPE Factory2RequestLicKey(
	idIClassFactory2 *This,
	DWORD dwReserved,
	BSTR *pBstrKey)
{
	HRESULT hr = S_OK;
	hr = ((IClassFactory2*)(This->lpVtbl->m_realFactory2Obj.pItf))->RequestLicKey(dwReserved, pBstrKey);
	return hr;
}

HRESULT STDMETHODCALLTYPE Factory2CreateInstanceLic(
	idIClassFactory2 *This,
	IUnknown *pUnkOuter,
	IUnknown *pUnkReserved,
	REFIID riid,
	BSTR bstrKey,
	PVOID *ppvObj)
{
	HRESULT hr = S_OK;
	hr = ((IClassFactory2*)(This->lpVtbl->m_realFactory2Obj.pItf))->CreateInstanceLic(pUnkOuter, pUnkReserved, riid, bstrKey, ppvObj);
	return hr;
}


