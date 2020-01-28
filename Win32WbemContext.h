#pragma once
#include "commutils.h"
#include "utils.h"
#include "Win32ClassFactory.h"
#include "Win32ClassFactory2.h"
#include "Win32ClientSecurity.h"
#include "Win32EnumWbemClassObject.h"
#include "Win32Marshal.h"
#include "Win32Unknown.h"
#include "Win32WbemCallResult.h"
#include "Win32WbemClassObject.h"
#include "Win32WbemLocator.h"
#include "Win32WbemObjectSink.h"
#include "Win32WbemPath.h"
#include "Win32WbemQualifierSet.h"
#include "Win32WbemServices.h"

typedef struct idIWbemContextVtbl
{
	BEGIN_INTERFACE

		HRESULT(STDMETHODCALLTYPE *QueryInterface)(
			idIWbemContext * This,
			/* [in] */ REFIID riid,
			/* [annotation][iid_is][out] */
			_COM_Outptr_  void **ppvObject);

	ULONG(STDMETHODCALLTYPE *AddRef)(
		idIWbemContext * This);

	ULONG(STDMETHODCALLTYPE *Release)(
		idIWbemContext * This);

	HRESULT(STDMETHODCALLTYPE *Clone)(
		idIWbemContext * This,
		/* [out] */ idIWbemContext **ppNewCopy);

	HRESULT(STDMETHODCALLTYPE *GetNames)(
		idIWbemContext * This,
		/* [in] */ long lFlags,
		/* [out] */ SAFEARRAY * *pNames);

	HRESULT(STDMETHODCALLTYPE *BeginEnumeration)(
		idIWbemContext * This,
		/* [in] */ long lFlags);

	HRESULT(STDMETHODCALLTYPE *Next)(
		idIWbemContext * This,
		/* [in] */ long lFlags,
		/* [out] */ BSTR *pstrName,
		/* [out] */ VARIANT *pValue);

	HRESULT(STDMETHODCALLTYPE *EndEnumeration)(
		idIWbemContext * This);

	HRESULT(STDMETHODCALLTYPE *SetValue)(
		idIWbemContext * This,
		/* [string][in] */ LPCWSTR wszName,
		/* [in] */ long lFlags,
		/* [in] */ VARIANT *pValue);

	HRESULT(STDMETHODCALLTYPE *GetValue)(
		idIWbemContext * This,
		/* [string][in] */ LPCWSTR wszName,
		/* [in] */ long lFlags,
		/* [out] */ VARIANT *pValue);

	HRESULT(STDMETHODCALLTYPE *DeleteValue)(
		idIWbemContext * This,
		/* [string][in] */ LPCWSTR wszName,
		/* [in] */ long lFlags);

	HRESULT(STDMETHODCALLTYPE *DeleteAll)(
		idIWbemContext * This);

	END_INTERFACE
} idIWbemContextVtbl;

interface idIWbemContext
{
	CONST_VTBL struct idIWbemContextVtbl *lpVtbl;
};