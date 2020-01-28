#pragma once
#include "commutils.h"
#include "utils.h"
#include "Win32ClassFactory.h"
#include "Win32ClassFactory2.h"
#include "Win32ClientSecurity.h"
#include "Win32EnumWbemClassObject.h"
#include "Win32Marshal.h"
#include "Win32WbemContext.h"
#include "Win32WbemCallResult.h"
#include "Win32WbemClassObject.h"
#include "Win32WbemLocator.h"
#include "Win32WbemObjectSink.h"
#include "Win32WbemPath.h"
#include "Win32WbemQualifierSet.h"
#include "Win32WbemServices.h"

typedef struct idIUnknownVtbl {
	BEGIN_INTERFACE

		/*** IUnknown methods ***/
		HRESULT(STDMETHODCALLTYPE *QueryInterface)(
			idIUnknown *This,
			REFIID riid,
			void **ppvObject);

	ULONG(STDMETHODCALLTYPE *AddRef)(
		idIUnknown *This);

	ULONG(STDMETHODCALLTYPE *Release)(
		idIUnknown *This);

	ULONG m_ref;
	MULTI_idQI m_realUnknownObj;

	END_INTERFACE
} idIUnknownVtbl;

interface idIUnknown {
	CONST_VTBL idIUnknownVtbl* lpVtbl;
};

ULONG STDMETHODCALLTYPE UnknownAddRef(
	idIUnknown * This);

ULONG STDMETHODCALLTYPE UnknownRelease(
	idIUnknown * This);

HRESULT STDMETHODCALLTYPE UnknownQueryInterface(
	idIUnknown * This,
	/* [in] */ REFIID riid,
	/* [annotation][iid_is][out] */
	_COM_Outptr_  void **ppvObject);





