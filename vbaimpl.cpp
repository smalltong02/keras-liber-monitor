#include "stdafx.h"
#include "hookimpl.h"
#include "HookImplementObject.h"
#include "utils.h"
#include "vbaimpl.h"
#include "UniversalObject.h"

using namespace cchips;

typedef std::map<int, std::wstring> ObjectList;
typedef std::list<std::wstring>    StringList;
ObjectList  g_object_list;
std::wstring g_function_name;

std::wstring GetObjectName(int Index)
{
    std::wstring name;

    if (g_object_list[(int)Index].length() == 0)
    {
        wchar_t buf[20] = {};
        swprintf(buf, L"%#x", Index);
        name = buf;
    }
    else
        name = g_object_list[(int)Index].c_str();
    return name;
}

bool CheckArgumentsValid(VARIANT* arguments, int count)
{
    bool bret = false;

    if (IsBadReadPtr(arguments, 4) != 0) return bret;

    for (int i = 0; i < count; i++)
    {
        switch (arguments[i].vt)
        {
        case VT_BSTR:
        case (VT_BSTR | VT_BYREF):
        case VT_I2:
        case VT_I4:
            bret = true;
            break;
        default:
            bret = false;
        }
    }
    return bret;
}

processing_status WINAPI CHookImplementObject::detour_internalFindWindowsExW(detour_node* node, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD param3)
{
    PRE_BEGIN(node)
    return processing_continue;
}

processing_status WINAPI CHookImplementObject::detour_rtcCreateObject2(detour_node* node, DWORD param1, LPCWSTR szName, DWORD param2)
{
    PRE_BEGIN(node)
    if (!szName) return processing_continue;
    BEGIN_LOG("CreateObject");
    if (std::stringstream str_val = OutputAnyValue(std::wstring(szName)); str_val.str().length())
        LOGGING("object_name", str_val.str());
    END_LOG(node->log_entry);
    return processing_continue;
}

processing_status WINAPI CHookImplementObject::detour_rtcCreateObject2_post(detour_node* node, DWORD param1, LPCWSTR szName, DWORD param2)
{
    POST_BEGIN(node)
    if (!szName) return processing_continue;
    std::shared_ptr<CObObject> return_ptr = node->function->GetIdentifier(SI_RETURN);
    if (!return_ptr) return processing_continue;
    std::any anyvalue = return_ptr->GetValue(static_cast<char*>(node->return_va));
    if (!anyvalue.has_value() || anyvalue.type() != typeid(LPVOID))
        return processing_continue;
    if (LPVOID ret_va = std::any_cast<LPVOID>(anyvalue); ret_va != nullptr) {
        VARIANT* ret = static_cast<VARIANT*>(ret_va);
        g_object_list[(int)ret->pdispVal] = szName;
    }
    return processing_continue;
}

processing_status WINAPI CHookImplementObject::detour_rtcCallByName(detour_node* node, LPVOID param1, LPVOID param2, LPCWSTR procName, DWORD callType, LPVOID* args, LPVOID param3)
{
    PRE_BEGIN(node)
    if (!procName) return processing_continue;
    if (!param2) return processing_continue;
    BEGIN_LOG("Call");
    if (std::stringstream str_val = OutputAnyValue(GetObjectName((int)param2)); str_val.str().length())
        LOGGING("object_name", str_val.str());
    if (std::stringstream str_val = OutputAnyValue(std::wstring(procName)); str_val.str().length())
        LOGGING("proc_name", str_val.str());
    std::stringstream ss; ss << callType;
    LOGGING("call_type", ss.str());
    END_LOG(node->log_entry);

    return processing_continue;
}

processing_status WINAPI CHookImplementObject::detour_rtcCallByName_post(detour_node* node, LPVOID param1, LPVOID param2, LPCWSTR procName, DWORD callType, LPVOID* args, LPVOID param3)
{
    POST_BEGIN(node)
    if (!procName) return processing_continue;
    std::shared_ptr<CObObject> return_ptr = node->function->GetIdentifier(SI_RETURN);
    if (!return_ptr) return processing_continue;
    std::any anyvalue = return_ptr->GetValue(static_cast<char*>(node->return_va));
    if (!anyvalue.has_value() || anyvalue.type() != typeid(LPVOID))
        return processing_continue;
    static std::wstring pre_proc_name;
    if (LPVOID ret_va = std::any_cast<LPVOID>(anyvalue); ret_va != nullptr) {
        VARIANT* ret = static_cast<VARIANT*>(ret_va);
        
        wchar_t* p_ret_name = nullptr;
        if (ret->vt == VT_BSTR)
        {
            p_ret_name = ret->bstrVal;
        }
        else if (ret->vt == (VT_BSTR | VT_BYREF))
        {
            p_ret_name = *ret->pbstrVal;
        }
        else if (ret->vt == VT_I4)
        {
            if (wcsicmp(procName, L"count") == 0 && wcsicmp(pre_proc_name.c_str(), L"tasks") == 0)
            {
                ret->lVal = 55;
            }
        }
        else if (ret->vt == VT_BOOL)
        {
            if (wcsicmp(procName, L"exists") == 0 && wcsicmp(pre_proc_name.c_str(), L"tasks") == 0)
            {
                if (!args) return processing_continue;
                ret->boolVal = FALSE;
                std::wstring task_name;
                SAFEARRAY* arg_array = *(SAFEARRAY **)args;
                if (!arg_array) return processing_continue;
                VARIANT *pv = (VARIANT *)arg_array->pvData;
                if (!pv) return processing_continue;
                for (int i = 0; i < arg_array->rgsabound[0].cElements; i++)
                {
                    if (pv->vt == (VT_BSTR | VT_BYREF))
                    {
                        task_name = *pv->pbstrVal;
                        break;
                    }
                    else if (pv->vt == VT_BSTR)
                    {
                        task_name = pv->bstrVal;
                        break;
                    }
                    pv++;
                }

                if (wcsicmp(task_name.c_str(), L"vbox") == 0 ||
                    wcsicmp(task_name.c_str(), L"vmware") == 0 ||
                    wcsicmp(task_name.c_str(), L"vmtools") == 0 ||
                    wcsicmp(task_name.c_str(), L"autoit") == 0 ||
                    wcsicmp(task_name.c_str(), L"wireshark") == 0 ||
                    wcsicmp(task_name.c_str(), L"process monitor") == 0 ||
                    wcsicmp(task_name.c_str(), L"fiddler") == 0 ||
                    wcsicmp(task_name.c_str(), L"tcpview") == 0 ||
                    wcsicmp(task_name.c_str(), L"vxstream") == 0 ||
                    wcsicmp(task_name.c_str(), L"process explorer") == 0)
                {
                    BEGIN_LOG("antivm");
                    if (std::stringstream str_val = OutputAnyValue(GetObjectName((int)param2)); str_val.str().length())
                        LOGGING("object_name", str_val.str());
                    if (std::stringstream str_val = OutputAnyValue(std::wstring(procName)); str_val.str().length())
                        LOGGING("proc_name", str_val.str());
                    if (std::stringstream str_val = OutputAnyValue(std::wstring(task_name)); str_val.str().length())
                        LOGGING("search", str_val.str());
                    std::stringstream ss; ss << callType;
                    LOGGING("call_type", ss.str());
                    END_LOG(node->log_entry);
                }
            }
        }

        if (p_ret_name != nullptr && wcsicmp(p_ret_name, L"VirtualBox") == 0)
        {
            memcpy(p_ret_name, L"Dell PC_48", sizeof(L"Dell PC_48"));
            BEGIN_LOG("antivm");
            if (std::stringstream str_val = OutputAnyValue(GetObjectName((int)param2)); str_val.str().length())
                LOGGING("object_name", str_val.str());
            if (std::stringstream str_val = OutputAnyValue(std::wstring(procName)); str_val.str().length())
                LOGGING("proc_name", str_val.str());
            LOGGING("return", "VirtualBox");
            LOGGING("modify", "Dell PC_48");
            std::stringstream ss; ss << callType;
            LOGGING("call_type", ss.str());
            END_LOG(node->log_entry);
        }
    }
    return processing_continue;
}

processing_status WINAPI CHookImplementObject::detour_rtcGetObject(detour_node* node, LPVOID param1, LPVOID object_var, LPVOID param2)
{
    PRE_BEGIN(node)
    if (!object_var) return processing_continue;
    BEGIN_LOG("GetObject");
    VARIANT* object = static_cast<VARIANT*>(object_var);
    wchar_t* object_name = nullptr;
    if (object->vt == VT_BSTR)
    {
        object_name = object->bstrVal;
    }

    if (object->vt == (VT_BSTR | VT_BYREF))
    {
        object_name = *object->pbstrVal;
    }
    if (std::stringstream str_val = OutputAnyValue(std::wstring(object_name)); str_val.str().length())
        LOGGING("object_name", str_val.str());
    END_LOG(node->log_entry);
    return processing_continue;
}
 
processing_status WINAPI CHookImplementObject::detour_rtcGetObject_post(detour_node* node, LPVOID param1, LPVOID object_var, LPVOID param2)
{
    POST_BEGIN(node)
    if (!object_var) return processing_continue;
    std::shared_ptr<CObObject> return_ptr = node->function->GetIdentifier(SI_RETURN);
    if (!return_ptr) return processing_continue;
    std::any anyvalue = return_ptr->GetValue(static_cast<char*>(node->return_va));
    if (!anyvalue.has_value() || anyvalue.type() != typeid(LPVOID))
        return processing_continue;
    if (LPVOID ret_va = std::any_cast<LPVOID>(anyvalue); ret_va != nullptr) {
        VARIANT* ret = static_cast<VARIANT*>(ret_va);
        VARIANT* object = static_cast<VARIANT*>(object_var);
        if (object->vt == VT_BSTR)
        {
            g_object_list[(int)ret->pdispVal] = object->bstrVal;
        }

        if (object->vt == (VT_BSTR | VT_BYREF))
        {
            g_object_list[(int)ret->pdispVal] = *object->pbstrVal;
        }
    }
    return processing_continue;
}

processing_status WINAPI CHookImplementObject::detour_rtcShell(detour_node* node, LPVOID shell_var, LPVOID param1)
{
    PRE_BEGIN(node)
    if (!shell_var) return processing_continue;
    BEGIN_LOG("Shell");
    wchar_t null_name[] = L"<NULL>";
    wchar_t* path_name = null_name;
    VARIANT* object = static_cast<VARIANT*>(shell_var);
    if (object->vt == VT_BSTR)
    {
        path_name = object->bstrVal;
    }

    if (object->vt == (VT_BSTR | VT_BYREF))
    {
        path_name = *object->pbstrVal;
    }
    if (std::stringstream str_val = OutputAnyValue(std::wstring(path_name)); str_val.str().length())
        LOGGING("path_name", str_val.str());
    END_LOG(node->log_entry);
    return processing_continue;
}

processing_status WINAPI CHookImplementObject::detour_rtcCallFunction(detour_node* node, LPCWSTR functionName, INT param1)
{
    PRE_BEGIN(node)
    if (!functionName) return processing_continue;
    g_function_name = functionName;
    return processing_continue;
}

processing_status WINAPI CHookImplementObject::detour_rtcComCall(detour_node* node, INT param1, INT param2, INT argType, LPDWORD arguments, INT argCount, INT param3)
{
    PRE_BEGIN(node)
    if (g_function_name.length() > 0)
    {
        VARIANT *var_args = nullptr;
        if (argType == 1)
        {
            if (CheckArgumentsValid((VARIANT *)arguments, argCount))
                var_args = (VARIANT *)arguments;
            else
            {
                if (CheckArgumentsValid((VARIANT *)(*(arguments + 3)), argCount))
                    var_args = (VARIANT *)(*(arguments + 3));
            }
        }
        else if (argType == 3)
        {
            if (CheckArgumentsValid((VARIANT *)(*(arguments + 3)), argCount))
                var_args = (VARIANT *)(*(arguments + 3));
            else
            {
                if (CheckArgumentsValid((VARIANT *)arguments, argCount))
                    var_args = (VARIANT *)arguments;
            }
        }

        for (int i = (argCount - 1); i >= 0; i--)
        {
            if (var_args[i].vt == VT_BSTR)
            {
                int length = wcslen(var_args[i].bstrVal);
                if (length > wcslen(L":Zone.Identifier"))
                {
                    if (wcsicmp(g_function_name.c_str(), L"FileExists") == 0)
                    {
                        if (wcsicmp((wchar_t*)&(var_args[i].bstrVal[length / sizeof(wchar_t) - wcslen(L":Zone.Identifier")]), L":Zone.Identifier") == 0)
                            var_args[i].bstrVal[length / sizeof(wchar_t) - wcslen(L":Zone.Identifier")] = L'\0';
                    }
                }
            }
            else if (var_args[i].vt == (VT_BSTR | VT_BYREF))
            {
                int length = wcslen(*var_args[i].pbstrVal);
                if (length / sizeof(wchar_t) > wcslen(L":Zone.Identifier"))
                {
                    if (wcsicmp(g_function_name.c_str(), L"FileExists") == 0)
                    {
                        if (wcsicmp((wchar_t*)&((*var_args[i].pbstrVal)[length / sizeof(wchar_t) - wcslen(L":Zone.Identifier")]), L":Zone.Identifier") == 0)
                            (*var_args[i].pbstrVal)[length / sizeof(wchar_t) - wcslen(L":Zone.Identifier")] = L'\0';
                    }
                }
            }
        }

        BEGIN_LOG("ComCall");
        if (std::stringstream str_val = OutputAnyValue(std::wstring(g_function_name)); str_val.str().length())
            LOGGING("func_name", str_val.str());
        END_LOG(node->log_entry);
        g_function_name.clear();
    }
    return processing_continue;
}

processing_status WINAPI CHookImplementObject::detour_rtcSetProp(detour_node* node, INT param1, INT param2, LPDWORD arguments, INT param3, INT param4)
{
    PRE_BEGIN(node)
    if (g_function_name.length() > 0)
    {
        BEGIN_LOG("SetProp");
        if (std::stringstream str_val = OutputAnyValue(std::wstring(g_function_name)); str_val.str().length())
            LOGGING("func_name", str_val.str());
        END_LOG(node->log_entry);
        g_function_name.clear();
    }
    return processing_continue;
}
