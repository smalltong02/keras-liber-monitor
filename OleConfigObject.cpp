#include "stdafx.h"
#include "OleConfigObject.h"
#include "utils.h"

COleConfig::COleConfig(void* initData, int len)
{
	m_driverMgr = nullptr;

	if (!initData) return;

	//Json::Value root;
	//Json::Reader reader;
	//std::string jason_init_data = std::string((char*)initData, len);

	//if (!reader.parse(jason_init_data, root) || !root.isObject() || root.isNull())
	//{
	//	//config data is incorrect.
	//	return;
	//}

	//Json::Value::Members root_members = root.getMemberNames();

	//for (int root_count = 0; root_count < root_members.size(); root_count++)
	//{
	//	Json::Value jason_Object = root[root_members[root_count]];

	//	if (!jason_Object.isObject() || jason_Object.isNull())
	//	{
	//		continue;
	//	}

	//	COleObject _OleObject;
	//	_OleObject.SetObjectName(A2W(root_members[root_count]));
	//	_OleObject.AddMemberHandle(HandleNon);
	//	Json::Value::Members member_members = jason_Object.getMemberNames();

	//	for (int member_count = 0; member_count < member_members.size(); member_count++)
	//	{
	//		if (stricmp(member_members[member_count].c_str(), "Data") == 0)
	//		{
	//			Json::Value data_Object = jason_Object[member_members[member_count]];

	//			if (!data_Object.isObject() || data_Object.isNull())
	//			{
	//				continue;
	//			}

	//			Json::Value::Members data_members = data_Object.getMemberNames();

	//			for (int data_count = 0; data_count < data_members.size(); data_count++)
	//			{
	//				if (stricmp(data_members[data_count].c_str(), PARAM_HANDLES) == 0)
	//				{
	//					Json::Value PropertyValue = data_Object[data_members[data_count]];

	//					PropertyHandle _Handle = _OleObject.GetParamHandle(PropertyValue.asString().c_str());
	//					_OleObject.AddMemberHandle(_Handle);
	//				}
	//				else
	//				{
	//					COleObject::Property _Property;
	//					_Property.PropertyName = A2W(data_members[data_count]);
	//					_Property.Value.vt = VT_EMPTY;
	//					_Property.Handle = HandleNon;
	//					Json::Value PropertyValue = data_Object[data_members[data_count]];

	//					if (!PropertyValue[PARAM_HANDLES].isNull())
	//					{
	//						_Property.Handle = _OleObject.GetParamHandle(PropertyValue[PARAM_HANDLES].asString());
	//					}

	//					if (!PropertyValue[PARAM_VALUE].isNull())
	//					{
	//						if (PropertyValue[PARAM_VALUE].isString())
	//						{
	//							if (stricmp(PropertyValue[PARAM_VALUE].asString().c_str(), "VT_EMPTY") == 0)
	//							{
	//								_Property.Value.vt = VT_EMPTY;
	//							}
	//							else if (stricmp(PropertyValue[PARAM_VALUE].asString().c_str(), "VT_NULL") == 0)
	//							{
	//								_Property.Value.vt = VT_NULL;
	//							}
	//							else
	//							{
	//								_Property.Value.vt = VT_BSTR;
	//								_Property.Value.bstrVal = SysAllocString(A2W(PropertyValue[PARAM_VALUE].asString()).c_str());
	//							}
	//						}
	//						else if (PropertyValue[PARAM_VALUE].isInt())
	//						{
	//							_Property.Value.vt = VT_I4;
	//							_Property.Value.intVal = PropertyValue[PARAM_VALUE].asInt();
	//						}
	//					}
	//					_OleObject.AddProperty(_Property);
	//				}
	//			}
	//		}
	//		else if (stricmp(member_members[member_count].c_str(), "Method") == 0)
	//		{
	//			Json::Value methodValue = jason_Object[member_members[member_count]];

	//			if (!methodValue.isObject() || methodValue.isNull())
	//			{
	//				continue;
	//			}

	//			Json::Value::Members method_members = methodValue.getMemberNames();

	//			for (int method_count = 0; method_count < method_members.size(); method_count++)
	//			{
	//				COleObject::Method _Method;
	//				_Method.MethodName = A2W(method_members[method_count]);

	//				Json::Value paramsArray = methodValue[method_members[method_count]];

	//				if (!paramsArray.isObject() || paramsArray.isNull())
	//				{
	//					continue;
	//				}

	//				Json::Value::Members params_members = paramsArray.getMemberNames();

	//				_Method.Handle = HandleNon;
	//				for (int param_count = 0; param_count < params_members.size(); param_count++)
	//				{
	//					if (stricmp(params_members[param_count].c_str(), PARAM_HANDLES) == 0)
	//					{
	//						_Method.Handle = _OleObject.GetParamHandle(paramsArray[params_members[param_count]].asString());
	//					}
	//					else
	//					{
	//						COleObject::Property _Property;
	//						_Property.PropertyName = A2W(params_members[param_count]);
	//						_Property.Value.vt = VT_EMPTY;
	//						_Property.Handle = HandleNon;

	//						Json::Value paramValue = paramsArray[params_members[param_count]];

	//						if (!paramValue[PARAM_TYPES].isNull())
	//						{
	//							_Property.Value.vt = _OleObject.GetParamType(paramValue[PARAM_TYPES].asString());
	//						}

	//						if (!paramValue[PARAM_HANDLES].isNull())
	//						{
	//							_Property.Handle = _OleObject.GetParamHandle(paramValue[PARAM_HANDLES].asString());
	//						}
	//						_Method.Params.push_back(_Property);
	//					}
	//				}

	//				_OleObject.AddMethod(_Method);
	//			}
	//		}
	//	}
	//	_OleObject.Active();
	//	AddObject(_OleObject);
	//}
	return;
}

COleConfig::~COleConfig()
{
	return;
}

int COleConfig::AddObject(COleObject& _Object)
{
	if (_Object.GetObjectName().length() == 0) return COleObject::InvalidOrdinal;

	m_ObjectArray.push_back(_Object);
	return (m_ObjectArray.size() - 1);
}

int COleConfig::GetObjectOrdinal(std::wstring& ObjectName)
{
	int Ordinal = 0;
	std::vector<COleObject>::iterator it_begin = m_ObjectArray.begin();
	std::vector<COleObject>::iterator it_end = m_ObjectArray.end();

	if (ObjectName.length() != 0)
	{
		for (; it_begin != it_end; it_begin++)
		{
			if (it_begin->GetObjectName().length() != 0 && wcsicmp(it_begin->GetObjectName().c_str(), ObjectName.c_str()) == 0)
				return Ordinal;
			Ordinal++;
		}
	}
	return COleConfig::InvalidOrdinal;
}

COleObject* COleConfig::GetOleObject(int Ordinal)
{
	if (CheckOrdinal(Ordinal))
		return &m_ObjectArray[Ordinal];
	else
		return nullptr;
}

PropertyHandle COleObject::GetParamHandle(std::string HandleString)
{
	PropertyHandle _Handle = HandleNon;

	if (HandleString.length() == 0) return HandleNon;

	if (stricmp(HandleString.c_str(), HANDLE_NON) == 0)
		_Handle = HandleNon;
	else if (stricmp(HandleString.c_str(), HANDLE_LOG) == 0)
		_Handle = HandleLog;
	else if (stricmp(HandleString.c_str(), HANDLE_ADDT) == 0)
		_Handle = HandleAddT;
	else if (stricmp(HandleString.c_str(), HANDLE_MODIFY) == 0)
		_Handle = HandleModify;
	else if (stricmp(HandleString.c_str(), HANDLE_DUP) == 0)
		_Handle = HandleDup;

	return _Handle;
}

VARTYPE COleObject::GetParamType(std::string TypeStr)
{
	VARTYPE _Type = VT_ILLEGAL;

	if (TypeStr.length() == 0) return TypeNon;

	if (stricmp(TypeStr.c_str(), TYPE_STRING) == 0)
		_Type = VT_BSTR;
	else if (stricmp(TypeStr.c_str(), TYPE_UINT16) == 0)
		_Type = VT_UI2;
	else if (stricmp(TypeStr.c_str(), TYPE_UINT32) == 0)
		_Type = VT_UI4;
	else if (stricmp(TypeStr.c_str(), TYPE_UINT64) == 0)
		_Type = VT_UI8;
	else if (stricmp(TypeStr.c_str(), TYPE_INT16) == 0)
		_Type = VT_I2;
	else if (stricmp(TypeStr.c_str(), TYPE_INT32) == 0)
		_Type = VT_I4;
	else if (stricmp(TypeStr.c_str(), TYPE_INT64) == 0)
		_Type = VT_I8;
	else if (stricmp(TypeStr.c_str(), TYPE_BYTE) == 0)
		_Type = VT_I1;
	else if (stricmp(TypeStr.c_str(), TYPE_FLOAT) == 0)
		_Type = VT_ILLEGAL;
	else if (stricmp(TypeStr.c_str(), TYPE_DATE) == 0)
		_Type = VT_DATE;
	else if (stricmp(TypeStr.c_str(), TYPE_PVOID) == 0)
		_Type = VT_VOID;
	else if (GetExpandType(TypeStr))
		_Type = TypeExpand;

	return _Type;
}

bool COleObject::GetExpandType(std::string TypeStr)
{
	return false;
}

bool COleObject::AddProperty(Property& _Property)
{
	m_Object.Members.Properties.push_back(_Property);
	return true;
}

bool COleObject::DeleteProperty(std::wstring& PropertyName)
{
	return false;
}

VARTYPE COleObject::GetPropertyType(int PropertyOrd)
{
	if (PropertyOrd >= m_Object.Members.Properties.size())
		return VT_ILLEGAL;

	return m_Object.Members.Properties[PropertyOrd].Value.vt;
}

VARTYPE COleObject::GetPropertyType(std::wstring& PropertyName)
{
	for (int PropertyCount = 0; PropertyCount < m_Object.Members.Properties.size(); PropertyCount++)
	{
		if (wcsicmp(m_Object.Members.Properties[PropertyCount].PropertyName.c_str(), PropertyName.c_str()) == 0)
		{
			return m_Object.Members.Properties[PropertyCount].Value.vt;
		}
	}
	return VT_ILLEGAL;
}

PropertyHandle COleObject::GetPropertyHandle(int PropertyOrd)
{
	if (PropertyOrd >= m_Object.Members.Properties.size())
		return HandleNon;

	return m_Object.Members.Properties[PropertyOrd].Handle;
}

PropertyHandle COleObject::GetPropertyHandle(std::wstring& PropertyName)
{
	for (int PropertyCount = 0; PropertyCount < m_Object.Members.Properties.size(); PropertyCount++)
	{
		if (wcsicmp(m_Object.Members.Properties[PropertyCount].PropertyName.c_str(), PropertyName.c_str()) == 0)
		{
			return m_Object.Members.Properties[PropertyCount].Handle;
		}
	}
	return HandleNon;
}

bool COleObject::GetPropertyValue(int PropertyOrd, VARIANT& Value)
{
	if (PropertyOrd >= m_Object.Members.Properties.size())
		return false;

		Value = m_Object.Members.Properties[PropertyOrd].Value;
		if (m_Object.Members.Properties[PropertyOrd].Value.vt == VT_BSTR)
		{
			Value.bstrVal = SysAllocString(m_Object.Members.Properties[PropertyOrd].Value.bstrVal);
		}
		return true;
}

bool COleObject::GetPropertyValue(std::wstring& PropertyName, VARIANT& Value)
{
	Property* _Property = nullptr;

	for (int PropertyCount = 0; PropertyCount < m_Object.Members.Properties.size(); PropertyCount++)
	{
		if (wcsicmp(m_Object.Members.Properties[PropertyCount].PropertyName.c_str(), PropertyName.c_str()) == 0)
		{
			_Property = &m_Object.Members.Properties[PropertyCount];
			break;
		}
	}

	if (_Property)
	{
		Value = _Property->Value;
		if (_Property->Value.vt == VT_BSTR)
		{
			Value.bstrVal = SysAllocString(_Property->Value.bstrVal);
		}
		return true;
	}

	return false;
}

bool COleObject::SetPropertyValue(int PropertyOrd, VARIANT& Value)
{
	if (PropertyOrd >= m_Object.Members.Properties.size())
		return false;

	m_Object.Members.Properties[PropertyOrd].Value = Value;
	if (Value.vt == VT_BSTR)
	{
		m_Object.Members.Properties[PropertyOrd].Value.bstrVal = SysAllocString(Value.bstrVal);
	}
	return true;
}

bool COleObject::SetPropertyValue(std::wstring& PropertyName, VARIANT& Value)
{
	Property* _Property = nullptr;

	for (int PropertyCount = 0; PropertyCount < m_Object.Members.Properties.size(); PropertyCount++)
	{
		if (wcsicmp(m_Object.Members.Properties[PropertyCount].PropertyName.c_str(), PropertyName.c_str()) == 0)
		{
			_Property = &m_Object.Members.Properties[PropertyCount];
			break;
		}
	}

	if (_Property)
	{
		_Property->Value = Value;
		if (Value.vt == VT_BSTR)
		{
			_Property->Value.bstrVal = SysAllocString(Value.bstrVal);
		}
		return true;
	}
	return false;
}

bool COleObject::GetPropertyName(int PropertyOrd, std::wstring& Name)
{
	if (PropertyOrd >= m_Object.Members.Properties.size())
		return false;

	Name = m_Object.Members.Properties[PropertyOrd].PropertyName;
	return true;
}

bool COleObject::AddMethod(Method& _Method)
{
	m_Object.Members.Methods.push_back(_Method);
	return true;
}

bool COleObject::GetMethodName(int MethodOrd, std::wstring& Name)
{
	if (MethodOrd >= m_Object.Members.Methods.size())
		return false;

	Name = m_Object.Members.Methods[MethodOrd].MethodName;
	return true;
}

PropertyHandle COleObject::GetMethodHandle(int MethodOrd)
{
	if (MethodOrd >= m_Object.Members.Methods.size())
		return HandleNon;
	PropertyHandle _Handle = m_Object.Members.Methods[MethodOrd].Handle;

	if (_Handle != HandleNon)
		return _Handle;

	for (int ParamCount = 0; ParamCount < m_Object.Members.Methods[MethodOrd].Params.size(); ParamCount++)
	{
		if (m_Object.Members.Methods[MethodOrd].Params[ParamCount].Handle != HandleNon)
			return m_Object.Members.Methods[MethodOrd].Params[ParamCount].Handle;
	}

	return HandleNon;
}

PropertyHandle COleObject::GetMethodHandle(std::wstring MethodName)
{
	int MethodOrd = COleObject::InvalidOrdinal;
	PropertyHandle _Handle = HandleNon;
	for (int MethodCount = 0; MethodCount < m_Object.Members.Methods.size(); MethodCount++)
	{
		if (wcsicmp(m_Object.Members.Methods[MethodCount].MethodName.c_str(), MethodName.c_str()) == 0)
		{
			_Handle = m_Object.Members.Methods[MethodCount].Handle;
			MethodOrd = MethodCount;
			break;
		}
	}

	if (_Handle != HandleNon)
		return _Handle;

	for (int ParamCount = 0; ParamCount < m_Object.Members.Methods[MethodOrd].Params.size(); ParamCount++)
	{
		if (m_Object.Members.Methods[MethodOrd].Params[ParamCount].Handle != HandleNon)
			return m_Object.Members.Methods[MethodOrd].Params[ParamCount].Handle;
	}

	return HandleNon;
}

int COleObject::GetMethodParamCount(int MethodOrd)
{
	if (MethodOrd >= m_Object.Members.Methods.size())
		return 0;
	return m_Object.Members.Methods[MethodOrd].Params.size();
}

int COleObject::GetMethodParamCount(std::wstring MethodName)
{
	for (int MethodCount = 0; MethodCount < m_Object.Members.Methods.size(); MethodCount++)
	{
		if (wcsicmp(m_Object.Members.Methods[MethodCount].MethodName.c_str(), MethodName.c_str()) == 0)
		{
			return m_Object.Members.Methods[MethodCount].Params.size();
		}
	}
	return 0;
}

bool COleObject::GetMethodParamName(int MethodOrd, int ParamOrd, std::wstring& Name)
{
	if (MethodOrd >= m_Object.Members.Methods.size())
		return false;

	if (ParamOrd >= m_Object.Members.Methods[MethodOrd].Params.size())
		return false;

	Name = m_Object.Members.Methods[MethodOrd].Params[ParamOrd].PropertyName;
	return true;
}

bool COleObject::GetMethodParamName(std::wstring MethodName, int ParamOrd, std::wstring& Name)
{
	int MethodCount = 0;
	for (; MethodCount < m_Object.Members.Methods.size(); MethodCount++)
	{
		if (wcsicmp(m_Object.Members.Methods[MethodCount].MethodName.c_str(), MethodName.c_str()) == 0)
		{
			break;
		}
	}

	if (MethodCount >= m_Object.Members.Methods.size())
		return false;

	Name = m_Object.Members.Methods[MethodCount].Params[ParamOrd].PropertyName;
	return true;
}

VARTYPE COleObject::GetMethodParamType(int MethodOrd, int ParamOrd)
{
	if (MethodOrd >= m_Object.Members.Methods.size())
		return VT_ILLEGAL;

	if (ParamOrd >= m_Object.Members.Methods[MethodOrd].Params.size())
		return VT_ILLEGAL;

	return m_Object.Members.Methods[MethodOrd].Params[ParamOrd].Value.vt;
}

VARTYPE COleObject::GetMethodParamType(std::wstring MethodName, int ParamOrd)
{
	int MethodCount = 0;
	for (; MethodCount < m_Object.Members.Methods.size(); MethodCount++)
	{
		if (wcsicmp(m_Object.Members.Methods[MethodCount].MethodName.c_str(), MethodName.c_str()) == 0)
		{
			break;
		}
	}

	if (MethodCount >= m_Object.Members.Methods.size())
		return VT_ILLEGAL;

	return m_Object.Members.Methods[MethodCount].Params[ParamOrd].Value.vt;
}

PropertyHandle COleObject::GetMethodParamHandle(int MethodOrd, int ParamOrd)
{
	if (MethodOrd >= m_Object.Members.Methods.size())
		return HandleNon;

	if (ParamOrd >= m_Object.Members.Methods[MethodOrd].Params.size())
		return HandleNon;

	return m_Object.Members.Methods[MethodOrd].Params[ParamOrd].Handle;
}

PropertyHandle COleObject::GetMethodParamHandle(std::wstring MethodName, int ParamOrd)
{
	int MethodCount = 0;
	for (; MethodCount < m_Object.Members.Methods.size(); MethodCount++)
	{
		if (wcsicmp(m_Object.Members.Methods[MethodCount].MethodName.c_str(), MethodName.c_str()) == 0)
		{
			break;
		}
	}

	if (MethodCount >= m_Object.Members.Methods.size())
		return HandleNon;

	return m_Object.Members.Methods[MethodCount].Params[ParamOrd].Handle;
}

int COleObject::GetMethodOrdinal(std::wstring MethodName)
{
	for (int MethodCount = 0; MethodCount < m_Object.Members.Methods.size(); MethodCount++)
	{
		if (wcsicmp(m_Object.Members.Methods[MethodCount].MethodName.c_str(), MethodName.c_str()) == 0)
		{
			return MethodCount;
		}
	}
	return COleObject::InvalidOrdinal;
}

bool COleObject::FindHandleInList(PropertyHandle& _Handle, std::vector<PropertyHandle>& _List)
{
	if (_Handle == HandleNon) return false;

	for (int i = 0; i < _List.size(); i++)
	{
		if (_Handle == _List[i])
			return true;
	}
	return false;
}

bool COleObject::GetSameHandleParamNameList(int MethodOrd, std::vector<PropertyHandle>& _HandleList, std::vector<std::wstring>& ParamList)
{
	if (MethodOrd >= m_Object.Members.Methods.size())
		return false;

	int nCount = m_Object.Members.Methods[MethodOrd].Params.size();

	if (FindHandleInList(m_Object.Members.Methods[MethodOrd].Handle, _HandleList))
	{
		for (int i = 0; i < nCount; i++)
		{
			ParamList.push_back(m_Object.Members.Methods[MethodOrd].Params[i].PropertyName);
		}
		return true;
	}
	else
	{
		if (nCount == 0) return false;

		for (int i = 0; i < nCount; i++)
		{
			if (FindHandleInList(m_Object.Members.Methods[MethodOrd].Params[i].Handle, _HandleList))
			{
				ParamList.push_back(m_Object.Members.Methods[MethodOrd].Params[i].PropertyName);
			}
		}

		if (ParamList.size() > 0)
			return true;
		else
			return false;
	}
}


