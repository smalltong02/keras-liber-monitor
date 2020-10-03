#pragma once
#include <stdio.h>
#include <windows.h>
#include <string>
#include <vector>
#include "FlagsObject.h"
#include "ParsingImpl.h"

namespace cchips {

#define COM_METHOD "Methods"
#define COM_MIDL "MIDL_INTERFACE"

    class CWmisCfgObject
    {
    public:
        CWmisCfgObject() { ; }
        CWmisCfgObject(std::string name) { m_name = name; }
        ~CWmisCfgObject() { ; }

        bool InitializeWmiDatas(const std::shared_ptr<CWmiObject>& wmi_object, const ConstRapidObject& Object);
        bool InitializeCheck(const std::shared_ptr<CWmiObject>& wmi_object, const ConstRapidObject& Object);
        bool InitializeWmiMethods(const std::shared_ptr<CWmiObject>& wmi_object, const ConstRapidObject& Object);
        bool InitializeWmiObject(const std::string& object_name, const CRapidJsonWrapper& document);
        bool Initialize(const std::string_view& json_str);
    private:
        bool m_bValid;
        std::string m_name;
        std::string m_category_name;
    };

    class CComsCfgObject
    {
    public:
        CComsCfgObject() { ; }
        CComsCfgObject(std::string name) { m_name = name; }
        ~CComsCfgObject() { ; }

        bool Initialize(const std::string_view& json_str);
    private:
        std::string m_name;
    };

} // namespace cchips
