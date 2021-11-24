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
#define FL_CONFIG "Config"
#define FL_MODE "HipsMode"
#define FL_LOGMODE "LogMode"
#define FL_SIGS "Sigs"
#define FL_COMS "Coms"
#define FL_WMIS "Wmis"
#define FL_FLAGS "Flags"

    class CSigsCfgObject;

    class CHipsCfgObject : public std::enable_shared_from_this<CHipsCfgObject>
    {
    public:
        // save base header information
        using _cfg_mode = enum {
            mode_normal = 0,
            mode_verifier = 1,
        };
        using _hips_mode = enum {
            hips_none_mode = 0,
            hips_hook_mode = 1,
            hips_trace_mode = 2,
        };
        using _log_mode = enum {
            log_none_mode = 0,
            log_pipe_mode = 1,
            log_local_mode = 2,
        };
        using _hips_info = struct {
            std::string Name;
            std::string Version;
            std::string CreateDate;
            std::string Description;
        };

        CHipsCfgObject(_cfg_mode mode = mode_normal) : m_bValid(false), m_cfg_mode(mode), m_hips_mode(hips_none_mode) { }
        ~CHipsCfgObject() = default;

        bool IsHookMode() const { return m_hips_mode & hips_hook_mode; }
        bool IsTraceMode() const { return m_hips_mode & hips_trace_mode; }
        bool IsPipeMode() const { return m_log_mode & log_pipe_mode; }
        bool IsLocalMode() const { return m_log_mode & log_local_mode; }
        bool InitializeFlagsObjects(const HMODULE handle, const CRapidJsonWrapper& document); // initialize flags array.
        bool InitializeSignsObjects(const HMODULE handle, const CRapidJsonWrapper& document); // initialize sigs array.
        bool InitializeWmisObjects(const HMODULE handle, const CRapidJsonWrapper& document); // initialize wmis array.
        bool InitializeComsObjects(const HMODULE handle, const CRapidJsonWrapper& document); // initialize coms array.
        bool Initialize(const std::string_view& json_str);
        void SetCfgMode(_cfg_mode mode = mode_verifier) { m_cfg_mode = mode; }
        static const int InvalidOrdinal = -1;
    private:
        bool m_bValid;
        _cfg_mode m_cfg_mode;
        _hips_mode m_hips_mode;
        _log_mode m_log_mode;
        _hips_info m_Info;
        std::vector<std::unique_ptr<CSigsCfgObject>> m_SigsObjects;
        std::vector<std::unique_ptr<CComsCfgObject>> m_ComsObjects;
        std::vector<std::unique_ptr<CWmisCfgObject>> m_WmisObjects;
        std::vector<std::unique_ptr<CFlagsCfgObject>> m_FlagsObjects;
    };

} // namespace cchips