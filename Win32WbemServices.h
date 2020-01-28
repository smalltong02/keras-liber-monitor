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
#include "Win32WbemPath.h"
#include "Win32WbemQualifierSet.h"

typedef struct idIWbemServicesVtbl
{
	BEGIN_INTERFACE

		HRESULT(STDMETHODCALLTYPE *QueryInterface)(
			idIWbemServices * This,
			/* [in] */ REFIID riid,
			/* [annotation][iid_is][out] */
			_COM_Outptr_  void **ppvObject);

	ULONG(STDMETHODCALLTYPE *AddRef)(
		idIWbemServices * This);

	ULONG(STDMETHODCALLTYPE *Release)(
		idIWbemServices * This);

	HRESULT(STDMETHODCALLTYPE *OpenNamespace)(
		idIWbemServices * This,
		/* [in] */ const BSTR strNamespace,
		/* [in] */ long lFlags,
		/* [in] */ idIWbemContext *pCtx,
		/* [unique][in][out] */ idIWbemServices **ppWorkingNamespace,
		/* [unique][in][out] */ idIWbemCallResult **ppResult);

	HRESULT(STDMETHODCALLTYPE *CancelAsyncCall)(
		idIWbemServices * This,
		/* [in] */ idIWbemObjectSink *pSink);

	HRESULT(STDMETHODCALLTYPE *QueryObjectSink)(
		idIWbemServices * This,
		/* [in] */ long lFlags,
		/* [out] */ idIWbemObjectSink **ppResponseHandler);

	HRESULT(STDMETHODCALLTYPE *GetObject)(
		idIWbemServices * This,
		/* [in] */ const BSTR strObjectPath,
		/* [in] */ long lFlags,
		/* [in] */ idIWbemContext *pCtx,
		/* [unique][in][out] */ idIWbemClassObject **ppObject,
		/* [unique][in][out] */ idIWbemCallResult **ppCallResult);

	HRESULT(STDMETHODCALLTYPE *GetObjectAsync)(
		idIWbemServices * This,
		/* [in] */ const BSTR strObjectPath,
		/* [in] */ long lFlags,
		/* [in] */ idIWbemContext *pCtx,
		/* [in] */ idIWbemObjectSink *pResponseHandler);

	HRESULT(STDMETHODCALLTYPE *PutClass)(
		idIWbemServices * This,
		/* [in] */ idIWbemClassObject *pObject,
		/* [in] */ long lFlags,
		/* [in] */ idIWbemContext *pCtx,
		/* [unique][in][out] */ idIWbemCallResult **ppCallResult);

	HRESULT(STDMETHODCALLTYPE *PutClassAsync)(
		idIWbemServices * This,
		/* [in] */ idIWbemClassObject *pObject,
		/* [in] */ long lFlags,
		/* [in] */ idIWbemContext *pCtx,
		/* [in] */ idIWbemObjectSink *pResponseHandler);

	HRESULT(STDMETHODCALLTYPE *DeleteClass)(
		idIWbemServices * This,
		/* [in] */ const BSTR strClass,
		/* [in] */ long lFlags,
		/* [in] */ idIWbemContext *pCtx,
		/* [unique][in][out] */ idIWbemCallResult **ppCallResult);

	HRESULT(STDMETHODCALLTYPE *DeleteClassAsync)(
		idIWbemServices * This,
		/* [in] */ const BSTR strClass,
		/* [in] */ long lFlags,
		/* [in] */ idIWbemContext *pCtx,
		/* [in] */ idIWbemObjectSink *pResponseHandler);

	HRESULT(STDMETHODCALLTYPE *CreateClassEnum)(
		idIWbemServices * This,
		/* [in] */ const BSTR strSuperclass,
		/* [in] */ long lFlags,
		/* [in] */ idIWbemContext *pCtx,
		/* [out] */ idIEnumWbemClassObject **ppEnum);

	HRESULT(STDMETHODCALLTYPE *CreateClassEnumAsync)(
		idIWbemServices * This,
		/* [in] */ const BSTR strSuperclass,
		/* [in] */ long lFlags,
		/* [in] */ idIWbemContext *pCtx,
		/* [in] */ idIWbemObjectSink *pResponseHandler);

	HRESULT(STDMETHODCALLTYPE *PutInstance)(
		idIWbemServices * This,
		/* [in] */ idIWbemClassObject *pInst,
		/* [in] */ long lFlags,
		/* [in] */ idIWbemContext *pCtx,
		/* [unique][in][out] */ idIWbemCallResult **ppCallResult);

	HRESULT(STDMETHODCALLTYPE *PutInstanceAsync)(
		idIWbemServices * This,
		/* [in] */ idIWbemClassObject *pInst,
		/* [in] */ long lFlags,
		/* [in] */ idIWbemContext *pCtx,
		/* [in] */ idIWbemObjectSink *pResponseHandler);

	HRESULT(STDMETHODCALLTYPE *DeleteInstance)(
		idIWbemServices * This,
		/* [in] */ const BSTR strObjectPath,
		/* [in] */ long lFlags,
		/* [in] */ idIWbemContext *pCtx,
		/* [unique][in][out] */ idIWbemCallResult **ppCallResult);

	HRESULT(STDMETHODCALLTYPE *DeleteInstanceAsync)(
		idIWbemServices * This,
		/* [in] */ const BSTR strObjectPath,
		/* [in] */ long lFlags,
		/* [in] */ idIWbemContext *pCtx,
		/* [in] */ idIWbemObjectSink *pResponseHandler);

	HRESULT(STDMETHODCALLTYPE *CreateInstanceEnum)(
		idIWbemServices * This,
		/* [in] */ const BSTR strFilter,
		/* [in] */ long lFlags,
		/* [in] */ idIWbemContext *pCtx,
		/* [out] */ idIEnumWbemClassObject **ppEnum);

	HRESULT(STDMETHODCALLTYPE *CreateInstanceEnumAsync)(
		idIWbemServices * This,
		/* [in] */ const BSTR strFilter,
		/* [in] */ long lFlags,
		/* [in] */ idIWbemContext *pCtx,
		/* [in] */ idIWbemObjectSink *pResponseHandler);

	HRESULT(STDMETHODCALLTYPE *ExecQuery)(
		idIWbemServices * This,
		/* [in] */ const BSTR strQueryLanguage,
		/* [in] */ const BSTR strQuery,
		/* [in] */ long lFlags,
		/* [in] */ idIWbemContext *pCtx,
		/* [out] */ idIEnumWbemClassObject **ppEnum);

	HRESULT(STDMETHODCALLTYPE *ExecQueryAsync)(
		idIWbemServices * This,
		/* [in] */ const BSTR strQueryLanguage,
		/* [in] */ const BSTR strQuery,
		/* [in] */ long lFlags,
		/* [in] */ idIWbemContext *pCtx,
		/* [in] */ idIWbemObjectSink *pResponseHandler);

	HRESULT(STDMETHODCALLTYPE *ExecNotificationQuery)(
		idIWbemServices * This,
		/* [in] */ const BSTR strQueryLanguage,
		/* [in] */ const BSTR strQuery,
		/* [in] */ long lFlags,
		/* [in] */ idIWbemContext *pCtx,
		/* [out] */ idIEnumWbemClassObject **ppEnum);

	HRESULT(STDMETHODCALLTYPE *ExecNotificationQueryAsync)(
		idIWbemServices * This,
		/* [in] */ const BSTR strQueryLanguage,
		/* [in] */ const BSTR strQuery,
		/* [in] */ long lFlags,
		/* [in] */ idIWbemContext *pCtx,
		/* [in] */ idIWbemObjectSink *pResponseHandler);

	HRESULT(STDMETHODCALLTYPE *ExecMethod)(
		idIWbemServices * This,
		/* [in] */ const BSTR strObjectPath,
		/* [in] */ const BSTR strMethodName,
		/* [in] */ long lFlags,
		/* [in] */ idIWbemContext *pCtx,
		/* [in] */ idIWbemClassObject *pInParams,
		/* [unique][in][out] */ idIWbemClassObject **ppOutParams,
		/* [unique][in][out] */ idIWbemCallResult **ppCallResult);

	HRESULT(STDMETHODCALLTYPE *ExecMethodAsync)(
		idIWbemServices * This,
		/* [in] */ const BSTR strObjectPath,
		/* [in] */ const BSTR strMethodName,
		/* [in] */ long lFlags,
		/* [in] */ idIWbemContext *pCtx,
		/* [in] */ idIWbemClassObject *pInParams,
		/* [in] */ idIWbemObjectSink *pResponseHandler);

	ULONG m_ref;
	MULTI_idQI m_realWbemServicesObj;

	END_INTERFACE
} idIWbemServicesVtbl;

interface idIWbemServices
{
	CONST_VTBL struct idIWbemServicesVtbl *lpVtbl;
};

HRESULT STDMETHODCALLTYPE WbemServicesQueryInterface(
	idIWbemServices * This,
	/* [in] */ REFIID riid,
	/* [annotation][iid_is][out] */
	_COM_Outptr_  void **ppvObject);

ULONG STDMETHODCALLTYPE WbemServicesAddRef(
	idIWbemServices * This);

ULONG STDMETHODCALLTYPE WbemServicesRelease(
	idIWbemServices * This);

HRESULT STDMETHODCALLTYPE WbemServicesOpenNamespace(
	idIWbemServices * This,
	/* [in] */ const BSTR strNamespace,
	/* [in] */ long lFlags,
	/* [in] */ idIWbemContext *pCtx,
	/* [unique][in][out] */ idIWbemServices **ppWorkingNamespace,
	/* [unique][in][out] */ idIWbemCallResult **ppResult);

HRESULT STDMETHODCALLTYPE WbemServicesCancelAsyncCall(
	idIWbemServices * This,
	/* [in] */ idIWbemObjectSink *pSink);

HRESULT STDMETHODCALLTYPE WbemServicesQueryObjectSink(
	idIWbemServices * This,
	/* [in] */ long lFlags,
	/* [out] */ idIWbemObjectSink **WbemServicesppResponseHandler);

HRESULT STDMETHODCALLTYPE WbemServicesGetObject(
	idIWbemServices * This,
	/* [in] */ const BSTR strObjectPath,
	/* [in] */ long lFlags,
	/* [in] */ idIWbemContext *pCtx,
	/* [unique][in][out] */ idIWbemClassObject **ppObject,
	/* [unique][in][out] */ idIWbemCallResult **ppCallResult);

HRESULT STDMETHODCALLTYPE WbemServicesGetObjectAsync(
	idIWbemServices * This,
	/* [in] */ const BSTR strObjectPath,
	/* [in] */ long lFlags,
	/* [in] */ idIWbemContext *pCtx,
	/* [in] */ idIWbemObjectSink *pResponseHandler);

HRESULT STDMETHODCALLTYPE WbemServicesPutClass(
	idIWbemServices * This,
	/* [in] */ idIWbemClassObject *pObject,
	/* [in] */ long lFlags,
	/* [in] */ idIWbemContext *pCtx,
	/* [unique][in][out] */ idIWbemCallResult **ppCallResult);

HRESULT STDMETHODCALLTYPE WbemServicesPutClassAsync(
	idIWbemServices * This,
	/* [in] */ idIWbemClassObject *pObject,
	/* [in] */ long lFlags,
	/* [in] */ idIWbemContext *pCtx,
	/* [in] */ idIWbemObjectSink *pResponseHandler);

HRESULT STDMETHODCALLTYPE WbemServicesDeleteClass(
	idIWbemServices * This,
	/* [in] */ const BSTR strClass,
	/* [in] */ long lFlags,
	/* [in] */ idIWbemContext *pCtx,
	/* [unique][in][out] */ idIWbemCallResult **ppCallResult);

HRESULT STDMETHODCALLTYPE WbemServicesDeleteClassAsync(
	idIWbemServices * This,
	/* [in] */ const BSTR strClass,
	/* [in] */ long lFlags,
	/* [in] */ idIWbemContext *pCtx,
	/* [in] */ idIWbemObjectSink *pResponseHandler);

HRESULT STDMETHODCALLTYPE WbemServicesCreateClassEnum(
	idIWbemServices * This,
	/* [in] */ const BSTR strSuperclass,
	/* [in] */ long lFlags,
	/* [in] */ idIWbemContext *pCtx,
	/* [out] */ idIEnumWbemClassObject **ppEnum);

HRESULT STDMETHODCALLTYPE WbemServicesCreateClassEnumAsync(
	idIWbemServices * This,
	/* [in] */ const BSTR strSuperclass,
	/* [in] */ long lFlags,
	/* [in] */ idIWbemContext *pCtx,
	/* [in] */ idIWbemObjectSink *pResponseHandler);

HRESULT STDMETHODCALLTYPE WbemServicesPutInstance(
	idIWbemServices * This,
	/* [in] */ idIWbemClassObject *pInst,
	/* [in] */ long lFlags,
	/* [in] */ idIWbemContext *pCtx,
	/* [unique][in][out] */ idIWbemCallResult **ppCallResult);

HRESULT STDMETHODCALLTYPE WbemServicesPutInstanceAsync(
	idIWbemServices * This,
	/* [in] */ idIWbemClassObject *pInst,
	/* [in] */ long lFlags,
	/* [in] */ idIWbemContext *pCtx,
	/* [in] */ idIWbemObjectSink *pResponseHandler);

HRESULT STDMETHODCALLTYPE WbemServicesDeleteInstance(
	idIWbemServices * This,
	/* [in] */ const BSTR strObjectPath,
	/* [in] */ long lFlags,
	/* [in] */ idIWbemContext *pCtx,
	/* [unique][in][out] */ idIWbemCallResult **ppCallResult);

HRESULT STDMETHODCALLTYPE WbemServicesDeleteInstanceAsync(
	idIWbemServices * This,
	/* [in] */ const BSTR strObjectPath,
	/* [in] */ long lFlags,
	/* [in] */ idIWbemContext *pCtx,
	/* [in] */ idIWbemObjectSink *pResponseHandler);

HRESULT STDMETHODCALLTYPE WbemServicesCreateInstanceEnum(
	idIWbemServices * This,
	/* [in] */ const BSTR strFilter,
	/* [in] */ long lFlags,
	/* [in] */ idIWbemContext *pCtx,
	/* [out] */ idIEnumWbemClassObject **ppEnum);

HRESULT STDMETHODCALLTYPE WbemServicesCreateInstanceEnumAsync(
	idIWbemServices * This,
	/* [in] */ const BSTR strFilter,
	/* [in] */ long lFlags,
	/* [in] */ idIWbemContext *pCtx,
	/* [in] */ idIWbemObjectSink *pResponseHandler);

HRESULT STDMETHODCALLTYPE WbemServicesExecQuery(
	idIWbemServices * This,
	/* [in] */ const BSTR strQueryLanguage,
	/* [in] */ const BSTR strQuery,
	/* [in] */ long lFlags,
	/* [in] */ idIWbemContext *pCtx,
	/* [out] */ idIEnumWbemClassObject **ppEnum);

HRESULT STDMETHODCALLTYPE WbemServicesExecQueryAsync(
	idIWbemServices * This,
	/* [in] */ const BSTR strQueryLanguage,
	/* [in] */ const BSTR strQuery,
	/* [in] */ long lFlags,
	/* [in] */ idIWbemContext *pCtx,
	/* [in] */ idIWbemObjectSink *pResponseHandler);

HRESULT STDMETHODCALLTYPE WbemServicesExecNotificationQuery(
	idIWbemServices * This,
	/* [in] */ const BSTR strQueryLanguage,
	/* [in] */ const BSTR strQuery,
	/* [in] */ long lFlags,
	/* [in] */ idIWbemContext *pCtx,
	/* [out] */ idIEnumWbemClassObject **ppEnum);

HRESULT STDMETHODCALLTYPE WbemServicesExecNotificationQueryAsync(
	idIWbemServices * This,
	/* [in] */ const BSTR strQueryLanguage,
	/* [in] */ const BSTR strQuery,
	/* [in] */ long lFlags,
	/* [in] */ idIWbemContext *pCtx,
	/* [in] */ idIWbemObjectSink *pResponseHandler);

HRESULT STDMETHODCALLTYPE WbemServicesExecMethod(
	idIWbemServices * This,
	/* [in] */ const BSTR strObjectPath,
	/* [in] */ const BSTR strMethodName,
	/* [in] */ long lFlags,
	/* [in] */ idIWbemContext *pCtx,
	/* [in] */ idIWbemClassObject *pInParams,
	/* [unique][in][out] */ idIWbemClassObject **ppOutParams,
	/* [unique][in][out] */ idIWbemCallResult **ppCallResult);

HRESULT STDMETHODCALLTYPE WbemServicesExecMethodAsync(
	idIWbemServices * This,
	/* [in] */ const BSTR strObjectPath,
	/* [in] */ const BSTR strMethodName,
	/* [in] */ long lFlags,
	/* [in] */ idIWbemContext *pCtx,
	/* [in] */ idIWbemClassObject *pInParams,
	/* [in] */ idIWbemObjectSink *pResponseHandler);



