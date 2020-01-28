#pragma once
#include "commutils.h"
#include "utils.h"
#include "Win32ClassFactory.h"
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
#include "Win32WbemQualifierSet.h"
#include "Win32WbemServices.h"

typedef struct idIClassFactory2Vtbl {
	BEGIN_INTERFACE

		/*** IUnknown methods ***/
		HRESULT(STDMETHODCALLTYPE *QueryInterface)(
			idIClassFactory2 *This,
			REFIID riid,
			void **ppvObject);

	ULONG(STDMETHODCALLTYPE *AddRef)(
		idIClassFactory2 *This);

	ULONG(STDMETHODCALLTYPE *Release)(
		idIClassFactory2 *This);

	/*** IClassFactory methods ***/
	HRESULT(STDMETHODCALLTYPE *CreateInstance)(
		idIClassFactory2 *This,
		IUnknown *pUnkOuter,
		REFIID riid,
		void **ppvObject);

	HRESULT(STDMETHODCALLTYPE *LockServer)(
		idIClassFactory2 *This,
		BOOL fLock);

	/*** IClassFactory2 methods ***/
	HRESULT(STDMETHODCALLTYPE *GetLicInfo)(
		idIClassFactory2 *This,
		LICINFO *pLicInfo);

	HRESULT(STDMETHODCALLTYPE *RequestLicKey)(
		idIClassFactory2 *This,
		DWORD dwReserved,
		BSTR *pBstrKey);

	HRESULT(STDMETHODCALLTYPE *CreateInstanceLic)(
		idIClassFactory2 *This,
		IUnknown *pUnkOuter,
		IUnknown *pUnkReserved,
		REFIID riid,
		BSTR bstrKey,
		PVOID *ppvObj);

	ULONG m_ref;
	MULTI_idQI m_realFactory2Obj;

	END_INTERFACE
} idIClassFactory2Vtbl;

interface idIClassFactory2 {
	CONST_VTBL idIClassFactory2Vtbl* lpVtbl;
};

HRESULT STDMETHODCALLTYPE Factory2QueryInterface(
	idIClassFactory2 *This,
	REFIID riid,
	void **ppvObject);

ULONG STDMETHODCALLTYPE Factory2AddRef(
	idIClassFactory2 *This);

ULONG STDMETHODCALLTYPE Factory2Release(
	idIClassFactory2 *This);

HRESULT STDMETHODCALLTYPE Factory2CreateInstance(
	idIClassFactory2 *This,
	IUnknown *pUnkOuter,
	REFIID riid,
	void **ppvObject);

HRESULT STDMETHODCALLTYPE Factory2LockServer(
	idIClassFactory2 *This,
	BOOL fLock);

HRESULT STDMETHODCALLTYPE Factory2GetLicInfo(
	idIClassFactory2 *This,
	LICINFO *pLicInfo);

HRESULT STDMETHODCALLTYPE Factory2RequestLicKey(
	idIClassFactory2 *This,
	DWORD dwReserved,
	BSTR *pBstrKey);

HRESULT STDMETHODCALLTYPE Factory2CreateInstanceLic(
	idIClassFactory2 *This,
	IUnknown *pUnkOuter,
	IUnknown *pUnkReserved,
	REFIID riid,
	BSTR bstrKey,
	PVOID *ppvObj);



