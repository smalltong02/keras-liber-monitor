#include "stdafx.h"
#include "utils.h"
#include "Win32WbemClassObject.h"

using namespace cchips;
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
    return hr;
}

ULONG STDMETHODCALLTYPE WbemClassObjectAddRef(
    idIWbemClassObject * This)
{
    ULONG Ref = ++This->lpVtbl->m_ref;
    return Ref;
}

ULONG STDMETHODCALLTYPE WbemClassObjectRelease(
    idIWbemClassObject * This)
{
    ULONG Ref = --This->lpVtbl->m_ref;
    if (!Ref)
    {
        free(This->lpVtbl);
        free(This);
    }
    return Ref;
}

HRESULT STDMETHODCALLTYPE WbemClassObjectGetQualifierSet(
    idIWbemClassObject * This,
    /* [out] */ PVOID **ppQualSet)
{
    HRESULT hr = S_OK;
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
    HRESULT hr = S_FALSE;

    ASSERT(This->lpVtbl->m_wmi_object);
    ASSERT(pVal != nullptr);
    if (wszName == nullptr) return hr;
    if (pVal == nullptr) return hr;
    std::unique_ptr<cchips::CLogHandle> log_handle = std::make_unique<cchips::CLogHandle>("W2", CLogObject::logtype::log_event);
    if (!log_handle) return hr;
    BEGIN_LOG("Duplicate_IWbemClassObject_Get");
    for (const auto& pcheck : This->lpVtbl->m_wmi_object->GetChecks(false))
    {
        ASSERT(pcheck != nullptr);
        if (pcheck == nullptr)
            continue;
        for (const auto& modify : pcheck->GetModifys())
        {
            ASSERT(modify != nullptr);
            if (!modify) continue;
            for (const auto& iden : modify->GetIdenifierSymbol())
            {
                if (_stricmp(pcheck->GetRealName(iden.first).c_str(), W2AString(wszName).c_str()) == 0)
                {
                    VARTYPE type = ConvertObTypeToVarintType(iden.second->GetName());
                    pVal->vt = VT_EMPTY;
                    CExpParsing::_ValueMap value_map;
                    std::any cur_anyvalue = iden.second->GetCurValue();
                    if (!modify->SetIdentifierValue(value_map, CExpParsing::ValuePair(iden.first, cur_anyvalue)))
                    {
                        return hr;
                    }
                    // back propagation
                    auto variant_value = modify->EvalExpression(value_map);
                    if (variant_value.index() != 1) break;
                    std::unique_ptr<CExpParsing::ValuePair> value = std::move(std::get<std::unique_ptr<CExpParsing::ValuePair>>(variant_value));
                    if (value == nullptr)
                        break;
                    if (_stricmp(iden.first.c_str(), value->first.c_str()) != 0)
                    {
                        ASSERT(0); continue;
                    }
                    std::any any_new_val = cur_anyvalue;
                    if (AssignAnyType(any_new_val, value->second))
                    {
                        pVal->vt = type;
                        if (!SetVariantValue(*pVal, any_new_val)) {
                            pVal->vt = VT_EMPTY;
                            return hr;
                        }
                        if (std::stringstream ss = OutputAnyValue(any_new_val); ss.str().length())
                            LOGGING(pcheck->GetRealName(iden.first), ss.str());
                        END_LOG(log_handle->GetHandle());
                        return S_OK;
                    }
                    return hr;
                }
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
    return hr;
}

HRESULT STDMETHODCALLTYPE WbemClassObjectDelete(
    idIWbemClassObject * This,
    /* [string][in] */ LPCWSTR wszName)
{
    HRESULT hr = S_OK;
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
    return hr;
}

HRESULT STDMETHODCALLTYPE WbemClassObjectBeginEnumeration(
    idIWbemClassObject * This,
    /* [in] */ long lEnumFlags)
{
    HRESULT hr = S_OK;
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
    return hr;
}

HRESULT STDMETHODCALLTYPE WbemClassObjectEndEnumeration(
    idIWbemClassObject * This)
{
    HRESULT hr = S_OK;
    return hr;
}

HRESULT STDMETHODCALLTYPE WbemClassObjectGetPropertyQualifierSet(
    idIWbemClassObject * This,
    /* [string][in] */ LPCWSTR wszProperty,
    /* [out] */ PVOID **ppQualSet)
{
    HRESULT hr = S_OK;
    return hr;
}

HRESULT STDMETHODCALLTYPE WbemClassObjectClone(
    idIWbemClassObject * This,
    /* [out] */ idIWbemClassObject **ppCopy)
{
    HRESULT hr = S_OK;
    return hr;
}

HRESULT STDMETHODCALLTYPE WbemClassObjectGetObjectText(
    idIWbemClassObject * This,
    /* [in] */ long lFlags,
    /* [out] */ BSTR *pstrObjectText)
{
    HRESULT hr = S_OK;
    return hr;
}

HRESULT STDMETHODCALLTYPE WbemClassObjectSpawnDerivedClass(
    idIWbemClassObject * This,
    /* [in] */ long lFlags,
    /* [out] */ idIWbemClassObject **ppNewClass)
{
    HRESULT hr = S_OK;
    return hr;
}

HRESULT STDMETHODCALLTYPE WbemClassObjectSpawnInstance(
    idIWbemClassObject * This,
    /* [in] */ long lFlags,
    /* [out] */ idIWbemClassObject **ppNewInstance)
{
    HRESULT hr = S_OK;
    return hr;
}

HRESULT STDMETHODCALLTYPE WbemClassObjectCompareTo(
    idIWbemClassObject * This,
    /* [in] */ long lFlags,
    /* [in] */ idIWbemClassObject *pCompareTo)
{
    HRESULT hr = S_OK;
    return hr;
}

HRESULT STDMETHODCALLTYPE WbemClassObjectGetPropertyOrigin(
    idIWbemClassObject * This,
    /* [string][in] */ LPCWSTR wszName,
    /* [out] */ BSTR *pstrClassName)
{
    HRESULT hr = S_OK;
    return hr;
}

HRESULT STDMETHODCALLTYPE WbemClassObjectInheritsFrom(
    idIWbemClassObject * This,
    /* [in] */ LPCWSTR strAncestor)
{
    HRESULT hr = S_OK;
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
    return hr;
}

HRESULT STDMETHODCALLTYPE WbemClassObjectDeleteMethod(
    idIWbemClassObject * This,
    /* [string][in] */ LPCWSTR wszName)
{
    HRESULT hr = S_OK;
    return hr;
}

HRESULT STDMETHODCALLTYPE WbemClassObjectBeginMethodEnumeration(
    idIWbemClassObject * This,
    /* [in] */ long lEnumFlags)
{
    HRESULT hr = S_OK;
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
    return hr;
}

HRESULT STDMETHODCALLTYPE WbemClassObjectEndMethodEnumeration(
    idIWbemClassObject * This)
{
    HRESULT hr = S_OK;
    return hr;
}

HRESULT STDMETHODCALLTYPE WbemClassObjectGetMethodQualifierSet(
    idIWbemClassObject * This,
    /* [string][in] */ LPCWSTR wszMethod,
    /* [out] */ PVOID **ppQualSet)
{
    HRESULT hr = S_OK;
    return hr;
}

HRESULT STDMETHODCALLTYPE WbemClassObjectGetMethodOrigin(
    idIWbemClassObject * This,
    /* [string][in] */ LPCWSTR wszMethodName,
    /* [out] */ BSTR *pstrClassName)
{
    HRESULT hr = S_OK;
    return hr;
}

void InitializeWin32WbemClassObject(idIWbemClassObject* p_IWbemClassObject, const std::shared_ptr<cchips::CWmiObject>& wmi_object)
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

    p_IWbemClassObject->lpVtbl->m_wmi_object = wmi_object;
    p_IWbemClassObject->lpVtbl->AddRef(p_IWbemClassObject);

    return;
}



