#include "stdafx.h"
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
#include "Win32WbemServices.h"

HWND g_CommunicationWnd = NULL;
extern COleConfig* g_oleConfig;

GUID IID_idIQueryUnknown = {
	0xffffffff,
	0xffff,
	0xffff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff
};

GUID IID_idIManagedObject = {
	0xc3fcc19e,
	0xa970,
	0x11d2,
	0x8b,0x5a,0x00,0xa0,0xc9,0xb7,0xc9,0xc4
};

GUID IID_idIMarshal = {
	0x00000003,
	0x0000,
	0x0000,
	0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x46
};

GUID IID_Win32PhysicalMemory = {
	0xFAF76B93,
	0x798C,
	0x11D2,
	0xAA,0xD1,0x00,0x60,0x08,0xC7,0x8B,0xC7
};

GUID IID_Win32PhysicalMemoryArray = {
	0xFAF76B99,
	0x798C,
	0x11D2,
	0xAA,0xD1,0x00,0x60,0x08,0xC7,0x8B,0xC7
};

GUID IID_Win32ComputerSystem = {
	0x8502C4B0,
	0x5FBB,
	0x11D2,
	0xAA,0xC1,0x00,0x60,0x08,0xC7,0x8B,0xC7
};

GUID IID_Win32DiskDrive = {
	0x8502C4B2,
	0x5FBB,
	0x11D2,
	0xAA,0xC1,0x00,0x60,0x08,0xC7,0x8B,0xC7
};

GUID IID_Win32Processor = {
	0x8502C4BB,
	0x5FBB,
	0x11D2,
	0xAA,0xC1,0x00,0x60,0x08,0xC7,0x8B,0xC7
};

GUID IID_Win32LogicalDisk = {
	0x8502C4B7,
	0x5FBB,
	0x11D2,
	0xAA,0xC1,0x00,0x60,0x08,0xC7,0x8B,0xC7
};

GUID IID_Win32NetworkAdapterConfiguration = {
	0x8502C515,
	0x5FBB,
	0x11D2,
	0xAA,0xC1,0x00,0x60,0x08,0xC7,0x8B,0xC7
};

GUID IID_Win32BaseBoard = {
	0xFAF76B95,
	0x798C,
	0x11D2,
	0xAA,0xD1,0x00,0x60,0x08,0xC7,0x8B,0xC7
};

GUID IID_Win32CDROMDrive = {
	0x8502C4B3,
	0x5FBB,
	0x11D2,
	0xAA,0xC1,0x00,0x60,0x08,0xC7,0x8B,0xC7
};

GUID IID_Win32BIOS = {
	0x8502C4E1,
	0x5FBB,
	0x11D2,
	0xAA,0xC1,0x00,0x60,0x08,0xC7,0x8B,0xC7
};

GUID IID_Win32LoggedOnUser = {
	0x8BB5B3EC,
	0xE1F7,
	0x4b39,
	0x94,0x2A,0x60,0x5D,0x5F,0x55,0x78,0x9A
};

GUID IID_Win32OperatingSystem = {
	0x8502C4DE,
	0x5FBB,
	0x11D2,
	0xAA,0xC1,0x00,0x60,0x08,0xC7,0x8B,0xC7
};

GUID IID_Win32Process = {
	0x8502C4DC,
	0x5FBB,
	0x11D2,
	0xAA,0xC1,0x00,0x60,0x08,0xC7,0x8B,0xC7
};

GUID IID_Win32ProcessStartup = {
	0x8502C4DB,
	0x5FBB,
	0x11D2,
	0xAA,0xC1,0x00,0x60,0x08,0xC7,0x8B,0xC7
};

ComClassGUIDString _ComGUIDString[] = {
	{ IID_Win32ComputerSystem , L"Win32_ComputerSystem" },
	{ IID_Win32DiskDrive , L"Win32_DiskDrive" },
	{ IID_Win32Processor , L"Win32_Processor" },
	{ IID_Win32LogicalDisk , L"Win32_LogicalDisk" },
	{ IID_Win32NetworkAdapterConfiguration , L"Win32_NetworkAdapterConfiguration" },
	{ IID_Win32BaseBoard , L"Win32_BaseBoard" },
	{ IID_Win32CDROMDrive , L"Win32_CDROMDrive" },
	{ IID_Win32BIOS , L"Win32_BIOS" },
	{ IID_Win32LoggedOnUser , L"Win32_LoggedOnUser" },
	{ IID_Win32OperatingSystem , L"Win32_OperatingSystem" },
	{ IID_Win32PhysicalMemory , L"Win32_PhysicalMemory" },
	{ IID_Win32PhysicalMemoryArray , L"Win32_PhysicalMemoryArray" },
	{ IID_Win32Process , L"Win32_Process" },
	{ IID_Win32ProcessStartup , L"Win32_ProcessStartup" },
};

WmiClassIdString _WmiIdString[] = {
	{ wmi_Win32_ComputerSystem , L"Win32_ComputerSystem" },
	{ wmi_Win32_DiskDrive , L"Win32_DiskDrive" },
	{ wmi_Win32_Processor , L"Win32_Processor" },
	{ wmi_Win32_LogicalDisk , L"Win32_LogicalDisk" },
	{ wmi_Win32_NetworkAdapterConfiguration , L"Win32_NetworkAdapterConfiguration" },
	{ wmi_Win32_BaseBoard , L"Win32_BaseBoard" },
	{ wmi_Win32_CDROMDrive , L"Win32_CDROMDrive" },
	{ wmi_Win32_BIOS , L"Win32_BIOS" },
	{ wmi_Win32_LoggedOnUser , L"Win32_LoggedOnUser" },
	{ wmi_Win32_OperatingSystem , L"Win32_OperatingSystem" },
	{ wmi_Win32_PhysicalMemory , L"Win32_PhysicalMemory" },
	{ wmi_Win32_PhysicalMemoryArray , L"Win32_PhysicalMemoryArray" },
	{ wmi_Win32_Process , L"Win32_Process" },
	{ wmi_Win32_ProcessStartup , L"Win32_ProcessStartup" },
};

bool IsValidGUID(GUID& iid)
{
	GUID EmptyIid = { 0 };
	if (iid == EmptyIid)
		return false;
	if (iid == IID_idIQueryUnknown)
		return false;
	return true;
}

GUID GetGUIDFromString(std::wstring& Name)
{
	int Size = sizeof(_ComGUIDString) / sizeof(ComClassGUIDString);
	int Count = 0;
	
	while (Count < Size)
	{
		if (wcsicmp(_ComGUIDString[Count].ObjectName, Name.c_str()) == 0)
			return _ComGUIDString[Count].iid;
		Count++;
	}
	return IID_idIQueryUnknown;
}

std::wstring GetStringFromGUID(GUID& iid)
{
	int Size = sizeof(_ComGUIDString) / sizeof(ComClassGUIDString);
	int Count = 0;

	while (Count < Size)
	{
		if (IsEqualIIID(&_ComGUIDString[Count].iid, &iid))
			return _ComGUIDString[Count].ObjectName;
		Count++;
	}
	return std::wstring(L"");
}

int GetClassIdFromString(std::wstring& Name)
{
	int Size = sizeof(_WmiIdString) / sizeof(WmiClassIdString);
	int Count = 0;

	while (Count < Size)
	{
		if (wcsicmp(_WmiIdString[Count].ObjectName, Name.c_str()) == 0)
			return _WmiIdString[Count].classId;
		Count++;
	}
	return 0;
}

std::wstring GetStringFromClassId(int& id)
{
	int Size = sizeof(_WmiIdString) / sizeof(WmiClassIdString);
	int Count = 0;

	while (Count < Size)
	{
		if (_WmiIdString[Count].classId == id)
			return _WmiIdString[Count].ObjectName;
		Count++;
	}
	return std::wstring(L"");
}

bool StartsWith(wchar_t *heystack, wchar_t *needle, bool case_sensitive)
{
	while (*needle != '\0')
	{
		if ((case_sensitive && (*needle == *heystack))
			|| (!case_sensitive && (towlower(*needle) == towlower(*heystack))))
		{
			needle++;
			heystack++;
		}
		else
		{
			break;
		}
	}
	return !*needle;
}

bool EndsWith(wchar_t *heystack, wchar_t *needle, bool case_sensitive)
{
	wchar_t *p = heystack + wcslen(heystack) - 1;
	wchar_t *q = needle + wcslen(needle) - 1;
	while (q != needle && p != heystack)
	{
		if ((case_sensitive && (*p == *q))
			|| (!case_sensitive && (towlower(*p) == towlower(*q))))
		{
			p--;
			q--;
		}
		else
		{
			break;
		}
	}
	if (case_sensitive)
		return *p == *q && q == needle;
	return (towlower(*p) == towlower(*q)) && q == needle;
}

GUID* GetQueryIID(const BSTR QueryName)
{
	if (EndsWith(QueryName, L"Win32_ComputerSystem", false))
	{
		return &IID_Win32ComputerSystem;
	}
	else if (EndsWith(QueryName, L"Win32_DiskDrive", false))
	{
		return &IID_Win32DiskDrive;
	}
	else if (EndsWith(QueryName, L"Win32_Processor", false))
	{
		return &IID_Win32Processor;
	}
	else if (EndsWith(QueryName, L"Win32_LogicalDisk", false))
	{
		return &IID_Win32LogicalDisk;
	}
	else if (EndsWith(QueryName, L"Win32_NetworkAdapterConfiguration", false))
	{
		return &IID_Win32NetworkAdapterConfiguration;
	}
	else if (EndsWith(QueryName, L"Win32_BaseBoard", false))
	{
		return &IID_Win32BaseBoard;
	}
	else if (EndsWith(QueryName, L"Win32_CDROMDrive", false))
	{
		return &IID_Win32CDROMDrive;
	}
	else if (EndsWith(QueryName, L"Win32_BIOS", false))
	{
		return &IID_Win32BIOS;
	}
	else if (EndsWith(QueryName, L"Win32_LoggedOnUser", false))
	{
		return &IID_Win32LoggedOnUser;
	}
	else if (EndsWith(QueryName, L"Win32_PhysicalMemoryArray", false))
	{
		return &IID_Win32PhysicalMemoryArray;
	}
	else if (EndsWith(QueryName, L"Win32_PhysicalMemory", false))
	{
		return &IID_Win32PhysicalMemory;
	}
	else if (EndsWith(QueryName, L"Win32_OperatingSystem", false))
	{
		return &IID_Win32OperatingSystem;
	}
	else if (EndsWith(QueryName, L"Win32_Process", false))
	{
		return &IID_Win32Process;
	}

	return &IID_idIQueryUnknown;
}

bool IsCloneComObject(MULTI_idQI& qi)
{
	if (qi.hr == APPX_E_INTERLEAVING_NOT_ALLOWED)
		return true;
	return false;
}

void CloneComObject(GUID* riid, IID* queryIID, void **ppvObject, void* pThis)
{
	idIWbemLocator* p_idIWbemLocator = NULL;
	idIWbemPath* p_idIWbemPath = NULL;
	idIClassFactory* p_idClassFactory = NULL;
	idIClassFactory2* p_idIClassFactory2 = NULL;
	idIWbemServices* p_idIWbemServices = NULL;
	idIEnumWbemClassObject* p_idIEnumWbemClassObject = NULL;
	idIWbemClassObject* p_idIWbemClassObject = NULL;
	idIClientSecurity* p_idIClientSecurity = NULL;
	idIWbemCallResult* p_idIWbemCallResult = NULL;

	if (ppvObject == NULL ||
		*ppvObject == NULL)
		return;

	if (IsEqualIIID(riid, &IID_idIWbemLocator))
	{
		p_idIWbemLocator = (idIWbemLocator *)malloc(sizeof(idIWbemLocator));
		if (p_idIWbemLocator)
		{
			p_idIWbemLocator->lpVtbl = (struct idIWbemLocatorVtbl *)malloc(sizeof(idIWbemLocatorVtbl));
			if (p_idIWbemLocator->lpVtbl)
			{
				p_idIWbemLocator->lpVtbl->AddRef = WbemAddRef;
				p_idIWbemLocator->lpVtbl->ConnectServer = WbemConnectServer;
				p_idIWbemLocator->lpVtbl->QueryInterface = WbemQueryInterface;
				p_idIWbemLocator->lpVtbl->Release = WbemRelease;
				p_idIWbemLocator->lpVtbl->m_realWbemObj.hr = APPX_E_INTERLEAVING_NOT_ALLOWED;
				p_idIWbemLocator->lpVtbl->m_realWbemObj.idIID = *riid;
				p_idIWbemLocator->lpVtbl->m_realWbemObj.queryIID = *queryIID;
				p_idIWbemLocator->lpVtbl->m_realWbemObj.pItf = (IUnknown*)*ppvObject;
				*ppvObject = p_idIWbemLocator;
			}
		}
	}
	else if (IsEqualIIID(riid, &IID_idIWbemPath))
	{
		p_idIWbemPath = (idIWbemPath *)malloc(sizeof(idIWbemPath));
		if (p_idIWbemPath)
		{
			p_idIWbemPath->lpVtbl = (struct idIWbemPathVtbl *)malloc(sizeof(idIWbemPathVtbl));
			if (p_idIWbemPath->lpVtbl)
			{
				p_idIWbemPath->lpVtbl->AddRef = WbemPathAddRef;
				p_idIWbemPath->lpVtbl->CreateClassPart = WbemPathCreateClassPart;
				p_idIWbemPath->lpVtbl->DeleteClassPart = WbemPathDeleteClassPart;
				p_idIWbemPath->lpVtbl->GetClassNameW = WbemPathGetClassName;
				p_idIWbemPath->lpVtbl->GetInfo = WbemPathGetInfo;
				p_idIWbemPath->lpVtbl->GetKeyList = WbemPathGetKeyList;
				p_idIWbemPath->lpVtbl->GetNamespaceAt = WbemPathGetNamespaceAt;
				p_idIWbemPath->lpVtbl->GetNamespaceCount = WbemPathGetNamespaceCount;
				p_idIWbemPath->lpVtbl->GetScope = WbemPathGetScope;
				p_idIWbemPath->lpVtbl->GetScopeAsText = WbemPathGetScopeAsText;
				p_idIWbemPath->lpVtbl->GetScopeCount = WbemPathGetScopeCount;
				p_idIWbemPath->lpVtbl->GetServer = WbemPathGetServer;
				p_idIWbemPath->lpVtbl->GetText = WbemPathGetText;
				p_idIWbemPath->lpVtbl->IsLocal = WbemPathIsLocal;
				p_idIWbemPath->lpVtbl->IsRelative = WbemPathIsRelative;
				p_idIWbemPath->lpVtbl->IsRelativeOrChild = WbemPathIsRelativeOrChild;
				p_idIWbemPath->lpVtbl->IsSameClassName = WbemPathIsSameClassName;
				p_idIWbemPath->lpVtbl->QueryInterface = WbemPathQueryInterface;
				p_idIWbemPath->lpVtbl->Release = WbemPathRelease;
				p_idIWbemPath->lpVtbl->RemoveAllNamespaces = WbemPathRemoveAllNamespaces;
				p_idIWbemPath->lpVtbl->RemoveAllScopes = WbemPathRemoveAllScopes;
				p_idIWbemPath->lpVtbl->RemoveNamespaceAt = WbemPathRemoveNamespaceAt;
				p_idIWbemPath->lpVtbl->RemoveScope = WbemPathRemoveScope;
				p_idIWbemPath->lpVtbl->SetClassName = WbemPathSetClassName;
				p_idIWbemPath->lpVtbl->SetNamespaceAt = WbemPathSetNamespaceAt;
				p_idIWbemPath->lpVtbl->SetScope = WbemPathSetScope;
				p_idIWbemPath->lpVtbl->SetScopeFromText = WbemPathSetScopeFromText;
				p_idIWbemPath->lpVtbl->SetServer = WbemPathSetServer;
				p_idIWbemPath->lpVtbl->SetText = WbemPathSetText;
				p_idIWbemPath->lpVtbl->m_realIWbemPathObj.hr = APPX_E_INTERLEAVING_NOT_ALLOWED;
				p_idIWbemPath->lpVtbl->m_realIWbemPathObj.idIID = *riid;
				p_idIWbemPath->lpVtbl->m_realIWbemPathObj.queryIID = *queryIID;
				p_idIWbemPath->lpVtbl->m_realIWbemPathObj.pItf = (IUnknown*)*ppvObject;
				*ppvObject = p_idIWbemPath;
			}
		}
	}
	else if (IsEqualIIID(riid, &IID_idIClassFactory))
	{
		p_idClassFactory = (idIClassFactory *)malloc(sizeof(idIClassFactory));
		if (p_idClassFactory)
		{
			p_idClassFactory->lpVtbl = (struct idIClassFactoryVtbl *)malloc(sizeof(idIClassFactoryVtbl));
			if (p_idClassFactory->lpVtbl)
			{
				p_idClassFactory->lpVtbl->AddRef = FactoryAddRef;
				p_idClassFactory->lpVtbl->CreateInstance = FactoryCreateInstance;
				p_idClassFactory->lpVtbl->LockServer = FactoryLockServer;
				p_idClassFactory->lpVtbl->QueryInterface = FactoryQueryInterface;
				p_idClassFactory->lpVtbl->Release = FactoryRelease;
				//p_idClassFactory->lpVtbl->m_realFactoryObj.hr = APPX_E_INTERLEAVING_NOT_ALLOWED;
				//p_idClassFactory->lpVtbl->m_realFactoryObj.idIID = *riid;
				p_idClassFactory->lpVtbl->m_realFactoryObj = (IUnknown*)*ppvObject;
				*ppvObject = p_idClassFactory;
			}
		}
	}
	else if (IsEqualIIID(riid, &IID_idIClassFactory2))
	{
		p_idIClassFactory2 = (idIClassFactory2 *)malloc(sizeof(idIClassFactory2));
		if (p_idIClassFactory2)
		{
			p_idIClassFactory2->lpVtbl = (struct idIClassFactory2Vtbl *)malloc(sizeof(idIClassFactory2Vtbl));
			if (p_idIClassFactory2->lpVtbl)
			{
				p_idIClassFactory2->lpVtbl->AddRef = Factory2AddRef;
				p_idIClassFactory2->lpVtbl->CreateInstance = Factory2CreateInstance;
				p_idIClassFactory2->lpVtbl->LockServer = Factory2LockServer;
				p_idIClassFactory2->lpVtbl->QueryInterface = Factory2QueryInterface;
				p_idIClassFactory2->lpVtbl->Release = Factory2Release;
				p_idIClassFactory2->lpVtbl->GetLicInfo = Factory2GetLicInfo;
				p_idIClassFactory2->lpVtbl->RequestLicKey = Factory2RequestLicKey;
				p_idIClassFactory2->lpVtbl->CreateInstanceLic = Factory2CreateInstanceLic;
				p_idIClassFactory2->lpVtbl->m_realFactory2Obj.hr = APPX_E_INTERLEAVING_NOT_ALLOWED;
				p_idIClassFactory2->lpVtbl->m_realFactory2Obj.idIID = *riid;
				p_idIClassFactory2->lpVtbl->m_realFactory2Obj.queryIID = *queryIID;
				p_idIClassFactory2->lpVtbl->m_realFactory2Obj.pItf = (IUnknown*)*ppvObject;
				*ppvObject = p_idIClassFactory2;
			}
		}
	}
	else if (IsEqualIIID(riid, &IID_idIWbemServices))
	{
		p_idIWbemServices = (idIWbemServices *)malloc(sizeof(idIWbemServices));
		if (p_idIWbemServices)
		{
			p_idIWbemServices->lpVtbl = (struct idIWbemServicesVtbl *)malloc(sizeof(idIWbemServicesVtbl));
			if (p_idIWbemServices->lpVtbl)
			{
				p_idIWbemServices->lpVtbl->AddRef = WbemServicesAddRef;
				p_idIWbemServices->lpVtbl->CancelAsyncCall = WbemServicesCancelAsyncCall;
				p_idIWbemServices->lpVtbl->CreateClassEnum = WbemServicesCreateClassEnum;
				p_idIWbemServices->lpVtbl->CreateClassEnumAsync = WbemServicesCreateClassEnumAsync;
				p_idIWbemServices->lpVtbl->CreateInstanceEnum = WbemServicesCreateInstanceEnum;
				p_idIWbemServices->lpVtbl->CreateInstanceEnumAsync = WbemServicesCreateInstanceEnumAsync;
				p_idIWbemServices->lpVtbl->DeleteClass = WbemServicesDeleteClass;
				p_idIWbemServices->lpVtbl->DeleteClassAsync = WbemServicesDeleteClassAsync;
				p_idIWbemServices->lpVtbl->DeleteInstance = WbemServicesDeleteInstance;
				p_idIWbemServices->lpVtbl->DeleteInstanceAsync = WbemServicesDeleteInstanceAsync;
				p_idIWbemServices->lpVtbl->ExecMethod = WbemServicesExecMethod;
				p_idIWbemServices->lpVtbl->ExecMethodAsync = WbemServicesExecMethodAsync;
				p_idIWbemServices->lpVtbl->ExecNotificationQuery = WbemServicesExecNotificationQuery;
				p_idIWbemServices->lpVtbl->ExecNotificationQueryAsync = WbemServicesExecNotificationQueryAsync;
				p_idIWbemServices->lpVtbl->ExecQuery = WbemServicesExecQuery;
				p_idIWbemServices->lpVtbl->ExecQueryAsync = WbemServicesExecQueryAsync;
				p_idIWbemServices->lpVtbl->GetObjectAsync = WbemServicesGetObjectAsync;
				p_idIWbemServices->lpVtbl->GetObjectW = WbemServicesGetObject;
				p_idIWbemServices->lpVtbl->OpenNamespace = WbemServicesOpenNamespace;
				p_idIWbemServices->lpVtbl->PutClass = WbemServicesPutClass;
				p_idIWbemServices->lpVtbl->PutClassAsync = WbemServicesPutClassAsync;
				p_idIWbemServices->lpVtbl->PutInstance = WbemServicesPutInstance;
				p_idIWbemServices->lpVtbl->PutInstanceAsync = WbemServicesPutInstanceAsync;
				p_idIWbemServices->lpVtbl->QueryInterface = WbemServicesQueryInterface;
				p_idIWbemServices->lpVtbl->QueryObjectSink = WbemServicesQueryObjectSink;
				p_idIWbemServices->lpVtbl->Release = WbemServicesRelease;
				p_idIWbemServices->lpVtbl->m_realWbemServicesObj.hr = APPX_E_INTERLEAVING_NOT_ALLOWED;
				p_idIWbemServices->lpVtbl->m_realWbemServicesObj.idIID = *riid;
				p_idIWbemServices->lpVtbl->m_realWbemServicesObj.queryIID = *queryIID;
				p_idIWbemServices->lpVtbl->m_realWbemServicesObj.pItf = (IUnknown*)*ppvObject;
				*ppvObject = p_idIWbemServices;
			}
		}
	}
	else if (IsEqualIIID(riid, &IID_idIEnumWbemClassObject))
	{
		p_idIEnumWbemClassObject = (idIEnumWbemClassObject *)malloc(sizeof(idIEnumWbemClassObject));
		if (p_idIEnumWbemClassObject)
		{
			p_idIEnumWbemClassObject->lpVtbl = (struct idIEnumWbemClassObjectVtbl *)malloc(sizeof(idIEnumWbemClassObjectVtbl));
			if (p_idIEnumWbemClassObject->lpVtbl)
			{
				p_idIEnumWbemClassObject->lpVtbl->AddRef = EnumWbemClassObjectAddRef;
				p_idIEnumWbemClassObject->lpVtbl->Clone = EnumWbemClassObjectClone;
				p_idIEnumWbemClassObject->lpVtbl->Next = EnumWbemClassObjectNext;
				p_idIEnumWbemClassObject->lpVtbl->NextAsync = EnumWbemClassObjectNextAsync;
				p_idIEnumWbemClassObject->lpVtbl->QueryInterface = EnumWbemClassObjectQueryInterface;
				p_idIEnumWbemClassObject->lpVtbl->Release = EnumWbemClassObjectRelease;
				p_idIEnumWbemClassObject->lpVtbl->Reset = EnumWbemClassObjectReset;
				p_idIEnumWbemClassObject->lpVtbl->Skip = EnumWbemClassObjectSkip;
				p_idIEnumWbemClassObject->lpVtbl->m_realEnumWbemClassObjectObj.hr = APPX_E_INTERLEAVING_NOT_ALLOWED;
				p_idIEnumWbemClassObject->lpVtbl->m_nItem = 0;
				p_idIEnumWbemClassObject->lpVtbl->m_realEnumWbemClassObjectObj.idIID = *riid;
				p_idIEnumWbemClassObject->lpVtbl->m_realEnumWbemClassObjectObj.queryIID = *queryIID;
				p_idIEnumWbemClassObject->lpVtbl->m_realEnumWbemClassObjectObj.pItf = (IUnknown*)*ppvObject;
				*ppvObject = p_idIEnumWbemClassObject;
			}
		}
	}
	else if (IsEqualIIID(riid, &IID_idIWbemClassObject))
	{
		p_idIWbemClassObject = (idIWbemClassObject *)malloc(sizeof(idIWbemClassObject));
		if (p_idIWbemClassObject)
		{
			p_idIWbemClassObject->lpVtbl = (struct idIWbemClassObjectVtbl *)malloc(sizeof(idIWbemClassObjectVtbl));
			if (p_idIWbemClassObject->lpVtbl)
			{
				memset(p_idIWbemClassObject->lpVtbl, 0, sizeof(idIWbemClassObjectVtbl));
				p_idIWbemClassObject->lpVtbl->AddRef = WbemClassObjectAddRef;
				p_idIWbemClassObject->lpVtbl->BeginEnumeration = WbemClassObjectBeginEnumeration;
				p_idIWbemClassObject->lpVtbl->BeginMethodEnumeration = WbemClassObjectBeginMethodEnumeration;
				p_idIWbemClassObject->lpVtbl->Clone = WbemClassObjectClone;
				p_idIWbemClassObject->lpVtbl->CompareTo = WbemClassObjectCompareTo;
				p_idIWbemClassObject->lpVtbl->Delete = WbemClassObjectDelete;
				p_idIWbemClassObject->lpVtbl->DeleteMethod = WbemClassObjectDeleteMethod;
				p_idIWbemClassObject->lpVtbl->EndEnumeration = WbemClassObjectEndEnumeration;
				p_idIWbemClassObject->lpVtbl->EndMethodEnumeration = WbemClassObjectEndMethodEnumeration;
				p_idIWbemClassObject->lpVtbl->Get = WbemClassObjectGet;
				p_idIWbemClassObject->lpVtbl->GetMethod = WbemClassObjectGetMethod;
				p_idIWbemClassObject->lpVtbl->GetMethodOrigin = WbemClassObjectGetMethodOrigin;
				p_idIWbemClassObject->lpVtbl->GetMethodQualifierSet = WbemClassObjectGetMethodQualifierSet;
				p_idIWbemClassObject->lpVtbl->GetNames = WbemClassObjectGetNames;
				p_idIWbemClassObject->lpVtbl->GetObjectText = WbemClassObjectGetObjectText;
				p_idIWbemClassObject->lpVtbl->GetPropertyOrigin = WbemClassObjectGetPropertyOrigin;
				p_idIWbemClassObject->lpVtbl->GetPropertyQualifierSet = WbemClassObjectGetPropertyQualifierSet;
				p_idIWbemClassObject->lpVtbl->GetQualifierSet = WbemClassObjectGetQualifierSet;
				p_idIWbemClassObject->lpVtbl->InheritsFrom = WbemClassObjectInheritsFrom;
				p_idIWbemClassObject->lpVtbl->Next = WbemClassObjectNext;
				p_idIWbemClassObject->lpVtbl->NextMethod = WbemClassObjectNextMethod;
				p_idIWbemClassObject->lpVtbl->Put = WbemClassObjectPut;
				p_idIWbemClassObject->lpVtbl->PutMethod = WbemClassObjectPutMethod;
				p_idIWbemClassObject->lpVtbl->QueryInterface = WbemClassObjectQueryInterface;
				p_idIWbemClassObject->lpVtbl->Release = WbemClassObjectRelease;
				p_idIWbemClassObject->lpVtbl->SpawnDerivedClass = WbemClassObjectSpawnDerivedClass;
				p_idIWbemClassObject->lpVtbl->SpawnInstance = WbemClassObjectSpawnInstance;
				p_idIWbemClassObject->lpVtbl->m_realWbemClassObjectObj.hr = APPX_E_INTERLEAVING_NOT_ALLOWED;
				p_idIWbemClassObject->lpVtbl->m_realWbemClassObjectObj.idIID = *riid;
				p_idIWbemClassObject->lpVtbl->m_realWbemClassObjectObj.queryIID = *queryIID;
				p_idIWbemClassObject->lpVtbl->m_realWbemClassObjectObj.pItf = (IUnknown*)*ppvObject;
				
				if (pThis != NULL)
				{
					p_idIWbemClassObject->lpVtbl->m_oleObject = ((idIWbemClassObject*)pThis)->lpVtbl->m_oleObject;
				}
				else if (g_oleConfig != nullptr)
				{
					std::wstring ObjectName = GetStringFromGUID(*queryIID);
					if (ObjectName.length() > 0)
					{
						int Ordinal = g_oleConfig->GetObjectOrdinal(ObjectName);
						if (g_oleConfig->CheckOrdinal(Ordinal))
						{
							COleObject* _Object = g_oleConfig->GetOleObject(Ordinal);
							if (_Object)
							{
								p_idIWbemClassObject->lpVtbl->m_oleObject = *_Object;
								if (p_idIWbemClassObject->lpVtbl->m_oleObject.GetMemberHandle() == HandleDup)
								{
									p_idIWbemClassObject->lpVtbl->m_oleObject.AddMemberHandle(HandleNon);
								}
							}
						}
						else
						{
							p_idIWbemClassObject->lpVtbl->m_oleObject.SetObjectName(ObjectName);
						}
					}
				}

				*ppvObject = p_idIWbemClassObject;
			}
		}
	}
	else if (IsEqualIIID(riid, &IID_idIClientSecurity))
	{
		p_idIClientSecurity = (idIClientSecurity *)malloc(sizeof(idIClientSecurity));
		if (p_idIClientSecurity)
		{
			p_idIClientSecurity->lpVtbl = (struct idIClientSecurityVtbl *)malloc(sizeof(idIClientSecurityVtbl));
			if (p_idIClientSecurity->lpVtbl)
			{
				p_idIClientSecurity->lpVtbl->AddRef = ClientSecurityAddRef;
				p_idIClientSecurity->lpVtbl->CopyProxy = ClientSecurityCopyProxy;
				p_idIClientSecurity->lpVtbl->QueryBlanket = ClientSecurityQueryBlanket;
				p_idIClientSecurity->lpVtbl->QueryInterface = ClientSecurityQueryInterface;
				p_idIClientSecurity->lpVtbl->Release = ClientSecurityRelease;
				p_idIClientSecurity->lpVtbl->SetBlanket = ClientSecuritySetBlanket;
				p_idIClientSecurity->lpVtbl->m_realClientSecurityObj.hr = APPX_E_INTERLEAVING_NOT_ALLOWED;
				p_idIClientSecurity->lpVtbl->m_realClientSecurityObj.idIID = *riid;
				p_idIClientSecurity->lpVtbl->m_realClientSecurityObj.queryIID = *queryIID;
				p_idIClientSecurity->lpVtbl->m_realClientSecurityObj.pItf = (IUnknown*)*ppvObject;
				*ppvObject = p_idIClientSecurity;
			}
		}
	}
	else if (IsEqualIIID(riid, &IID_idIWbemCallResult))
	{
		p_idIWbemCallResult = (idIWbemCallResult *)malloc(sizeof(idIWbemCallResult));
		if (p_idIWbemCallResult)
		{
			p_idIWbemCallResult->lpVtbl = (struct idIWbemCallResultVtbl *)malloc(sizeof(idIWbemCallResultVtbl));
			if (p_idIWbemCallResult->lpVtbl)
			{
				p_idIWbemCallResult->lpVtbl->AddRef = WbemCallResultAddRef;
				p_idIWbemCallResult->lpVtbl->GetCallStatus = WbemCallResultGetCallStatus;
				p_idIWbemCallResult->lpVtbl->GetResultObject = WbemCallResultGetResultObject;
				p_idIWbemCallResult->lpVtbl->GetResultServices = WbemCallResultGetResultServices;
				p_idIWbemCallResult->lpVtbl->GetResultString = WbemCallResultGetResultString;
				p_idIWbemCallResult->lpVtbl->QueryInterface = WbemCallResultQueryInterface;
				p_idIWbemCallResult->lpVtbl->Release = WbemCallResultRelease;
				p_idIWbemCallResult->lpVtbl->m_realWbemCallResultObj.hr = APPX_E_INTERLEAVING_NOT_ALLOWED;
				p_idIWbemCallResult->lpVtbl->m_realWbemCallResultObj.idIID = *riid;
				p_idIWbemCallResult->lpVtbl->m_realWbemCallResultObj.queryIID = *queryIID;
				p_idIWbemCallResult->lpVtbl->m_realWbemCallResultObj.pItf = (IUnknown*)*ppvObject;
				*ppvObject = p_idIWbemCallResult;
			}
		}
	}
	else
	{

	}


	return;
}

std::wstring A2W(std::string str)
{
	std::wstring str_w;
	str_w.resize(str.length());
	for (int x = 0; x<str.length(); ++x)
	{
		str_w[x] = str[x];
	}
	return str_w;
}

std::string W2A(std::wstring str)
{
	std::string str_a;
	str_a.resize(str.length());
	for (int x = 0; x<str.length(); ++x)
	{
		str_a[x] = str[x];
	}
	return str_a;
}

bool GetValueString(VARIANT& Value, std::wstring& ValueString)
{
	switch (Value.vt)
	{
	case VT_BSTR:
		ValueString = Value.bstrVal;
		return true;
	case VT_BOOL:
		if (Value.boolVal)
			ValueString = L"true";
		else
			ValueString = L"false";
		return true;
	case VT_UI1:
	case VT_I1:
		ValueString = std::to_wstring(Value.bVal);
		return true;
	case VT_I2:
	case VT_INT:
		ValueString = std::to_wstring(Value.iVal);
		return true;
	case VT_I4:
		ValueString = std::to_wstring(Value.intVal);
		return true;
	case VT_I8:
		ValueString = std::to_wstring(Value.llVal);
		return true;
	case VT_UI2:
	case VT_UINT:
		ValueString = std::to_wstring(Value.uiVal);
		return true;
	case VT_UI4:
		ValueString = std::to_wstring(Value.ulVal);
		return true;
	case VT_UI8:
		ValueString = std::to_wstring(Value.ullVal);
		return true;
	case VT_DATE:
		ValueString = std::to_wstring(Value.date);
		return true;
	case VT_VOID:
		ValueString = std::to_wstring((ULONG_PTR)Value.byref);
		return true;
	default:
		break;
	}

	return false;
}

bool SendToTracer(PVOID stream, ULONG length)
{
	if (g_CommunicationWnd == NULL)
	{
		g_CommunicationWnd = FindWindow(WND_UI_NAME, NULL);
	}

	if (g_CommunicationWnd == NULL)
	{
		return false;
	}

	PVOID pMyStruct = malloc(length);
	memcpy(pMyStruct, stream, length);

	COPYDATASTRUCT cds;
	cds.cbData = length;
	cds.lpData = pMyStruct;

	// Send the COPYDATASTRUCT struct through the WM_COPYDATA message to  
	// the receiving window. (The application must use SendMessage,  
	// instead of PostMessage to send WM_COPYDATA because the receiving  
	// application must accept while it is guaranteed to be valid.) 
	SendMessage(g_CommunicationWnd, WM_COPYDATA, NULL, (LPARAM)&cds);
	
	if(pMyStruct)
	{
		free(pMyStruct);
	}

	return true;
}

void WmiInterfaceQueryLog(int ClassId, std::wstring Properties, VARIANT& Value)
{
	//void* buffer = NULL;
	//size_t  blen = 0;
	//std::wstring ValueString;

	//if (GetValueString(Value, ValueString))
	//{
	//	WmiInterfaceQueryTracerLog log((HANDLE)GetCurrentProcessId(), (HANDLE)GetCurrentThreadId(), ClassId, Properties.c_str(), ValueString.c_str());
	//	log.Save(buffer, blen);
	//	SendToTracer(buffer, blen);
	//}
	//else
	//{
	//	DbgPrintLog(L"WmiInterfaceQueryLog: because Value.vt not support, so convert value failed!");
	//}
	return;
}

void DbgPrintLog(std::wstring notice_string)
{
//	void* buffer = NULL;
//	size_t  blen = 0;
//
//	DebugTracerLog log((HANDLE)GetCurrentProcessId(), (HANDLE)GetCurrentThreadId(), notice_string);
//	log.Save(buffer, blen);
//
//	SendToTracer(buffer, blen);
}

bool ExtractResource(HMODULE ModuleHandle, TCHAR const * ResourceName,
	TCHAR const * ResourceId, std::vector<BYTE>& ResoureBuffer)
{
	if (ModuleHandle == NULL)
	{
		ModuleHandle = GetModuleHandle(NULL);
	}

	HRSRC resourceInfo = FindResource(ModuleHandle, ResourceId, ResourceName);
	if (resourceInfo == NULL)
	{
		DWORD const lastError = GetLastError();
		return false;
	}

	DWORD const resourceSize = SizeofResource(ModuleHandle, resourceInfo);
	if (resourceSize == 0)
	{
		DWORD const lastError = GetLastError();
		return false;
	}

	HGLOBAL resourceHandle = LoadResource(ModuleHandle, resourceInfo);
	if (resourceHandle == NULL)
	{
		DWORD const lastError = GetLastError();
		return false;
	}

	void * resourceData = LockResource(resourceHandle);
	if (resourceData == NULL)
	{
		DWORD const lastError = GetLastError();
		return false;
	}

	ResoureBuffer.resize(resourceSize);
	memcpy_s(&ResoureBuffer[0], ResoureBuffer.size(), resourceData, resourceSize);

	return true;
}

