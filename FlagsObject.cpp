//===- FlagsObject.cpp - metadata type implement define -----------------------------*- C++ -*-===//
// 
// This file implements CFlagsCfgObject class code.
//
//===------------------------------------------------------------------------------------------===//
#include "stdafx.h"
#include "FlagsObject.h"
#include "utils.h"
#include "HipsCfgObject.h"
#include "ParsingImpl.h"

namespace cchips {

bool CFlagsCfgObject::InitializeAdditionalType(const rapidjson::Document& document)
{
	bool bret = true;

	if (document.HasMember(FL_ADDITIONAL) && document[FL_ADDITIONAL].IsObject())
	{
		CLexerObject lexer_object;
		for (auto& additional_object : document[FL_ADDITIONAL].GetObject())
		{
			if (additional_object.value.IsNull())
			{
				error_log("CFlagsConfigObject::Initialize the additional {}! is null!", additional_object.name.GetString());
				bret = false; break;
			}

			std::string alias_name = additional_object.name.GetString();
			if (additional_object.value.IsString())
			{
				std::string iden_name = additional_object.value.GetString();
				std::shared_ptr<CObObject> ob_object = lexer_object.GetTyIdentifier(iden_name);

				if (ob_object && !ob_object->IsBaseTy() && !ob_object->IsReference())
				{
					error_log("CFlagsConfigObject::Initialize the additional {}! data format incorrect!", additional_object.name.GetString());
					bret = false; break;
				}

				if (ob_object)
					bret = AddNewType(IDENPAIR(alias_name, ob_object));
				else 
					AddDelayedType(std::pair<std::string, std::string>(alias_name, iden_name));
			}
		}
	}
	return bret;
}

bool CFlagsCfgObject::InitializeStructType(const rapidjson::Document& document)
{
	bool bret = true;

	if (document.HasMember(FL_STRUCTURE) && document[FL_STRUCTURE].IsObject())
	{
		CLexerObject lexer_object;
		for (auto& struct_object : document[FL_STRUCTURE].GetObject())
		{
			if (!struct_object.value.IsObject() || struct_object.value.IsNull())
			{
				error_log("CFlagsConfigObject::Initialize the struct {}! not a object!", struct_object.name.GetString());
				bret = false; break;
			}
			bool bdelayed = false;
			std::string struct_name = struct_object.name.GetString();
			std::shared_ptr<CStructObject> ob_object = std::make_shared<CStructObject>(struct_name);
			for (auto& data_object : struct_object.value.GetObject())
			{
				std::string element_name = data_object.name.GetString();
				if (!data_object.value.IsNull())
				{
					if (data_object.value.IsString())
					{
						std::string iden_name = data_object.value.GetString();
						std::shared_ptr<CObObject> iden_object = lexer_object.GetTyIdentifier(iden_name);
						if (!iden_object) bdelayed = true;
						(*ob_object).AddElement(element_name, CStructObject::ElementTypeDefine(iden_name, { CStructObject::InvalidOffset, iden_object }));
					}
					else
					{
						error_log("CFlagsConfigObject::Initialize the element {}! not a string!", element_name);
						bret = false; break;
					}
				}
				else
				{
					error_log("CFlagsConfigObject::Initialize the element {}! is null!", data_object.name.GetString());
					bret = false; break;
				}
			}
			if (!bret) break;
			if (bdelayed) 
				AddDelayedStruct(std::pair<std::string, std::shared_ptr<CStructObject>>(struct_name, ob_object));
			else 
				bret = AddNewType(std::pair<std::string, std::shared_ptr<CStructObject>>(struct_name, ob_object));
		}
	}
	return bret;
}

bool CFlagsCfgObject::InitializeFlagType(const rapidjson::Document& document)
{
	bool bret = true;

	if (document.HasMember(FL_FLAGS) && document[FL_FLAGS].IsObject())
	{
		CLexerObject lexer_object;
		for (auto& flag_object : document[FL_FLAGS].GetObject())
		{
			std::string flag_name = flag_object.name.GetString();
			std::shared_ptr<CFlagObject> ob_object = std::make_shared<CFlagObject>(flag_name);
			if (!flag_object.value.HasMember(FLS_DATA) || !flag_object.value.HasMember(FLS_OP) || !flag_object.value.HasMember(FLS_VALUE))
			{
				error_log("CFlagsConfigObject::Initialize the flags {} is a invalid flags!", flag_name);
				bret = false; break;
			}

			if (!flag_object.value[FLS_DATA].IsString() || !flag_object.value[FLS_OP].IsString() || !flag_object.value[FLS_VALUE].IsObject())
			{
				error_log("CFlagsConfigObject::Initialize the flags {} data format incorrect!", flag_name);
				bret = false; break;
			}
			int flag_op = ob_object->GetFlagOp(std::string(flag_object.value[FLS_OP].GetString()));
			if (flag_op == CFlagObject::flag_invalid)
			{
				error_log("CFlagsConfigObject::Initialize the flags {} flag_op incorrect!", flag_name);
				bret = false; break;
			}
			std::shared_ptr<CObObject> data_object = lexer_object.GetTyIdentifier(flag_object.value[FLS_DATA].GetString());
			if (!data_object)
			{
				error_log("CFlagsConfigObject::Initialize the flags {} data_object incorrect!", flag_name);
				bret = false; break;
			}

			std::unique_ptr<std::map<std::string, int>> flag_values = std::make_unique<std::map<std::string, int>>();
			if (flag_values)
			{
				for (auto& value_object : flag_object.value[FLS_VALUE].GetObject())
					(*flag_values)[value_object.name.GetString()] = value_object.value.GetUint();
				if (flag_values->size() == 0)
				{
					error_log("CFlagsConfigObject::Initialize the flags {} flag_values null!", flag_name);
					bret = false; break;
				}
				if (!ob_object->AddFlags(data_object, flag_op, flag_values))
				{
					error_log("CFlagsConfigObject::Initialize the flags {} AddFlags failed!", flag_name);
					bret = false; break;
				}
				bret = AddNewType(std::pair<std::string, std::shared_ptr<CFlagObject>>(flag_name, ob_object));
			}
		}
	}
	return bret;
}

bool CFlagsCfgObject::Initialize(const std::string& json_str)
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

	if (!InitializeAdditionalType(document))
	{
		error_log("InitializeAdditionalType failed!");
		return false;
	}
	if (!InitializeStructType(document))
	{
		error_log("InitializeStructType failed!");
		return false;
	}
	if(!InitializeFlagType(document))
	{
		error_log("InitializeFlagType failed!");
		return false;
	}

	// processing delayed types
	ProcessingDelayTypes();
	m_bValid = true;
	return true;
}

bool CFlagsCfgObject::AddNewType(const IDENPAIR& delay_pair) const
{
	CLexerObject lexer_object;
	return lexer_object.AddTyIdentifier(delay_pair);
}

bool CFlagsCfgObject::AddDelayedType(const std::pair<std::string, std::string>& delay_pair)
{
	auto it = m_delayed_type_list.find(delay_pair.first);
	if (it != m_delayed_type_list.end())
	{
		error_log("AddDelayedType: the type {} has exist!", delay_pair.first);
		return false;
	}
	m_delayed_type_list[delay_pair.first] = delay_pair.second;
	return true;
}

bool CFlagsCfgObject::AddDelayedStruct(const std::pair<std::string, std::shared_ptr<CStructObject>>& delay_pair)
{
	auto it = m_delayed_struct_list.find(delay_pair.first);
	if (it != m_delayed_struct_list.end())
	{
		error_log("AddDelayedStruct: new struct {} has exist!", it->first);
		return false;
	}
	m_delayed_struct_list[delay_pair.first] = delay_pair.second;
	return true;
}

bool CFlagsCfgObject::ProcessingDelayType(const std::string& name)
{
	CLexerObject lexer_object;
	auto it = m_delayed_type_list.find(lexer_object.GetWordRoot(name));
	if (it == m_delayed_type_list.end()) return false;

	std::shared_ptr<CObObject> ob_object = lexer_object.GetTyIdentifier((*it).second);
	if (ob_object)
		return AddNewType(IDENPAIR((*it).first, ob_object));
	else
	{
		if (!ProcessingDelayTypes((*it).second))
		{
			error_log("ProcessingDelayType: delay type {} can not handle!", (*it).first);
			return false;
		}
		ob_object = lexer_object.GetTyIdentifier((*it).second);
		if (!ob_object)
		{
			error_log("ProcessingDelayType: delay type {} can not handle!", (*it).first);
			return false;
		}
		return AddNewType(IDENPAIR((*it).first, ob_object));
	}
	return false;
}

bool CFlagsCfgObject::ProcessingDelayStruct(const std::string& name)
{
	CLexerObject lexer_object;
	auto it = m_delayed_struct_list.find(lexer_object.GetWordRoot(name));
	if (it == m_delayed_struct_list.end()) return false;

	for (auto delay_elem : (*(it->second)))
	{
		if (!delay_elem.second.second.second)
		{
			std::shared_ptr<CObObject> ob_object = lexer_object.GetTyIdentifier(delay_elem.second.first);
			if (ob_object)
			{
				(*it->second).AddElement(delay_elem.first, CStructObject::ElementTypeDefine(delay_elem.second.first, { CStructObject::InvalidOffset,ob_object }));
			}
			else
			{
				if (!ProcessingDelayTypes(delay_elem.second.first))
				{
					error_log("ProcessingDelayStruct: delay struct({}) element({}) can not handle!", it->first, delay_elem.second.first);
					return false;
				}
				ob_object = lexer_object.GetTyIdentifier(delay_elem.second.first);
				if (!ob_object)
				{
					error_log("ProcessingDelayStruct: delay struct({}) element({}) can not handle!", it->first, delay_elem.second.first);
					return false;
				}
				(*it->second).AddElement(delay_elem.first, CStructObject::ElementTypeDefine(delay_elem.second.first, { CStructObject::InvalidOffset,ob_object }));
			}
		}
	}
	return AddNewType(std::pair<std::string, std::shared_ptr<CStructObject>>(it->first, it->second));
}

bool CFlagsCfgObject::ProcessingDelayTypes(const std::string& name)
{
	if (ProcessingDelayType(name))
		return true;
	return ProcessingDelayStruct(name);
}

void CFlagsCfgObject::ProcessingDelayType()
{
	for (auto& delay_elem : m_delayed_type_list)
		ProcessingDelayTypes(delay_elem.first);
	m_delayed_type_list.clear();
}

void CFlagsCfgObject::ProcessingDelayStruct()
{
	for (auto& delay_elem : m_delayed_struct_list)
		ProcessingDelayTypes(delay_elem.first);
	m_delayed_struct_list.clear();
}

void CFlagsCfgObject::ProcessingDelayTypes()
{
	ProcessingDelayType();
	ProcessingDelayStruct();
}

} // namespace cchips





