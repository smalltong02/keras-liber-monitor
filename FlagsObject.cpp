#include "stdafx.h"
#include "FlagsObject.h"
#include "utils.h"
#include "HipsConfigObject.h"
#include "rapidjson\document.h"

CFlagsConfigObject::TypeInfo _StandardTypeInfo[] = {
	{ L"VOID", 0, 0, 0 },
	{ L"BYTE", sizeof(BYTE), 0, 0 },
	{ L"BOOL", sizeof(BOOL), 0, 0 },
	{ L"CHAR", sizeof(CHAR), 0, 0 },
	{ L"UCHAR", sizeof(UCHAR), 0, 0 },
	{ L"INT16", sizeof(INT16), 0, 0 },
	{ L"UINT16", sizeof(UINT16), 0, 0 },
	{ L"SHORT", sizeof(SHORT), 0, 0 },
	{ L"USHORT", sizeof(USHORT), 0, 0 },
	{ L"WORD", sizeof(WORD), 0, 0 },
	{ L"WCHAR", sizeof(WCHAR), 0, 0 },
	{ L"INT", sizeof(INT), 0, 0 },
	{ L"UINT", sizeof(UINT), 0, 0 },
	{ L"LONG", sizeof(LONG), 0, 0 },
	{ L"ULONG", sizeof(ULONG), 0, 0 },
	{ L"ULONG_PTR", sizeof(ULONG_PTR), 0, 0 },
	{ L"DWORD", sizeof(DWORD), 0, 0 },
	{ L"DWORD_PTR", sizeof(DWORD_PTR), 0, 0 },
	{ L"FLOAT", sizeof(FLOAT), 0, 0 },
	{ L"INT64", sizeof(INT64), 0, 0 },
	{ L"UINT64", sizeof(UINT64), 0, 0 },
	{ L"LONGLONG", sizeof(LONGLONG), 0, 0 },
	{ L"ULONGLONG", sizeof(ULONGLONG), 0, 0 },
	{ L"QWORD", 8, 0, 0 },
	{ L"DOUBLE", sizeof(DOUBLE), 0, 0 },
	{ L"LARGE_INTEGER", sizeof(LARGE_INTEGER), 0, 0 },
	{ L"ULARGE_INTEGER", sizeof(ULARGE_INTEGER), 0, 0 },
};

CFlagsConfigObject::TypeInfo _AdditionalTypeInfo[] = {
	{ L"HMODULE", 4, 0, 0 },
	{ L"HRESULT", 4, 1, 0 },
	{ L"SC_HANDLE", 4, 0, 0 },
};

CFlagsConfigObject::CFlagsConfigObject()
{ 
	InitializePrerocessingType();
	m_bValid = false;
}

CFlagsConfigObject::CFlagsConfigObject(std::wstring name)
{ 
	m_name = name; 
	InitializePrerocessingType();
	m_bValid = false;
}

bool CFlagsConfigObject::Initialize(std::string& json_str)
{
	if (json_str.length() == 0) return false;

	rapidjson::Document document;
	document.Parse(json_str.c_str());
	if (!document.IsObject() || document.IsNull())
	{
		//config data is incorrect.
		error_log("CFlagsConfigObject::Initialize failed!");
		return false;
	}

	// processing additional type
	if (document.HasMember(FL_ADDITIONAL) && document[FL_ADDITIONAL].IsObject())
	{
		for (auto& additional_object : document[FL_ADDITIONAL].GetObject())
		{
			if (additional_object.value.IsNull())
			{
				error_log("CFlagsConfigObject::Initialize the additional %s! is null!", additional_object.name.GetString());
				continue;
			}

			bool bdelayed = false;
			TypeInfo type_info;
			std::wstring type_name = A2W(additional_object.name.GetString());
			type_info.type_name = type_name;
			if (additional_object.value.IsObject())
			{
				if (!additional_object.value.HasMember(AD_DATA))
				{
					error_log("CFlagsConfigObject::Initialize the additional %s! data format incorrect!", additional_object.name.GetString());
					continue;
				}

				type_info.type_success = false;
				type_info.n_value = 0;
				std::wstring value_name = A2W(additional_object.value[AD_DATA].GetString());
				if ((type_info.type_size = GetTypesSize(value_name)) == InvalidOrdinal)
				{
					// not found type information, maybe need to delay processing the type
					type_info.type_name = value_name;
					bdelayed = true;
				}
				if (additional_object.value.HasMember(AD_SUCCESS))
				{
					type_info.type_success = true;
					type_info.n_value = additional_object.value[AD_SUCCESS].GetInt();
				}
				if (additional_object.value.HasMember(AD_FAILED))
				{
					type_info.type_success = false;
					type_info.n_value = additional_object.value[AD_FAILED].GetInt();
				}
			}
			else
			{
				std::wstring value_name = A2W(additional_object.value.GetString());
				if ((type_info.type_size = GetTypesSize(value_name)) == InvalidOrdinal)
				{
					// not found type information, maybe need to delay processing the type
					type_info.type_name = value_name;
					bdelayed = true;
				}
				type_info.type_success = false;
				type_info.n_value = 0;
			}
			if (bdelayed)
				AddDelayedType(type_name, type_info);
			else
				AddNewType(type_name, type_info);
		}
	}

	// processing struct
	if (document.HasMember(FL_STRUCTURE) && document[FL_STRUCTURE].IsObject())
	{
		for (auto& struct_object : document[FL_STRUCTURE].GetObject())
		{
			if (!struct_object.value.IsObject() || struct_object.value.IsNull())
			{
				error_log("CFlagsConfigObject::Initialize the struct %s! not a object!", struct_object.name.GetString());
				continue;
			}

			bool bdelayed = false;
			StructureInfo struct_info;
			struct_info.struct_size = 0;
			struct_info.struct_name = A2W(struct_object.name.GetString());
			for (auto& data_object : struct_object.value.GetObject())
			{
				ElementInfo element;
				element.element_name = A2W(data_object.name.GetString());
				if (!data_object.value.IsNull())
				{
					if (data_object.value.IsString())
					{
						element.value_name = A2W(data_object.value.GetString());
						if ((element.element_size = GetTypesSize(element.value_name)) == InvalidOrdinal)
						{
							// not found type information, maybe need to delay processing the struct
							bdelayed = true;
						}
						struct_info.element.push_back(element);
					}
					else
					{
						error_log("CFlagsConfigObject::Initialize the element %ws! not a string!", element.element_name.c_str());
					}
				}
				else
				{
					error_log("CFlagsConfigObject::Initialize the element %s! is null!", data_object.name.GetString());
				}
			}
			if (bdelayed)
				AddDelayedStruct(struct_info.struct_name, struct_info);
			else
			{
				AddNewStruct(struct_info.struct_name, struct_info);
			}
		}
	}

	if (document.HasMember(FL_FLAGS) && document[FL_FLAGS].IsObject())
	{
		for (auto& flag_object : document[FL_FLAGS].GetObject())
		{
			FlagInfo flag_info;
			flag_info.flag_name = A2W(flag_object.name.GetString());
			if (!flag_object.value.HasMember(FLS_DATA) || !flag_object.value.HasMember(FLS_OP) || !flag_object.value.HasMember(FLS_VALUE))
			{
				error_log("CFlagsConfigObject::Initialize the flags %ws is a invalid flags!", flags_name.c_str());
				continue;
			}

			if (!flag_object.value[FLS_DATA].IsString() || !flag_object.value[FLS_OP].IsString() || !flag_object.value[FLS_VALUE].IsObject())
			{
				error_log("CFlagsConfigObject::Initialize the flags %ws data format incorrect!", flags_name.c_str());
				continue;
			}
			flag_info.flag_op = GetFlagOp(std::string(flag_object.value[FLS_OP].GetString()));
			if (flag_info.flag_op == FlagOperation::op_undefined)
			{
				continue;
			}
			if ((flag_info.flag_size = GetTypesSize(A2W(flag_object.value[FLS_DATA].GetString()))) == InvalidOrdinal)
			{
				continue;
			}

			for (auto& value_object : flag_object.value[FLS_VALUE].GetObject())
			{
				FlagValue value;
				value.value_name = A2W(value_object.name.GetString());
				value.n_value = value_object.value.GetUint();
				flag_info.flag_value.push_back(value);
			}
			AddNewFlag(flag_info.flag_name, flag_info);
		}
	}

	// processing delayed types
	ProcessingDelayTypes();
	m_bValid = true;
	return true;
}

void CFlagsConfigObject::GenerateWordPoint1(TypeInfo& type_info, TypeInfo& new_type)
{
	new_type.type_name = L"LP" + type_info.type_name;
	new_type.type_size = sizeof(LPVOID);
	new_type.type_success = false;
	new_type.n_value = 0;
}

void CFlagsConfigObject::GenerateWordPoint2(TypeInfo& type_info, TypeInfo& new_type)
{
	new_type.type_name = type_info.type_name + L"*";
	new_type.type_size = sizeof(LPVOID);
	new_type.type_success = false;
	new_type.n_value = 0;
}

void CFlagsConfigObject::InitializePrerocessingType()
{
	// initialize standard type
	int Size = sizeof(_StandardTypeInfo) / sizeof(CFlagsConfigObject::TypeInfo);
	int Count = 0;
	while (Count < Size)
	{
		TypeInfo type_info = _StandardTypeInfo[Count];
		AddNewType(type_info.type_name, type_info);
		Count++;
	}

	// initialize additional type
	Size = sizeof(_AdditionalTypeInfo) / sizeof(CFlagsConfigObject::TypeInfo);
	Count = 0;
	while (Count < Size)
	{
		TypeInfo type_info = _AdditionalTypeInfo[Count];
		AddNewType(type_info.type_name, type_info);
		Count++;
	}

	return;
}

bool CFlagsConfigObject::IsWordPoint(std::wstring& type_name)
{
	bool bret = false;

	if (type_name[0] == L'L' && type_name[1] == L'P')
	{
		bret = true;
	}
	else if (type_name[type_name.length()-1] == L'*')
	{
		bret = true;
	}

	return bret;
}

bool CFlagsConfigObject::GetWordRoot(std::wstring& type_name, std::wstring& root_name, int& dimension)
{
	bool bret = false;
	root_name = type_name;
	
	if (type_name[0] == L'L' && type_name[1] == L'P')
	{
		dimension++;
		root_name = type_name.substr(2);
		bret = true;
	}
	else if (type_name[type_name.length()-1] == L'*')
	{
		dimension++;
		root_name = type_name.substr(0, type_name.length() - 1);
		bret = true;
	}
	if (bret)
		GetWordRoot(root_name, root_name, dimension);
	return bret;
}

bool CFlagsConfigObject::IsArray(std::wstring& type_name)
{
	bool bret = false;
	if (type_name[0] == L'[')
	{
		bret = true;
	}
	else if (type_name[type_name.length() - 1] == L']')
	{
		bret = true;
	}
	return bret;
}

bool CFlagsConfigObject::GetArrayRoot(std::wstring& type_name, std::wstring& root_name, std::vector<int>& array_dim)
{
	bool bret = false;
	root_name = type_name;

	if (type_name[0] == L'[')
	{
		size_t pos = type_name.find(L']');
		if (pos == -1 || pos <= 1)
			except_throw("Fatal error: array %ws define error!", type_name.c_str());
		int dim = _wtoi(type_name.substr(1, pos - 1).c_str());
		root_name = type_name.substr(pos+1, type_name.length()-1);
		array_dim.push_back(dim);
		GetArrayRoot(root_name, root_name, array_dim);
		bret = true;
	}
	else if (type_name[type_name.length() - 1] == L']')
	{
		size_t pos = type_name.rfind(L'[');
		if (pos == -1 || pos > type_name.length() - 2)
			except_throw("Fatal error: array %ws define error!", type_name.c_str());
		int dim = _wtoi(type_name.substr(pos + 1, type_name.length() - 2).c_str());
		root_name = type_name.substr(0, pos);
		GetArrayRoot(root_name, root_name, array_dim);
		array_dim.push_back(dim);
		bret = true;
	}
		
	return bret;
}

int CFlagsConfigObject::ConvertToOnedim(std::vector<int>& array_dim)
{
	int size = 1;
	for (auto& i : array_dim)
	{
		size *= i;
	}
	return size;
}

int CFlagsConfigObject::GetTypesSize(std::wstring type_name)
{
	int dimension = 0;
	if (IsWordPoint(type_name))
	{
		GetWordRoot(type_name, type_name, dimension);
	}

	std::vector<int> array_dim;
	if( IsArray(type_name))
	{
		GetArrayRoot(type_name, type_name, array_dim);
	}

	TypeInfo type_info;
	if (GetTypeInfo(type_name, type_info))
	{
		if (dimension)
		{
			if(!array_dim.size())
				return sizeof(LPVOID);
			else
				return ConvertToOnedim(array_dim) * sizeof(LPVOID);
		}
		else
		{
			if (!array_dim.size())
				return type_info.type_size;
			else
				return ConvertToOnedim(array_dim) * type_info.type_size;
		}
	}

	StructureInfo struct_info;
	if (GetStructInfo(type_name, struct_info))
	{
		if (dimension)
		{
			if (!array_dim.size())
				return sizeof(LPVOID);
			else
				return ConvertToOnedim(array_dim) * sizeof(LPVOID);
		}
		else
		{
			if (!array_dim.size())
				return struct_info.struct_size;
			else
				return ConvertToOnedim(array_dim) * struct_info.struct_size;
		}
	}

	FlagInfo flag_info;
	if (GetFlagInfo(type_name, flag_info))
	{
		if (dimension)
		{
			if (!array_dim.size())
				return sizeof(LPVOID);
			else
				return ConvertToOnedim(array_dim) * sizeof(LPVOID);
		}
		else
		{
			if (!array_dim.size())
				return flag_info.flag_size;
			else
				return ConvertToOnedim(array_dim) * flag_info.flag_size;
		}
	}
	error_log("GetTypesSize: the types %ws query size failed!", type_name.c_str());
	return InvalidOrdinal;
}

CFlagsConfigObject::TypesType CFlagsConfigObject::GetTypesType(std::wstring type_name)
{
	int dimension = 0;
	if (IsWordPoint(type_name))
	{
		GetWordRoot(type_name, type_name, dimension);
	}

	std::vector<int> array_dim;
	if (IsArray(type_name))
	{
		GetArrayRoot(type_name, type_name, array_dim);
	}

	TypeInfo type_info;
	if (GetTypeInfo(type_name, type_info))
	{
		return types_type;
	}

	StructureInfo struct_info;
	if (GetStructInfo(type_name, struct_info))
	{
		return types_struct;
	}

	FlagInfo flag_info;
	if (GetFlagInfo(type_name, flag_info))
	{
		return types_flag;
	}
	error_log("GetTypesType: the types %ws query type failed!", type_name.c_str());
	return types_undefined;
}

bool CFlagsConfigObject::GetTypesInfo(std::wstring type_name, TypesInfo& types_info)
{
	int dimension = 0;
	if (IsWordPoint(type_name))
	{
		GetWordRoot(type_name, type_name, dimension);
	}

	std::vector<int> array_dim;
	if (IsArray(type_name))
	{
		GetArrayRoot(type_name, type_name, array_dim);
	}

	if (GetTypeInfo(type_name, &types_info.type_info))
	{
		types_info.types = types_type;
		return true;
	}

	if (GetStructInfo(type_name, &types_info.struct_info))
	{
		types_info.types = types_struct;
		return true;
	}

	if (GetFlagInfo(type_name, &types_info.flag_info))
	{
		types_info.types = types_flag;
		return true;
	}
	return false;
}

CFlagsConfigObject::TypesType CFlagsConfigObject::GetDelayType(std::wstring type_name)
{
	int dimension = 0;
	if (IsWordPoint(type_name))
	{
		GetWordRoot(type_name, type_name, dimension);
	}

	std::vector<int> array_dim;
	if (IsArray(type_name))
	{
		GetArrayRoot(type_name, type_name, array_dim);
	}

	TypeMap::iterator type_info = m_delayed_type_list.find(type_name);
	if (type_info != m_delayed_type_list.end())
	{
		return types_type;
	}

	StructMap::iterator struct_info = m_delayed_struct_list.find(type_name);
	if (struct_info != m_delayed_struct_list.end())
	{
		return types_struct;
	}
	return types_undefined;
}

bool CFlagsConfigObject::AddNewTypes(std::wstring& new_types, std::wstring& same_types)
{
	TypeInfo type_info;
	if (GetTypeInfo(same_types, type_info))
	{
		return AddNewType(new_types, type_info);
	}

	StructureInfo struct_info;
	if (GetStructInfo(same_types, struct_info))
	{
		return AddNewStruct(new_types, struct_info);
	}

	FlagInfo flag_info;
	if (GetFlagInfo(same_types, flag_info))
	{
		return AddNewFlag(new_types, flag_info);
	}
	error_log("AddNewTypes: new types %ws add failed!", new_types.c_str());
	return false;
}

bool CFlagsConfigObject::AddNewType(std::wstring& new_type, TypeInfo& type_info)
{
	TypeMap::iterator it = m_typeinfo_list.find(new_type);
	if (it != m_typeinfo_list.end())
	{
		error_log("AddNewType: new type %ws has exist!", new_type.c_str());
		return false;
	}
	TypeInfo new_info;
	m_typeinfo_list[new_type] = type_info;
	//GenerateWordPoint1(type_info, new_info);
	//m_typeinfo_list[new_info.type_name] = new_info;
	//GenerateWordPoint2(type_info, new_info);
	//m_typeinfo_list[new_info.type_name] = new_info;
	return true;
}

bool CFlagsConfigObject::AddDelayedType(std::wstring& new_type, TypeInfo& type_info)
{
	TypeMap::iterator it = m_delayed_type_list.find(new_type);
	if (it != m_delayed_type_list.end())
	{
		error_log("AddDelayedType: new type %ws has exist!", new_type.c_str());
		return false;
	}
	m_delayed_type_list[new_type] = type_info;
	return true;
}

bool CFlagsConfigObject::AddNewStruct(std::wstring& new_struct, StructureInfo& struct_info)
{
	StructMap::iterator it = m_structinfo_list.find(new_struct);
	if (it != m_structinfo_list.end())
	{
		error_log("AddNewStruct: new struct %ws has exist!", new_struct.c_str());
		return false;
	}
	if (struct_info.struct_size == 0 || struct_info.struct_size == InvalidOrdinal)
	{
		for (auto& element : struct_info.element)
		{
			struct_info.struct_size += element.element_size;
		}
	}
	m_structinfo_list[new_struct] = struct_info;
	return true;
}

bool CFlagsConfigObject::AddDelayedStruct(std::wstring& new_struct, StructureInfo& struct_info)
{
	StructMap::iterator it = m_delayed_struct_list.find(new_struct);
	if (it != m_delayed_struct_list.end())
	{
		error_log("AddDelayedStruct: new struct %ws has exist!", new_struct.c_str());
		return false;
	}
	m_delayed_struct_list[new_struct] = struct_info;
	return true;
}

bool CFlagsConfigObject::AddNewFlag(std::wstring& new_flag, FlagInfo& flag_info)
{
	FlagMap::iterator it = m_flaginfo_list.find(new_flag);
	if (it != m_flaginfo_list.end())
	{
		error_log("AddNewFlag: new flag %ws has exist!", flag_info.c_str());
		return false;
	}
	m_flaginfo_list[new_flag] = flag_info;
	return true;
}

bool CFlagsConfigObject::GetTypeInfo(std::wstring& type_name, CFlagsConfigObject::TypeInfo& type_info)
{
	TypeMap::iterator it = m_typeinfo_list.find(type_name);
	if (it == m_typeinfo_list.end())
	{
		return false;
	}
	type_info.type_name = (*it).second.type_name;
	type_info.type_size = (*it).second.type_size;
	type_info.type_success = (*it).second.type_success;
	type_info.n_value = (*it).second.n_value;
	return true;
}

bool CFlagsConfigObject::GetStructInfo(std::wstring& struct_name, StructureInfo& struct_info)
{
	StructMap::iterator it = m_structinfo_list.find(struct_name);
	if (it == m_structinfo_list.end())
	{
		return false;
	}
	struct_info.struct_name = (*it).second.struct_name;
	struct_info.struct_size = (*it).second.struct_size;
	struct_info.element = (*it).second.element;
	return true;
}

bool CFlagsConfigObject::GetFlagInfo(std::wstring& flag_name, FlagInfo& flag_info)
{
	FlagMap::iterator it = m_flaginfo_list.find(flag_name);
	if (it == m_flaginfo_list.end())
	{
		return false;
	}
	flag_info.flag_name = (*it).second.flag_name;
	flag_info.flag_op = (*it).second.flag_op;
	flag_info.flag_size = (*it).second.flag_size;
	flag_info.flag_value = (*it).second.flag_value;
	return true;
}

bool CFlagsConfigObject::GetTypeInfo(std::wstring& type_name, TypeInfo** type_info)
{
	if (type_info == nullptr) return false;

	TypeMap::iterator it = m_typeinfo_list.find(type_name);
	if (it == m_typeinfo_list.end())
	{
		return false;
	}
	*type_info = &((*it).second);
	return true;
}

bool CFlagsConfigObject::GetStructInfo(std::wstring& struct_name, StructureInfo** struct_info)
{
	if (struct_info == nullptr) return false;

	StructMap::iterator it = m_structinfo_list.find(struct_name);
	if (it == m_structinfo_list.end())
	{
		return false;
	}
	*struct_info = &((*it).second);
	return true;
}

bool CFlagsConfigObject::StructureInfo::GetElementType(std::wstring& element_name, ElementInfo& elem_info)
{
	for (auto& elem : element)
	{
		if (wcsicmp(elem.element_name.c_str(), element_name.c_str()) == 0)
		{
			elem_info = elem;
			return true;
		}
	}
	return false;
}

bool CFlagsConfigObject::StructureInfo::GetElementOffset(std::wstring& element_name, int& elem_offset)
{
	int offset = 0;
	for (auto& elem : element)
	{
		if (wcsicmp(elem.element_name.c_str(), element_name.c_str()) == 0)
		{
			elem_offset = offset;
			return true;
		}
		offset += elem.element_size;
	}
	return false;
}

bool CFlagsConfigObject::GetFlagInfo(std::wstring& flag_name, FlagInfo** flag_info)
{
	if (flag_info == nullptr) return false;

	FlagMap::iterator it = m_flaginfo_list.find(flag_name);
	if (it == m_flaginfo_list.end())
	{
		return false;
	}
	*flag_info = &((*it).second);
	return true;
}

CFlagsConfigObject::FlagOperation CFlagsConfigObject::GetFlagOp(std::string& op_name)
{
	FlagOperation op = op_undefined;
	if (stricmp(op_name.c_str(), "OR") == 0)
		op = op_or;
	else if (stricmp(op_name.c_str(), "AND") == 0)
		op = op_and;
	else if (stricmp(op_name.c_str(), "NOT") == 0)
		op = op_not;
	return op;
}

bool CFlagsConfigObject::AddNewTypeFromDelay(std::wstring type_name)
{
	int dimension = 0;
	if (IsWordPoint(type_name))
	{
		GetWordRoot(type_name, type_name, dimension);
	}

	std::vector<int> array_dim;
	if (IsArray(type_name))
	{
		GetArrayRoot(type_name, type_name, array_dim);
	}

	TypeMap::iterator it = m_delayed_type_list.find(type_name);
	if (it == m_delayed_type_list.end())
	{
		return false;
	}

	TypeInfo& type_info = (*it).second;

	bool bret = true;
	if (type_info.type_size == InvalidOrdinal)
	{
		if (!ProcessingDelayType(type_info.type_name))
		{
			bret = false;
		}
	}

	if (bret)
	{
		TypesType type = GetTypesType(type_info.type_name);
		if (type == types_type)
		{
			type_info.type_size = GetTypesSize(type_info.type_name);
			if (type_info.type_size == InvalidOrdinal)
				except_throw("ProcessingDelayType: error!");
			type_info.type_name = (*it).first;
			AddNewType(type_info.type_name, type_info);
		}
		else if (type == types_struct)
		{
			type_info.type_size = 0;
			if (IsWordPoint(type_info.type_name) || IsArray(type_info.type_name))
			{
				StructureInfo struct_info;
				struct_info.struct_name = (*it).first;
				struct_info.struct_size = GetTypesSize(type_info.type_name);
				if (type_info.type_size == InvalidOrdinal)
					except_throw("ProcessingDelayType: error!");
				ElementInfo element;
				element.element_name = L"Data1";
				element.element_size = struct_info.struct_size;
				element.value_name = type_info.type_name;
				struct_info.element.push_back(element);
				AddNewStruct(struct_info.struct_name, struct_info);
			}
			else
			{
				StructureInfo struct_info;
				if (GetStructInfo(type_info.type_name, struct_info))
				{
					struct_info.struct_name = (*it).first;
					AddNewStruct(struct_info.struct_name, struct_info);
				}
			}
		}
		else if (type == types_flag)
		{
			type_info.type_size = 0;
			if (IsWordPoint(type_info.type_name) || IsArray(type_info.type_name))
			{
				StructureInfo struct_info;
				struct_info.struct_name = (*it).first;
				struct_info.struct_size = GetTypesSize(type_info.type_name);
				if (type_info.type_size == InvalidOrdinal)
					except_throw("ProcessingDelayType: error!");
				ElementInfo element;
				element.element_name = L"Data1";
				element.element_size = struct_info.struct_size;
				element.value_name = type_info.type_name;
				struct_info.element.push_back(element);
				AddNewStruct(struct_info.struct_name, struct_info);
			}
			else
			{
				FlagInfo flag_info;
				if (GetFlagInfo(type_info.type_name, flag_info))
				{
					flag_info.flag_name = (*it).first;
					AddNewFlag(flag_info.flag_name, flag_info);
				}
			}
		}
	}

	return bret;
}

bool CFlagsConfigObject::AddNewStructFromDelay(std::wstring struct_name)
{
	int dimension = 0;
	if (IsWordPoint(struct_name))
	{
		GetWordRoot(struct_name, struct_name, dimension);
	}

	std::vector<int> array_dim;
	if (IsArray(struct_name))
	{
		GetArrayRoot(struct_name, struct_name, array_dim);
	}

	bool bret = true;
	StructMap::iterator it = m_delayed_struct_list.find(struct_name);
	if (it == m_delayed_struct_list.end())
	{
		return false;
	}

	for (auto& elemnet : (*it).second.element)
	{
		if (elemnet.element_size == InvalidOrdinal)
		{
			if (ProcessingDelayType(elemnet.value_name))
			{
				elemnet.element_size = GetTypesSize(elemnet.value_name);
			}
		}

		if (elemnet.element_size == InvalidOrdinal)
		{
			bret = false;
		}
	}

	if (bret)
		AddNewStruct(struct_name, (*it).second);

	return bret;
}

bool CFlagsConfigObject::ProcessingDelayType(std::wstring& type_name)
{
	TypesType type = GetTypesType(type_name);
	switch (type)
	{
	case types_type:
	case types_struct:
	case types_flag:
	{
		return true;
	}
	case types_undefined:
	{
		type = GetDelayType(type_name);
		switch (type)
		{
		case types_type:
		{
			return AddNewTypeFromDelay(type_name);
		}
		break;
		case types_struct:
		{
			return AddNewStructFromDelay(type_name);
		}
		break;
		}
	}
	}
	return false;
}

void CFlagsConfigObject::ProcessingDelayType()
{
	for (auto& type_info : m_delayed_type_list)
	{
		if (type_info.second.type_size == InvalidOrdinal)
		{
			if (ProcessingDelayType(type_info.second.type_name))
			{
				TypesType type = GetTypesType(type_info.second.type_name);
				if (type == types_type)
				{
					type_info.second.type_size = GetTypesSize(type_info.second.type_name);
					if (type_info.second.type_size == InvalidOrdinal)
						except_throw("ProcessingDelayType: error!");
					type_info.second.type_name = type_info.first;
					AddNewType(type_info.second.type_name, type_info.second);
				}
				else if (type == types_struct)
				{
					if (IsWordPoint(type_info.second.type_name) || IsArray(type_info.second.type_name))
					{
						StructureInfo struct_info;
						struct_info.struct_name = type_info.first;
						struct_info.struct_size = GetTypesSize(type_info.second.type_name);
						if (type_info.second.type_size == InvalidOrdinal)
							except_throw("ProcessingDelayType: error!");
						ElementInfo element;
						element.element_name = L"Data1";
						element.element_size = struct_info.struct_size;
						element.value_name = type_info.second.type_name;
						struct_info.element.push_back(element);
						AddNewStruct(struct_info.struct_name, struct_info);
					}
					else
					{
						StructureInfo struct_info;
						if (GetStructInfo(type_info.second.type_name, struct_info))
						{
							struct_info.struct_name = type_info.first;
							AddNewStruct(struct_info.struct_name, struct_info);
						}
					}
				}
				else if (type == types_flag)
				{
					if (IsWordPoint(type_info.second.type_name) || IsArray(type_info.second.type_name))
					{
						StructureInfo struct_info;
						struct_info.struct_name = type_info.first;
						struct_info.struct_size = GetTypesSize(type_info.second.type_name);
						if (type_info.second.type_size == InvalidOrdinal)
							except_throw("ProcessingDelayType: error!");
						ElementInfo element;
						element.element_name = L"Data1";
						element.element_size = struct_info.struct_size;
						element.value_name = type_info.second.type_name;
						struct_info.element.push_back(element);
						AddNewStruct(struct_info.struct_name, struct_info);
					}
					else
					{
						FlagInfo flag_info;
						if (GetFlagInfo(type_info.second.type_name, flag_info))
						{
							flag_info.flag_name = type_info.first;
							AddNewFlag(flag_info.flag_name, flag_info);
						}
					}
				}
			}
		}
	}
	m_delayed_type_list.clear();
}

void CFlagsConfigObject::ProcessingDelayStruct()
{
	for (auto& struct_info : m_delayed_struct_list)
	{
		bool bret = false;
		for (auto& elemnet : struct_info.second.element)
		{
			if (elemnet.element_size == InvalidOrdinal)
			{
				if (ProcessingDelayType(elemnet.value_name))
				{
					bret = true;
					elemnet.element_size = GetTypesSize(elemnet.value_name);
				}
			}

			if (elemnet.element_size == InvalidOrdinal)
			{
				bret = false;
				break;
			}
		}

		if (bret)
			AddNewStruct(std::wstring(struct_info.first), struct_info.second);
	}
	m_delayed_struct_list.clear();
}

void CFlagsConfigObject::ProcessingDelayTypes()
{
	ProcessingDelayType();
	ProcessingDelayStruct();
}

