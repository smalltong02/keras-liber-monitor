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
#include "Win32WbemQualifierSet.h"
#include "Win32WbemServices.h"

typedef struct idIWbemPathVtbl {
	BEGIN_INTERFACE

		/*** IUnknown methods ***/
		HRESULT(STDMETHODCALLTYPE *QueryInterface)(
			idIWbemPath *This,
			REFIID riid,
			void **ppvObject);

	ULONG(STDMETHODCALLTYPE *AddRef)(
		idIWbemPath *This);

	ULONG(STDMETHODCALLTYPE *Release)(
		idIWbemPath *This);

	/*** IWbemPath methods ***/
	HRESULT(STDMETHODCALLTYPE *SetText)(
		idIWbemPath *This,
		ULONG uMode,
		LPCWSTR pszPath);

	HRESULT(STDMETHODCALLTYPE *GetText)(
		idIWbemPath *This,
		LONG lFlags,
		ULONG *puBuffLength,
		LPWSTR pszText);

	HRESULT(STDMETHODCALLTYPE *GetInfo)(
		idIWbemPath *This,
		ULONG uRequestedInfo,
		ULONGLONG *puResponse);

	HRESULT(STDMETHODCALLTYPE *SetServer)(
		idIWbemPath *This,
		LPCWSTR Name);

	HRESULT(STDMETHODCALLTYPE *GetServer)(
		idIWbemPath *This,
		ULONG *puNameBufLength,
		LPWSTR pName);

	HRESULT(STDMETHODCALLTYPE *GetNamespaceCount)(
		idIWbemPath *This,
		ULONG *puCount);

	HRESULT(STDMETHODCALLTYPE *SetNamespaceAt)(
		idIWbemPath *This,
		ULONG uIndex,
		LPCWSTR pszName);

	HRESULT(STDMETHODCALLTYPE *GetNamespaceAt)(
		idIWbemPath *This,
		ULONG uIndex,
		ULONG *puNameBufLength,
		LPWSTR pName);

	HRESULT(STDMETHODCALLTYPE *RemoveNamespaceAt)(
		idIWbemPath *This,
		ULONG uIndex);

	HRESULT(STDMETHODCALLTYPE *RemoveAllNamespaces)(
		idIWbemPath *This);

	HRESULT(STDMETHODCALLTYPE *GetScopeCount)(
		idIWbemPath *This,
		ULONG *puCount);

	HRESULT(STDMETHODCALLTYPE *SetScope)(
		idIWbemPath *This,
		ULONG uIndex,
		LPWSTR pszClass);

	HRESULT(STDMETHODCALLTYPE *SetScopeFromText)(
		idIWbemPath *This,
		ULONG uIndex,
		LPWSTR pszText);

	HRESULT(STDMETHODCALLTYPE *GetScope)(
		idIWbemPath *This,
		ULONG uIndex,
		ULONG *puClassNameBufSize,
		LPWSTR pszClass,
		IWbemPathKeyList **pKeyList);

	HRESULT(STDMETHODCALLTYPE *GetScopeAsText)(
		idIWbemPath *This,
		ULONG uIndex,
		ULONG *puTextBufSize,
		LPWSTR pszText);

	HRESULT(STDMETHODCALLTYPE *RemoveScope)(
		idIWbemPath *This,
		ULONG uIndex);

	HRESULT(STDMETHODCALLTYPE *RemoveAllScopes)(
		idIWbemPath *This);

	HRESULT(STDMETHODCALLTYPE *SetClassName)(
		idIWbemPath *This,
		LPCWSTR Name);

	HRESULT(STDMETHODCALLTYPE *GetClassName)(
		idIWbemPath *This,
		ULONG *puBuffLength,
		LPWSTR pszName);

	HRESULT(STDMETHODCALLTYPE *GetKeyList)(
		idIWbemPath *This,
		IWbemPathKeyList **pOut);

	HRESULT(STDMETHODCALLTYPE *CreateClassPart)(
		idIWbemPath *This,
		LONG lFlags,
		LPCWSTR Name);

	HRESULT(STDMETHODCALLTYPE *DeleteClassPart)(
		idIWbemPath *This,
		LONG lFlags);

	BOOL(STDMETHODCALLTYPE *IsRelative)(
		idIWbemPath *This,
		LPWSTR wszMachine,
		LPWSTR wszNamespace);

	BOOL(STDMETHODCALLTYPE *IsRelativeOrChild)(
		idIWbemPath *This,
		LPWSTR wszMachine,
		LPWSTR wszNamespace,
		LONG lFlags);

	BOOL(STDMETHODCALLTYPE *IsLocal)(
		idIWbemPath *This,
		LPCWSTR wszMachine);

	BOOL(STDMETHODCALLTYPE *IsSameClassName)(
		idIWbemPath *This,
		LPCWSTR wszClass);

	ULONG m_ref;
	MULTI_idQI m_realIWbemPathObj;

	END_INTERFACE
} idIWbemPathVtbl;

interface idIWbemPath {
	CONST_VTBL idIWbemPathVtbl* lpVtbl;
};

HRESULT STDMETHODCALLTYPE WbemPathQueryInterface(
	idIWbemPath *This,
	REFIID riid,
	void **ppvObject);

ULONG STDMETHODCALLTYPE WbemPathAddRef(
	idIWbemPath *This);

ULONG STDMETHODCALLTYPE WbemPathRelease(
	idIWbemPath *This);

/*** IWbemPath methods ***/
HRESULT STDMETHODCALLTYPE WbemPathSetText(
	idIWbemPath *This,
	ULONG uMode,
	LPCWSTR pszPath);

HRESULT STDMETHODCALLTYPE WbemPathGetText(
	idIWbemPath *This,
	LONG lFlags,
	ULONG *puBuffLength,
	LPWSTR pszText);

HRESULT STDMETHODCALLTYPE WbemPathGetInfo(
	idIWbemPath *This,
	ULONG uRequestedInfo,
	ULONGLONG *puResponse);

HRESULT STDMETHODCALLTYPE WbemPathSetServer(
	idIWbemPath *This,
	LPCWSTR Name);

HRESULT STDMETHODCALLTYPE WbemPathGetServer(
	idIWbemPath *This,
	ULONG *puNameBufLength,
	LPWSTR pName);

HRESULT STDMETHODCALLTYPE WbemPathGetNamespaceCount(
	idIWbemPath *This,
	ULONG *puCount);

HRESULT STDMETHODCALLTYPE WbemPathSetNamespaceAt(
	idIWbemPath *This,
	ULONG uIndex,
	LPCWSTR pszName);

HRESULT STDMETHODCALLTYPE WbemPathGetNamespaceAt(
	idIWbemPath *This,
	ULONG uIndex,
	ULONG *puNameBufLength,
	LPWSTR pName);

HRESULT STDMETHODCALLTYPE WbemPathRemoveNamespaceAt(
	idIWbemPath *This,
	ULONG uIndex);

HRESULT STDMETHODCALLTYPE WbemPathRemoveAllNamespaces(
	idIWbemPath *This);

HRESULT STDMETHODCALLTYPE WbemPathGetScopeCount(
	idIWbemPath *This,
	ULONG *puCount);

HRESULT STDMETHODCALLTYPE WbemPathSetScope(
	idIWbemPath *This,
	ULONG uIndex,
	LPWSTR pszClass);

HRESULT STDMETHODCALLTYPE WbemPathSetScopeFromText(
	idIWbemPath *This,
	ULONG uIndex,
	LPWSTR pszText);

HRESULT STDMETHODCALLTYPE WbemPathGetScope(
	idIWbemPath *This,
	ULONG uIndex,
	ULONG *puClassNameBufSize,
	LPWSTR pszClass,
	IWbemPathKeyList **pKeyList);

HRESULT STDMETHODCALLTYPE WbemPathGetScopeAsText(
	idIWbemPath *This,
	ULONG uIndex,
	ULONG *puTextBufSize,
	LPWSTR pszText);

HRESULT STDMETHODCALLTYPE WbemPathRemoveScope(
	idIWbemPath *This,
	ULONG uIndex);

HRESULT STDMETHODCALLTYPE WbemPathRemoveAllScopes(
	idIWbemPath *This);

HRESULT STDMETHODCALLTYPE WbemPathSetClassName(
	idIWbemPath *This,
	LPCWSTR Name);

HRESULT STDMETHODCALLTYPE WbemPathGetClassName(
	idIWbemPath *This,
	ULONG *puBuffLength,
	LPWSTR pszName);

HRESULT STDMETHODCALLTYPE WbemPathGetKeyList(
	idIWbemPath *This,
	IWbemPathKeyList **pOut);

HRESULT STDMETHODCALLTYPE WbemPathCreateClassPart(
	idIWbemPath *This,
	LONG lFlags,
	LPCWSTR Name);

HRESULT STDMETHODCALLTYPE WbemPathDeleteClassPart(
	idIWbemPath *This,
	LONG lFlags);

BOOL STDMETHODCALLTYPE WbemPathIsRelative(
	idIWbemPath *This,
	LPWSTR wszMachine,
	LPWSTR wszNamespace);

BOOL STDMETHODCALLTYPE WbemPathIsRelativeOrChild(
	idIWbemPath *This,
	LPWSTR wszMachine,
	LPWSTR wszNamespace,
	LONG lFlags);

BOOL STDMETHODCALLTYPE WbemPathIsLocal(
	idIWbemPath *This,
	LPCWSTR wszMachine);

BOOL STDMETHODCALLTYPE WbemPathIsSameClassName(
	idIWbemPath *This,
	LPCWSTR wszClass);



