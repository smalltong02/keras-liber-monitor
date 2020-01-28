#include "stdafx.h"
#include "Win32WbemCallResult.h"

GUID IID_idIWbemCallResult = {
	0x44aca675,
	0xe8fc,
	0x11d0,
	0xa0,0x7c,0x00,0xc0,0x4f,0xb6,0x88,0x20
};

HRESULT STDMETHODCALLTYPE WbemCallResultQueryInterface(
	idIWbemCallResult * This,
	/* [in] */ REFIID riid,
	/* [annotation][iid_is][out] */
	_COM_Outptr_  void **ppvObject)
{
	HRESULT hr = S_OK;
	hr = ((IWbemCallResult*)(This->lpVtbl->m_realWbemCallResultObj.pItf))->QueryInterface(riid, ppvObject);
	if (hr == S_OK)
	{
		if (*ppvObject == This->lpVtbl->m_realWbemCallResultObj.pItf)
			*ppvObject = This;
		else if (IsEqualIIID(&IID_idIUnknown, &riid))
			CloneComObject(&IID_idIWbemCallResult, &This->lpVtbl->m_realWbemCallResultObj.queryIID, ppvObject);
		else
			CloneComObject((GUID*)&riid, &This->lpVtbl->m_realWbemCallResultObj.queryIID, ppvObject);
	}
	return hr;
}

ULONG STDMETHODCALLTYPE WbemCallResultAddRef(
	idIWbemCallResult * This)
{
	ULONG Ref = ((IWbemCallResult*)(This->lpVtbl->m_realWbemCallResultObj.pItf))->AddRef();
	This->lpVtbl->m_ref = Ref;
	return Ref;
}

ULONG STDMETHODCALLTYPE WbemCallResultRelease(
	idIWbemCallResult * This)
{
	ULONG Ref = ((IWbemCallResult*)(This->lpVtbl->m_realWbemCallResultObj.pItf))->Release();
	This->lpVtbl->m_ref = Ref;
	if (!Ref)
	{
		free(This->lpVtbl);
		free(This);
	}
	return Ref;
}

HRESULT STDMETHODCALLTYPE WbemCallResultGetResultObject(
	idIWbemCallResult * This,
	/* [in] */ long lTimeout,
	/* [out] */ idIWbemClassObject **ppResultObject)
{
	HRESULT hr = S_OK;
	hr = ((IWbemCallResult*)(This->lpVtbl->m_realWbemCallResultObj.pItf))->GetResultObject(lTimeout, (IWbemClassObject**)ppResultObject);
	if (hr == S_OK)
	{
		CloneComObject(&IID_idIWbemClassObject, &This->lpVtbl->m_realWbemCallResultObj.queryIID, (void**)ppResultObject);
	}
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemCallResultGetResultString(
	idIWbemCallResult * This,
	/* [in] */ long lTimeout,
	/* [out] */ BSTR *pstrResultString)
{
	HRESULT hr = S_OK;
	hr = ((IWbemCallResult*)(This->lpVtbl->m_realWbemCallResultObj.pItf))->GetResultString(lTimeout, pstrResultString);
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemCallResultGetResultServices(
	idIWbemCallResult * This,
	/* [in] */ long lTimeout,
	/* [out] */ idIWbemServices **ppServices)
{
	HRESULT hr = S_OK;
	hr = ((IWbemCallResult*)(This->lpVtbl->m_realWbemCallResultObj.pItf))->GetResultServices(lTimeout, (IWbemServices**)ppServices);
	if (hr == S_OK)
	{
		CloneComObject(&IID_idIWbemServices, &This->lpVtbl->m_realWbemCallResultObj.queryIID, (void**)ppServices);
	}
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemCallResultGetCallStatus(
	idIWbemCallResult * This,
	/* [in] */ long lTimeout,
	/* [out] */ long *plStatus)
{
	HRESULT hr = S_OK;
	hr = ((IWbemCallResult*)(This->lpVtbl->m_realWbemCallResultObj.pItf))->GetCallStatus(lTimeout, plStatus);
	return hr;
}


