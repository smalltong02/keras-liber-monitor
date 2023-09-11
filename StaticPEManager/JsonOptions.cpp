#include <iostream>
#include <fstream>
#include "JsonOptions.h"
#include "..\LogObject.h"
#include "..\utils.h"
#include "PackageWrapper.h"
#include "resource.h"

using namespace cchips;

namespace cchips {
    CJsonOptions::CJsonOptions(const std::string& config_path)
    {
        if (config_path.empty()) {
            error_log("config_path empty!");
            return;
        }
        if (!fs::exists(config_path)) {
            error_log("options file %s is not exist!", config_path.c_str());
            return;
        }
        if (!fs::is_regular_file(config_path)) {
            error_log("options file %s is not a regular file!", config_path.c_str());
            return;
        }
        auto filesize = fs::file_size(config_path);
        if (filesize == static_cast<uintmax_t>(-1) || filesize == 0) {
            error_log("get options file %s size failed!", config_path.c_str());
            return;
        }

        std::ifstream infile;
        infile.open(config_path, std::ios::in | std::ios::binary | std::ios::ate);
        if (!infile.is_open()) {
            error_log("open options file %s failed!", config_path.c_str());
            return;
        }
        infile.seekg(0, std::ios::beg);
        m_options_buffer.resize(filesize);
        infile.read((char*)&m_options_buffer[0], filesize);
        auto readed = infile.tellg();
        if (!readed) {
            error_log("read options file %s error!", config_path.c_str());
            m_options_buffer.resize(0);
            return;
        }
        info_log("read options file %s success!", config_path.c_str());
        return;
    }

    CJsonOptions::CJsonOptions(const std::string& res_text, unsigned int res_id)
    {
        if (res_text.empty()) {
            error_log("res_text empty!");
            return;
        }
        HMODULE ModuleHandle = GetModuleHandle(NULL);
        if (ModuleHandle == NULL) {
            error_log("GetModuleHandle failed!");
            return;
        }

        bool result = ExtractResource(ModuleHandle,
            res_text.c_str(),
            MAKEINTRESOURCE(res_id),
            m_options_buffer);
        if (result && m_options_buffer.size() > 0) {
            info_log("ExtractResource IDR_JSONCONFIG success!");
        }
        else {
            error_log("ExtractResource IDR_JSONCONFIG failed!");
        }
        return;
    }

    CJsonOptions::~CJsonOptions()
    {
        return;
    }

    bool CJsonOptions::Parse()
    {
        if (!m_options_buffer.size())
            return false;
        CRapidJsonWrapper document(std::string_view((char*)&m_options_buffer[0], m_options_buffer.size()));
        if (!document.IsValid()) {
            //config data is incorrect.
            error_log("config data is incorrect.");
            return false;
        }

        if (auto anyvalue(document.GetMember(std::vector<std::string>{OP_NAME}));
            anyvalue.has_value() && anyvalue.type() == typeid(std::string_view))
            m_options_info.name = std::any_cast<std::string_view>(anyvalue);
        if (auto anyvalue(document.GetMember(std::vector<std::string>{OP_VERSION}));
            anyvalue.has_value() && anyvalue.type() == typeid(std::string_view))
            m_options_info.version = std::any_cast<std::string_view>(anyvalue);
        if (auto anyvalue(document.GetMember(std::vector<std::string>{OP_CREATEDATE}));
            anyvalue.has_value() && anyvalue.type() == typeid(std::string_view))
            m_options_info.createdate = std::any_cast<std::string_view>(anyvalue);
        if (auto anyvalue(document.GetMember(std::vector<std::string>{OP_DESCRIPTION}));
            anyvalue.has_value() && anyvalue.type() == typeid(std::string_view))
            m_options_info.description = std::any_cast<std::string_view>(anyvalue);

        if (auto anyvalue(document.GetMember(std::vector<std::string>{OP_CONFIG}));
            anyvalue.has_value() && anyvalue.type() == typeid(ConstRapidObject)) {
            if (auto configvalue(document.GetMember(std::vector<std::string>{OP_CONFIG, CF_IDENTIFIER}));
                configvalue.has_value() && configvalue.type() == typeid(std::string_view)) {
                m_options_info.config.identifier = std::any_cast<std::string_view>(configvalue);
            }
            if (auto configvalue(document.GetMember(std::vector<std::string>{OP_CONFIG, CF_FILETYPE}));
                configvalue.has_value() && configvalue.type() == typeid(std::string_view)) {
                m_options_info.config.filetype = std::any_cast<std::string_view>(configvalue);
            }
            if (auto configvalue(document.GetMember(std::vector<std::string>{OP_CONFIG, CF_PETYPE}));
                configvalue.has_value() && configvalue.type() == typeid(std::string_view)) {
                m_options_info.config.petype = std::any_cast<std::string_view>(configvalue);
            }
            if (auto configvalue(document.GetMember(std::vector<std::string>{OP_CONFIG, CF_EXTRATYPE}));
                configvalue.has_value() && configvalue.type() == typeid(std::string_view)) {
                m_options_info.config.extratype = std::any_cast<std::string_view>(configvalue);
            }
            if (auto configvalue(document.GetMember(std::vector<std::string>{OP_CONFIG, CF_SUBSYSTEM}));
                configvalue.has_value() && configvalue.type() == typeid(std::string_view)) {
                m_options_info.config.subsystem = std::any_cast<std::string_view>(configvalue);
            }
            if (auto configvalue(document.GetMember(std::vector<std::string>{OP_CONFIG, CF_OUTPUTTYPE}));
                configvalue.has_value() && configvalue.type() == typeid(std::string_view)) {
                m_options_info.config.outputtype = std::any_cast<std::string_view>(configvalue);
            }
            if (auto configvalue(document.GetMember(std::vector<std::string>{OP_CONFIG, CF_RUNNINGMODE}));
                configvalue.has_value() && configvalue.type() == typeid(int)) {
                m_options_info.config.runningmode = static_cast<_config::_runningmode>(std::any_cast<int>(configvalue));
            }
            if (auto configvalue(document.GetMember(std::vector<std::string>{OP_CONFIG, CF_BBASEINFO}));
                configvalue.has_value() && configvalue.type() == typeid(bool)) {
                m_options_info.config.bbaseinfo = std::any_cast<bool>(configvalue);
            }
            if (auto configvalue(document.GetMember(std::vector<std::string>{OP_CONFIG, CF_BVERINFO}));
                configvalue.has_value() && configvalue.type() == typeid(bool)) {
                m_options_info.config.bversioninfo = std::any_cast<bool>(configvalue);
            }
            if (auto configvalue(document.GetMember(std::vector<std::string>{OP_CONFIG, CF_BINSTALLINFO}));
                configvalue.has_value() && configvalue.type() == typeid(bool)) {
                m_options_info.config.binstallinfo = std::any_cast<bool>(configvalue);
            }
            if (auto configvalue(document.GetMember(std::vector<std::string>{OP_CONFIG, CF_BSIGNINFO}));
                configvalue.has_value() && configvalue.type() == typeid(bool)) {
                m_options_info.config.bsigninfo = std::any_cast<bool>(configvalue);
            }
            if (auto configvalue(document.GetMember(std::vector<std::string>{OP_CONFIG, CF_BPEINSIDEINFO}));
                configvalue.has_value() && configvalue.type() == typeid(bool)) {
                m_options_info.config.bpeinsideinfo = std::any_cast<bool>(configvalue);
            }
            if (auto configvalue(document.GetMember(std::vector<std::string>{OP_CONFIG, CF_BPEINSNFLOWINFO}));
                configvalue.has_value() && configvalue.type() == typeid(bool)) {
                m_options_info.config.bpeinsnflowinfo = std::any_cast<bool>(configvalue);
            }
        }
    
        if (auto anyvalue(document.GetMember(std::vector<std::string>{OP_BASEINFO}));
            anyvalue.has_value() && anyvalue.type() == typeid(ConstRapidObject)) {
            if (auto basevalue(document.GetMember(std::vector<std::string>{OP_BASEINFO, BI_BFILENAME}));
                basevalue.has_value() && basevalue.type() == typeid(bool)) {
                m_options_info.base_info.bfilename = std::any_cast<bool>(basevalue);
            }
            if (auto basevalue(document.GetMember(std::vector<std::string>{OP_BASEINFO, BI_BLOCATION}));
                basevalue.has_value() && basevalue.type() == typeid(bool)) {
                m_options_info.base_info.blocation = std::any_cast<bool>(basevalue);
            }
            if (auto basevalue(document.GetMember(std::vector<std::string>{OP_BASEINFO, BI_BFILETYPE}));
                basevalue.has_value() && basevalue.type() == typeid(bool)) {
                m_options_info.base_info.bfiletype = std::any_cast<bool>(basevalue);
            }
            if (auto basevalue(document.GetMember(std::vector<std::string>{OP_BASEINFO, BI_BFILESIZE}));
                basevalue.has_value() && basevalue.type() == typeid(bool)) {
                m_options_info.base_info.bfilesize = std::any_cast<bool>(basevalue);
            }
            if (auto basevalue(document.GetMember(std::vector<std::string>{OP_BASEINFO, BI_BFILEATTRIBUTE}));
                basevalue.has_value() && basevalue.type() == typeid(bool)) {
                m_options_info.base_info.bfileattribute = std::any_cast<bool>(basevalue);
            }
            if (auto basevalue(document.GetMember(std::vector<std::string>{OP_BASEINFO, BI_BSHA256}));
                basevalue.has_value() && basevalue.type() == typeid(bool)) {
                m_options_info.base_info.bsha256 = std::any_cast<bool>(basevalue);
            }
            if (auto basevalue(document.GetMember(std::vector<std::string>{OP_BASEINFO, BI_BSIZEONDISK}));
                basevalue.has_value() && basevalue.type() == typeid(bool)) {
                m_options_info.base_info.bsizeondisk = std::any_cast<bool>(basevalue);
            }
            if (auto basevalue(document.GetMember(std::vector<std::string>{OP_BASEINFO, BI_BCREATETIME}));
                basevalue.has_value() && basevalue.type() == typeid(bool)) {
                m_options_info.base_info.bcreatetime = std::any_cast<bool>(basevalue);
            }
            if (auto basevalue(document.GetMember(std::vector<std::string>{OP_BASEINFO, BI_BMODIFYTIME}));
                basevalue.has_value() && basevalue.type() == typeid(bool)) {
                m_options_info.base_info.bmodifytime = std::any_cast<bool>(basevalue);
            }
        }

        if (auto anyvalue(document.GetMember(std::vector<std::string>{OP_VERSIONINFO}));
            anyvalue.has_value() && anyvalue.type() == typeid(ConstRapidObject)) {
            if (auto versionvalue(document.GetMember(std::vector<std::string>{OP_VERSIONINFO, VI_BDESCRIPTION}));
                versionvalue.has_value() && versionvalue.type() == typeid(bool)) {
                m_options_info.version_info.bdescription = std::any_cast<bool>(versionvalue);
            }
            if (auto versionvalue(document.GetMember(std::vector<std::string>{OP_VERSIONINFO, VI_BTYPE}));
                versionvalue.has_value() && versionvalue.type() == typeid(bool)) {
                m_options_info.version_info.btype = std::any_cast<bool>(versionvalue);
            }
            if (auto versionvalue(document.GetMember(std::vector<std::string>{OP_VERSIONINFO, VI_BVERSION}));
                versionvalue.has_value() && versionvalue.type() == typeid(bool)) {
                m_options_info.version_info.bversion = std::any_cast<bool>(versionvalue);
            }
            if (auto versionvalue(document.GetMember(std::vector<std::string>{OP_VERSIONINFO, VI_BPRODUCTNAME}));
                versionvalue.has_value() && versionvalue.type() == typeid(bool)) {
                m_options_info.version_info.bproductname = std::any_cast<bool>(versionvalue);
            }
            if (auto versionvalue(document.GetMember(std::vector<std::string>{OP_VERSIONINFO, VI_BPRODUCTVERSION}));
                versionvalue.has_value() && versionvalue.type() == typeid(bool)) {
                m_options_info.version_info.bproductversion = std::any_cast<bool>(versionvalue);
            }
            if (auto versionvalue(document.GetMember(std::vector<std::string>{OP_VERSIONINFO, VI_BCOPYRIGHT}));
                versionvalue.has_value() && versionvalue.type() == typeid(bool)) {
                m_options_info.version_info.bcopyright = std::any_cast<bool>(versionvalue);
            }
        }

        if (auto anyvalue(document.GetMember(std::vector<std::string>{OP_INSTALLINFO}));
            anyvalue.has_value() && anyvalue.type() == typeid(ConstRapidObject)) {
            if (auto installvalue(document.GetMember(std::vector<std::string>{OP_INSTALLINFO, II_BNAME}));
                installvalue.has_value() && installvalue.type() == typeid(bool)) {
                m_options_info.install_info.bname = std::any_cast<bool>(installvalue);
            }
            if (auto installvalue(document.GetMember(std::vector<std::string>{OP_INSTALLINFO, II_BVERSION}));
                installvalue.has_value() && installvalue.type() == typeid(bool)) {
                m_options_info.install_info.bversion = std::any_cast<bool>(installvalue);
            }
            if (auto installvalue(document.GetMember(std::vector<std::string>{OP_INSTALLINFO, II_BPID}));
                installvalue.has_value() && installvalue.type() == typeid(bool)) {
                m_options_info.install_info.bpid = std::any_cast<bool>(installvalue);
            }
            if (auto installvalue(document.GetMember(std::vector<std::string>{OP_INSTALLINFO, II_BLOCATION}));
                installvalue.has_value() && installvalue.type() == typeid(bool)) {
                m_options_info.install_info.blocation = std::any_cast<bool>(installvalue);
            }
            if (auto installvalue(document.GetMember(std::vector<std::string>{OP_INSTALLINFO, II_BPUBLISHER}));
                installvalue.has_value() && installvalue.type() == typeid(bool)) {
                m_options_info.install_info.bpublisher = std::any_cast<bool>(installvalue);
            }
        }

        if (auto anyvalue(document.GetMember(std::vector<std::string>{OP_SIGNATUREINFO}));
            anyvalue.has_value() && anyvalue.type() == typeid(ConstRapidObject)) {
            if (auto signaturevalue(document.GetMember(std::vector<std::string>{OP_SIGNATUREINFO, SI_BROOTCERT}));
                signaturevalue.has_value() && signaturevalue.type() == typeid(bool)) {
                m_options_info.signature_info.brootcert = std::any_cast<bool>(signaturevalue);
            }
            if (auto signaturevalue(document.GetMember(std::vector<std::string>{OP_SIGNATUREINFO, SI_BCOUNTERSIGN}));
                signaturevalue.has_value() && signaturevalue.type() == typeid(bool)) {
                m_options_info.signature_info.bcountersign = std::any_cast<bool>(signaturevalue);
            }
            if (auto signaturevalue(document.GetMember(std::vector<std::string>{OP_SIGNATUREINFO, SI_BNESTEDSIGN}));
                signaturevalue.has_value() && signaturevalue.type() == typeid(bool)) {
                m_options_info.signature_info.bnestedsign = std::any_cast<bool>(signaturevalue);
            }
            if (auto signaturevalue(document.GetMember(std::vector<std::string>{OP_SIGNATUREINFO, SI_CERTIFICATEINFO}));
                signaturevalue.has_value() && signaturevalue.type() == typeid(ConstRapidObject)) {
                if (auto certificatevalue(document.GetMember(std::vector<std::string>{OP_SIGNATUREINFO, SI_CERTIFICATEINFO, CI_BDIGEST}));
                    certificatevalue.has_value() && certificatevalue.type() == typeid(bool)) {
                    m_options_info.signature_info.certificate_info.bdigestalgorithm = std::any_cast<bool>(certificatevalue);
                }
                if (auto certificatevalue(document.GetMember(std::vector<std::string>{OP_SIGNATUREINFO, SI_CERTIFICATEINFO, CI_BVERSION}));
                    certificatevalue.has_value() && certificatevalue.type() == typeid(bool)) {
                    m_options_info.signature_info.certificate_info.bversion = std::any_cast<bool>(certificatevalue);
                }
                if (auto certificatevalue(document.GetMember(std::vector<std::string>{OP_SIGNATUREINFO, SI_CERTIFICATEINFO, CI_BSERIALNUMBER}));
                    certificatevalue.has_value() && certificatevalue.type() == typeid(bool)) {
                    m_options_info.signature_info.certificate_info.bserialnumber = std::any_cast<bool>(certificatevalue);
                }
                if (auto certificatevalue(document.GetMember(std::vector<std::string>{OP_SIGNATUREINFO, SI_CERTIFICATEINFO, CI_BSIGNALGORITHM}));
                    certificatevalue.has_value() && certificatevalue.type() == typeid(bool)) {
                    m_options_info.signature_info.certificate_info.bsignalgorithm = std::any_cast<bool>(certificatevalue);
                }
                if (auto certificatevalue(document.GetMember(std::vector<std::string>{OP_SIGNATUREINFO, SI_CERTIFICATEINFO, CI_BSIGNHASHALGORITHM}));
                    certificatevalue.has_value() && certificatevalue.type() == typeid(bool)) {
                    m_options_info.signature_info.certificate_info.bsignhashalgorithm = std::any_cast<bool>(certificatevalue);
                }
                if (auto certificatevalue(document.GetMember(std::vector<std::string>{OP_SIGNATUREINFO, SI_CERTIFICATEINFO, CI_BISSUER}));
                    certificatevalue.has_value() && certificatevalue.type() == typeid(bool)) {
                    m_options_info.signature_info.certificate_info.bissuer = std::any_cast<bool>(certificatevalue);
                }
                if (auto certificatevalue(document.GetMember(std::vector<std::string>{OP_SIGNATUREINFO, SI_CERTIFICATEINFO, CI_BTIMEFROM}));
                    certificatevalue.has_value() && certificatevalue.type() == typeid(bool)) {
                    m_options_info.signature_info.certificate_info.btimefrom = std::any_cast<bool>(certificatevalue);
                }
                if (auto certificatevalue(document.GetMember(std::vector<std::string>{OP_SIGNATUREINFO, SI_CERTIFICATEINFO, CI_BTIMETO}));
                    certificatevalue.has_value() && certificatevalue.type() == typeid(bool)) {
                    m_options_info.signature_info.certificate_info.btimeto = std::any_cast<bool>(certificatevalue);
                }
                if (auto certificatevalue(document.GetMember(std::vector<std::string>{OP_SIGNATUREINFO, SI_CERTIFICATEINFO, CI_BSUBJECT}));
                    certificatevalue.has_value() && certificatevalue.type() == typeid(bool)) {
                    m_options_info.signature_info.certificate_info.bsubject = std::any_cast<bool>(certificatevalue);
                }
                if (auto certificatevalue(document.GetMember(std::vector<std::string>{OP_SIGNATUREINFO, SI_CERTIFICATEINFO, CI_BPUBLICKEY}));
                    certificatevalue.has_value() && certificatevalue.type() == typeid(bool)) {
                    m_options_info.signature_info.certificate_info.bpublickey = std::any_cast<bool>(certificatevalue);
                }
                if (auto certificatevalue(document.GetMember(std::vector<std::string>{OP_SIGNATUREINFO, SI_CERTIFICATEINFO, CI_BPUBLICKEYALGORITHM}));
                    certificatevalue.has_value() && certificatevalue.type() == typeid(bool)) {
                    m_options_info.signature_info.certificate_info.bpublickeyalgorithm = std::any_cast<bool>(certificatevalue);
                }
                if (auto certificatevalue(document.GetMember(std::vector<std::string>{OP_SIGNATUREINFO, SI_CERTIFICATEINFO, CI_BPUBLICKEYPARAM}));
                    certificatevalue.has_value() && certificatevalue.type() == typeid(bool)) {
                    m_options_info.signature_info.certificate_info.bpublickeyparam = std::any_cast<bool>(certificatevalue);
                }
                if (auto certificatevalue(document.GetMember(std::vector<std::string>{OP_SIGNATUREINFO, SI_CERTIFICATEINFO, CI_BAUTHORITHKEYIDEN}));
                    certificatevalue.has_value() && certificatevalue.type() == typeid(bool)) {
                    m_options_info.signature_info.certificate_info.bauthoritykeyiden = std::any_cast<bool>(certificatevalue);
                }
                if (auto certificatevalue(document.GetMember(std::vector<std::string>{OP_SIGNATUREINFO, SI_CERTIFICATEINFO, CI_BSUBJECTKEYIDEN}));
                    certificatevalue.has_value() && certificatevalue.type() == typeid(bool)) {
                    m_options_info.signature_info.certificate_info.bsubjectkeyiden = std::any_cast<bool>(certificatevalue);
                }
                if (auto certificatevalue(document.GetMember(std::vector<std::string>{OP_SIGNATUREINFO, SI_CERTIFICATEINFO, CI_BSUBJECTALTERNAME}));
                    certificatevalue.has_value() && certificatevalue.type() == typeid(bool)) {
                    m_options_info.signature_info.certificate_info.bsubjectaltername = std::any_cast<bool>(certificatevalue);
                }
                if (auto certificatevalue(document.GetMember(std::vector<std::string>{OP_SIGNATUREINFO, SI_CERTIFICATEINFO, CI_BENHANCEKEYUSAGE}));
                    certificatevalue.has_value() && certificatevalue.type() == typeid(bool)) {
                    m_options_info.signature_info.certificate_info.benhancedkeyusage = std::any_cast<bool>(certificatevalue);
                }
                if (auto certificatevalue(document.GetMember(std::vector<std::string>{OP_SIGNATUREINFO, SI_CERTIFICATEINFO, CI_BCRLDISTRUBTIONPOINTS}));
                    certificatevalue.has_value() && certificatevalue.type() == typeid(bool)) {
                    m_options_info.signature_info.certificate_info.bcrldistrubtionpoints = std::any_cast<bool>(certificatevalue);
                }
                if (auto certificatevalue(document.GetMember(std::vector<std::string>{OP_SIGNATUREINFO, SI_CERTIFICATEINFO, CI_BCERTIFICATEPOLICIES}));
                    certificatevalue.has_value() && certificatevalue.type() == typeid(bool)) {
                    m_options_info.signature_info.certificate_info.bcertificatepolicies = std::any_cast<bool>(certificatevalue);
                }
                if (auto certificatevalue(document.GetMember(std::vector<std::string>{OP_SIGNATUREINFO, SI_CERTIFICATEINFO, CI_BAUTHORITYINFORMATION}));
                    certificatevalue.has_value() && certificatevalue.type() == typeid(bool)) {
                    m_options_info.signature_info.certificate_info.bauthorityinformation = std::any_cast<bool>(certificatevalue);
                }
                if (auto certificatevalue(document.GetMember(std::vector<std::string>{OP_SIGNATUREINFO, SI_CERTIFICATEINFO, CI_BKEYUSAGE}));
                    certificatevalue.has_value() && certificatevalue.type() == typeid(bool)) {
                    m_options_info.signature_info.certificate_info.bkeyusage = std::any_cast<bool>(certificatevalue);
                }
                if (auto certificatevalue(document.GetMember(std::vector<std::string>{OP_SIGNATUREINFO, SI_CERTIFICATEINFO, CI_BBASICCONSTRAINTS}));
                    certificatevalue.has_value() && certificatevalue.type() == typeid(bool)) {
                    m_options_info.signature_info.certificate_info.bbasicconstraints = std::any_cast<bool>(certificatevalue);
                }
                if (auto certificatevalue(document.GetMember(std::vector<std::string>{OP_SIGNATUREINFO, SI_CERTIFICATEINFO, CI_BTHUMBPRINT}));
                    certificatevalue.has_value() && certificatevalue.type() == typeid(bool)) {
                    m_options_info.signature_info.certificate_info.bthumbprint = std::any_cast<bool>(certificatevalue);
                }
                if (auto certificatevalue(document.GetMember(std::vector<std::string>{OP_SIGNATUREINFO, SI_CERTIFICATEINFO, CI_BISSUERTHUMBPRINT}));
                    certificatevalue.has_value() && certificatevalue.type() == typeid(bool)) {
                    m_options_info.signature_info.certificate_info.bissuerthumbprint = std::any_cast<bool>(certificatevalue);
                }
            }
        }

        if (auto anyvalue(document.GetMember(std::vector<std::string>{OP_PEINSIDEINFO}));
            anyvalue.has_value() && anyvalue.type() == typeid(ConstRapidObject)) {
            if (auto peinsidevalue(document.GetMember(std::vector<std::string>{OP_PEINSIDEINFO, PI_BPEBASICFEATURE}));
                peinsidevalue.has_value() && peinsidevalue.type() == typeid(bool)) {
                m_options_info.peinside_info.bpebasicfeature = std::any_cast<bool>(peinsidevalue);
            }
            if (auto peinsidevalue(document.GetMember(std::vector<std::string>{OP_PEINSIDEINFO, PI_BSTATFEATURE}));
                peinsidevalue.has_value() && peinsidevalue.type() == typeid(bool)) {
                m_options_info.peinside_info.bstatfeature = std::any_cast<bool>(peinsidevalue);
            }
            if (auto peinsidevalue(document.GetMember(std::vector<std::string>{OP_PEINSIDEINFO, PI_BCROSSREFFEATURE}));
                peinsidevalue.has_value() && peinsidevalue.type() == typeid(bool)) {
                m_options_info.peinside_info.bcrossreffeature = std::any_cast<bool>(peinsidevalue);
            }
            if (auto peinsidevalue(document.GetMember(std::vector<std::string>{OP_PEINSIDEINFO, PI_BPACKERFEATURE}));
                peinsidevalue.has_value() && peinsidevalue.type() == typeid(bool)) {
                m_options_info.peinside_info.bpackerfeature = std::any_cast<bool>(peinsidevalue);
            }
            if (auto peinsidevalue(document.GetMember(std::vector<std::string>{OP_PEINSIDEINFO, PI_BDATADIRECTORYFEATURE}));
                peinsidevalue.has_value() && peinsidevalue.type() == typeid(bool)) {
                m_options_info.peinside_info.bdatadirectoryfeature = std::any_cast<bool>(peinsidevalue);
            }
            if (auto peinsidevalue(document.GetMember(std::vector<std::string>{OP_PEINSIDEINFO, PI_BDEBUGDATADIRECTORYFEATURE}));
                peinsidevalue.has_value() && peinsidevalue.type() == typeid(bool)) {
                m_options_info.peinside_info.bdebugdatadirectoryfeature = std::any_cast<bool>(peinsidevalue);
            }
            if (auto peinsidevalue(document.GetMember(std::vector<std::string>{OP_PEINSIDEINFO, PI_BEMBEDDEDFEATURE}));
                peinsidevalue.has_value() && peinsidevalue.type() == typeid(bool)) {
                m_options_info.peinside_info.bembeddedfeature = std::any_cast<bool>(peinsidevalue);
            }
            if (auto peinsidevalue(document.GetMember(std::vector<std::string>{OP_PEINSIDEINFO, PI_BIMPORTTABLEFEATURE}));
                peinsidevalue.has_value() && peinsidevalue.type() == typeid(bool)) {
                m_options_info.peinside_info.bimporttablefeature = std::any_cast<bool>(peinsidevalue);
            }
            if (auto peinsidevalue(document.GetMember(std::vector<std::string>{OP_PEINSIDEINFO, PI_BEXPORTTABLEFEATURE}));
                peinsidevalue.has_value() && peinsidevalue.type() == typeid(bool)) {
                m_options_info.peinside_info.bexporttablefeature = std::any_cast<bool>(peinsidevalue);
            }
            if (auto peinsidevalue(document.GetMember(std::vector<std::string>{OP_PEINSIDEINFO, PI_BICONFEATURE}));
                peinsidevalue.has_value() && peinsidevalue.type() == typeid(bool)) {
                m_options_info.peinside_info.biconfeature = std::any_cast<bool>(peinsidevalue);
            }
            if (auto peinsidevalue(document.GetMember(std::vector<std::string>{OP_PEINSIDEINFO, PI_BMANIFEASTFEATURE}));
                peinsidevalue.has_value() && peinsidevalue.type() == typeid(bool)) {
                m_options_info.peinside_info.bmanifestfeature = std::any_cast<bool>(peinsidevalue);
            }
            if (auto peinsidevalue(document.GetMember(std::vector<std::string>{OP_PEINSIDEINFO, PI_BRESOURCEFEATURE}));
                peinsidevalue.has_value() && peinsidevalue.type() == typeid(bool)) {
                m_options_info.peinside_info.bresourcefeature = std::any_cast<bool>(peinsidevalue);
            }
            if (auto peinsidevalue(document.GetMember(std::vector<std::string>{OP_PEINSIDEINFO, PI_BRICHHEADERFEATURE}));
                peinsidevalue.has_value() && peinsidevalue.type() == typeid(bool)) {
                m_options_info.peinside_info.brichheaderfeature = std::any_cast<bool>(peinsidevalue);
            }
            if (auto peinsidevalue(document.GetMember(std::vector<std::string>{OP_PEINSIDEINFO, PI_BSECTIONFEATURE}));
                peinsidevalue.has_value() && peinsidevalue.type() == typeid(bool)) {
                m_options_info.peinside_info.bsectionfeature = std::any_cast<bool>(peinsidevalue);
            }
            if (auto peinsidevalue(document.GetMember(std::vector<std::string>{OP_PEINSIDEINFO, PI_BSTRINGSFEATURE}));
                peinsidevalue.has_value() && peinsidevalue.type() == typeid(bool)) {
                m_options_info.peinside_info.bstringsfeature = std::any_cast<bool>(peinsidevalue);
            }
            if (auto peinsidevalue(document.GetMember(std::vector<std::string>{OP_PEINSIDEINFO, PI_BUNPACKERFEATURE}));
                peinsidevalue.has_value() && peinsidevalue.type() == typeid(bool)) {
                m_options_info.peinside_info.bunpackerfeature = std::any_cast<bool>(peinsidevalue);
            }
            if (auto peinsidevalue(document.GetMember(std::vector<std::string>{OP_PEINSIDEINFO, PI_BDOTNETFEATURE}));
                peinsidevalue.has_value() && peinsidevalue.type() == typeid(bool)) {
                m_options_info.peinside_info.bdotnetfeature = std::any_cast<bool>(peinsidevalue);
            }
        }

        if (auto anyvalue(document.GetMember(std::vector<std::string>{OP_PEINSNFLOWINFO}));
            anyvalue.has_value() && anyvalue.type() == typeid(ConstRapidObject)) {
            if (auto peinsideflowvalue(document.GetMember(std::vector<std::string>{OP_PEINSNFLOWINFO, FI_APIFLOWFEATURE}));
                peinsideflowvalue.has_value() && peinsideflowvalue.type() == typeid(bool)) {
                m_options_info.peinsnflow_info.apiflowfeature = std::any_cast<bool>(peinsideflowvalue);
            }
            if (auto peinsideflowvalue(document.GetMember(std::vector<std::string>{OP_PEINSNFLOWINFO, FI_BLOCKFLOWFEATURE}));
                peinsideflowvalue.has_value() && peinsideflowvalue.type() == typeid(bool)) {
                m_options_info.peinsnflow_info.blockflowfeature = std::any_cast<bool>(peinsideflowvalue);
            }
            if (auto peinsideflowvalue(document.GetMember(std::vector<std::string>{OP_PEINSNFLOWINFO, FI_INSNFLOWFEATURE}));
                peinsideflowvalue.has_value() && peinsideflowvalue.type() == typeid(bool)) {
                m_options_info.peinsnflow_info.insnflowfeature = std::any_cast<bool>(peinsideflowvalue);
            }
        }
        return true;
    }

    bool CJsonOptions::GetOptionsInfo(_info_type type, std::pair<unsigned char*, size_t> pinfo)
    {
        bool bret = false;
        if (pinfo.first == nullptr || pinfo.second == 0)
            return bret;
        switch (type) {
        case _info_type::info_type_base:
        {
            if (pinfo.second != sizeof(_base_info))
                break;
            memcpy(pinfo.first, &m_options_info.base_info, sizeof(_base_info));
            bret = true;
        }
        break;
        case _info_type::info_type_install:
        {
            if (pinfo.second != sizeof(_install_info))
                break;
            memcpy(pinfo.first, &m_options_info.install_info, sizeof(_install_info));
            bret = true;
        }
        break;
        case _info_type::info_type_peinside:
        {
            if (pinfo.second != sizeof(_peinside_info))
                break;
            memcpy(pinfo.first, &m_options_info.peinside_info, sizeof(_peinside_info));
            bret = true;
        }
        break;
        case _info_type::info_type_peinsnflow:
        {
            if (pinfo.second != sizeof(_peinsnflow_info))
                break;
            memcpy(pinfo.first, &m_options_info.peinsnflow_info, sizeof(_peinsnflow_info));
            bret = true;
        }
        break;
        case _info_type::info_type_signature:
        {
            if (pinfo.second != sizeof(_signature_info))
                break;
            memcpy(pinfo.first, &m_options_info.signature_info, sizeof(_signature_info));
            bret = true;
        }
        break;
        case _info_type::info_type_version:
        {
            if (pinfo.second != sizeof(_version_info))
                break;
            memcpy(pinfo.first, &m_options_info.version_info, sizeof(_version_info));
            bret = true;
        }
        break;
        case _info_type::info_type_unknown:
        default:
            break;
        }
        return bret;
    }
} // namespace cchips
