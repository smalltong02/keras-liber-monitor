#pragma once
#include "commutils.h"
#include "utils.h"
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
#include "Win32WbemQualifierSet.h"
#include "Win32WbemServices.h"

typedef struct idIClassFactoryVtbl {
	BEGIN_INTERFACE

		/*** IUnknown methods ***/
		HRESULT(STDMETHODCALLTYPE *QueryInterface)(
			idIClassFactory *This,
			REFIID riid,
			void **ppvObject);

	ULONG(STDMETHODCALLTYPE *AddRef)(
		idIClassFactory *This);

	ULONG(STDMETHODCALLTYPE *Release)(
		idIClassFactory *This);

	/*** IClassFactory methods ***/
	HRESULT(STDMETHODCALLTYPE *CreateInstance)(
		idIClassFactory *This,
		IUnknown *pUnkOuter,
		REFIID riid,
		void **ppvObject);

	HRESULT(STDMETHODCALLTYPE *LockServer)(
		idIClassFactory *This,
		BOOL fLock);

	ULONG m_ref;
	PVOID m_realFactoryObj;

	END_INTERFACE
} idIClassFactoryVtbl;

interface idIClassFactory {
	CONST_VTBL idIClassFactoryVtbl* lpVtbl;
};

HRESULT STDMETHODCALLTYPE FactoryQueryInterface(
	idIClassFactory *This,
	REFIID riid,
	void **ppvObject);

ULONG STDMETHODCALLTYPE FactoryAddRef(
	idIClassFactory *This);

ULONG STDMETHODCALLTYPE FactoryRelease(
	idIClassFactory *This);

HRESULT STDMETHODCALLTYPE FactoryCreateInstance(
	idIClassFactory *This,
	IUnknown *pUnkOuter,
	REFIID riid,
	void **ppvObject);

HRESULT STDMETHODCALLTYPE FactoryLockServer(
	idIClassFactory *This,
	BOOL fLock);




