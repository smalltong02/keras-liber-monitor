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
#include "Win32WbemLocator.h"
#include "Win32WbemObjectSink.h"
#include "Win32WbemPath.h"
#include "Win32WbemQualifierSet.h"
#include "Win32WbemServices.h"
#include "OleConfigObject.h"

typedef struct idIWbemClassObjectVtbl
{
	BEGIN_INTERFACE

		HRESULT(STDMETHODCALLTYPE *QueryInterface)(
			idIWbemClassObject * This,
			/* [in] */ REFIID riid,
			/* [annotation][iid_is][out] */
			_COM_Outptr_  void **ppvObject);

	ULONG(STDMETHODCALLTYPE *AddRef)(
		idIWbemClassObject * This);

	ULONG(STDMETHODCALLTYPE *Release)(
		idIWbemClassObject * This);

	HRESULT(STDMETHODCALLTYPE *GetQualifierSet)(
		idIWbemClassObject * This,
		/* [out] */ idIWbemQualifierSet **ppQualSet);

	HRESULT(STDMETHODCALLTYPE *Get)(
		idIWbemClassObject * This,
		/* [string][in] */ LPCWSTR wszName,
		/* [in] */ long lFlags,
		/* [unique][in][out] */ VARIANT *pVal,
		/* [unique][in][out] */ long *pType,
		/* [unique][in][out] */ long *plFlavor);

	HRESULT(STDMETHODCALLTYPE *Put)(
		idIWbemClassObject * This,
		/* [string][in] */ LPCWSTR wszName,
		/* [in] */ long lFlags,
		/* [in] */ VARIANT *pVal,
		/* [in] */ long Type);

	HRESULT(STDMETHODCALLTYPE *Delete)(
		idIWbemClassObject * This,
		/* [string][in] */ LPCWSTR wszName);

	HRESULT(STDMETHODCALLTYPE *GetNames)(
		idIWbemClassObject * This,
		/* [string][in] */ LPCWSTR wszQualifierName,
		/* [in] */ long lFlags,
		/* [in] */ VARIANT *pQualifierVal,
		/* [out] */ SAFEARRAY * *pNames);

	HRESULT(STDMETHODCALLTYPE *BeginEnumeration)(
		idIWbemClassObject * This,
		/* [in] */ long lEnumFlags);

	HRESULT(STDMETHODCALLTYPE *Next)(
		idIWbemClassObject * This,
		/* [in] */ long lFlags,
		/* [unique][in][out] */ BSTR *strName,
		/* [unique][in][out] */ VARIANT *pVal,
		/* [unique][in][out] */ long *pType,
		/* [unique][in][out] */ long *plFlavor);

	HRESULT(STDMETHODCALLTYPE *EndEnumeration)(
		idIWbemClassObject * This);

	HRESULT(STDMETHODCALLTYPE *GetPropertyQualifierSet)(
		idIWbemClassObject * This,
		/* [string][in] */ LPCWSTR wszProperty,
		/* [out] */ idIWbemQualifierSet **ppQualSet);

	HRESULT(STDMETHODCALLTYPE *Clone)(
		idIWbemClassObject * This,
		/* [out] */ idIWbemClassObject **ppCopy);

	HRESULT(STDMETHODCALLTYPE *GetObjectText)(
		idIWbemClassObject * This,
		/* [in] */ long lFlags,
		/* [out] */ BSTR *pstrObjectText);

	HRESULT(STDMETHODCALLTYPE *SpawnDerivedClass)(
		idIWbemClassObject * This,
		/* [in] */ long lFlags,
		/* [out] */ idIWbemClassObject **ppNewClass);

	HRESULT(STDMETHODCALLTYPE *SpawnInstance)(
		idIWbemClassObject * This,
		/* [in] */ long lFlags,
		/* [out] */ idIWbemClassObject **ppNewInstance);

	HRESULT(STDMETHODCALLTYPE *CompareTo)(
		idIWbemClassObject * This,
		/* [in] */ long lFlags,
		/* [in] */ idIWbemClassObject *pCompareTo);

	HRESULT(STDMETHODCALLTYPE *GetPropertyOrigin)(
		idIWbemClassObject * This,
		/* [string][in] */ LPCWSTR wszName,
		/* [out] */ BSTR *pstrClassName);

	HRESULT(STDMETHODCALLTYPE *InheritsFrom)(
		idIWbemClassObject * This,
		/* [in] */ LPCWSTR strAncestor);

	HRESULT(STDMETHODCALLTYPE *GetMethod)(
		idIWbemClassObject * This,
		/* [string][in] */ LPCWSTR wszName,
		/* [in] */ long lFlags,
		/* [out] */ idIWbemClassObject **ppInSignature,
		/* [out] */ idIWbemClassObject **ppOutSignature);

	HRESULT(STDMETHODCALLTYPE *PutMethod)(
		idIWbemClassObject * This,
		/* [string][in] */ LPCWSTR wszName,
		/* [in] */ long lFlags,
		/* [in] */ idIWbemClassObject *pInSignature,
		/* [in] */ idIWbemClassObject *pOutSignature);

	HRESULT(STDMETHODCALLTYPE *DeleteMethod)(
		idIWbemClassObject * This,
		/* [string][in] */ LPCWSTR wszName);

	HRESULT(STDMETHODCALLTYPE *BeginMethodEnumeration)(
		idIWbemClassObject * This,
		/* [in] */ long lEnumFlags);

	HRESULT(STDMETHODCALLTYPE *NextMethod)(
		idIWbemClassObject * This,
		/* [in] */ long lFlags,
		/* [unique][in][out] */ BSTR *pstrName,
		/* [unique][in][out] */ idIWbemClassObject **ppInSignature,
		/* [unique][in][out] */ idIWbemClassObject **ppOutSignature);

	HRESULT(STDMETHODCALLTYPE *EndMethodEnumeration)(
		idIWbemClassObject * This);

	HRESULT(STDMETHODCALLTYPE *GetMethodQualifierSet)(
		idIWbemClassObject * This,
		/* [string][in] */ LPCWSTR wszMethod,
		/* [out] */ idIWbemQualifierSet **ppQualSet);

	HRESULT(STDMETHODCALLTYPE *GetMethodOrigin)(
		idIWbemClassObject * This,
		/* [string][in] */ LPCWSTR wszMethodName,
		/* [out] */ BSTR *pstrClassName);

	ULONG m_ref;
	MULTI_idQI m_realWbemClassObjectObj;
	COleObject m_oleObject;

	END_INTERFACE
} idIWbemClassObjectVtbl;

interface idIWbemClassObject
{
	CONST_VTBL struct idIWbemClassObjectVtbl *lpVtbl;
};

HRESULT STDMETHODCALLTYPE WbemClassObjectQueryInterface(
	idIWbemClassObject * This,
	/* [in] */ REFIID riid,
	/* [annotation][iid_is][out] */
	_COM_Outptr_  void **ppvObject);

ULONG STDMETHODCALLTYPE WbemClassObjectAddRef(
	idIWbemClassObject * This);

ULONG STDMETHODCALLTYPE WbemClassObjectRelease(
	idIWbemClassObject * This);

HRESULT STDMETHODCALLTYPE WbemClassObjectGetQualifierSet(
	idIWbemClassObject * This,
	/* [out] */ idIWbemQualifierSet **ppQualSet);

HRESULT STDMETHODCALLTYPE WbemClassObjectGet(
	idIWbemClassObject * This,
	/* [string][in] */ LPCWSTR wszName,
	/* [in] */ long lFlags,
	/* [unique][in][out] */ VARIANT *pVal,
	/* [unique][in][out] */ long *pType,
	/* [unique][in][out] */ long *plFlavor);

HRESULT STDMETHODCALLTYPE WbemClassObjectPut(
	idIWbemClassObject * This,
	/* [string][in] */ LPCWSTR wszName,
	/* [in] */ long lFlags,
	/* [in] */ VARIANT *pVal,
	/* [in] */ long Type);

HRESULT STDMETHODCALLTYPE WbemClassObjectDelete(
	idIWbemClassObject * This,
	/* [string][in] */ LPCWSTR wszName);

HRESULT STDMETHODCALLTYPE WbemClassObjectGetNames(
	idIWbemClassObject * This,
	/* [string][in] */ LPCWSTR wszQualifierName,
	/* [in] */ long lFlags,
	/* [in] */ VARIANT *pQualifierVal,
	/* [out] */ SAFEARRAY * *pNames);

HRESULT STDMETHODCALLTYPE WbemClassObjectBeginEnumeration(
	idIWbemClassObject * This,
	/* [in] */ long lEnumFlags);

HRESULT STDMETHODCALLTYPE WbemClassObjectNext(
	idIWbemClassObject * This,
	/* [in] */ long lFlags,
	/* [unique][in][out] */ BSTR *strName,
	/* [unique][in][out] */ VARIANT *pVal,
	/* [unique][in][out] */ long *pType,
	/* [unique][in][out] */ long *plFlavor);

HRESULT STDMETHODCALLTYPE WbemClassObjectEndEnumeration(
	idIWbemClassObject * This);

HRESULT STDMETHODCALLTYPE WbemClassObjectGetPropertyQualifierSet(
	idIWbemClassObject * This,
	/* [string][in] */ LPCWSTR wszProperty,
	/* [out] */ idIWbemQualifierSet **ppQualSet);

HRESULT STDMETHODCALLTYPE WbemClassObjectClone(
	idIWbemClassObject * This,
	/* [out] */ idIWbemClassObject **ppCopy);

HRESULT STDMETHODCALLTYPE WbemClassObjectGetObjectText(
	idIWbemClassObject * This,
	/* [in] */ long lFlags,
	/* [out] */ BSTR *pstrObjectText);

HRESULT STDMETHODCALLTYPE WbemClassObjectSpawnDerivedClass(
	idIWbemClassObject * This,
	/* [in] */ long lFlags,
	/* [out] */ idIWbemClassObject **ppNewClass);

HRESULT STDMETHODCALLTYPE WbemClassObjectSpawnInstance(
	idIWbemClassObject * This,
	/* [in] */ long lFlags,
	/* [out] */ idIWbemClassObject **ppNewInstance);

HRESULT STDMETHODCALLTYPE WbemClassObjectCompareTo(
	idIWbemClassObject * This,
	/* [in] */ long lFlags,
	/* [in] */ idIWbemClassObject *pCompareTo);

HRESULT STDMETHODCALLTYPE WbemClassObjectGetPropertyOrigin(
	idIWbemClassObject * This,
	/* [string][in] */ LPCWSTR wszName,
	/* [out] */ BSTR *pstrClassName);

HRESULT STDMETHODCALLTYPE WbemClassObjectInheritsFrom(
	idIWbemClassObject * This,
	/* [in] */ LPCWSTR strAncestor);

HRESULT STDMETHODCALLTYPE WbemClassObjectGetMethod(
	idIWbemClassObject * This,
	/* [string][in] */ LPCWSTR wszName,
	/* [in] */ long lFlags,
	/* [out] */ idIWbemClassObject **ppInSignature,
	/* [out] */ idIWbemClassObject **ppOutSignature);

HRESULT STDMETHODCALLTYPE WbemClassObjectPutMethod(
	idIWbemClassObject * This,
	/* [string][in] */ LPCWSTR wszName,
	/* [in] */ long lFlags,
	/* [in] */ idIWbemClassObject *pInSignature,
	/* [in] */ idIWbemClassObject *pOutSignature);

HRESULT STDMETHODCALLTYPE WbemClassObjectDeleteMethod(
	idIWbemClassObject * This,
	/* [string][in] */ LPCWSTR wszName);

HRESULT STDMETHODCALLTYPE WbemClassObjectBeginMethodEnumeration(
	idIWbemClassObject * This,
	/* [in] */ long lEnumFlags);

HRESULT STDMETHODCALLTYPE WbemClassObjectNextMethod(
	idIWbemClassObject * This,
	/* [in] */ long lFlags,
	/* [unique][in][out] */ BSTR *pstrName,
	/* [unique][in][out] */ idIWbemClassObject **ppInSignature,
	/* [unique][in][out] */ idIWbemClassObject **ppOutSignature);

HRESULT STDMETHODCALLTYPE WbemClassObjectEndMethodEnumeration(
	idIWbemClassObject * This);

HRESULT STDMETHODCALLTYPE WbemClassObjectGetMethodQualifierSet(
	idIWbemClassObject * This,
	/* [string][in] */ LPCWSTR wszMethod,
	/* [out] */ idIWbemQualifierSet **ppQualSet);

HRESULT STDMETHODCALLTYPE WbemClassObjectGetMethodOrigin(
	idIWbemClassObject * This,
	/* [string][in] */ LPCWSTR wszMethodName,
	/* [out] */ BSTR *pstrClassName);

void InitializeWin32WbemClassObject(idIWbemClassObject* p_IWbemClassObject, COleObject& OleObject);

