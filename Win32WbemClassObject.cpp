#include "stdafx.h"
#include "Win32WbemClassObject.h"
#include "OleConfigObject.h"

extern COleConfig* g_oleConfig;

GUID IID_idIWbemClassObject = {
	0xdc12a681,
	0x737f,
	0x11cf,
	0x88,0x4d,0x00,0xaa,0x00,0x4b,0x2e,0x24
};

HRESULT STDMETHODCALLTYPE WbemClassObjectQueryInterface(
	idIWbemClassObject * This,
	/* [in] */ REFIID riid,
	/* [annotation][iid_is][out] */
	_COM_Outptr_  void **ppvObject)
{
	HRESULT hr = S_OK;
	COleObject* _Object = &This->lpVtbl->m_oleObject;
	//pre process
	if (_Object->IsValidObject() && _Object->GetMemberHandle() == HandleDup)
	{
		if (IsEqualIIID(&IID_idIUnknown, &riid) ||
			IsEqualIIID(&IID_idIWbemClassObject, &riid))
		{
			*ppvObject = This;
			WbemClassObjectAddRef(This);
			return S_OK;
		}
		else if (IsEqualIIID(&IID_idIMarshal, &riid))
		{
			idWin32IMarshal* p_IWin32IMarshal = (idWin32IMarshal *)malloc(sizeof(idWin32IMarshal));
			if (p_IWin32IMarshal)
			{
				p_IWin32IMarshal->lpVtbl = (idWin32IMarshalVtbl *)malloc(sizeof(idWin32IMarshalVtbl));
				if (p_IWin32IMarshal->lpVtbl)
				{
					memset(p_IWin32IMarshal->lpVtbl, 0, sizeof(idWin32IMarshalVtbl));
					InitializeWin32IMarshal(p_IWin32IMarshal);
					*ppvObject = p_IWin32IMarshal;
					return S_OK;
				}
			}
		}
		return E_NOINTERFACE;
	}

	hr = ((IWbemClassObject*)(This->lpVtbl->m_realWbemClassObjectObj.pItf))->QueryInterface(riid, ppvObject);
	if (hr == S_OK)
	{
		//post process
		if (*ppvObject == This->lpVtbl->m_realWbemClassObjectObj.pItf)
			*ppvObject = This;
		else if (IsEqualIIID(&IID_idIUnknown, &riid))
			CloneComObject(&IID_idIWbemClassObject, &This->lpVtbl->m_realWbemClassObjectObj.queryIID, ppvObject);
		else
			CloneComObject((GUID*)&riid, &This->lpVtbl->m_realWbemClassObjectObj.queryIID, ppvObject);
	}
	return hr;
}

ULONG STDMETHODCALLTYPE WbemClassObjectAddRef(
	idIWbemClassObject * This)
{
	COleObject* _Object = &This->lpVtbl->m_oleObject;
	//pre process
	if (_Object->IsValidObject() && _Object->GetMemberHandle() == HandleDup)
	{
		ULONG Ref = ++This->lpVtbl->m_ref;
		return Ref;
	}

	ULONG Ref = ((IWbemClassObject*)(This->lpVtbl->m_realWbemClassObjectObj.pItf))->AddRef();
	This->lpVtbl->m_ref = Ref;
	return Ref;
}

ULONG STDMETHODCALLTYPE WbemClassObjectRelease(
	idIWbemClassObject * This)
{
	COleObject* _Object = &This->lpVtbl->m_oleObject;
	//pre process
	if (_Object->IsValidObject() && _Object->GetMemberHandle() == HandleDup)
	{
		ULONG Ref = --This->lpVtbl->m_ref;
		if (!Ref)
		{
			free(This->lpVtbl);
			free(This);
		}
		return Ref;
	}

	ULONG Ref = ((IWbemClassObject*)(This->lpVtbl->m_realWbemClassObjectObj.pItf))->Release();
	This->lpVtbl->m_ref = Ref;
	if (!Ref)
	{
		free(This->lpVtbl);
		free(This);
	}
	return Ref;
}

HRESULT STDMETHODCALLTYPE WbemClassObjectGetQualifierSet(
	idIWbemClassObject * This,
	/* [out] */ idIWbemQualifierSet **ppQualSet)
{
	HRESULT hr = S_OK;
	COleObject* _Object = &This->lpVtbl->m_oleObject;
	//pre process
	if (_Object->IsValidObject() && _Object->GetMemberHandle() == HandleDup)
	{
		return S_FALSE;
	}

	hr = ((IWbemClassObject*)(This->lpVtbl->m_realWbemClassObjectObj.pItf))->GetQualifierSet((IWbemQualifierSet**)ppQualSet);
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemClassObjectGet(
	idIWbemClassObject * This,
	/* [string][in] */ LPCWSTR wszName,
	/* [in] */ long lFlags,
	/* [unique][in][out] */ VARIANT *pVal,
	/* [unique][in][out] */ long *pType,
	/* [unique][in][out] */ long *plFlavor)
{
	HRESULT hr = S_OK;
	COleObject* _Object = &This->lpVtbl->m_oleObject;
	//pre process
	if (_Object->IsValidObject() && _Object->GetMemberHandle() == HandleDup)
	{
		hr = S_OK;
	}
	else
	{
		hr = ((IWbemClassObject*)(This->lpVtbl->m_realWbemClassObjectObj.pItf))->Get(wszName, lFlags, pVal, pType, plFlavor);
	}
	if (hr == S_OK)
	{
		//post process
		if (pVal && This->lpVtbl->m_oleObject.IsValidObject())
		{
			COleObject* _Object = &This->lpVtbl->m_oleObject;
			// modify element and return.
			PropertyHandle _Handle = _Object->GetPropertyHandle(std::wstring(wszName));
			if (_Handle == HandleModify)
			{
				VARIANT Value;
				if (_Object->GetPropertyValue(std::wstring(wszName), Value))
				{
					*pVal = Value;
					WmiInterfaceQueryLog(GetClassIdFromString(_Object->GetObjectName()), wszName, Value);
				}
			} // log element.
			else if (_Handle == HandleLog)
			{
				WmiInterfaceQueryLog(GetClassIdFromString(_Object->GetObjectName()), wszName, *pVal);
			}
		}
	}
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemClassObjectPut(
	idIWbemClassObject * This,
	/* [string][in] */ LPCWSTR wszName,
	/* [in] */ long lFlags,
	/* [in] */ VARIANT *pVal,
	/* [in] */ long Type)
{
	HRESULT hr = S_OK;
	COleObject* _Object = &This->lpVtbl->m_oleObject;
	//pre process
	if (_Object->IsValidObject() && _Object->GetMemberHandle() == HandleDup)
	{
		hr = S_OK;
	}
	else
	{
		hr = ((IWbemClassObject*)(This->lpVtbl->m_realWbemClassObjectObj.pItf))->Put(wszName, lFlags, pVal, Type);
	}
	if (hr == S_OK)
	{
		if (pVal && This->lpVtbl->m_oleObject.IsValidObject())
		{
			COleObject* _Object = &This->lpVtbl->m_oleObject;
			//update or add element
			PropertyHandle _Handle = _Object->GetPropertyHandle(std::wstring(wszName));
			if (_Handle == HandleModify || _Handle == HandleLog)
			{
				//update
				_Object->SetPropertyValue(std::wstring(wszName), *pVal);
			}
			else if (_Object->GetPropertyType(std::wstring(wszName)) == VT_ILLEGAL)
			{
				//add
				COleObject::Property _Property;
				_Property.PropertyName = wszName;
				_Property.Value = *pVal;
				_Property.Handle = HandleNon;
				_Object->AddProperty(_Property);
			}
		}
	}
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemClassObjectDelete(
	idIWbemClassObject * This,
	/* [string][in] */ LPCWSTR wszName)
{
	HRESULT hr = S_OK;
	COleObject* _Object = &This->lpVtbl->m_oleObject;
	//pre process
	if (_Object->IsValidObject() && _Object->GetMemberHandle() == HandleDup)
	{
		hr = S_OK;
	}
	else
	{
		hr = ((IWbemClassObject*)(This->lpVtbl->m_realWbemClassObjectObj.pItf))->Delete(wszName);
	}
	if (hr == S_OK)
	{
		if (This->lpVtbl->m_oleObject.IsValidObject())
		{
			COleObject* _Object = &This->lpVtbl->m_oleObject;

			if (_Object->GetPropertyType(std::wstring(wszName)) != VT_ILLEGAL)
			{
				std::wstring PropertyName = wszName;
				_Object->DeleteProperty(PropertyName);
			}
		}
	}
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemClassObjectGetNames(
	idIWbemClassObject * This,
	/* [string][in] */ LPCWSTR wszQualifierName,
	/* [in] */ long lFlags,
	/* [in] */ VARIANT *pQualifierVal,
	/* [out] */ SAFEARRAY * *pNames)
{
	HRESULT hr = S_OK;
	COleObject* _Object = &This->lpVtbl->m_oleObject;
	//pre process
	if (_Object->IsValidObject() && _Object->GetMemberHandle() == HandleDup)
	{
		return S_FALSE;
	}

	hr = ((IWbemClassObject*)(This->lpVtbl->m_realWbemClassObjectObj.pItf))->GetNames(wszQualifierName, lFlags, pQualifierVal, pNames);
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemClassObjectBeginEnumeration(
	idIWbemClassObject * This,
	/* [in] */ long lEnumFlags)
{
	HRESULT hr = S_OK;
	COleObject* _Object = &This->lpVtbl->m_oleObject;
	//pre process
	if (_Object->IsValidObject() && _Object->GetMemberHandle() == HandleDup)
	{
		return S_FALSE;
	}

	hr = ((IWbemClassObject*)(This->lpVtbl->m_realWbemClassObjectObj.pItf))->BeginEnumeration(lEnumFlags);
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemClassObjectNext(
	idIWbemClassObject * This,
	/* [in] */ long lFlags,
	/* [unique][in][out] */ BSTR *strName,
	/* [unique][in][out] */ VARIANT *pVal,
	/* [unique][in][out] */ long *pType,
	/* [unique][in][out] */ long *plFlavor)
{
	HRESULT hr = S_OK;
	COleObject* _Object = &This->lpVtbl->m_oleObject;
	//pre process
	if (_Object->IsValidObject() && _Object->GetMemberHandle() == HandleDup)
	{
		hr = S_OK;
	}
	else
	{
		hr = ((IWbemClassObject*)(This->lpVtbl->m_realWbemClassObjectObj.pItf))->Next(lFlags, strName, pVal, pType, plFlavor);
	}
	if (hr == S_OK && *strName != nullptr)
	{
		if (pVal && This->lpVtbl->m_oleObject.IsValidObject())
		{
			COleObject* _Object = &This->lpVtbl->m_oleObject;
			// modify element and return.
			PropertyHandle _Handle = _Object->GetPropertyHandle(std::wstring(*strName));
			if (_Handle == HandleModify)
			{
				VARIANT Value;
				if (_Object->GetPropertyValue(std::wstring(*strName), Value))
				{
					*pVal = Value;
					WmiInterfaceQueryLog(GetClassIdFromString(_Object->GetObjectName()), *strName, Value);
				}
			} // log element.
			else if (_Handle == HandleLog)
			{
				WmiInterfaceQueryLog(GetClassIdFromString(_Object->GetObjectName()), *strName, *pVal);
			}
		}
	}
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemClassObjectEndEnumeration(
	idIWbemClassObject * This)
{
	HRESULT hr = S_OK;
	COleObject* _Object = &This->lpVtbl->m_oleObject;
	//pre process
	if (_Object->IsValidObject() && _Object->GetMemberHandle() == HandleDup)
	{
		return S_FALSE;
	}

	hr = ((IWbemClassObject*)(This->lpVtbl->m_realWbemClassObjectObj.pItf))->EndEnumeration();
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemClassObjectGetPropertyQualifierSet(
	idIWbemClassObject * This,
	/* [string][in] */ LPCWSTR wszProperty,
	/* [out] */ idIWbemQualifierSet **ppQualSet)
{
	HRESULT hr = S_OK;
	COleObject* _Object = &This->lpVtbl->m_oleObject;
	//pre process
	if (_Object->IsValidObject() && _Object->GetMemberHandle() == HandleDup)
	{
		return S_FALSE;
	}

	hr = ((IWbemClassObject*)(This->lpVtbl->m_realWbemClassObjectObj.pItf))->GetPropertyQualifierSet(wszProperty, (IWbemQualifierSet**)ppQualSet);
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemClassObjectClone(
	idIWbemClassObject * This,
	/* [out] */ idIWbemClassObject **ppCopy)
{
	HRESULT hr = S_OK;
	COleObject* _Object = &This->lpVtbl->m_oleObject;
	//pre process
	if (_Object->IsValidObject() && _Object->GetMemberHandle() == HandleDup)
	{
		return S_FALSE;
	}

	hr = ((IWbemClassObject*)(This->lpVtbl->m_realWbemClassObjectObj.pItf))->Clone((IWbemClassObject**)ppCopy);
	if (hr == S_OK)
	{
		CloneComObject(&IID_idIWbemClassObject, &This->lpVtbl->m_realWbemClassObjectObj.queryIID, (void**)ppCopy, This);
	}
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemClassObjectGetObjectText(
	idIWbemClassObject * This,
	/* [in] */ long lFlags,
	/* [out] */ BSTR *pstrObjectText)
{
	HRESULT hr = S_OK;
	COleObject* _Object = &This->lpVtbl->m_oleObject;
	//pre process
	if (_Object->IsValidObject() && _Object->GetMemberHandle() == HandleDup)
	{
		return S_FALSE;
	}

	hr = ((IWbemClassObject*)(This->lpVtbl->m_realWbemClassObjectObj.pItf))->GetObjectText(lFlags, pstrObjectText);
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemClassObjectSpawnDerivedClass(
	idIWbemClassObject * This,
	/* [in] */ long lFlags,
	/* [out] */ idIWbemClassObject **ppNewClass)
{
	HRESULT hr = S_OK;
	COleObject* _Object = &This->lpVtbl->m_oleObject;
	//pre process
	if (_Object->IsValidObject() && _Object->GetMemberHandle() == HandleDup)
	{
		return S_FALSE;
	}

	hr = ((IWbemClassObject*)(This->lpVtbl->m_realWbemClassObjectObj.pItf))->SpawnDerivedClass(lFlags, (IWbemClassObject**)ppNewClass);
	if (hr == S_OK)
	{
		CloneComObject(&IID_idIWbemClassObject, &This->lpVtbl->m_realWbemClassObjectObj.queryIID, (void**)ppNewClass);
	}
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemClassObjectSpawnInstance(
	idIWbemClassObject * This,
	/* [in] */ long lFlags,
	/* [out] */ idIWbemClassObject **ppNewInstance)
{
	HRESULT hr = S_OK;
	COleObject* _Object = &This->lpVtbl->m_oleObject;
	//pre process
	if (_Object->IsValidObject() && _Object->GetMemberHandle() == HandleDup)
	{
		return S_FALSE;
	}

	hr = ((IWbemClassObject*)(This->lpVtbl->m_realWbemClassObjectObj.pItf))->SpawnInstance(lFlags, (IWbemClassObject**)ppNewInstance);
	if (hr == S_OK)
	{
		CloneComObject(&IID_idIWbemClassObject, &This->lpVtbl->m_realWbemClassObjectObj.queryIID, (void**)ppNewInstance, This);
	}
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemClassObjectCompareTo(
	idIWbemClassObject * This,
	/* [in] */ long lFlags,
	/* [in] */ idIWbemClassObject *pCompareTo)
{
	HRESULT hr = S_OK;
	COleObject* _Object = &This->lpVtbl->m_oleObject;
	//pre process
	if (_Object->IsValidObject() && _Object->GetMemberHandle() == HandleDup)
	{
		return S_FALSE;
	}

	hr = ((IWbemClassObject*)(This->lpVtbl->m_realWbemClassObjectObj.pItf))->CompareTo(lFlags, (IWbemClassObject*)pCompareTo);
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemClassObjectGetPropertyOrigin(
	idIWbemClassObject * This,
	/* [string][in] */ LPCWSTR wszName,
	/* [out] */ BSTR *pstrClassName)
{
	HRESULT hr = S_OK;
	COleObject* _Object = &This->lpVtbl->m_oleObject;
	//pre process
	if (_Object->IsValidObject() && _Object->GetMemberHandle() == HandleDup)
	{
		return S_FALSE;
	}

	hr = ((IWbemClassObject*)(This->lpVtbl->m_realWbemClassObjectObj.pItf))->GetPropertyOrigin(wszName, pstrClassName);
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemClassObjectInheritsFrom(
	idIWbemClassObject * This,
	/* [in] */ LPCWSTR strAncestor)
{
	HRESULT hr = S_OK;
	COleObject* _Object = &This->lpVtbl->m_oleObject;
	//pre process
	if (_Object->IsValidObject() && _Object->GetMemberHandle() == HandleDup)
	{
		return S_FALSE;
	}

	hr = ((IWbemClassObject*)(This->lpVtbl->m_realWbemClassObjectObj.pItf))->InheritsFrom(strAncestor);
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemClassObjectGetMethod(
	idIWbemClassObject * This,
	/* [string][in] */ LPCWSTR wszName,
	/* [in] */ long lFlags,
	/* [out] */ idIWbemClassObject **ppInSignature,
	/* [out] */ idIWbemClassObject **ppOutSignature)
{
	HRESULT hr = S_OK;
	COleObject* _Object = &This->lpVtbl->m_oleObject;
	//pre process
	if (_Object->IsValidObject() && _Object->GetMemberHandle() == HandleDup)
	{
		hr = S_OK;
	}
	else
	{
		hr = ((IWbemClassObject*)(This->lpVtbl->m_realWbemClassObjectObj.pItf))->GetMethod(wszName, lFlags, (IWbemClassObject**)ppInSignature, (IWbemClassObject**)ppOutSignature);
	}
	if (hr == S_OK)
	{
		if (This->lpVtbl->m_oleObject.IsValidObject())
		{
			COleObject* _Object = &This->lpVtbl->m_oleObject;

			PropertyHandle _Handle = _Object->GetMethodHandle(wszName);
			if (_Handle != HandleNon)
			{
				if(ppInSignature && *ppInSignature)
					CloneComObject(&IID_idIWbemClassObject, &This->lpVtbl->m_realWbemClassObjectObj.queryIID, (void**)ppInSignature, This);
				if(ppOutSignature && *ppOutSignature)
					CloneComObject(&IID_idIWbemClassObject, &This->lpVtbl->m_realWbemClassObjectObj.queryIID, (void**)ppOutSignature, This);
			}
		}
	}
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemClassObjectPutMethod(
	idIWbemClassObject * This,
	/* [string][in] */ LPCWSTR wszName,
	/* [in] */ long lFlags,
	/* [in] */ idIWbemClassObject *pInSignature,
	/* [in] */ idIWbemClassObject *pOutSignature)
{
	HRESULT hr = S_OK;
	COleObject* _Object = &This->lpVtbl->m_oleObject;
	//pre process
	if (_Object->IsValidObject() && _Object->GetMemberHandle() == HandleDup)
	{
		return S_FALSE;
	}

	hr = ((IWbemClassObject*)(This->lpVtbl->m_realWbemClassObjectObj.pItf))->PutMethod(wszName, lFlags, (IWbemClassObject*)pInSignature, (IWbemClassObject*)pOutSignature);
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemClassObjectDeleteMethod(
	idIWbemClassObject * This,
	/* [string][in] */ LPCWSTR wszName)
{
	HRESULT hr = S_OK;
	COleObject* _Object = &This->lpVtbl->m_oleObject;
	//pre process
	if (_Object->IsValidObject() && _Object->GetMemberHandle() == HandleDup)
	{
		return S_FALSE;
	}

	hr = ((IWbemClassObject*)(This->lpVtbl->m_realWbemClassObjectObj.pItf))->DeleteMethod(wszName);
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemClassObjectBeginMethodEnumeration(
	idIWbemClassObject * This,
	/* [in] */ long lEnumFlags)
{
	HRESULT hr = S_OK;
	COleObject* _Object = &This->lpVtbl->m_oleObject;
	//pre process
	if (_Object->IsValidObject() && _Object->GetMemberHandle() == HandleDup)
	{
		return S_FALSE;
	}

	hr = ((IWbemClassObject*)(This->lpVtbl->m_realWbemClassObjectObj.pItf))->BeginMethodEnumeration(lEnumFlags);
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemClassObjectNextMethod(
	idIWbemClassObject * This,
	/* [in] */ long lFlags,
	/* [unique][in][out] */ BSTR *pstrName,
	/* [unique][in][out] */ idIWbemClassObject **ppInSignature,
	/* [unique][in][out] */ idIWbemClassObject **ppOutSignature)
{
	HRESULT hr = S_OK;
	COleObject* _Object = &This->lpVtbl->m_oleObject;
	//pre process
	if (_Object->IsValidObject() && _Object->GetMemberHandle() == HandleDup)
	{
		hr = S_OK;
	}
	else
	{
		hr = ((IWbemClassObject*)(This->lpVtbl->m_realWbemClassObjectObj.pItf))->NextMethod(lFlags, pstrName, (IWbemClassObject**)ppInSignature, (IWbemClassObject**)ppOutSignature);
	}
	if (hr == S_OK)
	{
		if (pstrName)
		{
			if (This->lpVtbl->m_oleObject.IsValidObject())
			{
				COleObject* _Object = &This->lpVtbl->m_oleObject;

				PropertyHandle _Handle = _Object->GetMethodHandle(*pstrName);
				if (_Handle != HandleNon)
				{
					if (ppInSignature && *ppInSignature)
						CloneComObject(&IID_idIWbemClassObject, &This->lpVtbl->m_realWbemClassObjectObj.queryIID, (void**)ppInSignature, This);
					if (ppOutSignature && *ppOutSignature)
						CloneComObject(&IID_idIWbemClassObject, &This->lpVtbl->m_realWbemClassObjectObj.queryIID, (void**)ppOutSignature, This);
				}
			}
		}
	}
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemClassObjectEndMethodEnumeration(
	idIWbemClassObject * This)
{
	HRESULT hr = S_OK;
	COleObject* _Object = &This->lpVtbl->m_oleObject;
	//pre process
	if (_Object->IsValidObject() && _Object->GetMemberHandle() == HandleDup)
	{
		return S_FALSE;
	}

	hr = ((IWbemClassObject*)(This->lpVtbl->m_realWbemClassObjectObj.pItf))->EndMethodEnumeration();
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemClassObjectGetMethodQualifierSet(
	idIWbemClassObject * This,
	/* [string][in] */ LPCWSTR wszMethod,
	/* [out] */ idIWbemQualifierSet **ppQualSet)
{
	HRESULT hr = S_OK;
	COleObject* _Object = &This->lpVtbl->m_oleObject;
	//pre process
	if (_Object->IsValidObject() && _Object->GetMemberHandle() == HandleDup)
	{
		return S_FALSE;
	}

	hr = ((IWbemClassObject*)(This->lpVtbl->m_realWbemClassObjectObj.pItf))->GetMethodQualifierSet(wszMethod, (IWbemQualifierSet**)ppQualSet);
	return hr;
}

HRESULT STDMETHODCALLTYPE WbemClassObjectGetMethodOrigin(
	idIWbemClassObject * This,
	/* [string][in] */ LPCWSTR wszMethodName,
	/* [out] */ BSTR *pstrClassName)
{
	HRESULT hr = S_OK;
	COleObject* _Object = &This->lpVtbl->m_oleObject;
	//pre process
	if (_Object->IsValidObject() && _Object->GetMemberHandle() == HandleDup)
	{
		return S_FALSE;
	}

	hr = ((IWbemClassObject*)(This->lpVtbl->m_realWbemClassObjectObj.pItf))->GetMethodOrigin(wszMethodName, pstrClassName);
	return hr;
}

void InitializeWin32WbemClassObject(idIWbemClassObject* p_IWbemClassObject, COleObject& OleObject)
{
	p_IWbemClassObject->lpVtbl->AddRef = WbemClassObjectAddRef;
	p_IWbemClassObject->lpVtbl->BeginEnumeration = WbemClassObjectBeginEnumeration;
	p_IWbemClassObject->lpVtbl->BeginMethodEnumeration = WbemClassObjectBeginMethodEnumeration;
	p_IWbemClassObject->lpVtbl->Clone = WbemClassObjectClone;
	p_IWbemClassObject->lpVtbl->CompareTo = WbemClassObjectCompareTo;
	p_IWbemClassObject->lpVtbl->Delete = WbemClassObjectDelete;
	p_IWbemClassObject->lpVtbl->DeleteMethod = WbemClassObjectDeleteMethod;
	p_IWbemClassObject->lpVtbl->EndEnumeration = WbemClassObjectEndEnumeration;
	p_IWbemClassObject->lpVtbl->EndMethodEnumeration = WbemClassObjectEndMethodEnumeration;
	p_IWbemClassObject->lpVtbl->Get = WbemClassObjectGet;
	p_IWbemClassObject->lpVtbl->GetMethod = WbemClassObjectGetMethod;
	p_IWbemClassObject->lpVtbl->GetMethodOrigin = WbemClassObjectGetMethodOrigin;
	p_IWbemClassObject->lpVtbl->GetMethodQualifierSet = WbemClassObjectGetMethodQualifierSet;
	p_IWbemClassObject->lpVtbl->GetNames = WbemClassObjectGetNames;
	p_IWbemClassObject->lpVtbl->GetObjectText = WbemClassObjectGetObjectText;
	p_IWbemClassObject->lpVtbl->GetPropertyOrigin = WbemClassObjectGetPropertyOrigin;
	p_IWbemClassObject->lpVtbl->GetPropertyQualifierSet = WbemClassObjectGetPropertyQualifierSet;
	p_IWbemClassObject->lpVtbl->GetQualifierSet = WbemClassObjectGetQualifierSet;
	p_IWbemClassObject->lpVtbl->InheritsFrom = WbemClassObjectInheritsFrom;
	p_IWbemClassObject->lpVtbl->Next = WbemClassObjectNext;
	p_IWbemClassObject->lpVtbl->NextMethod = WbemClassObjectNextMethod;
	p_IWbemClassObject->lpVtbl->Put = WbemClassObjectPut;
	p_IWbemClassObject->lpVtbl->PutMethod = WbemClassObjectPutMethod;
	p_IWbemClassObject->lpVtbl->QueryInterface = WbemClassObjectQueryInterface;
	p_IWbemClassObject->lpVtbl->Release = WbemClassObjectRelease;
	p_IWbemClassObject->lpVtbl->SpawnDerivedClass = WbemClassObjectSpawnDerivedClass;
	p_IWbemClassObject->lpVtbl->SpawnInstance = WbemClassObjectSpawnInstance;
	p_IWbemClassObject->lpVtbl->m_realWbemClassObjectObj.hr = APPX_E_INTERLEAVING_NOT_ALLOWED;
	p_IWbemClassObject->lpVtbl->m_realWbemClassObjectObj.pItf = nullptr;

	p_IWbemClassObject->lpVtbl->m_oleObject = OleObject;
	p_IWbemClassObject->lpVtbl->AddRef(p_IWbemClassObject);

	return;
}



