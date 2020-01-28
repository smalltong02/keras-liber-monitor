#pragma once
#include <stdio.h>
#include <windows.h>
#include <string>
#include <vector>
#include "HipsConfigObject.h"
#include "SigsObject.h"
#include "MinHook.h"

typedef enum _processing_status {
	processing_continue,
	processing_skip,
}processing_status;

#define MACRO_PROCESSING_STACK_(a__, b__, c__) \
	__asm {push ebp}                           \
	__asm {mov  ebp, esp}                      \
	__asm {sub  esp, 0CCh}                     \
	__asm {pushad}                             \
	__asm {mov  a__, ebp}                      \
	__asm {mov  eax, dword ptr[ebp + 4]}       \
	__asm {mov  b__, eax}                      \
	__asm {lea  eax, dword ptr[ebp + 0xC]}     \
	__asm {mov  c__, eax}                      

#define MACRO_CALL_ORGINAL_(a__, b__, c__, d__, e__) \
    __asm {push ebx}                            \
	for (int i = 0; i < b__ / 4; i++)           \
	{                                           \
		__asm {mov  eax, i}                     \
		__asm {imul ebx, eax, 4}                \
		__asm {mov  eax, a__}                   \
		__asm {sub  eax, ebx}                   \
		__asm {mov  eax, [eax]}                 \
		__asm {push eax}                        \
    }                                           \
	__asm {mov  eax, d__}                       \
	__asm {call eax}                            \
    __asm {mov  e__, eax}                       \
    __asm {cmp  b__, 0}                         \
    __asm {jz   std_quit}                       \
    __asm {cmp  c__, 1}                         \
    __asm {jz   cdecl_quit}                     \
    __asm {jmp  std_quit}                       \
    __asm {cdecl_quit:}                         \
    __asm {lea  eax, fix_stack}                 \
    __asm {add  eax, 2}                         \
    __asm {mov  ebx, b__}                       \
	__asm {mov  byte ptr[eax], bl}              \
    __asm {fix_stack:}                          \
    __asm {add  esp, 4}                         \
    __asm {std_quit:}                           \
    __asm {pop  ebx}                            

#define MACRO_PROCESSING_RETURN(a__, b__)       \
	__asm {push ebx}                            \
	__asm {lea  eax, quit}                      \
	__asm {mov  ebx, a__}                       \
	__asm {add  eax, 1}                         \
	__asm {mov  word ptr[eax], bx}              \
	__asm {pop  ebx}                            \
	__asm {popad}                               \
	__asm {mov  eax, b__}                       \
	__asm {add  esp, 0CCh}                      \
	__asm {mov  esp, ebp}                       \
	__asm {pop  ebp}                            \
	__asm {add  esp, 4}                         \
	__asm {quit:}                               \
	__asm {ret 4}                               \


class CHookImplementObject
{
public:
	CHookImplementObject();
	~CHookImplementObject();

	typedef struct _hook_node {
		bool bdelayed;
		CApiObject* api_object;
		void* hook_implement_object;
		void* orgin_api_implfunc;
	}hook_node;

	bool Initialize(CHipsConfigObject* configObject);
	processing_status Preprocessing(CHookImplementObject::hook_node* node_elem, PVOID param_addr, int& params_size, BYTE** new_params);
	processing_status Postprocessing(CHookImplementObject::hook_node* node_elem, PVOID param_addr, int& params_size, BYTE* new_params);

	bool HookAllApi();
	//test
	typedef struct _test_type {
		std::wstring type_name;
		int type_size;
		bool type_success;
		int n_value;
	}test_type;

	typedef struct _test_parameters {
		std::wstring name;
		test_type type_info;
	} test_parameters;

	typedef struct _test_api_struc {
		std::string api_name;
		CApiObject::call_convention call_conv;
		test_type return_va;
		std::vector<test_parameters> param_ar;
	}test_api_struc;

	typedef struct _test_hook_node {
		test_api_struc* api_struct;
		void* hook_implement_object;
		void* orgin_api_implfunc;
	}test_hook_node;

	//bool test_Initialize();
	//void test_HookApi();

private:
	bool m_bValid;
	void* m_hookImplementFunction;
	CHipsConfigObject* m_configObject;
	std::vector<hook_node> m_hookNodeList;
};

