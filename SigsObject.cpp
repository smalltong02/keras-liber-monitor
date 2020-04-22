#include "stdafx.h"
#include "SigsObject.h"
#include "utils.h"
#include "FlagsObject.h"
#include "HipsCfgObject.h"
#include "HookImplementObject.h"
#include "rapidjson\document.h"

namespace cchips {

CFunction::_call_convention CSigsCfgObject::GetCallConvention(const std::string& str_call) const
{
	if (_stricmp(str_call.c_str(), CCS_WINAPI) == 0 ||
		_stricmp(str_call.c_str(), CCS_STDCALL) == 0)
		return CFunction::call_stdcall;
	else if (_stricmp(str_call.c_str(), CCS_CDECL) == 0)
		return CFunction::call_cdecl;
	else if (_stricmp(str_call.c_str(), CCS_FASTCALL) == 0)
		return CFunction::call_fastcall;
	else if (_stricmp(str_call.c_str(), CCS_VECTORCALL) == 0)
		return CFunction::call_vectorcall;
	return CFunction::call_unknown;
}

bool CSigsCfgObject::Initialize(const std::string& json_str)
{
	if (json_str.length() == 0) return false;

	rapidjson::Document document;
	document.Parse(json_str.c_str());
	if (!document.IsObject() || document.IsNull())
	{
		//config data is incorrect.
		error_log("CSigsConfigObject::Initialize failed!");
		return false;
	}

	CFunction::_call_convention call_conv = CFunction::call_unknown;
	if (document.HasMember(SI_CATEGORY) && document[SI_CATEGORY].IsString())
		m_category_name = document[SI_CATEGORY].GetString();
	if (document.HasMember(SI_CALL) && document[SI_CALL].IsString())
		call_conv = GetCallConvention(document[SI_CALL].GetString());

	if (call_conv == CFunction::call_unknown)
	{
		error_log("CSigsConfigObject::Initialize get a error call convention!");
		return false;
	}

	for (const auto& api_docment : document.GetObject())
	{
		if (_stricmp(api_docment.name.GetString(), SI_CATEGORY) == 0)
			continue;
		else if (_stricmp(api_docment.name.GetString(), SI_CALL) == 0)
			continue;
		else
		{
			if (!InitializeFunction(call_conv, api_docment.name.GetString(), api_docment.value))
			{
				error_log("CSigsConfigObject::Initialize InitializeFunction({}) failed!", api_docment.name.GetString());
				return false;
			}
		}
	}
	m_bValid = true;
	return true;
}

bool CSigsCfgObject::InitializePrototype(const std::shared_ptr<CFunction>& func_object, const rapidjson::Value& Object)
{
	if (!func_object) return false;
	// set signature information
	if (!Object.HasMember(SI_SIGNATURE) || !Object[SI_SIGNATURE].IsObject())
	{
		error_log("CSigsConfigObject::InitializePrototype the signature not a object!");
		return false;
	}

	auto& sign_object = Object[SI_SIGNATURE].GetObject();
	if (!sign_object.HasMember(SI_LIBRARY) || !sign_object[SI_LIBRARY].IsString())
	{
		error_log("CSigsConfigObject::InitializePrototype the library not a object!");
		return false;
	}

	func_object->SetLibrary(sign_object[SI_LIBRARY].GetString());

	if (!sign_object.HasMember(SI_RETURN) || !sign_object[SI_RETURN].GetString())
	{
		error_log("CSigsConfigObject::InitializePrototype the return not a object!");
		return false;
	}

	std::string return_iden = sign_object[SI_RETURN].GetString();
	std::shared_ptr<CObObject> return_ptr = func_object->GetTyIdentifier(return_iden);
	if (!return_ptr)
	{
		error_log("CSigsConfigObject::InitializePrototype function({}) has invalid return type({})!", func_object->GetName(), return_iden);
		return false;
	}
	if (!func_object->AddReturn(IDENPAIR(SI_RETURN, return_ptr)))
	{
		error_log("CSigsConfigObject::InitializePrototype ({})AddReturn({}) failed!", func_object->GetName(), return_iden);
		return false;
	}
	if (sign_object.HasMember(SI_SPECIAL) && sign_object[SI_SPECIAL].IsBool())
	{
		func_object->SetSpecial(sign_object[SI_SPECIAL].GetBool());
	}
	// set parameter information
	if (Object.HasMember(SI_PARAMETERS) && Object[SI_PARAMETERS].IsObject())
	{
		for (auto& param_object : Object[SI_PARAMETERS].GetObject())
		{
			if (param_object.value.IsString())
			{
				std::string param_str = param_object.name.GetString();
				std::string param_iden = param_object.value.GetString();
				std::shared_ptr<CObObject> param_ptr = func_object->GetTyIdentifier(param_iden);
				if (!param_ptr)
				{
					error_log("CSigsConfigObject::InitializePrototype the parameter {}({}) is a invalid parameter!", param_str, param_iden);
					return false;
				}
				if (!func_object->AddArgument(IDENPAIR(param_str, param_ptr)))
				{
					error_log("CSigsConfigObject::InitializePrototype the function {} add parameter {} failed!", param_str, param_iden);
					return false;
				}
			}
			else
			{
				error_log("CSigsConfigObject::InitializePrototype the param {} not a string!", param_object.name.GetString());
				return false;
			}
		}
	}
	return true;
}

bool CSigsCfgObject::InitializeHandle(const std::shared_ptr<CFunction>& func_object, const rapidjson::Value& Object)
{
	if (!func_object) return false;
	// set handle information
	//if (Object.HasMember(SI_HANDLE) && Object[SI_HANDLE].IsObject())
	//{
	//	for (auto& array_elem : Object[SI_HANDLE].GetObject())
	//	{
	//		if (array_elem.value.IsString())
	//		{
	//			// not finished now
	//		}
	//	}
	//}

	//if (Object.HasMember(SI_HANDLE) && Object[SI_HANDLE].IsObject())
	//{
	//	for (auto& array_elem : Object[SI_HANDLE].GetObject())
	//	{
	//		if (array_elem.value.IsString())
	//		{
	//			// not finished now
	//		}
	//	}
	//}
	return true;
}

bool CSigsCfgObject::InitializeEnsure(const std::shared_ptr<CFunction>& func_object, const rapidjson::Value& Object)
{
	if (!func_object) return false;
	// set ensure information
	if (Object.HasMember(SI_ENSURE) && Object[SI_ENSURE].IsArray())
	{
		for (auto& array_elem : Object[SI_ENSURE].GetArray())
		{
			if (array_elem.IsString())
			{
				if (!func_object->AddEnsure(array_elem.GetString()))
					error_log("CSigsCfgObject::InitializeEnsure: add ensure {} failed!", array_elem.GetString());
			}
		}
	}
	return true;
}

bool CSigsCfgObject::InitializeLog(const std::shared_ptr<CFunction>& func_object, const rapidjson::Value& Object)
{
	if (!func_object) return false;
	// set log information
	bool blogging = true;
	if (Object.HasMember(SI_PRELOG) && Object[SI_PRELOG].IsArray())
	{
		for (const auto& array_elem : Object[SI_PRELOG].GetArray())
		{
			if (array_elem.IsString())
			{
				if (!func_object->AddLogging(array_elem.GetString()))
					error_log("CSigsCfgObject::InitializeLog: add logging {} failed!", array_elem.GetString());
				blogging = false;
			}
		}
	}

	if (Object.HasMember(SI_POSTLOG) && Object[SI_POSTLOG].IsArray())
	{
		for (const auto& array_elem : Object[SI_POSTLOG].GetArray())
		{
			if (array_elem.IsString())
			{
				if (!func_object->AddLogging(array_elem.GetString(), false))
					error_log("CSigsCfgObject::InitializeLog: add logging {} failed!", array_elem.GetString());
				blogging = false;
			}
		}
	}
	
	if (Object.HasMember(SI_LOGGING) && Object[SI_LOGGING].IsBool())
	{
		if (Object[SI_LOGGING].GetBool() && blogging)
		{
			for (const auto& i : (*func_object).GetArguments())
			{
				if (!func_object->AddLogging(i.first))
					error_log("CSigsCfgObject::InitializeLog: add logging {} failed!", i.first);
			}
			if (!func_object->AddLogging((*func_object).GetReturn().first, false))
				error_log("CSigsCfgObject::InitializeLog: add logging {} failed!", (*func_object).GetReturn().first);
		}
	}
	return true;
}

bool CSigsCfgObject::InitializeCheck(const std::shared_ptr<CFunction>& func_object, const rapidjson::Value& Object)
{
	if (!func_object) return false;

	// set precheck information
	if (Object.HasMember(SI_PRECHECK) && Object[SI_PRECHECK].IsObject())
	{
		for (auto& inspect_object : Object[SI_PRECHECK].GetObject())
		{
			if (!inspect_object.value.IsObject())
			{
				continue;
			}
			CFunction::_CheckPackage package;
			if (inspect_object.value.HasMember(SI_DEFINE) && inspect_object.value[SI_DEFINE].IsObject())
			{
				CFunction::_CheckDefine::DefinePair define_pair;
				for (auto& check_object : inspect_object.value[SI_DEFINE].GetObject())
				{
					if (!check_object.value.IsString())
					{
						continue;
					}
					define_pair.first = check_object.name.GetString();
					define_pair.second = check_object.value.GetString();
					package.define_pairs.push_back(define_pair);
				}
			}

			if (inspect_object.value.HasMember(SI_LOG) && inspect_object.value[SI_LOG].IsArray())
			{
				CFunction::_CheckDefine::CheckLog log;
				for (auto& array_element : inspect_object.value[SI_LOG].GetArray())
				{
					if (array_element.IsString())
					{
						log = array_element.GetString();
						package.logs.push_back(log);
					}
				}
			}

			if (inspect_object.value.HasMember(SI_HANDLE) && inspect_object.value[SI_HANDLE].IsArray())
			{
				CFunction::_CheckDefine::HandlePair handle_pair;
				for (auto& handle_object : inspect_object.value[SI_HANDLE].GetObject())
				{
					if (handle_object.value.IsString())
					{
						handle_pair.first = handle_object.name.GetString();
						handle_pair.second = handle_object.value.GetString();
						package.handle_pair.push_back(handle_pair);
					}
				}
			}

			if (inspect_object.value.HasMember(SI_CHECK) && inspect_object.value[SI_CHECK].IsObject())
			{
				for (auto& check_object : inspect_object.value[SI_CHECK].GetObject())
				{
					package.check_pair.push_back(CFunction::_CheckDefine::CheckPair(check_object.name.GetString(), check_object.value.GetString()));
				}
			}

			if (inspect_object.value.HasMember(SI_MODIFY) && inspect_object.value[SI_MODIFY].IsObject())
			{
				for (auto& modify_object : inspect_object.value[SI_MODIFY].GetObject())
				{
					package.modify_pair.push_back(CFunction::_CheckDefine::ModifyPair(modify_object.name.GetString(), modify_object.value.GetString()));
				}
			}
			if (!func_object->AddChecks(package))
				error_log("CSigsCfgObject::InitializeCheck: function({}) add pre checks failed!", func_object->GetName());
		}
	}
	// set postcheck information
	if (Object.HasMember(SI_POSTCHECK) && Object[SI_POSTCHECK].IsObject())
	{
		for (auto& inspect_object : Object[SI_POSTCHECK].GetObject())
		{
			if (!inspect_object.value.IsObject())
			{
				continue;
			}
			CFunction::_CheckPackage package;
			if (inspect_object.value.HasMember(SI_DEFINE) && inspect_object.value[SI_DEFINE].IsObject())
			{
				CFunction::_CheckDefine::DefinePair define_pair;
				for (auto& check_object : inspect_object.value[SI_DEFINE].GetObject())
				{
					if (!check_object.value.IsString())
					{
						continue;
					}
					define_pair.first = check_object.name.GetString();
					define_pair.second = check_object.value.GetString();
					package.define_pairs.push_back(define_pair);
				}
			}

			if (inspect_object.value.HasMember(SI_LOG) && inspect_object.value[SI_LOG].IsArray())
			{
				CFunction::_CheckDefine::CheckLog log;
				for (auto& array_element : inspect_object.value[SI_LOG].GetArray())
				{
					if (array_element.IsString())
					{
						log = array_element.GetString();
						package.logs.push_back(log);
					}
				}
			}

			if (inspect_object.value.HasMember(SI_HANDLE) && inspect_object.value[SI_HANDLE].IsObject())
			{
				CFunction::_CheckDefine::HandlePair handle_pair;
				for (auto& handle_object : inspect_object.value[SI_HANDLE].GetObject())
				{
					if (handle_object.value.IsString())
					{
						handle_pair.first = handle_object.name.GetString();
						handle_pair.second = handle_object.value.GetString();
						package.handle_pair.push_back(handle_pair);
					}
				}
			}

			if (inspect_object.value.HasMember(SI_CHECK) && inspect_object.value[SI_CHECK].IsObject())
			{
				for (auto& check_object : inspect_object.value[SI_CHECK].GetObject())
				{
					package.check_pair.push_back(CFunction::_CheckDefine::CheckPair(check_object.name.GetString(), check_object.value.GetString()));
				}
			}

			if (inspect_object.value.HasMember(SI_MODIFY) && inspect_object.value[SI_MODIFY].IsObject())
			{
				for (auto& modify_object : inspect_object.value[SI_MODIFY].GetObject())
				{
					package.modify_pair.push_back(CFunction::_CheckDefine::ModifyPair(modify_object.name.GetString(), modify_object.value.GetString()));
				}
			}
			if (!func_object->AddChecks(package, false))
				error_log("CSigsCfgObject::InitializeCheck: function({}) add pre checks failed!", func_object->GetName());
		}
	}

	return true;
}

bool CSigsCfgObject::InitializeFunction(CFunction::_call_convention call_conv, const std::string& func_name, const rapidjson::Value& Object, std::shared_ptr<CFunctionProtos> func_protos)
{
	if (!Object.IsObject() || Object.IsNull())
	{
		error_log("CSigsCfgObject::InitializeFunction the {} not a object!", func_name);
		return false;
	}
	// create api object
	std::shared_ptr<CFunction> func_object = std::make_shared<CFunction>(call_conv, func_name.c_str());
	if (!func_object)
	{
		error_log("CSigsConfigObject::InitializeFunction create func_object({}) failed!", func_name);
		return false;
	}

	if (!InitializePrototype(func_object, Object))
	{
		error_log("CSigsConfigObject::InitializePrototype failed!");
		return false;
	}
	if (!InitializeHandle(func_object, Object))
	{
		error_log("CSigsConfigObject::InitializeHandle failed!");
		return false;
	}
	if (!InitializeEnsure(func_object, Object))
	{
		error_log("CSigsConfigObject::InitializeEnsure failed!");
		return false;
	}
	if (!InitializeLog(func_object, Object))
	{
		error_log("CSigsConfigObject::InitializeLog failed!");
		return false;
	}
	if (!InitializeCheck(func_object, Object))
	{
		error_log("CSigsConfigObject::InitializeCheck failed!");
		return false;
	}
	std::shared_ptr<CFunctionProtos> protos = func_protos;
	if (!protos) protos = g_impl_object;
	assert(protos != nullptr);

	if (!protos || !protos->AddFunction(func_object))
	{
		error_log("g_impl_object->AddFunction({}) failed!", func_object->GetName());
		return false;
	}
	return true;
}

} // namespace cchips