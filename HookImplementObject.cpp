#include "stdafx.h"
#include "utils.h"
#include "HookImplementObject.h"

#pragma code_seg(".wrtest")
__declspec(naked) void STDMETHODCALLTYPE _hookImplementFunction(void)
{
	ULONG* __stack;
	void* __addr;
	CHookImplementObject::hook_node* __data;
	CHookImplementObject* __object;
	int __return;
	int __nsize;
	__asm {int 3}
	MACRO_PROCESSING_STACK_(__stack, __data, __addr)
	if (__data != nullptr && __addr != nullptr && (__object = (CHookImplementObject*)__data->hook_implement_object) != nullptr)
	{
		__nsize = 0; __return = 0;
		BYTE* new_params = nullptr;
		processing_status __status = __object->Preprocessing(__data, __addr, __nsize, &new_params);
		if (__status == processing_continue)
		{
			void* __func = __data->orgin_api_implfunc;
			char** __ptr = nullptr;
			if (new_params != nullptr)
				__ptr = (char**)((ULONG_PTR)new_params + __nsize - 4);
			else
				__ptr = (char**)((ULONG_PTR)__addr + __nsize - 4);
			CApiObject::call_convention __call_conv = __data->api_object->GetCallConvention();
			MACRO_CALL_ORGINAL_(__ptr, __nsize, __call_conv, __func, __return)
			__status = __object->Postprocessing(__data, __addr, __nsize, new_params);
		}
	}
	MACRO_PROCESSING_RETURN(__nsize, __return)
}

#pragma code_seg()
#pragma comment(linker, "/SECTION:.wrtest,ERW")

CHookImplementObject::CHookImplementObject()
{
	m_bValid = false;
	m_hookImplementFunction = (void*)_hookImplementFunction;
	m_configObject = nullptr;
}

CHookImplementObject::~CHookImplementObject()
{
	m_bValid = false;
	m_configObject = nullptr;
}

bool CHookImplementObject::Initialize(CHipsConfigObject* configObject)
{
	m_configObject = configObject;

	if (m_hookImplementFunction != nullptr)
	{
		MH_STATUS Status = MH_Initialize();
		if(Status == MH_OK)
			m_bValid = true;
	}
	return m_bValid;
}

bool CHookImplementObject::HookAllApi()
{
	if (!m_bValid)
		return false;

	int count = m_configObject->GetApiCount();
	hook_node init_node = { 0 };
	m_hookNodeList.resize(count + 5, init_node);

	count = 0;
	for (auto& sigs_obj : m_configObject->GetSigsObjects())
	{
		for (auto& api_obj : sigs_obj->GetApiObjects())
		{
			m_hookNodeList[count].orgin_api_implfunc = nullptr;
			m_hookNodeList[count].hook_implement_object = this;
			m_hookNodeList[count].api_object = &api_obj;
			
			MH_STATUS status;
			status = MH_CreateHookApiEx(api_obj.GetLibrary().c_str(), (LPCSTR)(W2A(api_obj.GetApiName()).c_str()), _hookImplementFunction, &(m_hookNodeList[count].orgin_api_implfunc), (LPVOID*)&m_hookNodeList[count], NULL);
			if (status == MH_OK)
			{
				error_log("api hook create: %ws success!", api_obj.GetApiName().c_str());
			}
			else
			{
				if (status == MH_ERROR_MODULE_NOT_FOUND)
				{
					// delayed hook;
					m_hookNodeList[count].bdelayed = true;
				}
				else
				{
					error_log("api hook create: %ws failed(%d)!", api_obj.GetApiName().c_str(), status);
				}
			}
			count++;
		}
	}

	MH_EnableHook(MH_ALL_HOOKS);
	return true;
}

processing_status CHookImplementObject::Preprocessing(CHookImplementObject::hook_node* node_elem, PVOID param_addr, int& params_size, BYTE** new_params)
{
//	bool blogging = false;
	if (node_elem->api_object == nullptr || node_elem->hook_implement_object == nullptr || node_elem->orgin_api_implfunc == nullptr)
		return processing_skip;
	if (new_params != nullptr) new_params = nullptr;
//	CApiObject* api_object = node_elem->api_object;
//	CHookImplementObject* hook_implement_object = (CHookImplementObject*)node_elem->hook_implement_object;
	if (node_elem->api_object->GetSpecial())
	{
		// If there are API hooks in the upper layer that have been processed, this hook will not be processed when Special is true
	}

	//params_size = api_object->GetParamsSize();
	//int param_offset = 0;
	//CApiObject::parameters param;
	//CFlagsConfigObject::TypesInfo types_info;
	//for (auto& param_ord : api_object->GetPrelog())
	//{
	//	int a = param_ord;
	//	a = 0;
	//	hook_implement_object = nullptr;
	//	param_offset = 0;
	//}

	//for (auto& check : api_object->GetPreCheck())
	//{
	//	int a = check.param_array.size();
	//	a = 0;
	//}
	return processing_continue;
}

processing_status CHookImplementObject::Postprocessing(CHookImplementObject::hook_node* node_elem, PVOID param_addr, int& params_size, BYTE* new_params)
{
	if(new_params != nullptr)
		free(new_params);
	return processing_continue;
}



