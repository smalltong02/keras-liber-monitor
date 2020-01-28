#pragma once
#include "commutils.h"
#include "utils.h"
#include "Win32ClassFactory.h"
#include "Win32ClassFactory2.h"
#include "Win32ClientSecurity.h"
#include "Win32EnumWbemClassObject.h"
#include "Win32WbemContext.h"
#include "Win32Unknown.h"
#include "Win32WbemCallResult.h"
#include "Win32WbemClassObject.h"
#include "Win32WbemLocator.h"
#include "Win32WbemObjectSink.h"
#include "Win32WbemPath.h"
#include "Win32WbemQualifierSet.h"
#include "Win32WbemServices.h"

typedef struct idWin32IMarshalVtbl {
	BEGIN_INTERFACE

	/*** IUnknown methods ***/
	HRESULT(STDMETHODCALLTYPE *QueryInterface)(
		idWin32IMarshal *This,
		REFIID riid,
		void **ppvObject);

	ULONG(STDMETHODCALLTYPE *AddRef)(
		idWin32IMarshal *This);

	ULONG(STDMETHODCALLTYPE *Release)(
		idWin32IMarshal *This);

	/*** IMarshal methods ***/
	HRESULT(STDMETHODCALLTYPE *GetUnmarshalClass)(
		idWin32IMarshal *This,
		REFIID riid,
		void *pv,
		DWORD dwDestContext,
		void *pvDestContext,
		DWORD mshlflags,
		CLSID *pCid);

	HRESULT(STDMETHODCALLTYPE *GetMarshalSizeMax)(
		idWin32IMarshal *This,
		REFIID riid,
		void *pv,
		DWORD dwDestContext,
		void *pvDestContext,
		DWORD mshlflags,
		DWORD *pSize);

	HRESULT(STDMETHODCALLTYPE *MarshalInterface)(
		idWin32IMarshal *This,
		IStream *pStm,
		REFIID riid,
		void *pv,
		DWORD dwDestContext,
		void *pvDestContext,
		DWORD mshlflags);

	HRESULT(STDMETHODCALLTYPE *UnmarshalInterface)(
		idWin32IMarshal *This,
		IStream *pStm,
		REFIID riid,
		void **ppv);

	HRESULT(STDMETHODCALLTYPE *ReleaseMarshalData)(
		idWin32IMarshal *This,
		IStream *pStm);

	HRESULT(STDMETHODCALLTYPE *DisconnectObject)(
		idWin32IMarshal *This,
		DWORD dwReserved);

	ULONG m_ref;

	END_INTERFACE
} idWin32IMarshalVtbl;

interface idWin32IMarshal {
	CONST_VTBL idWin32IMarshalVtbl* lpVtbl;
};

HRESULT STDMETHODCALLTYPE MarshalQueryInterface(
	idWin32IMarshal *This,
	REFIID riid,
	void **ppvObject);

ULONG STDMETHODCALLTYPE MarshalAddRef(
	idWin32IMarshal *This);

ULONG STDMETHODCALLTYPE MarshalRelease(
	idWin32IMarshal *This);

/*** IMarshal methods ***/
HRESULT STDMETHODCALLTYPE MarshalGetUnmarshalClass(
	idWin32IMarshal *This,
	REFIID riid,
	void *pv,
	DWORD dwDestContext,
	void *pvDestContext,
	DWORD mshlflags,
	CLSID *pCid);

HRESULT STDMETHODCALLTYPE MarshalGetMarshalSizeMax(
	idWin32IMarshal *This,
	REFIID riid,
	void *pv,
	DWORD dwDestContext,
	void *pvDestContext,
	DWORD mshlflags,
	DWORD *pSize);

HRESULT STDMETHODCALLTYPE MarshalMarshalInterface(
	idWin32IMarshal *This,
	IStream *pStm,
	REFIID riid,
	void *pv,
	DWORD dwDestContext,
	void *pvDestContext,
	DWORD mshlflags);

HRESULT STDMETHODCALLTYPE MarshalUnmarshalInterface(
	idWin32IMarshal *This,
	IStream *pStm,
	REFIID riid,
	void **ppv);

HRESULT STDMETHODCALLTYPE MarshalReleaseMarshalData(
	idWin32IMarshal *This,
	IStream *pStm);

HRESULT STDMETHODCALLTYPE MarshalDisconnectObject(
	idWin32IMarshal *This,
	DWORD dwReserved);

void InitializeWin32IMarshal(idWin32IMarshal* p_IWin32IMarshal);


