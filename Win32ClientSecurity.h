#pragma once
#include "commutils.h"
#include "utils.h"
#include "Win32ClassFactory.h"
#include "Win32ClassFactory2.h"
#include "Win32EnumWbemClassObject.h"
#include "Win32Marshal.h"
#include "Win32WbemContext.h"
#include "Win32Unknown.h"
#include "Win32WbemCallResult.h"
#include "Win32WbemClassObject.h"
#include "Win32WbemLocator.h"
#include "Win32WbemObjectSink.h"
#include "Win32WbemPath.h"
#include "Win32WbemQualifierSet.h"
#include "Win32WbemServices.h"

typedef struct idIClientSecurityVtbl {
	BEGIN_INTERFACE

		/*** IUnknown methods ***/
		HRESULT(STDMETHODCALLTYPE *QueryInterface)(
			idIClientSecurity *This,
			REFIID riid,
			void **ppvObject);

	ULONG(STDMETHODCALLTYPE *AddRef)(
		idIClientSecurity *This);

	ULONG(STDMETHODCALLTYPE *Release)(
		idIClientSecurity *This);

	HRESULT(STDMETHODCALLTYPE *QueryBlanket)(
		idIClientSecurity *This,
		IUnknown *pProxy,
		DWORD *pAuthnSvc,
		DWORD *pAuthzSvc,
		OLECHAR **pServerPrincName,
		DWORD *pAuthnLevel,
		DWORD *pImpLevel,
		void **pAuthInfo,
		DWORD *pCapabilities);

	HRESULT(STDMETHODCALLTYPE *SetBlanket)(
		idIClientSecurity *This,
		IUnknown *pProxy,
		DWORD AuthnSvc,
		DWORD AuthzSvc,
		OLECHAR *pServerPrincName,
		DWORD AuthnLevel,
		DWORD ImpLevel,
		void *pAuthInfo,
		DWORD Capabilities);

	HRESULT(STDMETHODCALLTYPE *CopyProxy)(
		idIClientSecurity *This,
		IUnknown *pProxy,
		IUnknown **ppCopy);

	ULONG m_ref;
	MULTI_idQI m_realClientSecurityObj;

	END_INTERFACE
} idIClientSecurityVtbl;

interface idIClientSecurity {
	CONST_VTBL idIClientSecurityVtbl* lpVtbl;
};

HRESULT STDMETHODCALLTYPE ClientSecurityQueryInterface(
	idIClientSecurity *This,
	REFIID riid,
	void **ppvObject);

ULONG STDMETHODCALLTYPE ClientSecurityAddRef(
	idIClientSecurity *This);

ULONG STDMETHODCALLTYPE ClientSecurityRelease(
	idIClientSecurity *This);

HRESULT STDMETHODCALLTYPE ClientSecurityQueryBlanket(
	idIClientSecurity *This,
	IUnknown *pProxy,
	DWORD *pAuthnSvc,
	DWORD *pAuthzSvc,
	OLECHAR **pServerPrincName,
	DWORD *pAuthnLevel,
	DWORD *pImpLevel,
	void **pAuthInfo,
	DWORD *pCapabilities);

HRESULT STDMETHODCALLTYPE ClientSecuritySetBlanket(
	idIClientSecurity *This,
	IUnknown *pProxy,
	DWORD AuthnSvc,
	DWORD AuthzSvc,
	OLECHAR *pServerPrincName,
	DWORD AuthnLevel,
	DWORD ImpLevel,
	void *pAuthInfo,
	DWORD Capabilities);

HRESULT STDMETHODCALLTYPE ClientSecurityCopyProxy(
	idIClientSecurity *This,
	IUnknown *pProxy,
	IUnknown **ppCopy);



