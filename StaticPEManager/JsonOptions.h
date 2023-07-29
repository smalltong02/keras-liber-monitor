#pragma once
#include <sstream>
#include "re2/re2.h"
#include <windows.h>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

#define OP_NAME "Name"
#define OP_VERSION "Version"
#define OP_CREATEDATE "CreateDate"
#define OP_DESCRIPTION "Description"

#define OP_CONFIG "config"
#define CF_IDENTIFIER "identifier"
#define CF_FILETYPE "filetype"
#define CF_PETYPE "petype"
#define CF_SUBSYSTEM "subsystem"
#define CF_OUTPUTTYPE "outputtype"
#define CF_RUNNINGMODE "runningmode"
#define CF_BBASEINFO "bbaseinfo"
#define CF_BVERINFO "bversioninfo"
#define CF_BINSTALLINFO "binstallinfo"
#define CF_BSIGNINFO "bsigninfo"
#define CF_BPEINSIDEINFO "bpeinsideinfo"
#define CF_BPEINSNFLOWINFO "bpeinsnflowinfo"

#define OP_BASEINFO "base_info"
#define BI_BFILENAME "bfilename"
#define BI_BLOCATION "blocation"
#define BI_BFILETYPE "bfiletype"
#define BI_BFILESIZE "bfilesize"
#define BI_BFILEATTRIBUTE "bfileattribute"
#define BI_BSHA256 "bsha256"
#define BI_BSIZEONDISK "bsizeondisk"
#define BI_BCREATETIME "bcreatetime"
#define BI_BMODIFYTIME "bmodifytime"

#define OP_VERSIONINFO "version_info"
#define VI_BDESCRIPTION "bdescription"
#define VI_BTYPE "btype"
#define VI_BVERSION "bversion"
#define VI_BPRODUCTNAME "bproductname"
#define VI_BPRODUCTVERSION "bproductversion"
#define VI_BCOPYRIGHT "bcopyright"

#define OP_INSTALLINFO "install_info"
#define II_BNAME "bname"
#define II_BVERSION "bversion"
#define II_BPID "bpid"
#define II_BLOCATION "blocation"
#define II_BPUBLISHER "bpublisher"

#define OP_SIGNATUREINFO "signature_info"
#define SI_BROOTCERT "brootcert"
#define SI_BCOUNTERSIGN "bcountersign"
#define SI_BNESTEDSIGN "bnestedsign"
#define SI_CERTIFICATEINFO "certificate_info"
#define CI_BDIGEST "bdigestalgorithm"
#define CI_BVERSION "bversion"
#define CI_BSERIALNUMBER "bserialnumber"
#define CI_BSIGNALGORITHM "bsignalgorithm"
#define CI_BSIGNHASHALGORITHM "bsignhashalgorithm"
#define CI_BISSUER "bissuer"
#define CI_BTIMEFROM "btimefrom"
#define CI_BTIMETO "btimeto"
#define CI_BSUBJECT "bsubject"
#define CI_BPUBLICKEY "bpublickey"
#define CI_BPUBLICKEYALGORITHM "bpublickeyalgorithm"
#define CI_BPUBLICKEYPARAM "bpublickeyparam"
#define CI_BAUTHORITHKEYIDEN "bauthoritykeyiden"
#define CI_BSUBJECTKEYIDEN "bsubjectkeyiden"
#define CI_BSUBJECTALTERNAME "bsubjectaltername"
#define CI_BENHANCEKEYUSAGE "benhancedkeyusage"
#define CI_BCRLDISTRUBTIONPOINTS "bcrldistrubtionpoints"
#define CI_BCERTIFICATEPOLICIES "bcertificatepolicies"
#define CI_BAUTHORITYINFORMATION "bauthorityinformation"
#define CI_BKEYUSAGE "bkeyusage"
#define CI_BBASICCONSTRAINTS "bbasicconstraints"
#define CI_BTHUMBPRINT "bthumbprint"
#define CI_BISSUERTHUMBPRINT "bissuerthumbprint"

#define OP_PEINSIDEINFO "peinside_info"
#define PI_BPEBASICFEATURE "bpebasicfeature"
#define PI_BSTATFEATURE "bstatfeature"
#define PI_BCROSSREFFEATURE "bcrossreffeature"
#define PI_BPACKERFEATURE "bpackerfeature"
#define PI_BDATADIRECTORYFEATURE "bdatadirectoryfeature"
#define PI_BDEBUGDATADIRECTORYFEATURE "bdebugdatadirectoryfeature"
#define PI_BEMBEDDEDFEATURE "bembeddedfeature"
#define PI_BIMPORTTABLEFEATURE "bimporttablefeature"
#define PI_BEXPORTTABLEFEATURE "bexporttablefeature"
#define PI_BICONFEATURE "biconfeature"
#define PI_BMANIFEASTFEATURE "bmanifestfeature"
#define PI_BRESOURCEFEATURE "bresourcefeature"
#define PI_BRICHHEADERFEATURE "brichheaderfeature"
#define PI_BSECTIONFEATURE "bsectionfeature"
#define PI_BSTRINGSFEATURE "bstringsfeature"
#define PI_BUNPACKERFEATURE "bunpackerfeature"
#define PI_BDOTNETFEATURE "bdotnetfeature"

#define OP_PEINSNFLOWINFO "peinsnflow_info"
#define FI_APIFLOWFEATURE "apiflowfeature"
#define FI_BLOCKFLOWFEATURE "blockflowfeature"
#define FI_INSNFLOWFEATURE "insnflowfeature"

namespace cchips {
    struct CJsonOptions
    {
    public:
        enum class _info_type {
            info_type_unknown,
            info_type_base,
            info_type_version,
            info_type_install,
            info_type_signature,
            info_type_peinside,
            info_type_peinsnflow,
        };

        struct _config {
            using _runningmode = enum {
                mode_normal = 0,
                mode_debug,
                mode_performance,
            };

            using _outputtype = enum {
                output_logformat = 0,
                output_jsonformat,
                output_pipeformat,
                output_ipformat,
                output_mongodbformat,
                output_memformat,
            };

            std::string identifier;
            std::string filetype;
            std::string petype;
            std::string subsystem;
            std::string outputtype;
            _runningmode runningmode;
            bool bbaseinfo;
            bool bversioninfo;
            bool binstallinfo;
            bool bsigninfo;
            bool bpeinsideinfo;
            bool bpeinsnflowinfo;

            bool MatchSubsystem(const std::string& type_str) const {
                if (!_pattern_subsystem) {
                    if (subsystem.length()) {
                        _pattern_subsystem = std::make_unique<RE2>(subsystem, RE2::Quiet);
                    }
                    if (!_pattern_subsystem) {
                        return false;
                    }
                }
                return RE2::FullMatch(type_str, *_pattern_subsystem);
            }
            bool MatchFileType(const std::string& file_str) const {
                if (!_pattern_filetype) {
                    if (petype.length()) {
                        _pattern_filetype = std::make_unique<RE2>(filetype, RE2::Quiet);
                    }
                    if (!_pattern_filetype) {
                        return false;
                    }
                }
                return RE2::FullMatch(file_str, *_pattern_filetype);
            }
            bool MatchPeType(const std::string& pe_str) const {
                if (!_pattern_petype) {
                    if (petype.length()) {
                        _pattern_petype = std::make_unique<RE2>(petype, RE2::Quiet);
                    }
                    if (!_pattern_petype) {
                        return false;
                    }
                }
                return RE2::FullMatch(pe_str, *_pattern_petype);
            }
            bool IsDebugMode() const {
                if (runningmode == _runningmode::mode_debug)
                    return true;
                return false;
            }
            bool IsPerformanceMode() const {
                if (runningmode == _runningmode::mode_performance)
                    return true;
                return false;
            }
            bool IsNormalMode() const {
                if (IsDebugMode() || IsPerformanceMode())
                    return false;
                return true;
            }
            _outputtype GetOutputType() const {
                _outputtype format = _outputtype::output_logformat;
                if (outputtype.empty()) {
                    return format;
                }
                if (_stricmp(outputtype.c_str(), "logfile") == 0) {
                    format = _outputtype::output_logformat;
                }
                else if (_stricmp(outputtype.c_str(), "json") == 0) {
                    format = _outputtype::output_jsonformat;
                }
                else if (_stricmp(outputtype.c_str(), "pipe") == 0) {
                    format = _outputtype::output_pipeformat;
                }
                else if (_stricmp(outputtype.c_str(), "traffic") == 0) {
                    format = _outputtype::output_ipformat;
                }
                else if (_stricmp(outputtype.c_str(), "mongodb") == 0) {
                    format = _outputtype::output_mongodbformat;
                }
                else if (_stricmp(outputtype.c_str(), "memory") == 0) {
                    format = _outputtype::output_memformat;
                }

                return format;
            }
        private:
            mutable std::unique_ptr<RE2> _pattern_subsystem{};
            mutable std::unique_ptr<RE2> _pattern_petype{};
            mutable std::unique_ptr<RE2> _pattern_filetype{};
        };

        using _base_info = struct {
            bool bfilename;
            bool blocation;
            bool bfiletype;
            bool bfilesize;
            bool bfileattribute;
            bool bsha256;
            bool bsizeondisk;
            bool bcreatetime;
            bool bmodifytime;
        };

        using _version_info = struct {
            bool bdescription;
            bool btype;
            bool bversion;
            bool bproductname;
            bool bproductversion;
            bool bcopyright;
        };

        using _install_info = struct {
            bool bname;
            bool bversion;
            bool bpid;
            bool blocation;
            bool bpublisher;
        };

        using _certificate_info = struct {
            bool bdigestalgorithm;
            bool bversion;
            bool bserialnumber;
            bool bsignalgorithm;
            bool bsignhashalgorithm;
            bool bissuer;
            bool btimefrom;
            bool btimeto;
            bool bsubject;
            bool bpublickey;
            bool bpublickeyalgorithm;
            bool bpublickeyparam;
            bool bauthoritykeyiden;
            bool bsubjectkeyiden;
            bool bsubjectaltername;
            bool benhancedkeyusage;
            bool bcrldistrubtionpoints;
            bool bcertificatepolicies;
            bool bauthorityinformation;
            bool bkeyusage;
            bool bbasicconstraints;
            bool bthumbprint;
            bool bissuerthumbprint;
        };

        using _signature_info = struct {
            bool brootcert;
            bool bcountersign;
            bool bnestedsign;
            _certificate_info certificate_info;
        };

        using _peinside_info = struct {
            bool bpebasicfeature;
            bool bstatfeature;
            bool bcrossreffeature;
            bool bpackerfeature;
            bool bdatadirectoryfeature;
            bool bdebugdatadirectoryfeature;
            bool bembeddedfeature;
            bool bimporttablefeature;
            bool bexporttablefeature;
            bool biconfeature;
            bool bmanifestfeature;
            bool bresourcefeature;
            bool brichheaderfeature;
            bool bsectionfeature;
            bool bstringsfeature;
            bool bunpackerfeature;
            bool bdotnetfeature;
        };

        using _peinsnflow_info = struct {
            bool apiflowfeature;
            bool blockflowfeature;
            bool insnflowfeature;
        };

        using _options_info = struct {
            std::string name;
            std::string version;
            std::string createdate;
            std::string description;
            _config config;
            _base_info base_info;
            _version_info version_info;
            _install_info install_info;
            _signature_info signature_info;
            _peinside_info peinside_info;
            _peinsnflow_info peinsnflow_info;
        };

        CJsonOptions(const std::string& config_path);
        CJsonOptions(const std::string& res_text, unsigned int res_id);
        ~CJsonOptions();

        CJsonOptions() = delete;
        CJsonOptions(const CJsonOptions&) = delete;
        void operator=(const CJsonOptions&) = delete;

        bool Parse();
        bool RequestBaseInfo() const { return m_options_info.config.bbaseinfo; }
        bool RequestVersionInfo() const { return m_options_info.config.bversioninfo; }
        bool RequestInstallInfo() const { return m_options_info.config.binstallinfo; }
        bool RequestSignatureInfo() const { return m_options_info.config.bsigninfo; }
        bool RequestPeinsideInfo() const { return m_options_info.config.bpeinsideinfo; }
        bool RequestPeinsnflowInfo() const { return m_options_info.config.bpeinsnflowinfo; }

        const _config& GetConfigInfo() const { return m_options_info.config; }
        bool GetOptionsInfo(_info_type type, std::pair<unsigned char*, size_t> pinfo);
    private:
        std::vector<BYTE> m_options_buffer;
        _options_info m_options_info{};
    };
} // namespace cchips
