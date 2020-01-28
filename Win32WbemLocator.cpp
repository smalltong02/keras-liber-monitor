#include "stdafx.h"
#include "Win32WbemLocator.h"

GUID CLSID_idWbemLocator = {
	0x4590f811,
	0x1d3A,
	0x11d0,
	0x89,0x1f,0x00,0xaa,0x00,0x4b,0x2e,0x24
};

GUID IID_idIWbemLocator = {
	0xdc12a687,
	0x737f,
	0x11cf,
	0x88,0x4d,0x00,0xaa,0x00,0x4b,0x2e,0x24
};

HRESULT STDMETHODCALLTYPE WbemQueryInterface(
	idIWbemLocator * This,
	/* [in] */ REFIID riid,
	/* [annotation][iid_is][out] */
	_COM_Outptr_  void **ppvObject)
{
	HRESULT hr = S_OK;
	hr = ((IWbemLocator*)(This->lpVtbl->m_realWbemObj.pItf))->QueryInterface(riid, ppvObject);
	if (hr == S_OK)
	{
		if (*ppvObject == This->lpVtbl->m_realWbemObj.pItf)
			*ppvObject = This;
		else if (IsEqualIIID(&IID_idIUnknown, &riid))
			CloneComObject(&IID_idIWbemLocator, &This->lpVtbl->m_realWbemObj.queryIID, ppvObject);
		else
			CloneComObject((GUID*)&riid, &This->lpVtbl->m_realWbemObj.queryIID, ppvObject);
	}
	return hr;
}

ULONG STDMETHODCALLTYPE WbemAddRef(
	idIWbemLocator * This)
{
	ULONG Ref = ((IWbemLocator*)(This->lpVtbl->m_realWbemObj.pItf))->AddRef();
	This->lpVtbl->m_ref = Ref;
	return Ref;
}

ULONG STDMETHODCALLTYPE WbemRelease(
	idIWbemLocator * This)
{
	ULONG Ref = ((IWbemLocator*)(This->lpVtbl->m_realWbemObj.pItf))->Release();
	This->lpVtbl->m_ref = Ref;
	if (!Ref)
	{
		free(This->lpVtbl);
		free(This);
	}
	return Ref;
}

HRESULT STDMETHODCALLTYPE WbemConnectServer(
	idIWbemLocator * This,
	/* [in] */ const BSTR strNetworkResource,
	/* [in] */ const BSTR strUser,
	/* [in] */ const BSTR strPassword,
	/* [in] */ const BSTR strLocale,
	/* [in] */ long lSecurityFlags,
	/* [in] */ const BSTR strAuthority,
	/* [in] */ idIWbemContext *pCtx,
	/* [out] */ idIWbemServices **ppNamespace)
{
	HRESULT hr = S_OK;
	hr = ((IWbemLocator*)(This->lpVtbl->m_realWbemObj.pItf))->ConnectServer(strNetworkResource, strUser, strPassword, strLocale, lSecurityFlags, strAuthority, (IWbemContext *)pCtx, (IWbemServices **)ppNamespace);
	if (hr == S_OK)
	{
		if (EndsWith(strNetworkResource, L"ROOT\\CIMV2", false))
		{
			CloneComObject(&IID_idIWbemServices, &This->lpVtbl->m_realWbemObj.queryIID, (void**)ppNamespace);
		}
	}
	return hr;
}

