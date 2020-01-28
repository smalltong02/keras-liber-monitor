#pragma once
#include <stdio.h>
#include <windows.h>
#include <guiddef.h>
#include <WbemCli.h>
#include <WMIUtils.h>
#include <OCIdl.h>
#include <winioctl.h>
#include <string>
#include <vector>
#include "DriverMgr.h"

#define PARAM_TYPES "Type"
#define PARAM_HANDLES "Handle"
#define PARAM_VALUE "Value"

#define TYPE_STRING	"String"
#define TYPE_UINT32 "UINT32"
#define TYPE_UINT64 "UINT64"
#define TYPE_INT32	"INT32"
#define TYPE_INT64	"INT64"
#define TYPE_BYTE	"BYTE"
#define TYPE_UINT16 "UINT16"
#define TYPE_INT16	"INT16"
#define TYPE_FLOAT	"FLOAT"
#define TYPE_DATE	"DATE"
#define TYPE_PVOID	"VOID"

#define HANDLE_NON "Non"
#define HANDLE_LOG "Log"
#define HANDLE_ADDT "AddTarget"
#define HANDLE_MODIFY "Modify"
#define HANDLE_DUP "Dup"

typedef enum _ParamType
{
	TypeNon,
	TypeByte,
	TypeString,
	TypeUint16,
	TypeUint32,
	TypeUint64,
	TypeInt16,
	TypeInt32,
	TypeInt64,
	TypeFloat,
	TypeDouble,
	TypePvoid,
	TypeExpand,
} ParamType;

typedef enum _PropertyHandle
{
	HandleNon,
	HandleLog,
	HandleModify,
	HandleAddT,
	HandleDup,
} PropertyHandle;

class COleObject
{
public:
	typedef struct _Property
	{
		PropertyHandle Handle;
		std::wstring PropertyName;
		VARIANT Value;
	} Property;

	typedef struct _Method
	{
		PropertyHandle Handle;
		std::wstring MethodName;
		std::vector<Property> Params;
	} Method;

	typedef struct _Member
	{
		PropertyHandle Handle;
		std::vector<Method> Methods;
		std::vector<Property> Properties;
	} Member;

	typedef struct _ComObject
	{
		std::wstring ObjectName;
		Member Members;
	} ComObject;

	COleObject() { m_bValid = false; }
	~COleObject() { ; }

	// Object function
	void SetObjectName(std::wstring Name) { m_Object.ObjectName = Name; }
	std::wstring GetObjectName() { return m_Object.ObjectName; }
	void Active() { m_bValid = true; }
	bool IsValidObject() { return m_bValid; }

	// Element function
	bool AddProperty(Property& _Property);
	bool AddMethod(Method& _Method);
	bool DeleteProperty(std::wstring& PropertyName);
	int GetPropertyCount() { return m_Object.Members.Properties.size(); }
	int GetMethodCount() { return m_Object.Members.Methods.size(); }
	VARTYPE GetPropertyType(int PropertyOrd);
	VARTYPE GetPropertyType(std::wstring& PropertyName);
	PropertyHandle GetPropertyHandle(int PropertyOrd);
	PropertyHandle GetPropertyHandle(std::wstring& PropertyName);
	bool GetPropertyValue(int PropertyOrd, VARIANT& Value);
	bool GetPropertyValue(std::wstring& PropertyName, VARIANT& Value);
	bool SetPropertyValue(int PropertyOrd, VARIANT& Value);
	bool SetPropertyValue(std::wstring& PropertyName, VARIANT& Value);
	bool GetPropertyName(int PropertyOrd, std::wstring& Name);

	// Member function
	void AddMemberHandle(PropertyHandle _Handle) { m_Object.Members.Handle = _Handle; }
	PropertyHandle GetMemberHandle() { return m_Object.Members.Handle; }

	// Method function
	bool GetMethodName(int MethodOrd, std::wstring& Name);
	int GetMethodOrdinal(std::wstring MethodName);
	PropertyHandle GetMethodHandle(int MethodOrd);
	PropertyHandle GetMethodHandle(std::wstring MethodName);
	int GetMethodParamCount(int MethodOrd);
	int GetMethodParamCount(std::wstring MethodName);
	bool GetMethodParamName(int MethodOrd, int ParamOrd, std::wstring& Name);
	bool GetMethodParamName(std::wstring MethodName, int ParamOrd, std::wstring& Name);
	VARTYPE GetMethodParamType(int MethodOrd, int ParamOrd);
	VARTYPE GetMethodParamType(std::wstring MethodName, int ParamOrd);
	PropertyHandle GetMethodParamHandle(int MethodOrd, int ParamOrd);
	PropertyHandle GetMethodParamHandle(std::wstring MethodName, int ParamOrd);

	PropertyHandle GetParamHandle(std::string HandleString);
	VARTYPE GetParamType(std::string TypeStr);
	bool GetExpandType(std::string TypeStr);

	//
	bool FindHandleInList(PropertyHandle& _Handle, std::vector<PropertyHandle>& _List);
	bool GetSameHandleParamNameList(int MethodOrd, std::vector<PropertyHandle>& _HandleList, std::vector<std::wstring>& ParamList);

	static const int InvalidOrdinal = -1;
private:
	ComObject m_Object;
	bool m_bValid;
};

class COleConfig
{
public:
	COleConfig(void* initData, int len);
	~COleConfig();

	void SetDriverMgr(CDriverMgr* pMgr) { m_driverMgr = pMgr; }
	CDriverMgr* GetDriverMgr() { return m_driverMgr; }
	int GetObjectOrdinal(std::wstring& ObjectName);
	int GetObjectCount() { return m_ObjectArray.size(); }
	bool CheckOrdinal(int Ordinal) { return Ordinal < (int)m_ObjectArray.size(); }
	int AddObject(COleObject& _Object);
	COleObject* GetOleObject(int Ordinal);

	static const int InvalidOrdinal = -1;
private:
	std::vector<COleObject> m_ObjectArray;
	CDriverMgr* m_driverMgr;
};


