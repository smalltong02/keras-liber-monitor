//===- HipsCfgObject.h - metadata type implement define -----------------------------*- C++ -*-===//
// 
// This file contains CHipsCfgObject class, the class read config and generate basic data structure.
//
//===------------------------------------------------------------------------------------------===//
#pragma once
#include <stdio.h>
#include <windows.h>
#include <string>
#include <vector>
#include <map>
#include "SigsObject.h"
#include "FlagsObject.h"
#include "ComsObject.h"
#include "rapidjson\document.h"

namespace cchips {

#define except_throw()

#define FL_NAME "Name"
#define FL_VERSION "Version"
#define FL_CREATEDATE "CreateDate"
#define FL_DESCRIPTION "Description"
#define FL_SIGS "Sigs"
#define FL_COMS "Coms"
#define FL_WMIS "Wmis"
#define FL_FLAGS "Flags"

    class CSigsCfgObject;

    class CHipsCfgObject : public std::enable_shared_from_this<CHipsCfgObject>
    {
    public:
        // save base header information
        typedef struct _HipsInfo {
            std::string Name;
            std::string Version;
            std::string CreateDate;
            std::string Description;
        }HipsInfo, *PHipsInfo;

        CHipsCfgObject() : m_bValid(false) { }
        ~CHipsCfgObject() = default;

        bool InitializeFlagsObjects(const HMODULE handle, const rapidjson::Document& document); // initialize flags array.
        bool InitializeSignsObjects(const HMODULE handle, const rapidjson::Document& document); // initialize sigs array.
        bool InitializeWmisObjects(const HMODULE handle, const rapidjson::Document& document); // initialize wmis array.
        bool InitializeComsObjects(const HMODULE handle, const rapidjson::Document& document); // initialize coms array.
        bool Initialize(const std::string& json_str);
        static const int InvalidOrdinal = -1;
    private:
        bool m_bValid;
        HipsInfo m_Info;
        std::vector<std::unique_ptr<CSigsCfgObject>> m_SigsObjects;
        std::vector<std::unique_ptr<CComsCfgObject>> m_ComsObjects;
        std::vector<std::unique_ptr<CWmisCfgObject>> m_WmisObjects;
        std::vector<std::unique_ptr<CFlagsCfgObject>> m_FlagsObjects;
    };

} // namespace cchips