#include "stdafx.h"
#include "utils.h"
#include "Win32EnumWbemClassObject.h"
#include "HookImplementObject.h"

using namespace cchips;
extern bool process_duplicate_for_wmiobject(CHookImplementObject::detour_node* node, const std::shared_ptr<CWmiObject>& wmi_object, IWbemClassObject **apObjects, ULONG *puReturned, std::shared_ptr<CLogHandle>& log_handle);

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
    ASSERT(This->lpVtbl->pItf);
    if (ppvObject == nullptr) return S_FALSE;
    hr = ((IEnumWbemClassObject*)(This->lpVtbl->pItf))->QueryInterface(riid, ppvObject);
    return hr;
}

ULONG STDMETHODCALLTYPE EnumWbemClassObjectAddRef(
    idIEnumWbemClassObject * This)
{
    return ((IEnumWbemClassObject*)(This->lpVtbl->pItf))->AddRef();
}

ULONG STDMETHODCALLTYPE EnumWbemClassObjectRelease(
    idIEnumWbemClassObject * This)
{
    ULONG Ref = ((IEnumWbemClassObject*)(This->lpVtbl->pItf))->Release();
    if (Ref <= 1)
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
    hr = ((IEnumWbemClassObject*)(This->lpVtbl->pItf))->Reset();
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
    hr = ((IEnumWbemClassObject*)(This->lpVtbl->pItf))->Next(lTimeout, uCount, (IWbemClassObject**)apObjects, puReturned);
    if (hr != S_OK && This->lpVtbl->m_nItem == 0)
    {
        This->lpVtbl->m_nItem++;
        // the class object not instance, so clone a new object instance.
        if (This->lpVtbl->m_wmi_object == nullptr) return hr;
        std::string ObjectName = This->lpVtbl->m_wmi_object->GetName();
        std::unique_ptr<cchips::CLogHandle> log_handle = std::make_unique<cchips::CLogHandle>("W1", CLogObject::logtype::log_event);
        if (!log_handle) return hr;
        BEGIN_LOG("Duplicate_IEnumWbemClassObject_Next");
        LOGGING("Class", ObjectName);
        cchips::CHookImplementObject::detour_node node = { nullptr, nullptr, 0, nullptr, nullptr, log_handle->GetHandle() };
        if (process_duplicate_for_wmiobject(&node, This->lpVtbl->m_wmi_object, (IWbemClassObject**)apObjects, puReturned, LOGGER))
        {
            LOGGING("Duplicate", "Success");
            hr = S_OK;
        }
        else {
            LOGGING("Duplicate", "Failed");
        }
        END_LOG(log_handle->GetHandle());
    }
    return hr;
}

HRESULT STDMETHODCALLTYPE EnumWbemClassObjectNextAsync(
    idIEnumWbemClassObject * This,
    /* [in] */ ULONG uCount,
    /* [in] */ IWbemObjectSink *pSink)
{
    HRESULT hr = S_OK;
    hr = ((IEnumWbemClassObject*)(This->lpVtbl->pItf))->NextAsync(uCount, (IWbemObjectSink*)pSink);
    return hr;
}

HRESULT STDMETHODCALLTYPE EnumWbemClassObjectClone(
    idIEnumWbemClassObject * This,
    /* [out] */ idIEnumWbemClassObject **ppEnum)
{
    HRESULT hr = S_OK;
    hr = ((IEnumWbemClassObject*)(This->lpVtbl->pItf))->Clone((IEnumWbemClassObject **)ppEnum);
    return hr;
}

HRESULT STDMETHODCALLTYPE EnumWbemClassObjectSkip(
    idIEnumWbemClassObject * This,
    /* [in] */ long lTimeout,
    /* [in] */ ULONG nCount)
{
    HRESULT hr = S_OK;
    hr = ((IEnumWbemClassObject*)(This->lpVtbl->pItf))->Skip(lTimeout, nCount);
    return hr;
}

void InitializeWin32EnumWbemClassObject(idIEnumWbemClassObject* p_IEnumWbemClassObject, IEnumWbemClassObject* pItf, const std::shared_ptr<cchips::CWmiObject>& wmi_object)
{
    p_IEnumWbemClassObject->lpVtbl->AddRef = EnumWbemClassObjectAddRef;
    p_IEnumWbemClassObject->lpVtbl->Release = EnumWbemClassObjectRelease;
    p_IEnumWbemClassObject->lpVtbl->Reset = EnumWbemClassObjectReset;
    p_IEnumWbemClassObject->lpVtbl->Next = EnumWbemClassObjectNext;
    p_IEnumWbemClassObject->lpVtbl->NextAsync = EnumWbemClassObjectNextAsync;
    p_IEnumWbemClassObject->lpVtbl->Clone = EnumWbemClassObjectClone;
    p_IEnumWbemClassObject->lpVtbl->Skip = EnumWbemClassObjectSkip;
    p_IEnumWbemClassObject->lpVtbl->QueryInterface = EnumWbemClassObjectQueryInterface;
    p_IEnumWbemClassObject->lpVtbl->pItf = pItf;
    p_IEnumWbemClassObject->lpVtbl->m_wmi_object = wmi_object;
    p_IEnumWbemClassObject->lpVtbl->AddRef(p_IEnumWbemClassObject);
    return;
}
