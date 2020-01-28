#pragma once
#include <stdio.h>
#include <windows.h>
#include <string>
#include <vector>
#include <queue>
#include <stack>
#include "HipsConfigObject.h"
#include "FlagsObject.h"

#define SI_CATEGORY "Category"
#define SI_CALL "Calling convention"

#define SI_SIGNATURE "Signature"
#define SI_PARAMETERS "Parameters"
#define SI_ENSURE "Ensure"
#define SI_PRELOG "PreLog"
#define SI_POSTLOG "PostLog"
#define SI_PRECHECK "PreCheck"
#define SI_POSTCHECK "PostCheck"
#define SI_LOGGING "Logging"

#define SI_LIBRARY "Library"
#define SI_RETURN "Return"
#define SI_SUCCESS "Success"
#define SI_FAILED "Failed"
#define SI_SPECIAL "Special"

#define SI_CHECK "Check"
#define SI_MODIFY "Modify"
#define SI_LOG "Log"
#define SI_DEFINE "Define"

#define CCS_WINAPI "WINAPI"
#define CCS_CDECL "CDECL"
#define CCS_FASTCALL "FASTCALL"
#define CCS_STDCALL "STDCALL"
#define CCS_VECTORCALL "VECTORCALL"

#define VAR_CONSTANT L"%%var%d"

class CHipsConfigObject;

class CMathExprObject
{
public:
#define isvariablechar(c) (isalpha(c) || c == '_')

	enum ValueType
	{
		va_undefined = 0,
		va_token,
		va_number,
		va_string,

	};
	struct Value
	{
		Value() :type(va_undefined) {};
		Value(std::string str, ValueType t) :type(t), string(str) {};
		Value(std::string str) :type(va_string), string(str) {};
		Value(float n) :type(va_number), number(n) {};
		Value& operator=(std::string str) {
			type = va_string;
			string = str;
			return *this;
		}
		Value& operator=(float n) {
			type = va_number;
			number = n;
			return *this;
		}

		unsigned type;
		float number;
		std::string string;

		bool isToken()const { return type == va_token; };
		bool isString()const { return !isToken() && (type & va_string); };
		bool isNumber()const { return !isToken() && (type & va_number); };

		float toNumber();
		std::string toString();
	};

	typedef std::map<std::string, Value> ValueMap;
	typedef std::queue<Value*> ValuePtrQueue;
	typedef std::stack<Value> ValueStack;
	typedef std::map<std::string, int> IntMap;

	CMathExprObject();
	~CMathExprObject() { ; }

	static size_t FindMemberDelimiter(std::wstring input_string, int& ntype);
	static bool SplitStringArray(std::wstring input_string, std::vector<std::wstring>& out_array);
	static bool SplitStringMember(std::wstring input_string, std::vector<std::wstring>& out_array);

	bool IsValid() { return m_logical_expr.length() != 0; }
	void SetLogicalExpr(std::wstring logical_expr) { m_logical_expr = logical_expr; }
	std::wstring& GetLogicalExpr() { return m_logical_expr; }
	bool InitializeLogicalExpr(std::vector<std::wstring>& param_array);
private:
	static ValuePtrQueue toRPN(const char* expr, ValueMap* vars, IntMap opPrecedence);
	static bool fnmatch(const char *pat, const char *str);
	static bool widcardCompare(std::string str, std::string wc);
	bool Eval(Value& value, const char* expr, ValueMap* vars = 0);

	IntMap m_op_precedence;
	std::wstring m_logical_expr;
};

class CApiObject
{
public:
	typedef enum _call_convention {
		call_unknown = 0,
		call_cdecl,
		call_fastcall,
		call_stdcall,
		call_vectorcall,
	}call_convention;
	typedef struct _signature_info {
		std::wstring library;
		bool special;
	} signature_info;
	typedef struct _data_shape {
		int offset;
		int size;
	} data_shape;
	typedef struct _parameters {
		data_shape shape;
		std::wstring name;
		std::wstring type_name;
	} parameters;
	typedef struct _param_shape {
		std::wstring shape_name;
		std::vector<parameters> param_array;
	} param_shape;
	typedef struct _checks {
		std::vector<param_shape> shape_array;
		CMathExprObject expr_object;
	}checks;
	typedef struct _inspects {
		std::vector<parameters> define_array;
		std::vector<checks> check_array;
		std::vector<checks> modify_array;
		std::vector<param_shape> log_array;
	}inspects;

	CApiObject();
	~CApiObject();

	void SetApiName(std::wstring name) { m_name = name; }
	std::wstring GetApiName() { return m_name; }
	void SetCallConvention(call_convention call_conv) { m_call_conv = call_conv; }
	call_convention GetCallConvention() { return m_call_conv; }
	void SetLibrary(std::wstring library) { m_sign_info.library = library; }
	std::wstring GetLibrary() { return m_sign_info.library; }
	void SetSpecial(bool bvalue) { m_sign_info.special = bvalue; }
	bool GetSpecial() { return m_sign_info.special; }
	void SetParameter(parameters& param);
	bool GetParameter(int param_ord, parameters& param);
	bool GetParameter(std::wstring& param_name, parameters& param);
	bool GetParameterOffset(int param_ord, int& offset);
	bool GetParameter(std::vector<int>& param_ord_ar, std::vector<std::wstring>& param_name_ar);
	bool GetParameter(std::vector<int>& param_ord_ar, std::vector<parameters>& param_ar);
	bool GetParameterOrd(std::wstring param_name, int& param_ord);
	bool GetParameterOrd(std::vector<std::wstring>& param_name_ar, std::vector<int>& param_ord_ar);
	int GetParamsSize();
	int GetReturnValueSize();
	bool GetHandle(std::wstring& param_name, parameters& param) { return false; }
	void SetEnsure(param_shape shape) { m_ensure.push_back(shape); }
	void SetPrelog(param_shape shape) { m_prelog.push_back(shape); m_logging = true; }
	void SetPostlog(param_shape shape) { m_postlog.push_back(shape); m_logging = true; }
	void SetPreCheck(inspects& pre_check) { m_precheck.push_back(pre_check); }
	void SetPostCheck(inspects& post_check) { m_postcheck.push_back(post_check); }
	void SetLogging() { m_logging = true; }
	void SetHipsObject(CHipsConfigObject* hips_object) { m_hips_object = hips_object; }
	bool InitializeMultipleMembersShape(std::wstring& members_string, param_shape& shape);

	bool GetLogging() { return m_logging; }
	std::vector<param_shape>& GetPrelog() { return m_prelog; }
	std::vector<inspects>& GetPreCheck() { return m_precheck; }
	std::vector<inspects>& GetPostCheck() { return m_postcheck; }
	std::vector<param_shape>& GetPostlog() { return m_postlog; }

	static const int InvalidOrdinal = -1;
private:
	call_convention m_call_conv;
	signature_info m_sign_info;
	std::wstring m_name;
	std::vector<parameters> m_parameters;
	std::vector<parameters> m_prehandles;
	std::vector<parameters> m_posthandles;
	std::vector<param_shape> m_ensure;
	std::vector<inspects> m_precheck;
	std::vector<inspects> m_postcheck;
	std::vector<param_shape> m_prelog;
	std::vector<param_shape> m_postlog;
	bool m_logging;
	CHipsConfigObject* m_hips_object;
};

class CSigsConfigObject
{
public:
	CSigsConfigObject();
	CSigsConfigObject(std::wstring name, CHipsConfigObject* p_hipsobject);
	~CSigsConfigObject() { ; }

	void SetSigsName(std::wstring name) { m_name = name; }
	void SetHipsObject(CHipsConfigObject* p_hipsobject) { m_hips_object = p_hipsobject; }
	bool Initialize(std::string& json_str);
	void AddApiObject(CApiObject& api_object) { m_api_list.push_back(api_object); }
	std::vector<CApiObject>& GetApiObjects() { return m_api_list; }

	static const int InvalidOrdinal = -1;
private:
	bool m_bValid;
	std::wstring m_name;
	std::wstring m_category_name;
	std::vector<CApiObject> m_api_list;
	CHipsConfigObject* m_hips_object;
	CApiObject::call_convention GetCallConvention(std::string str_call);
};


