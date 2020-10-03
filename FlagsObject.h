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
#include "MetadataTypeImpl.h"
#include "PackageWrapper.h"

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
        using DelayTp = std::map<std::string, std::string>;
        using StructTp = std::map<std::string, std::shared_ptr<CStructObject>>;
        using DELTPAIR = std::pair<std::string, std::string>;
        using DELSPAIR = std::pair<std::string, std::shared_ptr<CStructObject>>;

        CFlagsCfgObject() : m_bValid(false) { }
        CFlagsCfgObject(std::string name) : m_bValid(false), m_name(name) { }
        ~CFlagsCfgObject() = default;

        void SetFlagsName(const std::string& name) { m_name = name; }
        void InitializeAdditionalType(const CRapidJsonWrapper& document);
        void InitializeStructType(const CRapidJsonWrapper& document);
        void InitializeFlagType(const CRapidJsonWrapper& document);
        bool Initialize(const std::string_view& json_str);

    private:
        bool AddNewType(const IDENPAIR& delay_pair) const;
        bool AddDelayedType(const DELTPAIR& delay_pair);
        bool AddDelayedStruct(const DELSPAIR& delay_pair);
        void ProcessingDelayTypes();
        void ProcessingDelayType();
        void ProcessingDelayStruct();
        bool ProcessingDelayTypes(const std::string& name);
        bool ProcessingDelayType(const DELTPAIR& pair);
        bool ProcessingDelayType(const std::string& name);
        bool ProcessingDelayStruct(const DELSPAIR& pair);
        bool ProcessingDelayStruct(const std::string& name);

        bool m_bValid;
        std::string m_name;
        std::string m_category_name;
        DelayTp m_delayed_type_list;
        StructTp m_delayed_struct_list;
    };

} // namespace cchips