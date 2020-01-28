#pragma once
#include "commutils.h"
#include "utils.h"
#include "Win32ClassFactory.h"
#include "Win32ClassFactory2.h"
#include "Win32ClientSecurity.h"
#include "Win32EnumWbemClassObject.h"
#include "Win32Marshal.h"
#include "Win32WbemContext.h"
#include "Win32Unknown.h"
#include "Win32WbemCallResult.h"
#include "Win32WbemClassObject.h"
#include "Win32WbemObjectSink.h"
#include "Win32WbemPath.h"
#include "Win32WbemQualifierSet.h"
#include "Win32WbemServices.h"

typedef struct idIWbemLocatorVtbl
{
	BEGIN_INTERFACE

		HRESULT(STDMETHODCALLTYPE *QueryInterface)(
			idIWbemLocator * This,
			/* [in] */ REFIID riid,
			/* [annotation][iid_is][out] */
			_COM_Outptr_  void **ppvObject);

	ULONG(STDMETHODCALLTYPE *AddRef)(
		idIWbemLocator * This);

	ULONG(STDMETHODCALLTYPE *Release)(
		idIWbemLocator * This);

	HRESULT(STDMETHODCALLTYPE *ConnectServer)(
		idIWbemLocator * This,
		/* [in] */ const BSTR strNetworkResource,
		/* [in] */ const BSTR strUser,
		/* [in] */ const BSTR strPassword,
		/* [in] */ const BSTR strLocale,
		/* [in] */ long lSecurityFlags,
		/* [in] */ const BSTR strAuthority,
		/* [in] */ idIWbemContext *pCtx,
		/* [out] */ idIWbemServices **ppNamespace);

	ULONG m_ref;
	MULTI_idQI m_realWbemObj;

	END_INTERFACE
} idIWbemLocatorVtbl;

interface idIWbemLocator
{
	struct idIWbemLocatorVtbl *lpVtbl;
};

HRESULT STDMETHODCALLTYPE WbemQueryInterface(
	idIWbemLocator * This,
	/* [in] */ REFIID riid,
	/* [annotation][iid_is][out] */
	_COM_Outptr_  void **ppvObject);

ULONG STDMETHODCALLTYPE WbemAddRef(
	idIWbemLocator * This);

ULONG STDMETHODCALLTYPE WbemRelease(
	idIWbemLocator * This);

HRESULT STDMETHODCALLTYPE WbemConnectServer(
	idIWbemLocator * This,
	/* [in] */ const BSTR strNetworkResource,
	/* [in] */ const BSTR strUser,
	/* [in] */ const BSTR strPassword,
	/* [in] */ const BSTR strLocale,
	/* [in] */ long lSecurityFlags,
	/* [in] */ const BSTR strAuthority,
	/* [in] */ idIWbemContext *pCtx,
	/* [out] */ idIWbemServices **ppNamespace);







