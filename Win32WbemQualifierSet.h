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
#include "Win32WbemLocator.h"
#include "Win32WbemObjectSink.h"
#include "Win32WbemPath.h"
#include "Win32WbemServices.h"

typedef struct idIWbemQualifierSetVtbl
{
	BEGIN_INTERFACE

		HRESULT(STDMETHODCALLTYPE *QueryInterface)(
			idIWbemQualifierSet * This,
			/* [in] */ REFIID riid,
			/* [annotation][iid_is][out] */
			_COM_Outptr_  void **ppvObject);

	ULONG(STDMETHODCALLTYPE *AddRef)(
		idIWbemQualifierSet * This);

	ULONG(STDMETHODCALLTYPE *Release)(
		idIWbemQualifierSet * This);

	HRESULT(STDMETHODCALLTYPE *Get)(
		idIWbemQualifierSet * This,
		/* [string][in] */ LPCWSTR wszName,
		/* [in] */ long lFlags,
		/* [unique][in][out] */ VARIANT *pVal,
		/* [unique][in][out] */ long *plFlavor);

	HRESULT(STDMETHODCALLTYPE *Put)(
		idIWbemQualifierSet * This,
		/* [string][in] */ LPCWSTR wszName,
		/* [in] */ VARIANT *pVal,
		/* [in] */ long lFlavor);

	HRESULT(STDMETHODCALLTYPE *Delete)(
		idIWbemQualifierSet * This,
		/* [string][in] */ LPCWSTR wszName);

	HRESULT(STDMETHODCALLTYPE *GetNames)(
		idIWbemQualifierSet * This,
		/* [in] */ long lFlags,
		/* [out] */ SAFEARRAY * *pNames);

	HRESULT(STDMETHODCALLTYPE *BeginEnumeration)(
		idIWbemQualifierSet * This,
		/* [in] */ long lFlags);

	HRESULT(STDMETHODCALLTYPE *Next)(
		idIWbemQualifierSet * This,
		/* [in] */ long lFlags,
		/* [unique][in][out] */ BSTR *pstrName,
		/* [unique][in][out] */ VARIANT *pVal,
		/* [unique][in][out] */ long *plFlavor);

	HRESULT(STDMETHODCALLTYPE *EndEnumeration)(
		idIWbemQualifierSet * This);

	END_INTERFACE
} idIWbemQualifierSetVtbl;

interface idIWbemQualifierSet
{
	CONST_VTBL struct idIWbemQualifierSetVtbl *lpVtbl;
};

