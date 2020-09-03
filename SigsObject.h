#pragma once
#include <stdio.h>
#include <windows.h>
#include <string>
#include <vector>
#include <queue>
#include <stack>
#include "HipsCfgObject.h"
#include "FlagsObject.h"
#include "ParsingImpl.h"

namespace cchips {

#define SI_CATEGORY "Category"
#define SI_CALL "Calling convention"

#define SI_SIGNATURE "Signature"
#define SI_CLASS "Class"
#define SI_PARAMETERS "Parameters"
#define SI_HANDLE "Handle"
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
#define SI_FEATURE "Feature"

#define SI_CTYPE "Type"
#define SI_CNAME "Name"
#define SI_CDELAY "Delay"
#define SI_CVTBLIDX "VtblIdx"

#define SI_CCODE "CCode"
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

    class CSigsCfgObject
    {
    public:
        CSigsCfgObject() = delete;
        CSigsCfgObject(const std::string& name) : m_bValid(false), m_name(name) {}
        ~CSigsCfgObject() = default;

        static bool InitializeCheck(const std::shared_ptr<CFunction>& func_object, const ConstRapidObject& Object);
        static bool InitializeLog(const std::shared_ptr<CFunction>& func_object, const ConstRapidObject& Object);
        static bool InitializeEnsure(const std::shared_ptr<CFunction>& func_object, const ConstRapidObject& Object);
        static bool InitializeHandle(const std::shared_ptr<CFunction>& func_object, const ConstRapidObject& Object);
        static bool InitializePrototype(const std::shared_ptr<CFunction>& func_object, const ConstRapidObject& Object);
//        static bool InitializeCCode(const std::shared_ptr<CFunction>& func_object, const ConstRapidObject& Object);
        static bool InitializeFunction(CFunction::_call_convention call_conv, const std::string& func_name, const ConstRapidObject& Object, std::shared_ptr<CFunctionProtos> func_protos = nullptr);
        bool Initialize(const std::string_view& json_str);

        static const int InvalidOrdinal = -1;
    private:
        bool m_bValid;
        std::string m_name;
        std::string m_category_name;
        CFunction::_call_convention GetCallConvention(const std::string_view& str_call) const;
    };

} // namespace cchips


