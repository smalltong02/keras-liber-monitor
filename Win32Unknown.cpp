#include "stdafx.h"
#include "Win32Unknown.h"

GUID IID_idIUnknown = {
	0x00000000,
	0x0000,
	0x0000,
	0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x46
};

ULONG STDMETHODCALLTYPE UnknownAddRef(
	idIUnknown * This)
{
	ULONG Ref = ((IUnknown*)(This->lpVtbl->m_realUnknownObj.pItf))->AddRef();
	This->lpVtbl->m_ref = Ref;
	return Ref;
}

ULONG STDMETHODCALLTYPE UnknownRelease(
	idIUnknown * This)
{
	ULONG Ref = ((IUnknown*)(This->lpVtbl->m_realUnknownObj.pItf))->Release();
	This->lpVtbl->m_ref = Ref;
	if (!Ref)
	{
		free(This->lpVtbl);
		free(This);
	}
	return Ref;
}

HRESULT STDMETHODCALLTYPE UnknownQueryInterface(
	idIUnknown * This,
	/* [in] */ REFIID riid,
	/* [annotation][iid_is][out] */
	_COM_Outptr_  void **ppvObject)
{
	HRESULT hr = S_OK;

	hr = ((IUnknown*)(This->lpVtbl->m_realUnknownObj.pItf))->QueryInterface(riid, ppvObject);
	if (hr == S_OK)
	{
		if (*ppvObject == This->lpVtbl->m_realUnknownObj.pItf)
			*ppvObject = This;
		else
			CloneComObject((GUID*)&riid, &This->lpVtbl->m_realUnknownObj.queryIID, ppvObject);
	}

	return hr;
}



