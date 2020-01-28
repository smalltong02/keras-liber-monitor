#pragma once
#include <stdio.h>
#include <windows.h>
#include <string>
#include <vector>
#include <map>

#define FL_CATEGORY "Category"
#define FL_STANDARD "Standard"
#define FL_ADDITIONAL "Additional"
#define FL_STRUCTURE "Structure"
#define FL_FLAGS "Flags"

#define AD_DATA "Data"
#define AD_SUCCESS "Success"
#define AD_FAILED "Failed"
#define FLS_DATA "Data"
#define FLS_OP "OPERATION"
#define FLS_VALUE "Value"

class CFlagsConfigObject
{
public:
	enum TypesType
	{
		types_undefined = 0,
		types_type,
		types_struct,
		types_flag,
	};
	enum FlagOperation
	{
		op_undefined = 0,
		op_and,
		op_or,
		op_not,
	};

	typedef struct _TypeInfo {
		std::wstring type_name;
		int type_size;
		bool type_success;
		int n_value;
	}TypeInfo;
	typedef struct _ElementInfo {
		std::wstring element_name;
		int element_size;
		std::wstring value_name;
	}ElementInfo;
	typedef struct _StructureInfo {
		std::wstring struct_name;
		int struct_size;
		std::vector<ElementInfo> element;
		bool GetElementType(std::wstring& element_name, ElementInfo& elem_info);
		bool GetElementOffset(std::wstring& element_name, int& elem_offset);
	}StructureInfo;
	typedef struct _FlagValue {
		std::wstring value_name;
		int n_value;
	}FlagValue;
	typedef struct _FlagInfo {
		std::wstring flag_name;
		int flag_size;
		FlagOperation flag_op;
		std::vector<FlagValue> flag_value;
	}FlagInfo;

	typedef struct _TypesInfo {
		TypesType types;
		union {
			TypeInfo* type_info;
			StructureInfo* struct_info;
			FlagInfo* flag_info;
		};
	}TypesInfo;

	typedef std::map<std::wstring, TypeInfo> TypeMap;
	typedef std::map<std::wstring, StructureInfo> StructMap;
	typedef std::map<std::wstring, FlagInfo> FlagMap;

	CFlagsConfigObject();
	CFlagsConfigObject(std::wstring name);
	~CFlagsConfigObject() { ; }

	void SetFlagsName(std::wstring name) { m_name = name; }
	bool Initialize(std::string& json_str);
	
	int GetTypesSize(std::wstring type_name);
	TypesType GetTypesType(std::wstring type_name);
	bool GetTypesInfo(std::wstring type_name, TypesInfo& types_info);

	static const int InvalidOrdinal = -1;
private:
	void InitializePrerocessingType();
	bool AddNewTypes(std::wstring& new_types, std::wstring& same_types);
	bool AddNewType(std::wstring& new_type, TypeInfo& type_info);
	bool AddNewStruct(std::wstring& new_struct, StructureInfo& struct_info);
	bool AddNewFlag(std::wstring& new_flag, FlagInfo& flag_info);
	bool AddDelayedType(std::wstring& new_type, TypeInfo& type_info);
	bool AddDelayedStruct(std::wstring& new_struct, StructureInfo& struct_info);
	bool GetTypeInfo(std::wstring& type_name, TypeInfo& type_info);
	bool GetStructInfo(std::wstring& struct_name, StructureInfo& struct_info);
	bool GetFlagInfo(std::wstring& flag_name, FlagInfo& flag_info);
	bool GetTypeInfo(std::wstring& type_name, TypeInfo** type_info);
	bool GetStructInfo(std::wstring& struct_name, StructureInfo** struct_info);
	bool GetFlagInfo(std::wstring& flag_name, FlagInfo** flag_info);
	TypesType GetDelayType(std::wstring type_name);
	bool AddNewTypeFromDelay(std::wstring type_name);
	bool AddNewStructFromDelay(std::wstring struct_name);
	FlagOperation GetFlagOp(std::string& op_name);
	void ProcessingDelayTypes();
	void ProcessingDelayType();
	void ProcessingDelayStruct();
	bool ProcessingDelayType(std::wstring& type_name);
	bool ProcessingDelayStruct(std::wstring& struct_name);
	
	bool IsWordPoint(std::wstring& type_name);
	bool GetWordRoot(std::wstring& type_name, std::wstring& root_name, int& dimension);
	void GenerateWordPoint1(TypeInfo& type_info, TypeInfo& new_type);
	void GenerateWordPoint2(TypeInfo& type_info, TypeInfo& new_type);

	bool IsArray(std::wstring& type_name);
	bool GetArrayRoot(std::wstring& type_name, std::wstring& root_name, std::vector<int>& array_dim);
	int ConvertToOnedim(std::vector<int>& array_dim);

	bool m_bValid;
	std::wstring m_name;
	std::wstring m_category_name;
	TypeMap m_typeinfo_list;
	StructMap m_structinfo_list;
	FlagMap m_flaginfo_list;
	TypeMap m_delayed_type_list;
	StructMap m_delayed_struct_list;
};