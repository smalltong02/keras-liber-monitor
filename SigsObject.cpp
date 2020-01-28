#include "stdafx.h"
#include "SigsObject.h"
#include "utils.h"
#include "FlagsObject.h"
#include "HipsConfigObject.h"
#include "rapidjson\document.h"

CSigsConfigObject::CSigsConfigObject()
{
	m_bValid = false;
	m_hips_object = nullptr;
}

CSigsConfigObject::CSigsConfigObject(std::wstring name, CHipsConfigObject* p_hipsobject)
{ 
	m_name = name; 
	m_hips_object = p_hipsobject; 
	m_bValid = false;
}

CApiObject::call_convention CSigsConfigObject::GetCallConvention(std::string str_call)
{
	if (stricmp(str_call.c_str(), CCS_WINAPI) == 0 ||
		stricmp(str_call.c_str(), CCS_STDCALL) == 0)
	{
		return CApiObject::call_stdcall;
	}
	else if (stricmp(str_call.c_str(), CCS_CDECL) == 0)
	{
		return CApiObject::call_cdecl;
	}
	else if (stricmp(str_call.c_str(), CCS_FASTCALL) == 0)
	{
		return CApiObject::call_fastcall;
	}
	else if (stricmp(str_call.c_str(), CCS_VECTORCALL) == 0)
	{
		return CApiObject::call_vectorcall;
	}
	return CApiObject::call_unknown;
}

bool CSigsConfigObject::Initialize(std::string& json_str)
{
	if (m_hips_object == nullptr) return false;
	if (json_str.length() == 0) return false;

	rapidjson::Document document;
	document.Parse(json_str.c_str());
	if (!document.IsObject() || document.IsNull())
	{
		//config data is incorrect.
		error_log("CSigsConfigObject::Initialize failed!");
		return false;
	}

	CApiObject::call_convention call_conv = CApiObject::call_unknown;
	if (document.HasMember(SI_CATEGORY) && document[SI_CATEGORY].IsString())
	{
		m_category_name = A2W(document[SI_CATEGORY].GetString());
	}

	if (document.HasMember(SI_CALL) && document[SI_CALL].IsString())
	{
		call_conv = GetCallConvention(document[SI_CALL].GetString());
	}

	if (call_conv == CApiObject::call_unknown)
	{
		error_log("CSigsConfigObject::Initialize get a error call convention!");
		return false;
	}

	for (auto& api_docment : document.GetObject())
	{
		if (stricmp(api_docment.name.GetString(), SI_CATEGORY) == 0)
		{
			continue;
		}
		else if (stricmp(api_docment.name.GetString(), SI_CALL) == 0)
		{
			continue;
		}
		else
		{
			if (!api_docment.value.IsObject() || api_docment.value.IsNull())
			{
				error_log("CSigsConfigObject::Initialize the %s not a object!", root_members[root_count].c_str());
				continue;
			}
			// set api name
			CApiObject api_object;
			api_object.SetHipsObject(m_hips_object);
			api_object.SetApiName(A2W(api_docment.name.GetString()));
			api_object.SetCallConvention(call_conv);

			if (!api_docment.value.HasMember(SI_SIGNATURE) || !api_docment.value[SI_SIGNATURE].IsObject())
			{
				error_log("CSigsConfigObject::Initialize the signature not a object!");
				continue;
			}
			// set signature information
			auto& sign_Object = api_docment.value[SI_SIGNATURE].GetObject();
			if (!sign_Object.HasMember(SI_LIBRARY) || !sign_Object[SI_LIBRARY].IsString())
			{
				error_log("CSigsConfigObject::Initialize the library not a object!");
				continue;
			}
			api_object.SetLibrary(A2W(sign_Object[SI_LIBRARY].GetString()));

			if (!sign_Object.HasMember(SI_RETURN) || !sign_Object[SI_RETURN].GetString())
			{
				error_log("CSigsConfigObject::Initialize the return not a object!");
				continue;
			}

			CApiObject::parameters param_return;
			param_return.name = A2W(SI_RETURN);
			param_return.type_name = A2W(sign_Object[SI_RETURN].GetString());
			param_return.shape.offset = 0;
			param_return.shape.size = m_hips_object->GetTypesSize(param_return.type_name);
			if (param_return.shape.size == InvalidOrdinal)
			{
				error_log("CSigsConfigObject::Initialize the return value type not defined!");
				continue;
			}
			api_object.SetParameter(param_return);

			if (sign_Object.HasMember(SI_SPECIAL) && sign_Object[SI_SPECIAL].IsBool())
			{
				api_object.SetSpecial(sign_Object[SI_SPECIAL].GetBool());
			}

			// set parameter information
			if (api_docment.value.HasMember(SI_PARAMETERS) && api_docment.value[SI_PARAMETERS].IsObject())
			{
				for(auto& param_Object : api_docment.value[SI_PARAMETERS].GetObject())
				{
					if (param_Object.value.IsString())
					{
						CApiObject::parameters param;
						param.name = A2W(param_Object.name.GetString());
						param.type_name = A2W(param_Object.value.GetString());
						param.shape.size = m_hips_object->GetTypesSize(param.type_name);
						if (param.shape.size == InvalidOrdinal)
						{
							error_log("CSigsConfigObject::Initialize the parameter %ws failed, don't get param_size!", param.name.c_str());
							continue;
						}
						param.shape.offset = api_object.GetParamsSize();
						api_object.SetParameter(param);
					}
					else
					{
						error_log("CSigsConfigObject::Initialize the param %ws not a string!", A2W(param_members[param_count]).c_str());
					}
				}
			}
			// set ensure information
			if (api_docment.value.HasMember(SI_ENSURE) && api_docment.value[SI_ENSURE].IsArray())
			{
				for (auto& array_element : api_docment.value[SI_ENSURE].GetArray())
				{
					if (array_element.IsString())
					{
						CApiObject::param_shape shape;
						if(api_object.InitializeMultipleMembersShape(A2W(array_element.GetString()), shape))
							api_object.SetEnsure(shape);
					}
				}
			}
			// set prelog information
			if (api_docment.value.HasMember(SI_PRELOG) && api_docment.value[SI_PRELOG].IsArray())
			{
				for (auto& array_element : api_docment.value[SI_PRELOG].GetArray())
				{
					if (array_element.IsString())
					{
						CApiObject::param_shape shape;
						if (api_object.InitializeMultipleMembersShape(A2W(array_element.GetString()), shape))
							api_object.SetPrelog(shape);
					}
				}
			}
			// set postlog information
			if (api_docment.value.HasMember(SI_POSTLOG) && api_docment.value[SI_POSTLOG].IsArray())
			{
				for (auto& array_element : api_docment.value[SI_POSTLOG].GetArray())
				{
					if (array_element.IsString())
					{
						CApiObject::param_shape shape;
						if (api_object.InitializeMultipleMembersShape(A2W(array_element.GetString()), shape))
							api_object.SetPostlog(shape);
					}
				}
			}
			// set logging information
			if (api_docment.value.HasMember(SI_LOGGING) && api_docment.value[SI_LOGGING].IsBool())
			{
				if(api_docment.value[SI_LOGGING].GetBool())
					api_object.SetLogging();
			}

			// set precheck information
			if (api_docment.value.HasMember(SI_PRECHECK) && api_docment.value[SI_PRECHECK].IsObject())
			{
				for (auto& inspect_object : api_docment.value[SI_PRECHECK].GetObject())
				{
					if (!inspect_object.value.IsObject())
					{
						continue;
					}
					CApiObject::inspects inspect;
					if (inspect_object.value.HasMember(SI_DEFINE) && inspect_object.value[SI_DEFINE].IsObject())
					{
						for (auto& check_object : inspect_object.value[SI_DEFINE].GetObject())
						{
							CApiObject::parameters param;
							if (!check_object.value.IsString())
							{
								continue;
							}
							if (!api_object.GetParameter(A2W(check_object.name.GetString()), param))
							{
								continue;
							}
							param.type_name = A2W(check_object.value.GetString());
							inspect.define_array.push_back(param);
						}
					}

					if (inspect_object.value.HasMember(SI_LOG) && inspect_object.value[SI_LOG].IsArray())
					{
						for (auto& array_element : inspect_object.value[SI_LOG].GetArray())
						{
							if (array_element.IsString())
							{
								CApiObject::param_shape shape;
								if (api_object.InitializeMultipleMembersShape(A2W(array_element.GetString()), shape))
								{
									inspect.log_array.push_back(shape);
								}
							}
						}
					}

					if (inspect_object.value.HasMember(SI_CHECK) && inspect_object.value[SI_CHECK].IsObject())
					{
						for (auto& check_object : inspect_object.value[SI_CHECK].GetObject())
						{
							CApiObject::checks post_check;
							std::wstring string_array = A2W(check_object.name.GetString());
							std::vector<std::wstring> param_array;
							if (!check_object.value.IsString())
								continue;
							if (post_check.expr_object.SplitStringArray(string_array, param_array))
							{
								for (auto& shape_string : param_array)
								{
									CApiObject::param_shape shape;
									if (api_object.InitializeMultipleMembersShape(shape_string, shape))
									{
										post_check.shape_array.push_back(shape);
									}
									else
										continue;
								}
							}
							else continue;
							post_check.expr_object.SetLogicalExpr(A2W(check_object.value.GetString()));
							inspect.check_array.push_back(post_check);
						}
					}

					if (inspect_object.value.HasMember(SI_MODIFY) && inspect_object.value[SI_MODIFY].IsObject())
					{
						for (auto& check_object : inspect_object.value[SI_MODIFY].GetObject())
						{
							CApiObject::checks post_check;
							std::wstring string_array = A2W(check_object.name.GetString());
							std::vector<std::wstring> param_array;
							if (!check_object.value.IsString())
								continue;
							if (post_check.expr_object.SplitStringArray(string_array, param_array))
							{
								for (auto& shape_string : param_array)
								{
									CApiObject::param_shape shape;
									if (api_object.InitializeMultipleMembersShape(shape_string, shape))
									{
										post_check.shape_array.push_back(shape);
									}
								}
							}
							post_check.expr_object.SetLogicalExpr(A2W(check_object.value.GetString()));
							inspect.modify_array.push_back(post_check);
						}
					}
					api_object.SetPreCheck(inspect);
				}
			}
			// set postcheck information
			if (api_docment.value.HasMember(SI_POSTCHECK) && api_docment.value[SI_POSTCHECK].IsObject())
			{
				for (auto& inspect_object : api_docment.value[SI_POSTCHECK].GetObject())
				{
					if (!inspect_object.value.IsObject())
					{
						continue;
					}
					CApiObject::inspects inspect;
					if (inspect_object.value.HasMember(SI_DEFINE) && inspect_object.value[SI_DEFINE].IsObject())
					{
						for (auto& check_object : inspect_object.value[SI_DEFINE].GetObject())
						{
							CApiObject::parameters param;
							if (!check_object.value.IsString())
							{
								continue;
							}
							if (!api_object.GetParameter(A2W(check_object.name.GetString()), param))
							{
								continue;
							}
							param.type_name = A2W(check_object.value.GetString());
							inspect.define_array.push_back(param);
						}
					}

					if (inspect_object.value.HasMember(SI_LOG) && inspect_object.value[SI_LOG].IsArray())
					{
						for (auto& array_element : inspect_object.value[SI_LOG].GetArray())
						{
							if (array_element.IsString())
							{
								CApiObject::param_shape shape;
								if (api_object.InitializeMultipleMembersShape(A2W(array_element.GetString()), shape))
								{
									inspect.log_array.push_back(shape);
								}
							}
						}
					}

					if (inspect_object.value.HasMember(SI_CHECK) && inspect_object.value[SI_CHECK].IsObject())
					{
						for (auto& check_object : inspect_object.value[SI_CHECK].GetObject())
						{
							CApiObject::checks post_check;
							std::wstring string_array = A2W(check_object.name.GetString());
							std::vector<std::wstring> param_array;
							if (!check_object.value.IsString())
								continue;
							if (post_check.expr_object.SplitStringArray(string_array, param_array))
							{
								for (auto& shape_string : param_array)
								{
									CApiObject::param_shape shape;
									if (api_object.InitializeMultipleMembersShape(shape_string, shape))
									{
										post_check.shape_array.push_back(shape);
									}
									else
										continue;
								}
							}
							else continue;
							post_check.expr_object.SetLogicalExpr(A2W(check_object.value.GetString()));
							if (!post_check.expr_object.InitializeLogicalExpr(param_array))
								continue;
							inspect.check_array.push_back(post_check);
						}
					}

					if (inspect_object.value.HasMember(SI_MODIFY) && inspect_object.value[SI_MODIFY].IsObject())
					{
						for (auto& check_object : inspect_object.value[SI_MODIFY].GetObject())
						{
							CApiObject::checks post_check;
							std::wstring string_array = A2W(check_object.name.GetString());
							std::vector<std::wstring> param_array;
							if (!check_object.value.IsString())
								continue;
							if (post_check.expr_object.SplitStringArray(string_array, param_array))
							{
								for (auto& shape_string : param_array)
								{
									CApiObject::param_shape shape;
									if (api_object.InitializeMultipleMembersShape(shape_string, shape))
									{
										post_check.shape_array.push_back(shape);
									}
								}
							}
							post_check.expr_object.SetLogicalExpr(A2W(check_object.value.GetString()));
							if (!post_check.expr_object.InitializeLogicalExpr(param_array))
								continue;
							inspect.modify_array.push_back(post_check);
						}
					}
					api_object.SetPostCheck(inspect);
				}
			}
			AddApiObject(api_object);
		}
	}
	m_bValid = true;
	return true;
}

CApiObject::CApiObject()
{ 
	m_call_conv = CApiObject::call_unknown;
	m_sign_info.special = false;
	m_logging = false;
	m_hips_object = nullptr;
}
CApiObject::~CApiObject()
{ 
	;
}

void CApiObject::SetParameter(CApiObject::parameters& param)
{
	m_parameters.push_back(param);
}

bool CApiObject::GetParameterOffset(int param_ord, int& offset)
{
	offset = 0;
	bool bret = false;
	if (param_ord >= 1 && param_ord < m_parameters.size())
	{
		CApiObject::parameters param;
		for (int i = 1; i < param_ord; i++)
		{
			param = m_parameters[param_ord - 1];
			offset = param.shape.size;
		}
		bret = true;
	}
	return bret;
}

bool CApiObject::GetParameter(int param_ord, CApiObject::parameters& param)
{
	if (param_ord >= 0 && param_ord < m_parameters.size())
	{
		param = m_parameters[param_ord-1];
		return true;
	}
	return false;
}

bool CApiObject::GetParameter(std::wstring& param_name, parameters& param)
{
	for (int i = 0; i < m_parameters.size(); i++)
	{
		if (wcsicmp(param_name.c_str(), m_parameters[i].name.c_str()) == 0)
		{
			param = m_parameters[i];
			return true;
		}
	}
	return false;
}

bool CApiObject::GetParameter(std::vector<int>& param_ord_ar, std::vector<parameters>& param_ar)
{
	bool bret = false;
	parameters param;
	param_ar.clear();

	for (int i = 0; i < param_ord_ar.size(); i++)
	{
		if (!GetParameter(param_ord_ar[i], param))
		{
			param_ar.clear();
			bret = false;
			break;
		}
		param_ar.push_back(param);
	}
	if (param_ar.size() > 0)
		bret = true;
	return bret;
}

bool CApiObject::GetParameter(std::vector<int>& param_ord_ar, std::vector<std::wstring>& param_name_ar)
{
	bool bret = false;
	parameters param;
	for (int i = 0; i < param_ord_ar.size(); i++)
	{
		if (GetParameter(param_ord_ar[i], param))
		{
			param_name_ar.push_back(param.name);
			continue;
		}
		
		param_name_ar.clear();
		bret = false;
		break;
	}
	return bret;
}

bool CApiObject::GetParameterOrd(std::wstring param_name, int& param_ord)
{
	param_ord = CApiObject::InvalidOrdinal;

	for (int i = 0; i < m_parameters.size(); i++)
	{
		if (wcsicmp(param_name.c_str(), m_parameters[i].name.c_str()) == 0)
		{
			param_ord = i;
			return true;
		}
	}
	return false;
}

bool CApiObject::GetParameterOrd(std::vector<std::wstring>& param_name_ar, std::vector<int>& param_ord_ar)
{
	bool bret = false;
	int param_ord = CApiObject::InvalidOrdinal;
	param_ord_ar.clear();

	for (int i = 0; i < param_name_ar.size(); i++)
	{
		if (!GetParameterOrd(param_name_ar[i], param_ord))
		{
			param_ord_ar.clear();
			bret = false;
			break;
		}
		param_ord_ar.push_back(param_ord);
	}
	if(param_ord_ar.size() > 0)
		bret = true;
	return bret;
}

int CApiObject::GetParamsSize()
{
	int params_size = 0;
	int count = 0;
	// don't add the size of the return value
	for (auto& param : m_parameters)
	{
		if(count != 0)
			params_size += param.shape.size;
		count++;
	}
	return params_size;
}

int CApiObject::GetReturnValueSize()
{
	if (m_parameters.size() == 0) return 0;
	return m_parameters[0].shape.size;
}

bool CApiObject::InitializeMultipleMembersShape(std::wstring& members_string, param_shape& shape)
{
	if (m_hips_object == nullptr) return false;

	std::vector<std::wstring> str_array;
	if (CMathExprObject::SplitStringMember(members_string, str_array))
	{
		CApiObject::parameters param;
		CFlagsConfigObject::TypesInfo types_info;
		shape.shape_name = members_string;
		for (auto& it = str_array.begin(); it != str_array.end(); it++)
		{
			if (it == str_array.begin())
			{
				if (GetParameter((*it), param))
				{
					shape.param_array.push_back(param);
				}
				else if (GetHandle((*it), param))
				{
					shape.param_array.push_back(param);
				}
				else
				{
					except_throw("CSigsConfigObject::Initialize fatal error! this value not define in api struct.");
					return false;
				}
			}
			else
			{
				std::wstring pre_type_name = shape.param_array[shape.param_array.size() - 1].type_name;
				if (m_hips_object->GetTypesInfo(pre_type_name, types_info))
				{
					if (types_info.types != CFlagsConfigObject::types_struct)
					{
						except_throw("CSigsConfigObject::Initialize fatal error! this value is not a struct, but expected to it is a structure type.");
						return false;
					}
					param.name = (*it);
					CFlagsConfigObject::ElementInfo element;
					if (!types_info.struct_info->GetElementType(param.name, element))
					{
						except_throw("CSigsConfigObject::Initialize fatal error!");
					}
					param.type_name = element.value_name;
					param.shape.size = element.element_size;
					int offset = 0;
					if (!types_info.struct_info->GetElementOffset(param.name, offset))
					{
						except_throw("CSigsConfigObject::Initialize fatal error!");
						return false;
					}
					param.shape.offset = offset;
					shape.param_array.push_back(param);
				}
				else
				{
					except_throw("CSigsConfigObject::Initialize fatal error!");
					return false;
				}
			}
		}
	}
	else
	{
		except_throw("CSigsConfigObject::Initialize fatal error!");
		return false;
	}
	return true;
}

float CMathExprObject::Value::toNumber()
{
	if (isToken()) return 0;
	if (isNumber()) return number;

	number = atof(string.c_str());
	type |= va_number;

	return number;
}
std::string CMathExprObject::Value::toString()
{
	if (isToken()) return string;
	if (isString()) return string;

	char str[16];
	sprintf(str, "%f", number);
	string = str;
	type |= va_string;

	return string;
}

CMathExprObject::CMathExprObject()
{
	// 1. Create the operator precedence map.
	m_op_precedence["("] = -10;
	m_op_precedence["&&"] = -2; m_op_precedence["||"] = -3;
	m_op_precedence[">"] = -1; m_op_precedence[">="] = -1;
	m_op_precedence["<"] = -1; m_op_precedence["<="] = -1;
	m_op_precedence["=="] = -1; m_op_precedence["!="] = -1;
	m_op_precedence["<<"] = 1; m_op_precedence[">>"] = 1;
	m_op_precedence["+"] = 2; m_op_precedence["-"] = 2;
	m_op_precedence["*"] = 3; m_op_precedence["/"] = 3;
	m_op_precedence["^"] = 4;
	m_op_precedence["!"] = 5;
}

bool CMathExprObject::InitializeLogicalExpr(std::vector<std::wstring>& param_array)
{
	wchar_t var_constant[20];
	std::wstring logical_expr = GetLogicalExpr();

	for (int i = 0; i < param_array.size(); i++)
	{
		swprintf_s(var_constant, 20, VAR_CONSTANT, i);
		for (std::wstring::size_type found = 0;;)
		{
			found = logical_expr.find(var_constant, found);
			if (found == std::string::npos)
				break;
			logical_expr.replace(found, wcslen(var_constant), param_array[i]);
		}
	}

	if (logical_expr.find(L"%%var") != std::string::npos)
		return false;

	SetLogicalExpr(logical_expr);
	return true;
}

bool CMathExprObject::SplitStringArray(std::wstring input_string, std::vector<std::wstring>& out_array)
{
	out_array.clear();
	if (input_string.length() == 0)
		return false;
	
	for(std::wstring::size_type found = 0;;)
	{
		found = input_string.find(L"|", found);
		if (found == std::string::npos)
			break;
		out_array.push_back(input_string.substr(0, found));
		input_string = input_string.substr(found+1);
	}
	out_array.push_back(input_string);
	return true;
}

size_t CMathExprObject::FindMemberDelimiter(std::wstring input_string, int& ntype)
{
	if (input_string.length() <= 0) return std::string::npos;

	for (int i = 0; i < input_string.length(); i++)
	{
		if (input_string[i] == L'.')
		{
			ntype = 1;
			return i;
		}
		if ((i < input_string.length()-1) && input_string[i] == L'-' && input_string[i+1] == L'>')
		{
			ntype = 2;
			return i;
		}
	}

	return std::string::npos;
}

bool CMathExprObject::SplitStringMember(std::wstring input_string, std::vector<std::wstring>& out_array)
{
	out_array.clear();
	if (input_string.length() == 0)
		return false;
	int ntype = 0;
	for (std::wstring::size_type found = 0;;)
	{
		found = FindMemberDelimiter(input_string, ntype);
		if (found == std::string::npos)
			break;
		out_array.push_back(input_string.substr(0, found));
		if(ntype == 2)
			input_string = input_string.substr(found + 2);
		else
			input_string = input_string.substr(found + 1);
	}
	out_array.push_back(input_string);
	return true;
}

CMathExprObject::ValuePtrQueue CMathExprObject::toRPN(const char* expr, ValueMap* vars, IntMap opPrecedence)
{
	ValuePtrQueue rpnQueue; std::stack<std::string> operatorStack;
	bool lastTokenWasOp = true;

	// In one pass, ignore whitespace and parse the expression into RPN
	// using Dijkstra's Shunting-yard algorithm.
	while (*expr && isspace(*expr)) ++expr;
	while (*expr)
	{
		if (isdigit(*expr))
		{
			// If the token is a number, add it to the output queue.
			char* nextChar = 0;
			float digit = strtod(expr, &nextChar);

			rpnQueue.push(new Value(digit));
			expr = nextChar;
			lastTokenWasOp = false;
		}
		else if (isvariablechar(*expr))
		{
			// If the function is a variable, resolve it and
			// add the parsed number to the output queue.
			if (!vars)
				error_log("Detected variable, but the variable map is null.");
			std::stringstream ss;
			ss << *expr;
			++expr;
			while (isvariablechar(*expr))
			{
				ss << *expr;
				++expr;
			}

			std::string key = ss.str();
			if (key == "true")
				rpnQueue.push(new Value(1));
			else if (key == "false")
				rpnQueue.push(new Value(0));
			else {
				ValueMap::iterator it = vars->find(key);
				if (it == vars->end())
					error_log("Unable to find the variable '" + key + "'.");

				rpnQueue.push(new Value(it->second));
			}

			lastTokenWasOp = false;
		}
		else if (*expr == '\'' || *expr == '"')
		{
			// It's a string value

			char startChr = *expr;

			std::stringstream ss;
			++expr;
			while (*expr && *expr != startChr)
			{
				ss << *expr;
				++expr;
			}
			if (*expr) expr++;

			rpnQueue.push(new Value(ss.str()));
			lastTokenWasOp = false;
		}
		else
		{
			// Otherwise, the variable is an operator or paranthesis.
			switch (*expr) {
			case '(':
				operatorStack.push("(");
				++expr;
				break;
			case ')':
				while (operatorStack.top().compare("(")) {
					rpnQueue.push(new Value(operatorStack.top(), va_token));
					operatorStack.pop();
				}
				operatorStack.pop();
				++expr;
				break;
			default:
			{
				// The token is an operator.
				//
				// Let p(o) denote the precedence of an operator o.
				//
				// If the token is an operator, o1, then
				//   While there is an operator token, o2, at the top
				//       and p(o1) <= p(o2), then
				//     pop o2 off the stack onto the output queue.
				//   Push o1 on the stack.
				std::stringstream ss;
				ss << *expr;
				++expr;
				while (*expr && !isspace(*expr) && !isdigit(*expr)
					&& !isvariablechar(*expr) && *expr != '(' && *expr != ')') {
					ss << *expr;
					++expr;
				}
				ss.clear();
				std::string str;
				ss >> str;

				if (lastTokenWasOp) {
					// Convert unary operators to binary in the RPN.
					if (!str.compare("-") || !str.compare("+") || !str.compare("!"))
						rpnQueue.push(new Value(0));
					else
						error_log("Unrecognized unary operator: '" + str + "'");

				}

				while (!operatorStack.empty() && opPrecedence[str] <= opPrecedence[operatorStack.top()])
				{
					rpnQueue.push(new Value(operatorStack.top(), va_token));
					operatorStack.pop();
				}
				operatorStack.push(str);
				lastTokenWasOp = true;
			}
			}
		}
		while (*expr && isspace(*expr)) ++expr;
	}
	while (!operatorStack.empty()) {
		rpnQueue.push(new Value(operatorStack.top(), va_token));
		operatorStack.pop();
	}
	return rpnQueue;
}

bool CMathExprObject::fnmatch(const char *pat, const char *str) {
	const char *s, *p;
	bool star = false;

loopStart:
	for (s = str, p = pat; *s; ++s, ++p) {
		switch (*p) {
		case '?':
			//if (*s == '.') goto starCheck;
			break;
		case '*':
			star = true;
			str = s, pat = p;
			if (!*++pat) return true;
			goto loopStart;
		default:
			if (towupper(*s) != towupper(*p))
				goto starCheck;
			break;
		} /* endswitch */
	} /* endfor */
	if (*p == '*') ++p;
	return (!*p);

starCheck:
	if (!star) return false;
	str++;
	goto loopStart;
}

bool CMathExprObject::widcardCompare(std::string str, std::string wc)
{
	return fnmatch(wc.c_str(), str.c_str());
}

bool CMathExprObject::Eval(CMathExprObject::Value& value, const char* expr, ValueMap* vars) {

	// Convert to RPN with Dijkstra's Shunting-yard algorithm.
	ValuePtrQueue rpn = toRPN(expr, vars, m_op_precedence);

	// Evaluate the expression in RPN form.
	ValueStack evaluation;

	while (!rpn.empty()) {
		Value* tok = rpn.front();
		rpn.pop();

		if (tok->isToken())
		{
			std::string str = tok->string;
			if (evaluation.size() < 2) {
				error_log("Invalid equation.");
				return false;
			}
			Value right = evaluation.top(); evaluation.pop();
			Value left = evaluation.top(); evaluation.pop();
			if (!str.compare("+") && left.isNumber())
				evaluation.push(left.number + right.toNumber());
			if (!str.compare("+") && left.isString())
				evaluation.push(left.string + right.toString());
			else if (!str.compare("*"))
				evaluation.push(left.toNumber() * right.toNumber());
			else if (!str.compare("-"))
				evaluation.push(left.toNumber() - right.toNumber());
			else if (!str.compare("/"))
			{
				float r = right.toNumber();
				if (r == 0)
					evaluation.push(0);
				else
					evaluation.push(left.toNumber() / r);
			}
			else if (!str.compare("<<"))
				evaluation.push((int)left.toNumber() << (int)right.toNumber());
			else if (!str.compare("^"))
				evaluation.push(pow(left.toNumber(), right.toNumber()));
			else if (!str.compare(">>"))
				evaluation.push((int)left.toNumber() >> (int)right.toNumber());
			else if (!str.compare(">"))
				evaluation.push(left.toNumber() > right.toNumber());
			else if (!str.compare(">="))
				evaluation.push(left.toNumber() >= right.toNumber());
			else if (!str.compare("<"))
				evaluation.push(left.toNumber() < right.toNumber());
			else if (!str.compare("<="))
				evaluation.push(left.toNumber() <= right.toNumber());
			else if (!str.compare("&&"))
				evaluation.push(left.toNumber() && right.toNumber());
			else if (!str.compare("||"))
				evaluation.push(left.toNumber() || right.toNumber());
			else if (!str.compare("=="))
			{
				if (left.isNumber() && right.isNumber())
					evaluation.push(left.number == right.number);
				else if (left.isString() && right.isString())
					evaluation.push(left.string == right.string);
				else if (left.isString())
					evaluation.push(left.string == right.toString());
				else
					evaluation.push(left.toNumber() == right.toNumber());
			}
			else if (!str.compare("!="))
			{
				if (left.isNumber() && right.isNumber())
					evaluation.push(left.number != right.number);
				else if (left.isString() && right.isString())
					evaluation.push(left.string != right.string);
				else if (left.isString())
					evaluation.push(left.string != right.toString());
				else
					evaluation.push(left.toNumber() != right.toNumber());
			}
			else if (!str.compare("=~"))
				evaluation.push(widcardCompare(left.toString(), right.toString()));
			else if (!str.compare("!~"))
				evaluation.push(!widcardCompare(left.toString(), right.toString()));
			else if (!str.compare("!"))
				evaluation.push(!right.toNumber());
			else
			{
				error_log("Unknown operator: " + left.toString() + " " + str + " " + right.toString() + ".");
				return false;
			}
		}
		else if (tok->isNumber() || tok->isString())
		{
			evaluation.push(*tok);
		}
		else
		{
			error_log("Invalid token '" + tok->toString() + "'.");
			return false;
		}

		delete tok;
	}
	value = evaluation.top();
	return true;
}