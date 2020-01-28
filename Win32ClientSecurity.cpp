#include "stdafx.h"
#include "Win32ClientSecurity.h"

GUID IID_idIClientSecurity = {
	0x0000013d,
	0x0000,
	0x0000,
	0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x46
};

HRESULT STDMETHODCALLTYPE ClientSecurityQueryInterface(
	idIClientSecurity *This,
	REFIID riid,
	void **ppvObject)
{
	HRESULT hr = S_OK;

	hr = ((IClientSecurity*)(This->lpVtbl->m_realClientSecurityObj.pItf))->QueryInterface(riid, ppvObject);
	if (hr == S_OK)
	{
		if (*ppvObject == This->lpVtbl->m_realClientSecurityObj.pItf)
			*ppvObject = This;
		else if (IsEqualIIID(&IID_idIUnknown, &riid))
			CloneComObject(&IID_idIClientSecurity, &This->lpVtbl->m_realClientSecurityObj.queryIID, ppvObject);
		else
			CloneComObject((GUID*)&riid, &This->lpVtbl->m_realClientSecurityObj.queryIID, ppvObject);
	}
	return hr;
}

ULONG STDMETHODCALLTYPE ClientSecurityAddRef(
	idIClientSecurity *This)
{
	ULONG Ref = ((IClientSecurity*)(This->lpVtbl->m_realClientSecurityObj.pItf))->AddRef();
	This->lpVtbl->m_ref = Ref;
	return Ref;
}

ULONG STDMETHODCALLTYPE ClientSecurityRelease(
	idIClientSecurity *This)
{
	ULONG Ref = ((IClientSecurity*)(This->lpVtbl->m_realClientSecurityObj.pItf))->Release();
	This->lpVtbl->m_ref = Ref;
	if (!Ref)
	{
		free(This->lpVtbl);
		free(This);
	}
	return Ref;
}

HRESULT STDMETHODCALLTYPE ClientSecurityQueryBlanket(
	idIClientSecurity *This,
	IUnknown *pProxy,
	DWORD *pAuthnSvc,
	DWORD *pAuthzSvc,
	OLECHAR **pServerPrincName,
	DWORD *pAuthnLevel,
	DWORD *pImpLevel,
	void **pAuthInfo,
	DWORD *pCapabilities)
{
	HRESULT hr = S_OK;
	idIWbemServices* p_idIWbemServices = (idIWbemServices*)pProxy;
	idIUnknown* p_idIUnknown = (idIUnknown*)pProxy;
	idIWbemLocator* p_idIWbemLocator = (idIWbemLocator*)pProxy;
	idIWbemPath* p_idIWbemPath = (idIWbemPath*)pProxy;
	idIEnumWbemClassObject* p_idIEnumWbemClassObject = (idIEnumWbemClassObject*)pProxy;
	idIWbemClassObject* p_idIWbemClassObject = (idIWbemClassObject*)pProxy;
	idIWbemCallResult* p_idIWbemCallResult = (idIWbemCallResult*)pProxy;

	if (p_idIWbemServices->lpVtbl->m_realWbemServicesObj.hr == APPX_E_INTERLEAVING_NOT_ALLOWED && IsEqualIIID(&p_idIWbemServices->lpVtbl->m_realWbemServicesObj.idIID, &IID_idIWbemServices))
		hr = ((IClientSecurity*)(This->lpVtbl->m_realClientSecurityObj.pItf))->QueryBlanket(p_idIWbemServices->lpVtbl->m_realWbemServicesObj.pItf, pAuthnSvc, pAuthzSvc, pServerPrincName, pAuthnLevel, pImpLevel, pAuthInfo, pCapabilities);
	else if (p_idIUnknown->lpVtbl->m_realUnknownObj.hr == APPX_E_INTERLEAVING_NOT_ALLOWED && IsEqualIIID(&p_idIUnknown->lpVtbl->m_realUnknownObj.idIID, &IID_idIUnknown))
		hr = ((IClientSecurity*)(This->lpVtbl->m_realClientSecurityObj.pItf))->QueryBlanket(p_idIUnknown->lpVtbl->m_realUnknownObj.pItf, pAuthnSvc, pAuthzSvc, pServerPrincName, pAuthnLevel, pImpLevel, pAuthInfo, pCapabilities);
	else if (p_idIWbemLocator->lpVtbl->m_realWbemObj.hr == APPX_E_INTERLEAVING_NOT_ALLOWED && IsEqualIIID(&p_idIWbemLocator->lpVtbl->m_realWbemObj.idIID, &IID_idIWbemLocator))
		hr = ((IClientSecurity*)(This->lpVtbl->m_realClientSecurityObj.pItf))->QueryBlanket(p_idIWbemLocator->lpVtbl->m_realWbemObj.pItf, pAuthnSvc, pAuthzSvc, pServerPrincName, pAuthnLevel, pImpLevel, pAuthInfo, pCapabilities);
	else if (p_idIWbemPath->lpVtbl->m_realIWbemPathObj.hr == APPX_E_INTERLEAVING_NOT_ALLOWED && IsEqualIIID(&p_idIWbemPath->lpVtbl->m_realIWbemPathObj.idIID, &IID_idIWbemPath))
		hr = ((IClientSecurity*)(This->lpVtbl->m_realClientSecurityObj.pItf))->QueryBlanket(p_idIWbemPath->lpVtbl->m_realIWbemPathObj.pItf, pAuthnSvc, pAuthzSvc, pServerPrincName, pAuthnLevel, pImpLevel, pAuthInfo, pCapabilities);
	else if (p_idIEnumWbemClassObject->lpVtbl->m_realEnumWbemClassObjectObj.hr == APPX_E_INTERLEAVING_NOT_ALLOWED && IsEqualIIID(&p_idIEnumWbemClassObject->lpVtbl->m_realEnumWbemClassObjectObj.idIID, &IID_idIEnumWbemClassObject))
		hr = ((IClientSecurity*)(This->lpVtbl->m_realClientSecurityObj.pItf))->QueryBlanket(p_idIEnumWbemClassObject->lpVtbl->m_realEnumWbemClassObjectObj.pItf, pAuthnSvc, pAuthzSvc, pServerPrincName, pAuthnLevel, pImpLevel, pAuthInfo, pCapabilities);
	else if (p_idIWbemClassObject->lpVtbl->m_realWbemClassObjectObj.hr == APPX_E_INTERLEAVING_NOT_ALLOWED && IsEqualIIID(&p_idIWbemClassObject->lpVtbl->m_realWbemClassObjectObj.idIID, &IID_idIWbemClassObject))
		hr = ((IClientSecurity*)(This->lpVtbl->m_realClientSecurityObj.pItf))->QueryBlanket(p_idIWbemClassObject->lpVtbl->m_realWbemClassObjectObj.pItf, pAuthnSvc, pAuthzSvc, pServerPrincName, pAuthnLevel, pImpLevel, pAuthInfo, pCapabilities);
	else if (p_idIWbemCallResult->lpVtbl->m_realWbemCallResultObj.hr == APPX_E_INTERLEAVING_NOT_ALLOWED && IsEqualIIID(&p_idIWbemCallResult->lpVtbl->m_realWbemCallResultObj.idIID, &IID_idIWbemCallResult))
		hr = ((IClientSecurity*)(This->lpVtbl->m_realClientSecurityObj.pItf))->QueryBlanket(p_idIWbemCallResult->lpVtbl->m_realWbemCallResultObj.pItf, pAuthnSvc, pAuthzSvc, pServerPrincName, pAuthnLevel, pImpLevel, pAuthInfo, pCapabilities);
	else
	{
		hr = ((IClientSecurity*)(This->lpVtbl->m_realClientSecurityObj.pItf))->QueryBlanket(pProxy, pAuthnSvc, pAuthzSvc, pServerPrincName, pAuthnLevel, pImpLevel, pAuthInfo, pCapabilities);
	}
	return hr;
}

HRESULT STDMETHODCALLTYPE ClientSecuritySetBlanket(
	idIClientSecurity *This,
	IUnknown *pProxy,
	DWORD AuthnSvc,
	DWORD AuthzSvc,
	OLECHAR *pServerPrincName,
	DWORD AuthnLevel,
	DWORD ImpLevel,
	void *pAuthInfo,
	DWORD Capabilities)
{
	HRESULT hr = S_OK;
	idIWbemServices* p_idIWbemServices = (idIWbemServices*)pProxy;
	idIUnknown* p_idIUnknown = (idIUnknown*)pProxy;
	idIWbemLocator* p_idIWbemLocator = (idIWbemLocator*)pProxy;
	idIWbemPath* p_idIWbemPath = (idIWbemPath*)pProxy;
	idIEnumWbemClassObject* p_idIEnumWbemClassObject = (idIEnumWbemClassObject*)pProxy;
	idIWbemClassObject* p_idIWbemClassObject = (idIWbemClassObject*)pProxy;
	idIWbemCallResult* p_idIWbemCallResult = (idIWbemCallResult*)pProxy;

	if (p_idIWbemServices->lpVtbl->m_realWbemServicesObj.hr == APPX_E_INTERLEAVING_NOT_ALLOWED && IsEqualIIID(&p_idIWbemServices->lpVtbl->m_realWbemServicesObj.idIID, &IID_idIWbemServices))
		hr = ((IClientSecurity*)(This->lpVtbl->m_realClientSecurityObj.pItf))->SetBlanket(p_idIWbemServices->lpVtbl->m_realWbemServicesObj.pItf, AuthnSvc, AuthzSvc, pServerPrincName, AuthnLevel, ImpLevel, pAuthInfo, Capabilities);
	else if (p_idIUnknown->lpVtbl->m_realUnknownObj.hr == APPX_E_INTERLEAVING_NOT_ALLOWED && IsEqualIIID(&p_idIUnknown->lpVtbl->m_realUnknownObj.idIID, &IID_idIUnknown))
		hr = ((IClientSecurity*)(This->lpVtbl->m_realClientSecurityObj.pItf))->SetBlanket(p_idIUnknown->lpVtbl->m_realUnknownObj.pItf, AuthnSvc, AuthzSvc, pServerPrincName, AuthnLevel, ImpLevel, pAuthInfo, Capabilities);
	else if (p_idIWbemLocator->lpVtbl->m_realWbemObj.hr == APPX_E_INTERLEAVING_NOT_ALLOWED && IsEqualIIID(&p_idIWbemLocator->lpVtbl->m_realWbemObj.idIID, &IID_idIWbemLocator))
		hr = ((IClientSecurity*)(This->lpVtbl->m_realClientSecurityObj.pItf))->SetBlanket(p_idIWbemLocator->lpVtbl->m_realWbemObj.pItf, AuthnSvc, AuthzSvc, pServerPrincName, AuthnLevel, ImpLevel, pAuthInfo, Capabilities);
	else if (p_idIWbemPath->lpVtbl->m_realIWbemPathObj.hr == APPX_E_INTERLEAVING_NOT_ALLOWED && IsEqualIIID(&p_idIWbemPath->lpVtbl->m_realIWbemPathObj.idIID, &IID_idIWbemPath))
		hr = ((IClientSecurity*)(This->lpVtbl->m_realClientSecurityObj.pItf))->SetBlanket(p_idIWbemPath->lpVtbl->m_realIWbemPathObj.pItf, AuthnSvc, AuthzSvc, pServerPrincName, AuthnLevel, ImpLevel, pAuthInfo, Capabilities);
	else if (p_idIEnumWbemClassObject->lpVtbl->m_realEnumWbemClassObjectObj.hr == APPX_E_INTERLEAVING_NOT_ALLOWED && IsEqualIIID(&p_idIEnumWbemClassObject->lpVtbl->m_realEnumWbemClassObjectObj.idIID, &IID_idIEnumWbemClassObject))
		hr = ((IClientSecurity*)(This->lpVtbl->m_realClientSecurityObj.pItf))->SetBlanket(p_idIEnumWbemClassObject->lpVtbl->m_realEnumWbemClassObjectObj.pItf, AuthnSvc, AuthzSvc, pServerPrincName, AuthnLevel, ImpLevel, pAuthInfo, Capabilities);
	else if (p_idIWbemClassObject->lpVtbl->m_realWbemClassObjectObj.hr == APPX_E_INTERLEAVING_NOT_ALLOWED && IsEqualIIID(&p_idIWbemClassObject->lpVtbl->m_realWbemClassObjectObj.idIID, &IID_idIWbemClassObject))
		hr = ((IClientSecurity*)(This->lpVtbl->m_realClientSecurityObj.pItf))->SetBlanket(p_idIWbemClassObject->lpVtbl->m_realWbemClassObjectObj.pItf, AuthnSvc, AuthzSvc, pServerPrincName, AuthnLevel, ImpLevel, pAuthInfo, Capabilities);
	else if (p_idIWbemCallResult->lpVtbl->m_realWbemCallResultObj.hr == APPX_E_INTERLEAVING_NOT_ALLOWED && IsEqualIIID(&p_idIWbemCallResult->lpVtbl->m_realWbemCallResultObj.idIID, &IID_idIWbemCallResult))
		hr = ((IClientSecurity*)(This->lpVtbl->m_realClientSecurityObj.pItf))->SetBlanket(p_idIWbemCallResult->lpVtbl->m_realWbemCallResultObj.pItf, AuthnSvc, AuthzSvc, pServerPrincName, AuthnLevel, ImpLevel, pAuthInfo, Capabilities);
	else
	{
		hr = ((IClientSecurity*)(This->lpVtbl->m_realClientSecurityObj.pItf))->SetBlanket(pProxy, AuthnSvc, AuthzSvc, pServerPrincName, AuthnLevel, ImpLevel, pAuthInfo, Capabilities);
	}
	return hr;
}

HRESULT STDMETHODCALLTYPE ClientSecurityCopyProxy(
	idIClientSecurity *This,
	IUnknown *pProxy,
	IUnknown **ppCopy)
{
	HRESULT hr = S_OK;
	idIWbemServices* p_idIWbemServices = (idIWbemServices*)pProxy;
	idIUnknown* p_idIUnknown = (idIUnknown*)pProxy;
	idIWbemLocator* p_idIWbemLocator = (idIWbemLocator*)pProxy;
	idIWbemPath* p_idIWbemPath = (idIWbemPath*)pProxy;
	idIEnumWbemClassObject* p_idIEnumWbemClassObject = (idIEnumWbemClassObject*)pProxy;
	idIWbemClassObject* p_idIWbemClassObject = (idIWbemClassObject*)pProxy;
	idIWbemCallResult* p_idIWbemCallResult = (idIWbemCallResult*)pProxy;

	if (p_idIWbemServices->lpVtbl->m_realWbemServicesObj.hr == APPX_E_INTERLEAVING_NOT_ALLOWED && IsEqualIIID(&p_idIWbemServices->lpVtbl->m_realWbemServicesObj.idIID, &IID_idIWbemServices))
	{
		hr = ((IClientSecurity*)(This->lpVtbl->m_realClientSecurityObj.pItf))->CopyProxy(p_idIWbemServices->lpVtbl->m_realWbemServicesObj.pItf, ppCopy);
		if(hr == S_OK)
			CloneComObject(&IID_idIWbemServices, &p_idIWbemServices->lpVtbl->m_realWbemServicesObj.queryIID, (void**)ppCopy);
	}
	else if (p_idIUnknown->lpVtbl->m_realUnknownObj.hr == APPX_E_INTERLEAVING_NOT_ALLOWED && IsEqualIIID(&p_idIUnknown->lpVtbl->m_realUnknownObj.idIID, &IID_idIUnknown))
	{
		hr = ((IClientSecurity*)(This->lpVtbl->m_realClientSecurityObj.pItf))->CopyProxy(p_idIUnknown->lpVtbl->m_realUnknownObj.pItf, ppCopy);
		if (hr == S_OK)
			CloneComObject(&IID_idIUnknown, &p_idIUnknown->lpVtbl->m_realUnknownObj.queryIID, (void**)ppCopy);
	}
	else if (p_idIWbemLocator->lpVtbl->m_realWbemObj.hr == APPX_E_INTERLEAVING_NOT_ALLOWED && IsEqualIIID(&p_idIWbemLocator->lpVtbl->m_realWbemObj.idIID, &IID_idIWbemLocator))
	{
		hr = ((IClientSecurity*)(This->lpVtbl->m_realClientSecurityObj.pItf))->CopyProxy(p_idIWbemLocator->lpVtbl->m_realWbemObj.pItf, ppCopy);
		if (hr == S_OK)
			CloneComObject(&IID_idIWbemLocator, &p_idIWbemLocator->lpVtbl->m_realWbemObj.queryIID, (void**)ppCopy);
	}
	else if (p_idIWbemPath->lpVtbl->m_realIWbemPathObj.hr == APPX_E_INTERLEAVING_NOT_ALLOWED && IsEqualIIID(&p_idIWbemPath->lpVtbl->m_realIWbemPathObj.idIID, &IID_idIWbemPath))
	{
		hr = ((IClientSecurity*)(This->lpVtbl->m_realClientSecurityObj.pItf))->CopyProxy(p_idIWbemPath->lpVtbl->m_realIWbemPathObj.pItf, ppCopy);
		if (hr == S_OK)
			CloneComObject(&IID_idIWbemPath, &p_idIWbemPath->lpVtbl->m_realIWbemPathObj.queryIID, (void**)ppCopy);
	}
	else if (p_idIEnumWbemClassObject->lpVtbl->m_realEnumWbemClassObjectObj.hr == APPX_E_INTERLEAVING_NOT_ALLOWED && IsEqualIIID(&p_idIEnumWbemClassObject->lpVtbl->m_realEnumWbemClassObjectObj.idIID, &IID_idIEnumWbemClassObject))
	{
		hr = ((IClientSecurity*)(This->lpVtbl->m_realClientSecurityObj.pItf))->CopyProxy(p_idIEnumWbemClassObject->lpVtbl->m_realEnumWbemClassObjectObj.pItf, ppCopy);
		if (hr == S_OK)
			CloneComObject(&IID_idIEnumWbemClassObject, &p_idIEnumWbemClassObject->lpVtbl->m_realEnumWbemClassObjectObj.queryIID, (void**)ppCopy);
	}
	else if (p_idIWbemClassObject->lpVtbl->m_realWbemClassObjectObj.hr == APPX_E_INTERLEAVING_NOT_ALLOWED && IsEqualIIID(&p_idIWbemClassObject->lpVtbl->m_realWbemClassObjectObj.idIID, &IID_idIWbemClassObject))
	{
		hr = ((IClientSecurity*)(This->lpVtbl->m_realClientSecurityObj.pItf))->CopyProxy(p_idIWbemClassObject->lpVtbl->m_realWbemClassObjectObj.pItf, ppCopy);
		if (hr == S_OK)
			CloneComObject(&IID_idIWbemClassObject, &p_idIWbemClassObject->lpVtbl->m_realWbemClassObjectObj.queryIID, (void**)ppCopy);
	}
	else if (p_idIWbemCallResult->lpVtbl->m_realWbemCallResultObj.hr == APPX_E_INTERLEAVING_NOT_ALLOWED && IsEqualIIID(&p_idIWbemCallResult->lpVtbl->m_realWbemCallResultObj.idIID, &IID_idIWbemCallResult))
	{
		hr = ((IClientSecurity*)(This->lpVtbl->m_realClientSecurityObj.pItf))->CopyProxy(p_idIWbemCallResult->lpVtbl->m_realWbemCallResultObj.pItf, ppCopy);
		if (hr == S_OK)
			CloneComObject(&IID_idIWbemCallResult, &p_idIWbemCallResult->lpVtbl->m_realWbemCallResultObj.queryIID, (void**)ppCopy);
	}
	else
	{
		hr = ((IClientSecurity*)(This->lpVtbl->m_realClientSecurityObj.pItf))->CopyProxy(pProxy, ppCopy);
	}

	return hr;
}



