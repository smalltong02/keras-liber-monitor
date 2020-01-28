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
#include "Win32WbemPath.h"
#include "Win32WbemQualifierSet.h"
#include "Win32WbemServices.h"

typedef struct idIWbemObjectSinkVtbl
{
	BEGIN_INTERFACE

		HRESULT(STDMETHODCALLTYPE *QueryInterface)(
			idIWbemObjectSink * This,
			/* [in] */ REFIID riid,
			/* [annotation][iid_is][out] */
			_COM_Outptr_  void **ppvObject);

	ULONG(STDMETHODCALLTYPE *AddRef)(
		idIWbemObjectSink * This);

	ULONG(STDMETHODCALLTYPE *Release)(
		idIWbemObjectSink * This);

	HRESULT(STDMETHODCALLTYPE *Indicate)(
		idIWbemObjectSink * This,
		/* [in] */ long lObjectCount,
		/* [size_is][in] */ idIWbemClassObject **apObjArray);

	HRESULT(STDMETHODCALLTYPE *SetStatus)(
		idIWbemObjectSink * This,
		/* [in] */ long lFlags,
		/* [in] */ HRESULT hResult,
		/* [unique][in] */ BSTR strParam,
		/* [unique][in] */ idIWbemClassObject *pObjParam);

	END_INTERFACE
} idIWbemObjectSinkVtbl;

interface idIWbemObjectSink
{
	CONST_VTBL struct idIWbemObjectSinkVtbl *lpVtbl;
};

