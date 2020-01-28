#pragma once
#include "commutils.h"
#include "utils.h"
#include "Win32ClassFactory.h"
#include "Win32ClassFactory2.h"
#include "Win32ClientSecurity.h"
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

typedef struct idIEnumWbemClassObjectVtbl
{
	BEGIN_INTERFACE

		HRESULT(STDMETHODCALLTYPE *QueryInterface)(
			idIEnumWbemClassObject * This,
			/* [in] */ REFIID riid,
			/* [annotation][iid_is][out] */
			_COM_Outptr_  void **ppvObject);

	ULONG(STDMETHODCALLTYPE *AddRef)(
		idIEnumWbemClassObject * This);

	ULONG(STDMETHODCALLTYPE *Release)(
		idIEnumWbemClassObject * This);

	HRESULT(STDMETHODCALLTYPE *Reset)(
		idIEnumWbemClassObject * This);

	HRESULT(STDMETHODCALLTYPE *Next)(
		idIEnumWbemClassObject * This,
		/* [in] */ long lTimeout,
		/* [in] */ ULONG uCount,
		/* [length_is][size_is][out] */ idIWbemClassObject **apObjects,
		/* [out] */ ULONG *puReturned);

	HRESULT(STDMETHODCALLTYPE *NextAsync)(
		idIEnumWbemClassObject * This,
		/* [in] */ ULONG uCount,
		/* [in] */ idIWbemObjectSink *pSink);

	HRESULT(STDMETHODCALLTYPE *Clone)(
		idIEnumWbemClassObject * This,
		/* [out] */ idIEnumWbemClassObject **ppEnum);

	HRESULT(STDMETHODCALLTYPE *Skip)(
		idIEnumWbemClassObject * This,
		/* [in] */ long lTimeout,
		/* [in] */ ULONG nCount);

	ULONG m_ref;
	ULONG m_nItem;
	MULTI_idQI m_realEnumWbemClassObjectObj;

	END_INTERFACE
} idIEnumWbemClassObjectVtbl;

interface idIEnumWbemClassObject
{
	CONST_VTBL struct idIEnumWbemClassObjectVtbl *lpVtbl;
};

HRESULT STDMETHODCALLTYPE EnumWbemClassObjectQueryInterface(
	idIEnumWbemClassObject * This,
	/* [in] */ REFIID riid,
	/* [annotation][iid_is][out] */
	_COM_Outptr_  void **ppvObject);

ULONG STDMETHODCALLTYPE EnumWbemClassObjectAddRef(
	idIEnumWbemClassObject * This);

ULONG STDMETHODCALLTYPE EnumWbemClassObjectRelease(
	idIEnumWbemClassObject * This);

HRESULT STDMETHODCALLTYPE EnumWbemClassObjectReset(
	idIEnumWbemClassObject * This);

HRESULT STDMETHODCALLTYPE EnumWbemClassObjectNext(
	idIEnumWbemClassObject * This,
	/* [in] */ long lTimeout,
	/* [in] */ ULONG uCount,
	/* [length_is][size_is][out] */ idIWbemClassObject **apObjects,
	/* [out] */ ULONG *puReturned);

HRESULT STDMETHODCALLTYPE EnumWbemClassObjectNextAsync(
	idIEnumWbemClassObject * This,
	/* [in] */ ULONG uCount,
	/* [in] */ idIWbemObjectSink *pSink);

HRESULT STDMETHODCALLTYPE EnumWbemClassObjectClone(
	idIEnumWbemClassObject * This,
	/* [out] */ idIEnumWbemClassObject **ppEnum);

HRESULT STDMETHODCALLTYPE EnumWbemClassObjectSkip(
	idIEnumWbemClassObject * This,
	/* [in] */ long lTimeout,
	/* [in] */ ULONG nCount);


