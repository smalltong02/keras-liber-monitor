#include "stdafx.h"
#include "Win32EnumWbemClassObject.h"
#include "OleConfigObject.h"

extern COleConfig* g_oleConfig;

GUID IID_idIEnumWbemClassObject = {
	0x027947e1,
	0xd731,
	0x11ce,
	0xa3,0x57,0x00,0x00,0x00,0x00,0x00,0x01
};

HRESULT STDMETHODCALLTYPE EnumWbemClassObjectQueryInterface(
	idIEnumWbemClassObject * This,
	/* [in] */ REFIID riid,
	/* [annotation][iid_is][out] */
	_COM_Outptr_  void **ppvObject)
{
	HRESULT hr = S_OK;

	hr = ((IEnumWbemClassObject*)(This->lpVtbl->m_realEnumWbemClassObjectObj.pItf))->QueryInterface(riid, ppvObject);
	if (hr == S_OK)
	{
		if (*ppvObject == This->lpVtbl->m_realEnumWbemClassObjectObj.pItf)
			*ppvObject = This;
		else if (IsEqualIIID(&IID_idIUnknown, &riid))
			CloneComObject(&IID_idIEnumWbemClassObject, &This->lpVtbl->m_realEnumWbemClassObjectObj.queryIID, ppvObject);
		else
			CloneComObject((GUID*)&riid, &This->lpVtbl->m_realEnumWbemClassObjectObj.queryIID, ppvObject);
	}
	return hr;
}

ULONG STDMETHODCALLTYPE EnumWbemClassObjectAddRef(
	idIEnumWbemClassObject * This)
{
	ULONG Ref = ((IEnumWbemClassObject*)(This->lpVtbl->m_realEnumWbemClassObjectObj.pItf))->AddRef();
	This->lpVtbl->m_ref = Ref;
	return Ref;
}

ULONG STDMETHODCALLTYPE EnumWbemClassObjectRelease(
	idIEnumWbemClassObject * This)
{
	ULONG Ref = ((IEnumWbemClassObject*)(This->lpVtbl->m_realEnumWbemClassObjectObj.pItf))->Release();
	This->lpVtbl->m_ref = Ref;
	if (!Ref)
	{
		free(This->lpVtbl);
		free(This);
	}
	return Ref;
}

HRESULT STDMETHODCALLTYPE EnumWbemClassObjectReset(
	idIEnumWbemClassObject * This)
{
	HRESULT hr = S_OK;
	hr = ((IEnumWbemClassObject*)(This->lpVtbl->m_realEnumWbemClassObjectObj.pItf))->Reset();
	return hr;
}

HRESULT STDMETHODCALLTYPE EnumWbemClassObjectNext(
	idIEnumWbemClassObject * This,
	/* [in] */ long lTimeout,
	/* [in] */ ULONG uCount,
	/* [length_is][size_is][out] */ idIWbemClassObject **apObjects,
	/* [out] */ ULONG *puReturned)
{
	HRESULT hr = S_OK;
	hr = ((IEnumWbemClassObject*)(This->lpVtbl->m_realEnumWbemClassObjectObj.pItf))->Next(lTimeout, uCount, (IWbemClassObject**)apObjects, puReturned);
	if (hr == S_OK)
	{
		CloneComObject(&IID_idIWbemClassObject, &This->lpVtbl->m_realEnumWbemClassObjectObj.queryIID, (void**)apObjects);
	}
	else if (This->lpVtbl->m_nItem == 0)
	{
		// the class object not instance, so clone a new object instance.
		std::wstring ObjectName = GetStringFromGUID(This->lpVtbl->m_realEnumWbemClassObjectObj.queryIID);
		if (ObjectName.length() > 0 && g_oleConfig != nullptr)
		{
			int Ordinal = g_oleConfig->GetObjectOrdinal(ObjectName);
			if (g_oleConfig->CheckOrdinal(Ordinal))
			{
				COleObject* _Object = g_oleConfig->GetOleObject(Ordinal);
				if (_Object->GetMemberHandle() == HandleDup)
				{
					idIWbemClassObject* p_IWbemClassObject = (idIWbemClassObject *)malloc(sizeof(idIWbemClassObject));
					if (p_IWbemClassObject)
					{
						p_IWbemClassObject->lpVtbl = (idIWbemClassObjectVtbl *)malloc(sizeof(idIWbemClassObjectVtbl));
						if (p_IWbemClassObject->lpVtbl)
						{
							memset(p_IWbemClassObject->lpVtbl, 0, sizeof(idIWbemClassObjectVtbl));
							InitializeWin32WbemClassObject(p_IWbemClassObject, *_Object);
							*apObjects = (idIWbemClassObject*)p_IWbemClassObject;
							*puReturned = 1;
							This->lpVtbl->m_nItem++;
							return S_OK;
						}
					}
				}
			}
		}
	}
	return hr;
}

HRESULT STDMETHODCALLTYPE EnumWbemClassObjectNextAsync(
	idIEnumWbemClassObject * This,
	/* [in] */ ULONG uCount,
	/* [in] */ idIWbemObjectSink *pSink)
{
	HRESULT hr = S_OK;
	hr = ((IEnumWbemClassObject*)(This->lpVtbl->m_realEnumWbemClassObjectObj.pItf))->NextAsync(uCount, (IWbemObjectSink *)pSink);
	return hr;
}

HRESULT STDMETHODCALLTYPE EnumWbemClassObjectClone(
	idIEnumWbemClassObject * This,
	/* [out] */ idIEnumWbemClassObject **ppEnum)
{
	HRESULT hr = S_OK;
	hr = ((IEnumWbemClassObject*)(This->lpVtbl->m_realEnumWbemClassObjectObj.pItf))->Clone((IEnumWbemClassObject **)ppEnum);
	if (hr == S_OK)
	{
		CloneComObject(&IID_idIEnumWbemClassObject, &This->lpVtbl->m_realEnumWbemClassObjectObj.queryIID, (void**)ppEnum);
	}
	return hr;
}

HRESULT STDMETHODCALLTYPE EnumWbemClassObjectSkip(
	idIEnumWbemClassObject * This,
	/* [in] */ long lTimeout,
	/* [in] */ ULONG nCount)
{
	HRESULT hr = S_OK;
	hr = ((IEnumWbemClassObject*)(This->lpVtbl->m_realEnumWbemClassObjectObj.pItf))->Skip(lTimeout, nCount);
	return hr;
}





