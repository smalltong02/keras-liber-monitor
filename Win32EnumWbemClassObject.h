#pragma once
#include "utils.h"
#include "ParsingImpl.h"
#include "Win32WbemClassObject.h"

typedef interface idIEnumWbemClassObject idIEnumWbemClassObject;

typedef struct idIEnumWbemClassObjectVtbl
{
    BEGIN_INTERFACE

        HRESULT(STDMETHODCALLTYPE *QueryInterface)(
            idIEnumWbemClassObject * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */
            _COM_Outptr_  void **ppvObject);

    ULONG(STDMETHODCALLTYPE *AddRef)(
        idIEnumWbemClassObject * This);

    ULONG(STDMETHODCALLTYPE *Release)(
        idIEnumWbemClassObject * This);

    HRESULT(STDMETHODCALLTYPE *Reset)(
        idIEnumWbemClassObject * This);

    HRESULT(STDMETHODCALLTYPE *Next)(
        idIEnumWbemClassObject * This,
        /* [in] */ long lTimeout,
        /* [in] */ ULONG uCount,
        /* [length_is][size_is][out] */ idIWbemClassObject **apObjects,
        /* [out] */ ULONG *puReturned);

    HRESULT(STDMETHODCALLTYPE *NextAsync)(
        idIEnumWbemClassObject * This,
        /* [in] */ ULONG uCount,
        /* [in] */ IWbemObjectSink *pSink);

    HRESULT(STDMETHODCALLTYPE *Clone)(
        idIEnumWbemClassObject * This,
        /* [out] */ idIEnumWbemClassObject **ppEnum);

    HRESULT(STDMETHODCALLTYPE *Skip)(
        idIEnumWbemClassObject * This,
        /* [in] */ long lTimeout,
        /* [in] */ ULONG nCount);

    std::shared_ptr<cchips::CWmiObject> m_wmi_object;
    ULONG m_nItem;
    ULONG m_ref;
    IEnumWbemClassObject *pItf;

    END_INTERFACE
} idIEnumWbemClassObjectVtbl;

interface idIEnumWbemClassObject
{
    CONST_VTBL struct idIEnumWbemClassObjectVtbl *lpVtbl;
};

HRESULT STDMETHODCALLTYPE EnumWbemClassObjectQueryInterface(
    idIEnumWbemClassObject * This,
    /* [in] */ REFIID riid,
    /* [annotation][iid_is][out] */
    _COM_Outptr_  void **ppvObject);

ULONG STDMETHODCALLTYPE EnumWbemClassObjectAddRef(
    idIEnumWbemClassObject * This);

ULONG STDMETHODCALLTYPE EnumWbemClassObjectRelease(
    idIEnumWbemClassObject * This);

HRESULT STDMETHODCALLTYPE EnumWbemClassObjectReset(
    idIEnumWbemClassObject * This);

HRESULT STDMETHODCALLTYPE EnumWbemClassObjectNext(
    idIEnumWbemClassObject * This,
    /* [in] */ long lTimeout,
    /* [in] */ ULONG uCount,
    /* [length_is][size_is][out] */ idIWbemClassObject **apObjects,
    /* [out] */ ULONG *puReturned);

HRESULT STDMETHODCALLTYPE EnumWbemClassObjectNextAsync(
    idIEnumWbemClassObject * This,
    /* [in] */ ULONG uCount,
    /* [in] */ IWbemObjectSink *pSink);

HRESULT STDMETHODCALLTYPE EnumWbemClassObjectClone(
    idIEnumWbemClassObject * This,
    /* [out] */ idIEnumWbemClassObject **ppEnum);

HRESULT STDMETHODCALLTYPE EnumWbemClassObjectSkip(
    idIEnumWbemClassObject * This,
    /* [in] */ long lTimeout,
    /* [in] */ ULONG nCount);

void InitializeWin32EnumWbemClassObject(idIEnumWbemClassObject* p_IEnumWbemClassObject, IEnumWbemClassObject* pItf, const std::shared_ptr<cchips::CWmiObject>& wmi_object);
