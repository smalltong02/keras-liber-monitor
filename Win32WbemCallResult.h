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
#include "Win32WbemClassObject.h"
#include "Win32WbemLocator.h"
#include "Win32WbemObjectSink.h"
#include "Win32WbemPath.h"
#include "Win32WbemQualifierSet.h"
#include "Win32WbemServices.h"

typedef struct idIWbemCallResultVtbl
{
	BEGIN_INTERFACE

		HRESULT(STDMETHODCALLTYPE *QueryInterface)(
			idIWbemCallResult * This,
			/* [in] */ REFIID riid,
			/* [annotation][iid_is][out] */
			_COM_Outptr_  void **ppvObject);

	ULONG(STDMETHODCALLTYPE *AddRef)(
		idIWbemCallResult * This);

	ULONG(STDMETHODCALLTYPE *Release)(
		idIWbemCallResult * This);

	HRESULT(STDMETHODCALLTYPE *GetResultObject)(
		idIWbemCallResult * This,
		/* [in] */ long lTimeout,
		/* [out] */ idIWbemClassObject **ppResultObject);

	HRESULT(STDMETHODCALLTYPE *GetResultString)(
		idIWbemCallResult * This,
		/* [in] */ long lTimeout,
		/* [out] */ BSTR *pstrResultString);

	HRESULT(STDMETHODCALLTYPE *GetResultServices)(
		idIWbemCallResult * This,
		/* [in] */ long lTimeout,
		/* [out] */ idIWbemServices **ppServices);

	HRESULT(STDMETHODCALLTYPE *GetCallStatus)(
		idIWbemCallResult * This,
		/* [in] */ long lTimeout,
		/* [out] */ long *plStatus);

	ULONG m_ref;
	MULTI_idQI m_realWbemCallResultObj;

	END_INTERFACE
} idIWbemCallResultVtbl;

interface idIWbemCallResult
{
	CONST_VTBL struct idIWbemCallResultVtbl *lpVtbl;
};

HRESULT STDMETHODCALLTYPE WbemCallResultQueryInterface(
	idIWbemCallResult * This,
	/* [in] */ REFIID riid,
	/* [annotation][iid_is][out] */
	_COM_Outptr_  void **ppvObject);

ULONG STDMETHODCALLTYPE WbemCallResultAddRef(
	idIWbemCallResult * This);

ULONG STDMETHODCALLTYPE WbemCallResultRelease(
	idIWbemCallResult * This);

HRESULT STDMETHODCALLTYPE WbemCallResultGetResultObject(
	idIWbemCallResult * This,
	/* [in] */ long lTimeout,
	/* [out] */ idIWbemClassObject **ppResultObject);

HRESULT STDMETHODCALLTYPE WbemCallResultGetResultString(
	idIWbemCallResult * This,
	/* [in] */ long lTimeout,
	/* [out] */ BSTR *pstrResultString);

HRESULT STDMETHODCALLTYPE WbemCallResultGetResultServices(
	idIWbemCallResult * This,
	/* [in] */ long lTimeout,
	/* [out] */ idIWbemServices **ppServices);

HRESULT STDMETHODCALLTYPE WbemCallResultGetCallStatus(
	idIWbemCallResult * This,
	/* [in] */ long lTimeout,
	/* [out] */ long *plStatus);


