#include "stdafx.h"
#include "Win32WbemServices.h"

GUID IID_idIWbemServices = {
	0x9556dc99,
	0x828c,
	0x11cf,
	0xa3,0x7e,0x00,0xaa,0x00,0x32,0x40,0xc7
};

extern COleConfig* g_oleConfig;

HRESULT STDMETHODCALLTYPE WbemServicesQueryInterface(
	idIWbemServices * This,
	/* [in] */ REFIID riid,
	/* [annotation][iid_is][out] */
	_COM_Outptr_  void **ppvObject)
{
	HRESULT hr = S_OK;

	hr = ((IWbemServices*)(This->lpVtbl->m_realWbemServicesObj.pItf))->QueryInterface(riid, ppvObject);
	if (hr == S_OK)
	{
		if (*ppvObject == This->lpVtbl->m_realWbemServicesObj.pItf)
			*ppvObject = This;
		else if (IsEqualIIID(&IID_idIUnknown, &riid))
			CloneComObject(&IID_idIWbemServices, &This->lpVtbl->m_realWbemServicesObj.queryIID, ppvObject);
		else
			CloneComObject((GUID*)&riid, &This->lpVtbl->m_realWbemServicesObj.queryIID, ppvObject);
	}
	return hr;
}

ULONG STDMETHODCALLTYPE WbemServicesAddRef(
	idIWbemServices * This)
{
	ULONG Ref = ((IWbemServices*)(This->lpVtbl->m_realWbemServicesObj.pItf))->AddRef();
	This->lpVtbl->m_ref = Ref;
	return Ref;
}

ULONG STDMETHODCALLTYPE WbemServicesRelease(
	idIWbemServices * This)
{
	ULONG Ref = ((IWbemServices*)(This->lpVtbl->m_realWbemServicesObj.pItf))->Release();
	This->lpVtbl->m_ref = Ref;
	if (!Ref)
	{
		free(This->lpVtbl);
		free(This);
	}
	return Ref;
}

HRESULT STDMETHODCALLTYPE WbemServicesOpenNamespace(
	idIWbemServices * This,
	/* [in] */ const BSTR strNamespace,
	/* [in] */ long lFlags,
	/* [in] */ idIWbemContext *pCtx,
	/* [unique][in][out] */ idIWbemServices **ppWorkingNamespace,
	/* [unique][in][out] */ idIWbemCallResult **ppResult)
{
	HRESULT hr = S_OK;
	hr = ((IWbemServices*)(This->lpVtbl->m_realWbemServicesObj.pItf))->OpenNamespace(strNamespace, lFlags, (IWbemContext *)pCtx, (IWbemServices **)ppWorkingNamespace, (IWbemCallResult **)ppResult);
	if (hr == S_OK)
	{
		CloneComObject(&IID_idIWbemServices, &This->lpVtbl->m_realWbemServicesObj.queryIID, (void**)ppWorkingNamespace);
		CloneComObject(&IID_idIWbemCallResult, &This->lpVtbl->m_realWbemServicesObj.queryIID, (void**)ppResult);
	}
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemServicesCancelAsyncCall(
	idIWbemServices * This,
	/* [in] */ idIWbemObjectSink *pSink)
{
	HRESULT hr = S_OK;
	hr = ((IWbemServices*)(This->lpVtbl->m_realWbemServicesObj.pItf))->CancelAsyncCall((IWbemObjectSink *)pSink);
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemServicesQueryObjectSink(
	idIWbemServices * This,
	/* [in] */ long lFlags,
	/* [out] */ idIWbemObjectSink **WbemServicesppResponseHandler)
{
	HRESULT hr = S_OK;
	hr = ((IWbemServices*)(This->lpVtbl->m_realWbemServicesObj.pItf))->QueryObjectSink(lFlags, (IWbemObjectSink **)WbemServicesppResponseHandler);
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemServicesGetObject(
	idIWbemServices * This,
	/* [in] */ const BSTR strObjectPath,
	/* [in] */ long lFlags,
	/* [in] */ idIWbemContext *pCtx,
	/* [unique][in][out] */ idIWbemClassObject **ppObject,
	/* [unique][in][out] */ idIWbemCallResult **ppCallResult)
{
	HRESULT hr = S_OK;
	hr = ((IWbemServices*)(This->lpVtbl->m_realWbemServicesObj.pItf))->GetObject(strObjectPath, lFlags, (IWbemContext *)pCtx, (IWbemClassObject **)ppObject, (IWbemCallResult **)ppCallResult);
	if (hr == S_OK)
	{
		GUID* pQueryIID = GetQueryIID(strObjectPath);
		if (IsValidGUID(*pQueryIID))
		{
			This->lpVtbl->m_realWbemServicesObj.queryIID = *pQueryIID;
			CloneComObject(&IID_idIWbemClassObject, pQueryIID, (void**)ppObject);
			CloneComObject(&IID_idIWbemCallResult, pQueryIID, (void**)ppCallResult);
		}
	}
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemServicesGetObjectAsync(
	idIWbemServices * This,
	/* [in] */ const BSTR strObjectPath,
	/* [in] */ long lFlags,
	/* [in] */ idIWbemContext *pCtx,
	/* [in] */ idIWbemObjectSink *pResponseHandler)
{
	HRESULT hr = S_OK;
	hr = ((IWbemServices*)(This->lpVtbl->m_realWbemServicesObj.pItf))->GetObjectAsync(strObjectPath, lFlags, (IWbemContext *)pCtx, (IWbemObjectSink *)pResponseHandler);
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemServicesPutClass(
	idIWbemServices * This,
	/* [in] */ idIWbemClassObject *pObject,
	/* [in] */ long lFlags,
	/* [in] */ idIWbemContext *pCtx,
	/* [unique][in][out] */ idIWbemCallResult **ppCallResult)
{
	HRESULT hr = S_OK;
	hr = ((IWbemServices*)(This->lpVtbl->m_realWbemServicesObj.pItf))->PutClass((IWbemClassObject *)pObject, lFlags, (IWbemContext *)pCtx, (IWbemCallResult **)ppCallResult);
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemServicesPutClassAsync(
	idIWbemServices * This,
	/* [in] */ idIWbemClassObject *pObject,
	/* [in] */ long lFlags,
	/* [in] */ idIWbemContext *pCtx,
	/* [in] */ idIWbemObjectSink *pResponseHandler)
{
	HRESULT hr = S_OK;
	hr = ((IWbemServices*)(This->lpVtbl->m_realWbemServicesObj.pItf))->PutClassAsync((IWbemClassObject *)pObject, lFlags, (IWbemContext *)pCtx, (IWbemObjectSink *)pResponseHandler);
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemServicesDeleteClass(
	idIWbemServices * This,
	/* [in] */ const BSTR strClass,
	/* [in] */ long lFlags,
	/* [in] */ idIWbemContext *pCtx,
	/* [unique][in][out] */ idIWbemCallResult **ppCallResult)
{
	HRESULT hr = S_OK;
	hr = ((IWbemServices*)(This->lpVtbl->m_realWbemServicesObj.pItf))->DeleteClass(strClass, lFlags, (IWbemContext *)pCtx, (IWbemCallResult **)ppCallResult);
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemServicesDeleteClassAsync(
	idIWbemServices * This,
	/* [in] */ const BSTR strClass,
	/* [in] */ long lFlags,
	/* [in] */ idIWbemContext *pCtx,
	/* [in] */ idIWbemObjectSink *pResponseHandler)
{
	HRESULT hr = S_OK;
	hr = ((IWbemServices*)(This->lpVtbl->m_realWbemServicesObj.pItf))->DeleteClassAsync(strClass, lFlags, (IWbemContext *)pCtx, (IWbemObjectSink *)pResponseHandler);
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemServicesCreateClassEnum(
	idIWbemServices * This,
	/* [in] */ const BSTR strSuperclass,
	/* [in] */ long lFlags,
	/* [in] */ idIWbemContext *pCtx,
	/* [out] */ idIEnumWbemClassObject **ppEnum)
{
	HRESULT hr = S_OK;
	hr = ((IWbemServices*)(This->lpVtbl->m_realWbemServicesObj.pItf))->CreateClassEnum(strSuperclass, lFlags, (IWbemContext *)pCtx, (IEnumWbemClassObject **)ppEnum);
	if (hr == S_OK)
	{
		CloneComObject(&IID_idIEnumWbemClassObject, &This->lpVtbl->m_realWbemServicesObj.queryIID, (void**)ppEnum);
	}
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemServicesCreateClassEnumAsync(
	idIWbemServices * This,
	/* [in] */ const BSTR strSuperclass,
	/* [in] */ long lFlags,
	/* [in] */ idIWbemContext *pCtx,
	/* [in] */ idIWbemObjectSink *pResponseHandler)
{
	HRESULT hr = S_OK;
	hr = ((IWbemServices*)(This->lpVtbl->m_realWbemServicesObj.pItf))->CreateClassEnumAsync(strSuperclass, lFlags, (IWbemContext *)pCtx, (IWbemObjectSink *)pResponseHandler);
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemServicesPutInstance(
	idIWbemServices * This,
	/* [in] */ idIWbemClassObject *pInst,
	/* [in] */ long lFlags,
	/* [in] */ idIWbemContext *pCtx,
	/* [unique][in][out] */ idIWbemCallResult **ppCallResult)
{
	HRESULT hr = S_OK;
	hr = ((IWbemServices*)(This->lpVtbl->m_realWbemServicesObj.pItf))->PutInstance((IWbemClassObject *)pInst, lFlags, (IWbemContext *)pCtx, (IWbemCallResult **)ppCallResult);
	if (hr == S_OK)
	{
		CloneComObject(&IID_idIWbemCallResult, &This->lpVtbl->m_realWbemServicesObj.queryIID, (void**)ppCallResult);
	}
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemServicesPutInstanceAsync(
	idIWbemServices * This,
	/* [in] */ idIWbemClassObject *pInst,
	/* [in] */ long lFlags,
	/* [in] */ idIWbemContext *pCtx,
	/* [in] */ idIWbemObjectSink *pResponseHandler)
{
	HRESULT hr = S_OK;
	hr = ((IWbemServices*)(This->lpVtbl->m_realWbemServicesObj.pItf))->PutInstanceAsync((IWbemClassObject *)pInst, lFlags, (IWbemContext *)pCtx, (IWbemObjectSink *)pResponseHandler);
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemServicesDeleteInstance(
	idIWbemServices * This,
	/* [in] */ const BSTR strObjectPath,
	/* [in] */ long lFlags,
	/* [in] */ idIWbemContext *pCtx,
	/* [unique][in][out] */ idIWbemCallResult **ppCallResult)
{
	HRESULT hr = S_OK;
	hr = ((IWbemServices*)(This->lpVtbl->m_realWbemServicesObj.pItf))->DeleteInstance(strObjectPath, lFlags, (IWbemContext *)pCtx, (IWbemCallResult **)ppCallResult);
	if (hr == S_OK)
	{
		CloneComObject(&IID_idIWbemCallResult, &This->lpVtbl->m_realWbemServicesObj.queryIID, (void**)ppCallResult);
	}
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemServicesDeleteInstanceAsync(
	idIWbemServices * This,
	/* [in] */ const BSTR strObjectPath,
	/* [in] */ long lFlags,
	/* [in] */ idIWbemContext *pCtx,
	/* [in] */ idIWbemObjectSink *pResponseHandler)
{
	HRESULT hr = S_OK;
	hr = ((IWbemServices*)(This->lpVtbl->m_realWbemServicesObj.pItf))->DeleteInstanceAsync(strObjectPath, lFlags, (IWbemContext *)pCtx, (IWbemObjectSink *)pResponseHandler);
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemServicesCreateInstanceEnum(
	idIWbemServices * This,
	/* [in] */ const BSTR strFilter,
	/* [in] */ long lFlags,
	/* [in] */ idIWbemContext *pCtx,
	/* [out] */ idIEnumWbemClassObject **ppEnum)
{
	HRESULT hr = S_OK;
	hr = ((IWbemServices*)(This->lpVtbl->m_realWbemServicesObj.pItf))->CreateInstanceEnum(strFilter, lFlags, (IWbemContext *)pCtx, (IEnumWbemClassObject **)ppEnum);
	if (hr == S_OK)
	{
		CloneComObject(&IID_idIEnumWbemClassObject, &This->lpVtbl->m_realWbemServicesObj.queryIID, (void**)ppEnum);
	}
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemServicesCreateInstanceEnumAsync(
	idIWbemServices * This,
	/* [in] */ const BSTR strFilter,
	/* [in] */ long lFlags,
	/* [in] */ idIWbemContext *pCtx,
	/* [in] */ idIWbemObjectSink *pResponseHandler)
{
	HRESULT hr = S_OK;
	hr = ((IWbemServices*)(This->lpVtbl->m_realWbemServicesObj.pItf))->CreateInstanceEnumAsync(strFilter, lFlags, (IWbemContext *)pCtx, (IWbemObjectSink *)pResponseHandler);
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemServicesExecQuery(
	idIWbemServices * This,
	/* [in] */ const BSTR strQueryLanguage,
	/* [in] */ const BSTR strQuery,
	/* [in] */ long lFlags,
	/* [in] */ idIWbemContext *pCtx,
	/* [out] */ idIEnumWbemClassObject **ppEnum)
{
	HRESULT hr = S_OK;
	hr = ((IWbemServices*)(This->lpVtbl->m_realWbemServicesObj.pItf))->ExecQuery(strQueryLanguage, strQuery, lFlags, (IWbemContext *)pCtx, (IEnumWbemClassObject **)ppEnum);
	if (hr == S_OK)
	{
		if (wcsicmp(strQueryLanguage, L"WQL") == 0)
		{
			GUID* pQueryIID = GetQueryIID(strQuery);
			if (IsValidGUID(*pQueryIID))
			{
				This->lpVtbl->m_realWbemServicesObj.queryIID = *pQueryIID;
				CloneComObject(&IID_idIEnumWbemClassObject, pQueryIID, (void**)ppEnum);
			}
		}
	}
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemServicesExecQueryAsync(
	idIWbemServices * This,
	/* [in] */ const BSTR strQueryLanguage,
	/* [in] */ const BSTR strQuery,
	/* [in] */ long lFlags,
	/* [in] */ idIWbemContext *pCtx,
	/* [in] */ idIWbemObjectSink *pResponseHandler)
{
	HRESULT hr = S_OK;
	hr = ((IWbemServices*)(This->lpVtbl->m_realWbemServicesObj.pItf))->ExecQueryAsync(strQueryLanguage, strQuery, lFlags, (IWbemContext *)pCtx, (IWbemObjectSink *)pResponseHandler);
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemServicesExecNotificationQuery(
	idIWbemServices * This,
	/* [in] */ const BSTR strQueryLanguage,
	/* [in] */ const BSTR strQuery,
	/* [in] */ long lFlags,
	/* [in] */ idIWbemContext *pCtx,
	/* [out] */ idIEnumWbemClassObject **ppEnum)
{
	HRESULT hr = S_OK;
	hr = ((IWbemServices*)(This->lpVtbl->m_realWbemServicesObj.pItf))->ExecNotificationQuery(strQueryLanguage, strQuery, lFlags, (IWbemContext *)pCtx, (IEnumWbemClassObject **)ppEnum);
	if (hr == S_OK)
	{
		CloneComObject(&IID_idIEnumWbemClassObject, &This->lpVtbl->m_realWbemServicesObj.queryIID, (void**)ppEnum);
	}
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemServicesExecNotificationQueryAsync(
	idIWbemServices * This,
	/* [in] */ const BSTR strQueryLanguage,
	/* [in] */ const BSTR strQuery,
	/* [in] */ long lFlags,
	/* [in] */ idIWbemContext *pCtx,
	/* [in] */ idIWbemObjectSink *pResponseHandler)
{
	HRESULT hr = S_OK;
	hr = ((IWbemServices*)(This->lpVtbl->m_realWbemServicesObj.pItf))->ExecNotificationQueryAsync(strQueryLanguage, strQuery, lFlags, (IWbemContext *)pCtx, (IWbemObjectSink *)pResponseHandler);
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemServicesExecMethod(
	idIWbemServices * This,
	/* [in] */ const BSTR strObjectPath,
	/* [in] */ const BSTR strMethodName,
	/* [in] */ long lFlags,
	/* [in] */ idIWbemContext *pCtx,
	/* [in] */ idIWbemClassObject *pInParams,
	/* [unique][in][out] */ idIWbemClassObject **ppOutParams,
	/* [unique][in][out] */ idIWbemCallResult **ppCallResult)
{
	HRESULT hr = S_OK;
	int MethodOrd = COleObject::InvalidOrdinal;
	COleObject* _Object = nullptr;
	IWbemClassObject *pOrginInParams = nullptr;

	if (IsCloneComObject(pInParams->lpVtbl->m_realWbemClassObjectObj))
		pOrginInParams = (IWbemClassObject*)pInParams->lpVtbl->m_realWbemClassObjectObj.pItf;
	else
		pOrginInParams = (IWbemClassObject*)pInParams;

	//pre process
	if (strObjectPath && strMethodName)
	{
		int Ordinal = g_oleConfig->GetObjectOrdinal(std::wstring(strObjectPath));
		if (g_oleConfig->CheckOrdinal(Ordinal))
		{
			_Object = g_oleConfig->GetOleObject(Ordinal);
			if (_Object && (MethodOrd = _Object->GetMethodOrdinal(std::wstring(strMethodName))) != COleObject::InvalidOrdinal && pInParams)
			{
				std::vector<std::wstring> _ParamList;
				std::vector<PropertyHandle> _HandleList;
				_HandleList.push_back(HandleAddT);
				if (_Object->GetSameHandleParamNameList(MethodOrd, _HandleList, _ParamList) && _ParamList.size())
				{
					// process AddT params, send to tracer driver.
					VARIANT ParamValue;
					CIMTYPE Type;
					HRESULT hres = S_OK;
					for (int i = 0; i < _ParamList.size(); i++)
					{
						hres = pOrginInParams->Get(_ParamList[i].c_str(), 0, &ParamValue, &Type, NULL);
						if (hres == S_OK)
						{
							if (g_oleConfig->GetDriverMgr() != nullptr && ParamValue.vt == VT_BSTR)
							{
								//DWORD BytesReturned = 0;
								//BOOL  Result = FALSE;
								//Result = g_oleConfig->GetDriverMgr()->IoControl((DWORD)IOCTL_HIPS_SETTARGETCMD, ParamValue.bstrVal, (wcslen(ParamValue.bstrVal) + 1) * sizeof(wchar_t), NULL, 0, &BytesReturned);
								//if (Result == TRUE)
								//{
								//	DbgPrintLog(L"WbemServicesExecMethod: set target cmdline success!");
								//}
								//else
								//{
								//	DbgPrintLog(L"WbemServicesExecMethod: set target cmdline failed!");
								//}
							}
						}
					}
				}
			}
		}
	}
	
	hr = ((IWbemServices*)(This->lpVtbl->m_realWbemServicesObj.pItf))->ExecMethod(strObjectPath, strMethodName, lFlags, (IWbemContext *)pCtx, (IWbemClassObject *)pOrginInParams, (IWbemClassObject **)ppOutParams, (IWbemCallResult **)ppCallResult);
	if (hr == S_OK)
	{
		//post process
		if (_Object && MethodOrd != COleObject::InvalidOrdinal && pInParams)
		{
			std::vector<std::wstring> _ParamList;
			std::vector<PropertyHandle> _HandleList;
			_HandleList.push_back(HandleLog);
			_HandleList.push_back(HandleAddT);
			if (_Object->GetSameHandleParamNameList(MethodOrd, _HandleList, _ParamList))
			{
				// record param for log
				VARIANT ParamValue;
				CIMTYPE Type;
				HRESULT hres = S_OK;
				for (int i = 0; i < _ParamList.size(); i++)
				{
					hres = ((IWbemClassObject*)pInParams)->Get(_ParamList[i].c_str(), 0, &ParamValue, &Type, NULL);
					if (hres == S_OK)
					{
						WmiInterfaceQueryLog(GetClassIdFromString(_Object->GetObjectName()), _ParamList[i], ParamValue);
					}
				}
			}
		}

		CloneComObject(&IID_idIWbemClassObject, &This->lpVtbl->m_realWbemServicesObj.queryIID, (void**)ppOutParams);
		CloneComObject(&IID_idIWbemCallResult, &This->lpVtbl->m_realWbemServicesObj.queryIID, (void**)ppCallResult);
	}
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemServicesExecMethodAsync(
	idIWbemServices * This,
	/* [in] */ const BSTR strObjectPath,
	/* [in] */ const BSTR strMethodName,
	/* [in] */ long lFlags,
	/* [in] */ idIWbemContext *pCtx,
	/* [in] */ idIWbemClassObject *pInParams,
	/* [in] */ idIWbemObjectSink *pResponseHandler)
{
	HRESULT hr = S_OK;
	hr = ((IWbemServices*)(This->lpVtbl->m_realWbemServicesObj.pItf))->ExecMethodAsync(strObjectPath, strMethodName, lFlags, (IWbemContext *)pCtx, (IWbemClassObject *)pInParams, (IWbemObjectSink *)pResponseHandler);
	return hr;
}

