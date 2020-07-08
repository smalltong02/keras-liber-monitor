//===- FlagsObject.h - metadata type implement define -----------------------------*- C++ -*-===//
// 
// This file contains CFlagsCfgObject class, the class read config and generate type define of cchips.
//
//===----------------------------------------------------------------------------------------===//
#pragma once
#include <cstdio>
#include <string>
#include <vector>
#include <map>
#include "rapidjson\document.h"
#include "MetadataTypeImpl.h"

namespace cchips {

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

    class CFlagsCfgObject
    {
    public:
        typedef std::map<std::string, std::string> DelayTp;
        typedef std::map<std::string, std::shared_ptr<CStructObject>> StructTp;

        CFlagsCfgObject() : m_bValid(false) { }
        CFlagsCfgObject(std::string name) : m_bValid(false), m_name(name) { }
        ~CFlagsCfgObject() = default;

        void SetFlagsName(const std::string& name) { m_name = name; }
        bool InitializeAdditionalType(const rapidjson::Document& document);
        bool InitializeStructType(const rapidjson::Document& document);
        bool InitializeFlagType(const rapidjson::Document& document);
        bool Initialize(const std::string& json_str);

    private:
        bool AddNewType(const IDENPAIR& delay_pair) const;
        bool AddDelayedType(const std::pair<std::string, std::string>& delay_pair);
        bool AddDelayedStruct(const std::pair<std::string, std::shared_ptr<CStructObject>>& delay_pair);
        void ProcessingDelayTypes();
        void ProcessingDelayType();
        void ProcessingDelayStruct();
        bool ProcessingDelayTypes(const std::string& name);
        bool ProcessingDelayType(const std::string& name);
        bool ProcessingDelayStruct(const std::string& name);

        bool m_bValid;
        std::string m_name;
        std::string m_category_name;
        DelayTp m_delayed_type_list;
        StructTp m_delayed_struct_list;
    };

} // namespace cchips