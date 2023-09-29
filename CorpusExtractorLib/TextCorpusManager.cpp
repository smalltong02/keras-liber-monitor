#include <bson.h>
#include "TextCorpusManager.h"
#include "PeJsonDefine.h"
#include "utils.h"

namespace cchips {
    const mongocxx::instance CTextCorpusManager::_mongodb_instance{};
    const std::uint32_t CTextCorpusManager::_min_corpus_lenth = 3;
    const std::uint32_t CTextCorpusManager::_max_corpus_lenth = 100;

    const std::vector <std::string> CTextCorpusManager::_special_vocab = {
        {SPECIAL_CORPUS_PAD},  {SPECIAL_CORPUS_UNK}, {SPECIAL_CORPUS_SEP}, {SPECIAL_CORPUS_CLS}, {SPECIAL_CORPUS_MSK}, {SPECIAL_CORPUS_EQU},
        {SPECIAL_CORPUS_NUM},  {SPECIAL_CORPUS_ZER}, {SPECIAL_CORPUS_LNK}, {SPECIAL_CORPUS_APL}, {SPECIAL_CORPUS_COMMA}, {SPECIAL_CORPUS_RES1},
        {SPECIAL_CORPUS_RES2}, {SPECIAL_CORPUS_RES3}, {SPECIAL_CORPUS_RES4},{SPECIAL_CORPUS_RES5}, {SPECIAL_CORPUS_RES6},{SPECIAL_CORPUS_RES7},
        {SPECIAL_CORPUS_RES8}, {SPECIAL_CORPUS_RES9}, {SPECIAL_CORPUS_RES10},{SPECIAL_CORPUS_RES11},{SPECIAL_CORPUS_RES12},{SPECIAL_CORPUS_RES13},
        {SPECIAL_CORPUS_RES14},{SPECIAL_CORPUS_RES15},{SPECIAL_CORPUS_RES16},{SPECIAL_CORPUS_RES17},{SPECIAL_CORPUS_RES18},{SPECIAL_CORPUS_RES19},
        {SPECIAL_CORPUS_RES20},{SPECIAL_CORPUS_NUM1}, {SPECIAL_CORPUS_NUM2}, {SPECIAL_CORPUS_NUM3}, {SPECIAL_CORPUS_NUM4}, {SPECIAL_CORPUS_NUM5},
        {SPECIAL_CORPUS_NUM6}, {SPECIAL_CORPUS_NUM7}, {SPECIAL_CORPUS_NUM8}, {SPECIAL_CORPUS_NUM9}, {SPECIAL_CORPUS_NUM0},
    };

    std::string DocviewToJson(const bsoncxx::v_noabi::document::view& view)
    {
        if (!view.length())
            return {};
        bson_t bson;
        bson_init_static(&bson, view.data(), view.length());
        size_t size;
        auto result = bson_as_json(&bson, &size);
        if (!result)
            return {};
        std::string json_str = std::string(result);
        bson_free(result);
        return std::move(json_str);
    }

    bool CTextCorpusManager::CorpusExtract() const {
        if (m_type == input_mongodbformat) {
            try {
                std::cout << "CorpusExtract start... " << std::endl;
                mongocxx::client client{mongocxx::uri{m_input_path.c_str()}};
                if (!client) return false;
                mongocxx::database db = client[_mongodb_database_name];
                if (!db) return false;
                mongocxx::collection coll_format = db[_mongodb_collector_format_info];
                if (!coll_format) return false;
                mongocxx::collection coll_inside = db[_mongodb_collector_inside_info];
                if (!coll_inside) return false;
                mongocxx::collection coll_insnflow = db[_mongodb_collector_insnflow_info];
                if (!coll_insnflow) return false;
                mongocxx::collection ext_format = db[_mongodb_corpusext_format];
                if (!ext_format) return false;
                mongocxx::collection ext_inside = db[_mongodb_corpusext_inside];
                if (!ext_inside) return false;
                mongocxx::collection ext_insnflow = db[_mongodb_corpusext_insnflow];
                if (!ext_insnflow) return false;
                mongocxx::collection textcorpus_dict = db[_mongodb_textcorpus_dict];
                if (!ext_insnflow) return false;

                std::shared_ptr<CSemanticGeneration> sgptr = std::make_shared<CCorpusExtractSG>();
                if (!sgptr) {
                    return false;
                }
                std::dynamic_pointer_cast<CCorpusExtractSG>(sgptr)->initDict(db);

                CorpusExtractPeFormat(coll_format, sgptr);
                CorpusExtractPeInside(coll_inside, sgptr);
                CorpusExtractPeinsnflow(coll_insnflow, sgptr);
                std::cout << "CorpusExtract end... " << std::endl;
            }
            catch (const std::exception& e) {
                std::cout << "Exception occurred during corpus extract: " << e.what() << std::endl;
                return false;
            }
        }

        return true;
    }

    bool CTextCorpusManager::CorpusExtractPeFormat(mongocxx::collection& source, std::shared_ptr<CSemanticGeneration> sgobject) const {
        std::cout << "CorpusExtractPeFormat start..." << std::endl;
        if (!sgobject) return false;
        mongocxx::cursor cursor = source.find({});
        for (const auto& doc : cursor) {
            std::string json_str = DocviewToJson(doc);
            std::unique_ptr<CRapidJsonWrapper> wrapper = std::make_unique<CRapidJsonWrapper>(json_str);
            if (!wrapper || !wrapper->IsValid())
                continue;
            if (auto anyvalue(wrapper->GetMember(std::vector<std::string>{PJ_SHA256}));
                !anyvalue.has_value() || anyvalue.type() != typeid(std::string_view)) {
                continue;
            }
            else {
                std::cout << "process " << std::any_cast<std::string_view>(anyvalue) << "..." << std::endl;
            }

            if (auto anyvalue(wrapper->GetMember(BI_BASEINFO));
                anyvalue.has_value() && anyvalue.type() == typeid(ConstRapidObject))
            {
                auto baseinfo = std::any_cast<ConstRapidObject>(anyvalue);
                CommonParseBaseInfo(baseinfo, sgobject);
            }
            if (auto anyvalue(wrapper->GetMember(VI_VERINFO));
                anyvalue.has_value() && anyvalue.type() == typeid(ConstRapidObject))
            {
                auto verinfo = std::any_cast<ConstRapidObject>(anyvalue);
                CommonParseVersionInfo(verinfo, sgobject);
            }
            if (auto anyvalue(wrapper->GetMember(II_INSTALLINFO));
                anyvalue.has_value() && anyvalue.type() == typeid(ConstRapidObject))
            {
                auto installinfo = std::any_cast<ConstRapidObject>(anyvalue);
                CommonParseInstallInfo(installinfo, sgobject);
            }
            if (auto anyvalue(wrapper->GetMember(SI_SIGNINFO));
                anyvalue.has_value() && anyvalue.type() == typeid(ConstRapidObject))
            {
                auto signinfo = std::any_cast<ConstRapidObject>(anyvalue);
                CommonParseSigntureInfo(signinfo, sgobject);
            }
        }
        return true;
    }

    bool CTextCorpusManager::CorpusExtractPeInside(mongocxx::collection& source, std::shared_ptr<CSemanticGeneration> sgobject) const {
        std::cout << "CorpusExtractPeInside start... " << std::endl;
        if (!sgobject) return false;
        mongocxx::cursor cursor = source.find({});
        for (const auto& doc : cursor) {
            std::string json_str = DocviewToJson(doc);
            std::unique_ptr<CRapidJsonWrapper> wrapper = std::make_unique<CRapidJsonWrapper>(json_str);
            if (!wrapper || !wrapper->IsValid())
                continue;
            if (auto anyvalue(wrapper->GetMember(std::vector<std::string>{PJ_SHA256}));
                !anyvalue.has_value() || anyvalue.type() != typeid(std::string_view)) {
                continue;
            }
            else {
                std::cout << "process " << std::any_cast<std::string_view>(anyvalue) << "..." << std::endl;
            }

            if (auto anyvalue(wrapper->GetMember(NI_INSIDEINFO));
                anyvalue.has_value() && anyvalue.type() == typeid(ConstRapidObject))
            {
                auto insideinfo = std::any_cast<ConstRapidObject>(anyvalue);
                const auto peinfo = insideinfo.FindMember(PI_PEINFO);
                if (peinfo != insideinfo.MemberEnd()) {
                    if (!peinfo->value.IsNull() && peinfo->value.IsObject()) {
                        auto peinfoobj = peinfo->value.GetObjectA();
                        CommonParsePeInfo(peinfoobj, sgobject);
                    }
                }
                const auto toolinfo = insideinfo.FindMember(TI_TOOLINFO);
                if (toolinfo != insideinfo.MemberEnd()) {
                    if (!toolinfo->value.IsNull() && toolinfo->value.IsObject()) {
                        auto toolinfoobj = toolinfo->value.GetObjectA();
                        CommonParseToolInfo(toolinfoobj, sgobject);
                    }
                }
                const auto datadir = insideinfo.FindMember(DD_DATADIRECTORY);
                if (datadir != insideinfo.MemberEnd()) {
                    if (!datadir->value.IsNull() && datadir->value.IsObject()) {
                        auto datadirobj = datadir->value.GetObjectA();
                        CommonParseDataDirectoryInfo(datadirobj, sgobject);
                    }
                }
                const auto pdbinfos = insideinfo.FindMember(PS_PDBINFOS);
                if (pdbinfos != insideinfo.MemberEnd()) {
                    if (!pdbinfos->value.IsNull() && pdbinfos->value.IsObject()) {
                        auto pdbinfosobj = pdbinfos->value.GetObjectA();
                        CommonParsePdbInfosInfo(pdbinfosobj, sgobject);
                    }
                }
                const auto embeddedpe = insideinfo.FindMember(ED_EMBEDDEDPE);
                if (embeddedpe != insideinfo.MemberEnd()) {
                    if (!embeddedpe->value.IsNull() && embeddedpe->value.IsObject()) {
                        auto embeddedpeobj = embeddedpe->value.GetObjectA();
                        CommonParseEmbeddedpeInfo(embeddedpeobj, sgobject);
                    }
                }
                const auto imports = insideinfo.FindMember(IT_IMPORTS);
                if (imports != insideinfo.MemberEnd()) {
                    if (!imports->value.IsNull() && imports->value.IsObject()) {
                        auto importsobj = imports->value.GetObjectA();
                        CommonParseImportsInfo(importsobj, sgobject);
                    }
                }
                const auto exports = insideinfo.FindMember(ET_EXPORTS);
                if (exports != insideinfo.MemberEnd()) {
                    if (!exports->value.IsNull() && exports->value.IsObject()) {
                        auto exportsobj = exports->value.GetObjectA();
                        CommonParseExportsInfo(exportsobj, sgobject);
                    }
                }
                const auto manifest = insideinfo.FindMember(MF_MANIFEST);
                if (manifest != insideinfo.MemberEnd()) {
                    if (!manifest->value.IsNull() && manifest->value.IsObject()) {
                        auto manifestobj = manifest->value.GetObjectA();
                        CommonParseManifestInfo(manifestobj, sgobject);
                    }
                }
                const auto resourceinfo = insideinfo.FindMember(RI_RESOURCEINFO);
                if (resourceinfo != insideinfo.MemberEnd()) {
                    if (!resourceinfo->value.IsNull() && resourceinfo->value.IsObject()) {
                        auto resourceinfoobj = resourceinfo->value.GetObjectA();
                        CommonParseResourceInfo(resourceinfoobj, sgobject);
                    }
                }
                const auto richheader = insideinfo.FindMember(RH_RICHHEADER);
                if (richheader != insideinfo.MemberEnd()) {
                    if (!richheader->value.IsNull() && richheader->value.IsObject()) {
                        auto richheaderobj = richheader->value.GetObjectA();
                        CommonParseRichheaderInfo(richheaderobj, sgobject);
                    }
                }
                const auto sections = insideinfo.FindMember(SS_SECTIONS);
                if (sections != insideinfo.MemberEnd()) {
                    if (!sections->value.IsNull() && sections->value.IsObject()) {
                        auto sectionsobj = sections->value.GetObjectA();
                        CommonParseSectionsInfo(sectionsobj, sgobject);
                    }
                }
                const auto strfeature = insideinfo.FindMember(SF_STRINGS_FEATURE);
                if (strfeature != insideinfo.MemberEnd()) {
                    if (!strfeature->value.IsNull() && strfeature->value.IsObject()) {
                        auto strfeatureobj = strfeature->value.GetObjectA();
                        CommonParseStrfeatureInfo(strfeatureobj, sgobject);
                    }
                }
            }
        }
        return true;
    }

    bool CTextCorpusManager::CorpusExtractPeinsnflow(mongocxx::collection& source, std::shared_ptr<CSemanticGeneration> sgobject) const {
        std::cout << "CorpusExtractPeinsnflow start... " << std::endl;
        if (!sgobject) return false;
        return true;
    }

    bool CTextCorpusManager::AppendLinetoFile(const std::string& outputfile, const std::stringstream& datasets) const {
        std::ofstream outfile(outputfile, std::ios::app);
        if (!outfile.is_open())
            return false;
        outfile << datasets.str() << std::endl;
        return true;
    }

    bool CTextCorpusManager::SaveStatisticianData(const fs::path& outputfile) const {
        if (outputfile.empty()) {
            return false;
        }
        fs::path statfile = outputfile.parent_path();
        statfile = statfile.append("statisticianinfo.txt");
        std::ofstream outfile(statfile, std::ios::out);
        if (!outfile.is_open())
            return false;
        outfile << "skip counts: " << m_skip_lists.size() << std::endl;
        outfile << "no enough feature counts: " << m_notenough_lists.size() << std::endl;
        outfile << "duplicate iden counts: " << m_dup_idencounts << std::endl;
        outfile << "success counts: " << m_scan_lists.size() << std::endl << std::endl;

        outfile << "    skip lists: " << std::endl;
        for (auto& item : m_skip_lists) {
            outfile << "        " << item << std::endl;
        }
        outfile << std::endl;
        outfile << "    noenough lists: " << std::endl;
        for (auto& item : m_notenough_lists) {
            outfile << "        " << item << std::endl;
        }
        outfile << std::endl;
        outfile << "    success lists: " << std::endl;
        for (auto& item : m_scan_lists) {
            outfile << "        " << item << std::endl;
        }
        outfile << std::endl;
        return true;
    }

    bool CTextCorpusManager::GeneratingFastTextDatasets(std::unique_ptr<CRapidJsonWrapper> wrapper, std::stringstream& datasets, const std::string& label) {
        if (!wrapper) return false;
        if (auto anyvalue(wrapper->GetMember(std::vector<std::string>{PJ_SHA256}));
            !anyvalue.has_value() || anyvalue.type() != typeid(std::string_view)) {
            return false;
        }
        else {
            //std::cout << "process " << std::any_cast<std::string_view>(anyvalue) << "..." << std::endl;
        }
        std::shared_ptr<CSemanticGeneration> sgptr = std::make_shared<CFastTextSG>();
        if (!sgptr) {
            return false;
        }
        if (auto anyvalue(wrapper->GetMember(BI_BASEINFO));
            anyvalue.has_value() && anyvalue.type() == typeid(ConstRapidObject))
        {
            auto baseinfo = std::any_cast<ConstRapidObject>(anyvalue);
            CommonParseBaseInfo(baseinfo, sgptr);
        }
        if (auto anyvalue(wrapper->GetMember(VI_VERINFO));
            anyvalue.has_value() && anyvalue.type() == typeid(ConstRapidObject))
        {
            auto verinfo = std::any_cast<ConstRapidObject>(anyvalue);
            CommonParseVersionInfo(verinfo, sgptr);
        }
        if (auto anyvalue(wrapper->GetMember(II_INSTALLINFO));
            anyvalue.has_value() && anyvalue.type() == typeid(ConstRapidObject))
        {
            auto installinfo = std::any_cast<ConstRapidObject>(anyvalue);
            CommonParseInstallInfo(installinfo, sgptr);
        }
        if (auto anyvalue(wrapper->GetMember(SI_SIGNINFO));
            anyvalue.has_value() && anyvalue.type() == typeid(ConstRapidObject))
        {
            auto signinfo = std::any_cast<ConstRapidObject>(anyvalue);
            CommonParseSigntureInfo(signinfo, sgptr);
        }
        if (auto anyvalue(wrapper->GetMember(NI_INSIDEINFO));
            anyvalue.has_value() && anyvalue.type() == typeid(ConstRapidObject))
        {
            auto insideinfo = std::any_cast<ConstRapidObject>(anyvalue);
            const auto peinfo = insideinfo.FindMember(PI_PEINFO);
            if (peinfo != insideinfo.MemberEnd()) {
                if (!peinfo->value.IsNull() && peinfo->value.IsObject()) {
                    auto peinfoobj = peinfo->value.GetObjectA();
                    CommonParsePeInfo(peinfoobj, sgptr);
                }
            }
            const auto toolinfo = insideinfo.FindMember(TI_TOOLINFO);
            if (toolinfo != insideinfo.MemberEnd()) {
                if (!toolinfo->value.IsNull() && toolinfo->value.IsObject()) {
                    auto toolinfoobj = toolinfo->value.GetObjectA();
                    CommonParseToolInfo(toolinfoobj, sgptr);
                }
            }
            const auto datadir = insideinfo.FindMember(DD_DATADIRECTORY);
            if (datadir != insideinfo.MemberEnd()) {
                if (!datadir->value.IsNull() && datadir->value.IsObject()) {
                    auto datadirobj = datadir->value.GetObjectA();
                    CommonParseDataDirectoryInfo(datadirobj, sgptr);
                }
            }
            const auto pdbinfos = insideinfo.FindMember(PS_PDBINFOS);
            if (pdbinfos != insideinfo.MemberEnd()) {
                if (!pdbinfos->value.IsNull() && pdbinfos->value.IsObject()) {
                    auto pdbinfosobj = pdbinfos->value.GetObjectA();
                    CommonParsePdbInfosInfo(pdbinfosobj, sgptr);
                }
            }
            const auto embeddedpe = insideinfo.FindMember(ED_EMBEDDEDPE);
            if (embeddedpe != insideinfo.MemberEnd()) {
                if (!embeddedpe->value.IsNull() && embeddedpe->value.IsObject()) {
                    auto embeddedpeobj = embeddedpe->value.GetObjectA();
                    CommonParseEmbeddedpeInfo(embeddedpeobj, sgptr);
                }
            }
            const auto imports = insideinfo.FindMember(IT_IMPORTS);
            if (imports != insideinfo.MemberEnd()) {
                if (!imports->value.IsNull() && imports->value.IsObject()) {
                    auto importsobj = imports->value.GetObjectA();
                    CommonParseImportsInfo(importsobj, sgptr);
                }
            }
            const auto exports = insideinfo.FindMember(ET_EXPORTS);
            if (exports != insideinfo.MemberEnd()) {
                if (!exports->value.IsNull() && exports->value.IsObject()) {
                    auto exportsobj = exports->value.GetObjectA();
                    CommonParseExportsInfo(exportsobj, sgptr);
                }
            }
            const auto manifest = insideinfo.FindMember(MF_MANIFEST);
            if (manifest != insideinfo.MemberEnd()) {
                if (!manifest->value.IsNull() && manifest->value.IsObject()) {
                    auto manifestobj = manifest->value.GetObjectA();
                    CommonParseManifestInfo(manifestobj, sgptr);
                }
            }
            const auto resourceinfo = insideinfo.FindMember(RI_RESOURCEINFO);
            if (resourceinfo != insideinfo.MemberEnd()) {
                if (!resourceinfo->value.IsNull() && resourceinfo->value.IsObject()) {
                    auto resourceinfoobj = resourceinfo->value.GetObjectA();
                    CommonParseResourceInfo(resourceinfoobj, sgptr);
                }
            }
            const auto richheader = insideinfo.FindMember(RH_RICHHEADER);
            if (richheader != insideinfo.MemberEnd()) {
                if (!richheader->value.IsNull() && richheader->value.IsObject()) {
                    auto richheaderobj = richheader->value.GetObjectA();
                    CommonParseRichheaderInfo(richheaderobj, sgptr);
                }
            }
            const auto sections = insideinfo.FindMember(SS_SECTIONS);
            if (sections != insideinfo.MemberEnd()) {
                if (!sections->value.IsNull() && sections->value.IsObject()) {
                    auto sectionsobj = sections->value.GetObjectA();
                    CommonParseSectionsInfo(sectionsobj, sgptr);
                }
            }
            const auto strfeature = insideinfo.FindMember(SF_STRINGS_FEATURE);
            if (strfeature != insideinfo.MemberEnd()) {
                if (!strfeature->value.IsNull() && strfeature->value.IsObject()) {
                    auto strfeatureobj = strfeature->value.GetObjectA();
                    CommonParseStrfeatureInfo(strfeatureobj, sgptr);
                }
            }
        }
        if (label.length()) {
            datasets << label << " " << sgptr->fulldump();
        }
        else {
            datasets << sgptr->fulldump();
        }
        return true;
    }

    bool CTextCorpusManager::GeneratingGruDatasets(std::unique_ptr<CRapidJsonWrapper> wrapper, std::stringstream& datasets, const std::string& label) {
        return true;
    }

    bool CTextCorpusManager::GeneratingTransformersDatasets(std::unique_ptr<CRapidJsonWrapper> wrapper, std::stringstream& datasets, const std::string& label) {
        if (!wrapper) return false;
        std::string identifier;
        if (auto anyvalue(wrapper->GetMember(std::vector<std::string>{PJ_SHA256}));
            !anyvalue.has_value() || anyvalue.type() != typeid(std::string_view)) {
            return false;
        }
        else {
            identifier = std::any_cast<std::string_view>(anyvalue);
            std::cout << "process " << identifier << "..." << std::endl;
            auto it = m_scaned_idenlist.find(identifier);
            if (it != m_scaned_idenlist.end()) {
                m_dup_idencounts++;
                std::cout << "It's already been processed. Exit." << std::endl;
                return false;
            }
            m_scaned_idenlist.insert(identifier);
        }
        std::shared_ptr<CSemanticGeneration> tfptr = std::make_shared<CTransformersSG>();
        if (!tfptr) {
            return false;
        }
        if (auto anyvalue(wrapper->GetMember(BI_BASEINFO));
            anyvalue.has_value() && anyvalue.type() == typeid(ConstRapidObject))
        {
            std::string filename_str;
            auto baseinfo = std::any_cast<ConstRapidObject>(anyvalue);
            const auto bi_filename = baseinfo.FindMember(BI_FILENAME);
            if (bi_filename != baseinfo.MemberEnd()) {
                if (!bi_filename->value.IsNull() && bi_filename->value.IsString()) {
                    filename_str = bi_filename->value.GetString();
                    if (filename_str.length()) {
                        fs::path filename(filename_str);
                        filename_str = StringToLower(filename.u8string());
                        if (InFilterList(filename_str)) {
                            m_skip_lists.push_back(identifier);
                            std::cout << "the filename " << filename_str << " needs to be skipped." << std::endl;
                            return false;
                        }
                    }
                }
            }
        }
        if (auto anyvalue(wrapper->GetMember(SI_SIGNINFO));
            anyvalue.has_value() && anyvalue.type() == typeid(ConstRapidObject))
        {
            auto signinfo = std::any_cast<ConstRapidObject>(anyvalue);
            CommonParseSigntureInfo(signinfo, tfptr);
        }
        if (auto anyvalue(wrapper->GetMember(NI_INSIDEINFO));
            anyvalue.has_value() && anyvalue.type() == typeid(ConstRapidObject))
        {
            auto insideinfo = std::any_cast<ConstRapidObject>(anyvalue);
            const auto peinfo = insideinfo.FindMember(PI_PEINFO);
            if (peinfo != insideinfo.MemberEnd()) {
                if (!peinfo->value.IsNull() && peinfo->value.IsObject()) {
                    auto peinfoobj = peinfo->value.GetObjectA();
                    CommonParsePeInfo(peinfoobj, tfptr);
                }
            }
            const auto toolinfo = insideinfo.FindMember(TI_TOOLINFO);
            if (toolinfo != insideinfo.MemberEnd()) {
                if (!toolinfo->value.IsNull() && toolinfo->value.IsObject()) {
                    auto toolinfoobj = toolinfo->value.GetObjectA();
                    CommonParseToolInfo(toolinfoobj, tfptr);
                }
            }
            const auto datadir = insideinfo.FindMember(DD_DATADIRECTORY);
            if (datadir != insideinfo.MemberEnd()) {
                if (!datadir->value.IsNull() && datadir->value.IsObject()) {
                    auto datadirobj = datadir->value.GetObjectA();
                    CommonParseDataDirectoryInfo(datadirobj, tfptr);
                }
            }
            const auto pdbinfos = insideinfo.FindMember(PS_PDBINFOS);
            if (pdbinfos != insideinfo.MemberEnd()) {
                if (!pdbinfos->value.IsNull() && pdbinfos->value.IsObject()) {
                    auto pdbinfosobj = pdbinfos->value.GetObjectA();
                    CommonParsePdbInfosInfo(pdbinfosobj, tfptr);
                }
            }
            const auto embeddedpe = insideinfo.FindMember(ED_EMBEDDEDPE);
            if (embeddedpe != insideinfo.MemberEnd()) {
                if (!embeddedpe->value.IsNull() && embeddedpe->value.IsObject()) {
                    auto embeddedpeobj = embeddedpe->value.GetObjectA();
                    CommonParseEmbeddedpeInfo(embeddedpeobj, tfptr);
                }
            }
            const auto imports = insideinfo.FindMember(IT_IMPORTS);
            if (imports != insideinfo.MemberEnd()) {
                if (!imports->value.IsNull() && imports->value.IsObject()) {
                    auto importsobj = imports->value.GetObjectA();
                    CommonParseImportsInfo(importsobj, tfptr);
                }
            }
            const auto exports = insideinfo.FindMember(ET_EXPORTS);
            if (exports != insideinfo.MemberEnd()) {
                if (!exports->value.IsNull() && exports->value.IsObject()) {
                    auto exportsobj = exports->value.GetObjectA();
                    CommonParseExportsInfo(exportsobj, tfptr);
                }
            }
            const auto manifest = insideinfo.FindMember(MF_MANIFEST);
            if (manifest != insideinfo.MemberEnd()) {
                if (!manifest->value.IsNull() && manifest->value.IsObject()) {
                    auto manifestobj = manifest->value.GetObjectA();
                    CommonParseManifestInfo(manifestobj, tfptr);
                }
            }
            const auto resourceinfo = insideinfo.FindMember(RI_RESOURCEINFO);
            if (resourceinfo != insideinfo.MemberEnd()) {
                if (!resourceinfo->value.IsNull() && resourceinfo->value.IsObject()) {
                    auto resourceinfoobj = resourceinfo->value.GetObjectA();
                    CommonParseResourceInfo(resourceinfoobj, tfptr);
                }
            }
            const auto richheader = insideinfo.FindMember(RH_RICHHEADER);
            if (richheader != insideinfo.MemberEnd()) {
                if (!richheader->value.IsNull() && richheader->value.IsObject()) {
                    auto richheaderobj = richheader->value.GetObjectA();
                    CommonParseRichheaderInfo(richheaderobj, tfptr);
                }
            }
            const auto sections = insideinfo.FindMember(SS_SECTIONS);
            if (sections != insideinfo.MemberEnd()) {
                if (!sections->value.IsNull() && sections->value.IsObject()) {
                    auto sectionsobj = sections->value.GetObjectA();
                    CommonParseSectionsInfo(sectionsobj, tfptr);
                }
            }
            const auto strfeature = insideinfo.FindMember(SF_STRINGS_FEATURE);
            if (strfeature != insideinfo.MemberEnd()) {
                if (!strfeature->value.IsNull() && strfeature->value.IsObject()) {
                    auto strfeatureobj = strfeature->value.GetObjectA();
                    CommonParseStrfeatureInfo(strfeatureobj, tfptr, (_maximum_feature_counts - tfptr->getelemcount()));
                }
            }
        }

        if (tfptr->getelemcount() <= _minimum_feature_counts) {
            if (auto anyvalue(wrapper->GetMember(FI_INSNINFO));
                anyvalue.has_value() && anyvalue.type() == typeid(ConstRapidObject))
            {
                auto insninfo = std::any_cast<ConstRapidObject>(anyvalue);
                const auto cfgflowgraph = insninfo.FindMember(FI_CFGFLOWGRAPH);
                if (cfgflowgraph != insninfo.MemberEnd()) {
                    if (!cfgflowgraph->value.IsNull() && cfgflowgraph->value.IsObject()) {
                        auto graphobj = cfgflowgraph->value.GetObjectA();
                        CommonParseFlowGraph(graphobj, tfptr, (_maximum_feature_counts - tfptr->getelemcount()));
                    }
                }
                const auto ilcode = insninfo.FindMember(FI_ILCODE);
                if (ilcode != insninfo.MemberEnd()) {
                    if (!ilcode->value.IsNull() && ilcode->value.IsArray()) {
                        CommonParseIlcode(insninfo, tfptr, (_maximum_feature_counts - tfptr->getelemcount()));
                    }
                }
            }
        }
        std::uint32_t word_counts = tfptr->getelemcount();
        std::cout << "word counts: " << tfptr->getelemcount() << std::endl;
        if (word_counts <= _minimum_word_counts) {
            m_notenough_lists.push_back(identifier);
            std::cout << "not enough features, return false!" << std::endl;
            return false;
        }
        if (label.length()) {
            datasets << label << tfptr->fulldump();
        }
        else {
            datasets << tfptr->fulldump();
        }
        m_scan_lists.push_back(identifier);
        return true;
    }

    bool CTextCorpusManager::CommonParseBaseInfo(ConstRapidObject& baseinfo, std::shared_ptr<CSemanticGeneration> sgobject) const {
        if (!sgobject) return false;
        auto subsg = sgobject->makeSubSg();
        if (!subsg) return false;
        std::string filename_str;
        const auto bi_filename = baseinfo.FindMember(BI_FILENAME);
        if (bi_filename != baseinfo.MemberEnd()) {
            if (!bi_filename->value.IsNull() && bi_filename->value.IsString()) {
                filename_str = bi_filename->value.GetString();
                if (filename_str.length()) {
                    fs::path filename(filename_str);
                    filename_str = StringToLower(filename.stem().u8string());
                }
            }
        }
        subsg->addElement(BI_FILENAME, filename_str);
        std::string type_str;
        const auto bi_filetype = baseinfo.FindMember(BI_FILETYPE);
        if (bi_filetype != baseinfo.MemberEnd()) {
            if (!bi_filetype->value.IsNull() && bi_filetype->value.IsString()) {
                type_str = bi_filetype->value.GetString();
                if (type_str.length()) {
                    type_str = StringToLower(type_str);
                }
            }
        }
        subsg->addElement(BI_FILETYPE, type_str);
        std::uint32_t file_size = 0;
        const auto bi_filesize = baseinfo.FindMember(BI_FILESIZE);
        if (bi_filesize != baseinfo.MemberEnd()) {
            if (!bi_filesize->value.IsNull() && bi_filesize->value.IsInt()) {
                file_size = std::any_cast<std::int32_t>(bi_filesize->value.GetInt());
            }
        }
        subsg->addElement(BI_FILESIZE, file_size);
        std::string fileattr_str;
        const auto bi_fileattr = baseinfo.FindMember(BI_FILEATTR);
        if (bi_fileattr != baseinfo.MemberEnd()) {
            if (!bi_fileattr->value.IsNull() && bi_fileattr->value.IsString()) {
                fileattr_str = bi_fileattr->value.GetString();
                if (fileattr_str.length()) {
                    fileattr_str = StringToLower(fileattr_str);
                }
            }
        }
        subsg->addElement(BI_FILEATTR, fileattr_str);
        return sgobject->addElement(BI_BASEINFO, std::move(subsg));
    }

    bool CTextCorpusManager::CommonParseVersionInfo(ConstRapidObject& versioninfo, std::shared_ptr<CSemanticGeneration> sgobject) const {
        if (!sgobject) return false;
        auto subsg = sgobject->makeSubSg();
        if (!subsg) return false;
        std::vector<std::string> detected_l;
        std::string description_str;
        const auto bi_description = versioninfo.FindMember(VI_DESCRIPTION);
        if (bi_description != versioninfo.MemberEnd()) {
            if (!bi_description->value.IsNull() && bi_description->value.IsString()) {
                description_str = bi_description->value.GetString();
                if (description_str.length()) {
                    std::string des_str;
                    if (CStrExtractor::detectedUSCorpus(description_str, detected_l)) {
                        for (auto& detect : detected_l) {
                            if (!des_str.length())
                                des_str = detect;
                            else
                                des_str += " " + detect;
                        }
                    }
                    description_str = des_str;
                }
            }
        }
        subsg->addElement(VI_DESCRIPTION, description_str);
        std::string filetype_str;
        const auto bi_filetype = versioninfo.FindMember(VI_FILETYPE);
        if (bi_filetype != versioninfo.MemberEnd()) {
            if (!bi_filetype->value.IsNull() && bi_filetype->value.IsString()) {
                filetype_str = bi_filetype->value.GetString();
                if (filetype_str.length()) {
                    filetype_str = StringToLower(filetype_str);
                }
            }
        }
        subsg->addElement(VI_FILETYPE, filetype_str);
        std::uint32_t file_version = 0;
        const auto bi_filever = versioninfo.FindMember(VI_FILEVER);
        if (bi_filever != versioninfo.MemberEnd()) {
            if (!bi_filever->value.IsNull() && bi_filever->value.IsString()) {
                std::string filever_str = bi_filever->value.GetString();
                if (filever_str.length()) {
                    file_version = 5;
                }
            }
        }
        subsg->addElement(VI_FILEVER, file_version);
        std::string productname_str;
        const auto bi_productname = versioninfo.FindMember(VI_PRODUCTNAME);
        if (bi_productname != versioninfo.MemberEnd()) {
            if (!bi_productname->value.IsNull() && bi_productname->value.IsString()) {
                productname_str = bi_productname->value.GetString();
                if (productname_str.length()) {
                    std::string name_str;
                    if (CStrExtractor::detectedUSCorpus(productname_str, detected_l)) {
                        for (auto& detect : detected_l) {
                            if (!name_str.length())
                                name_str = detect;
                            else
                                name_str += " " + detect;
                        }
                    }
                    productname_str = name_str;
                }
            }
        }
        subsg->addElement(VI_PRODUCTNAME, productname_str);
        std::uint32_t product_version = 0;
        const auto bi_productver = versioninfo.FindMember(VI_PRODUCTVER);
        if (bi_productver != versioninfo.MemberEnd()) {
            if (!bi_productver->value.IsNull() && bi_productver->value.IsString()) {
                std::string productver_str = bi_productver->value.GetString();
                if (productver_str.length()) {
                    product_version = 5;
                }
            }
        }
        subsg->addElement(VI_PRODUCTVER, product_version);
        return sgobject->addElement(VI_VERINFO, std::move(subsg));
    }

    bool CTextCorpusManager::CommonParseInstallInfo(ConstRapidObject& installinfo, std::shared_ptr<CSemanticGeneration> sgobject) const {
        if (!sgobject) return false;
        return true;
    }

    bool CTextCorpusManager::CommonParseSigntureInfo(ConstRapidObject& signinfo, std::shared_ptr<CSemanticGeneration> sgobject) const {
        if (!sgobject) return false;
        std::vector <std::string> detected_l;
        auto subsg = sgobject->makeSubSg();
        if (!subsg) return false;
        bool bsigned = false;
        const auto bi_bsigned = signinfo.FindMember(SI_BSIGNED);
        if (bi_bsigned != signinfo.MemberEnd()) {
            if (!bi_bsigned->value.IsNull() && bi_bsigned->value.IsBool()) {
                bsigned = bi_bsigned->value.GetBool();
            }
        }
        subsg->addElement(SI_BSIGNED, bsigned);
        bool bverified = false;
        const auto bi_bverified = signinfo.FindMember(SI_BVERIFIED);
        if (bi_bverified != signinfo.MemberEnd()) {
            if (!bi_bverified->value.IsNull() && bi_bverified->value.IsBool()) {
                bverified = bi_bverified->value.GetBool();
            }
        }
        subsg->addElement(SI_BVERIFIED, bverified);
        bool bcountersign = false;
        const auto bi_bcountersign = signinfo.FindMember(SI_BCOUNTERSIGN);
        if (bi_bcountersign != signinfo.MemberEnd()) {
            if (!bi_bcountersign->value.IsNull() && bi_bcountersign->value.IsBool()) {
                bcountersign = bi_bcountersign->value.GetBool();
            }
        }
        subsg->addElement(SI_BCOUNTERSIGN, bcountersign);
        bool bmutilcerts = false;
        const auto bi_bmutilcerts = signinfo.FindMember(SI_BMUTILCERTS);
        if (bi_bmutilcerts != signinfo.MemberEnd()) {
            if (!bi_bmutilcerts->value.IsNull() && bi_bmutilcerts->value.IsBool()) {
                bmutilcerts = bi_bmutilcerts->value.GetBool();
            }
        }
        subsg->addElement(SI_BMUTILCERTS, bmutilcerts);

        const auto bi_primarycert = signinfo.FindMember(SI_PRIMARYCERT);
        if (bi_primarycert != signinfo.MemberEnd()) {
            auto primarysg = subsg->makeSubSg();
            if (primarysg) {
                if (!bi_primarycert->value.IsNull() && bi_primarycert->value.IsArray()) {
                    auto primarycert = bi_primarycert->value.GetArray();
                    for (auto& it : primarycert) {
                        if (!it.IsObject()) continue;
                        for (auto& member : it.GetObjectA()) {
                            if (stricmp(member.name.GetString(), SI_DIGEST) == 0) {
                                if (!member.value.IsString()) continue;
                                std::string digest = member.value.GetString();
                                primarysg->addElement(SI_DIGEST, digest);
                            }
                            if (stricmp(member.name.GetString(), SI_ISSUER) == 0) {
                                if (!member.value.IsString()) continue;
                                if (CStrExtractor::detectedUSCorpus(member.value.GetString(), detected_l)) {
                                    std::string issuer_str;
                                    for (auto& detect : detected_l) {
                                        if (!issuer_str.length())
                                            issuer_str = detect;
                                        else
                                            issuer_str += " " + detect;
                                    }
                                    primarysg->addElement(SI_ISSUER, issuer_str);
                                }
                            }
                        }
                    }
                }
                subsg->addElement(SI_PRIMARYCERT, std::move(primarysg));
            }
        }
        return sgobject->addElement(SI_SIGNINFO, std::move(subsg));
    }

    bool CTextCorpusManager::CommonParsePeInfo(ConstRapidObject& peinfo, std::shared_ptr<CSemanticGeneration> sgobject) const {
        if (!sgobject) return false;
        std::vector <std::string> detected_l;
        auto subsg = sgobject->makeSubSg();
        if (!subsg) return false;
        std::string exetype_str;
        const auto bi_exetype = peinfo.FindMember(PI_EXETYPE);
        if (bi_exetype != peinfo.MemberEnd()) {
            if (!bi_exetype->value.IsNull() && bi_exetype->value.IsString()) {
                exetype_str = bi_exetype->value.GetString();
                if (exetype_str.length()) {
                    exetype_str = StringToLower(exetype_str);
                }
            }
        }
        subsg->addElement(PI_EXETYPE, exetype_str);
        bool bimports = false;
        const auto bi_bimports = peinfo.FindMember(PI_IMPORTS);
        if (bi_bimports != peinfo.MemberEnd()) {
            if (!bi_bimports->value.IsNull() && bi_bimports->value.IsBool()) {
                bimports = bi_bimports->value.GetBool();
            }
        }
        subsg->addElement(PI_IMPORTS, bimports);
        bool bexports = false;
        const auto bi_bexports = peinfo.FindMember(PI_EXPORTS);
        if (bi_bexports != peinfo.MemberEnd()) {
            if (!bi_bexports->value.IsNull() && bi_bexports->value.IsBool()) {
                bexports = bi_bexports->value.GetBool();
            }
        }
        subsg->addElement(PI_EXPORTS, bexports);
        bool bresources = false;
        const auto bi_bresources = peinfo.FindMember(PI_RESOURCES);
        if (bi_bresources != peinfo.MemberEnd()) {
            if (!bi_bresources->value.IsNull() && bi_bresources->value.IsBool()) {
                bresources = bi_bresources->value.GetBool();
            }
        }
        subsg->addElement(PI_RESOURCES, bresources);
        bool bsecurity = false;
        const auto bi_bsecurity = peinfo.FindMember(PI_SECURITY);
        if (bi_bsecurity != peinfo.MemberEnd()) {
            if (!bi_bsecurity->value.IsNull() && bi_bsecurity->value.IsBool()) {
                bsecurity = bi_bsecurity->value.GetBool();
            }
        }
        subsg->addElement(PI_SECURITY, bsecurity);
        bool breloc = false;
        const auto bi_breloc = peinfo.FindMember(PI_RELOC);
        if (bi_breloc != peinfo.MemberEnd()) {
            if (!bi_breloc->value.IsNull() && bi_breloc->value.IsBool()) {
                breloc = bi_breloc->value.GetBool();
            }
        }
        subsg->addElement(PI_RELOC, breloc);
        bool btls = false;
        const auto bi_btls = peinfo.FindMember(PI_TLS);
        if (bi_btls != peinfo.MemberEnd()) {
            if (!bi_btls->value.IsNull() && bi_btls->value.IsBool()) {
                btls = bi_btls->value.GetBool();
            }
        }
        subsg->addElement(PI_TLS, btls);
        bool bboundimport = false;
        const auto bi_bboundimport = peinfo.FindMember(PI_BOUND_IMPORT);
        if (bi_bboundimport != peinfo.MemberEnd()) {
            if (!bi_bboundimport->value.IsNull() && bi_bboundimport->value.IsBool()) {
                bboundimport = bi_bboundimport->value.GetBool();
            }
        }
        subsg->addElement(PI_BOUND_IMPORT, bboundimport);
        bool bdelayimport = false;
        const auto bi_bdelayimport = peinfo.FindMember(PI_DELAY_IMPORT);
        if (bi_bdelayimport != peinfo.MemberEnd()) {
            if (!bi_bdelayimport->value.IsNull() && bi_bdelayimport->value.IsBool()) {
                bdelayimport = bi_bdelayimport->value.GetBool();
            }
        }
        subsg->addElement(PI_DELAY_IMPORT, bdelayimport);
        bool bdotnet = false;
        const auto bi_bdotnet = peinfo.FindMember(PI_DOTNET);
        if (bi_bdotnet != peinfo.MemberEnd()) {
            if (!bi_bdotnet->value.IsNull() && bi_bdotnet->value.IsBool()) {
                bdotnet = bi_bdotnet->value.GetBool();
            }
        }
        subsg->addElement(PI_DOTNET, bdotnet);
        bool bdebug = false;
        const auto bi_bdebug = peinfo.FindMember(PI_DEBUG);
        if (bi_bdebug != peinfo.MemberEnd()) {
            if (!bi_bdebug->value.IsNull() && bi_bdebug->value.IsBool()) {
                bdebug = bi_bdebug->value.GetBool();
            }
        }
        subsg->addElement(PI_DEBUG, bdebug);
        bool bhasoverlay = false;
        const auto bi_bhasoverlay = peinfo.FindMember(PI_HAS_OVERLAY);
        if (bi_bhasoverlay != peinfo.MemberEnd()) {
            if (!bi_bhasoverlay->value.IsNull() && bi_bhasoverlay->value.IsBool()) {
                bhasoverlay = bi_bhasoverlay->value.GetBool();
            }
        }
        subsg->addElement(PI_HAS_OVERLAY, bhasoverlay);
        std::string entropy_str = PI_UNPACK;
        double entropyd = 0.0;
        const auto bi_entropyd = peinfo.FindMember(PI_ENTROPY);
        if (bi_entropyd != peinfo.MemberEnd()) {
            if (!bi_entropyd->value.IsNull() && bi_entropyd->value.IsDouble()) {
                entropyd = bi_entropyd->value.GetDouble();
                if (IsPackData(entropyd)) {
                    entropy_str = PI_PACK;
                }
                else if (MaybePackData(entropyd)) {
                    entropy_str = PI_MAYPACK;
                }
            }
        }
        subsg->addElement(PI_ENTROPY, entropy_str);
        return sgobject->addElement(PI_PEINFO, std::move(subsg));
    }

    bool CTextCorpusManager::CommonParseToolInfo(ConstRapidObject& toolinfo, std::shared_ptr<CSemanticGeneration> sgobject) const {
        if (!sgobject) return false;
        std::vector <std::string> detected_l;
        auto subsg = sgobject->makeSubSg();
        if (!subsg) return false;
        std::string compiler_str;
        const auto bi_compiler = toolinfo.FindMember(TI_COMPILER);
        if (bi_compiler != toolinfo.MemberEnd()) {
            if (!bi_compiler->value.IsNull() && bi_compiler->value.IsString()) {
                if (CStrExtractor::detectedVSCorpus(bi_compiler->value.GetString(), detected_l)) {
                    for (auto& detect : detected_l) {
                        if (!compiler_str.length())
                            compiler_str = detect;
                        else
                            compiler_str += " " + detect;
                    }
                }
            }
        }
        subsg->addElement(TI_COMPILER, compiler_str);
        std::string linker_str;
        const auto bi_linker = toolinfo.FindMember(TI_LINKER);
        if (bi_linker != toolinfo.MemberEnd()) {
            if (!bi_linker->value.IsNull() && bi_linker->value.IsString()) {
                if (CStrExtractor::detectedVSCorpus(bi_linker->value.GetString(), detected_l)) {
                    for (auto& detect : detected_l) {
                        if (!linker_str.length())
                            linker_str = detect;
                        else
                            linker_str += " " + detect;
                    }
                }
            }
        }
        subsg->addElement(TI_LINKER, linker_str);
        std::string packer_str;
        const auto bi_packer = toolinfo.FindMember(TI_PACKER);
        if (bi_packer != toolinfo.MemberEnd()) {
            if (!bi_packer->value.IsNull() && bi_packer->value.IsString()) {
                if (CStrExtractor::detectedVSCorpus(bi_packer->value.GetString(), detected_l)) {
                    for (auto& detect : detected_l) {
                        if (!packer_str.length())
                            packer_str = detect;
                        else
                            packer_str += " " + detect;
                    }
                }
            }
        }
        subsg->addElement(TI_PACKER, packer_str);
        std::string installer_str;
        const auto bi_installer = toolinfo.FindMember(TI_INSTALLER);
        if (bi_installer != toolinfo.MemberEnd()) {
            if (!bi_installer->value.IsNull() && bi_installer->value.IsString()) {
                if (CStrExtractor::detectedUSCorpus(bi_installer->value.GetString(), detected_l)) {
                    for (auto& detect : detected_l) {
                        if (!installer_str.length())
                            installer_str = detect;
                        else
                            installer_str += " " + detect;
                    }
                }
            }
        }
        subsg->addElement(TI_INSTALLER, installer_str);
        return sgobject->addElement(TI_TOOLINFO, std::move(subsg));
    }

    bool CTextCorpusManager::CommonParseDataDirectoryInfo(ConstRapidObject& ddinfo, std::shared_ptr<CSemanticGeneration> sgobject) const {
        if (!sgobject) return false;
        std::vector <std::string> detected_l;
        auto subsg = sgobject->makeSubSg();
        if (!subsg) return false;
        bool bexportdirectory = false;
        const auto bi_exportdirectory = ddinfo.FindMember(DD_EXPORT_DIRECTORY);
        if (bi_exportdirectory != ddinfo.MemberEnd()) {
            if (!bi_exportdirectory->value.IsNull() && bi_exportdirectory->value.IsObject()) {
                auto object = bi_exportdirectory->value.GetObjectA();
                auto dd_reladdr = object.FindMember(DD_RELADDR);
                if (dd_reladdr != object.MemberEnd()) {
                    if (!dd_reladdr->value.IsNull() && dd_reladdr->value.IsInt()) {
                        if (dd_reladdr->value.GetInt()) {
                            bexportdirectory = true;
                        }
                    }
                }
            }
        }
        subsg->addElement(DD_EXPORT_DIRECTORY, bexportdirectory);
        bool bimportdirectory = false;
        const auto bi_importdirectory = ddinfo.FindMember(DD_IMPORT_DIRECTORY);
        if (bi_importdirectory != ddinfo.MemberEnd()) {
            if (!bi_importdirectory->value.IsNull() && bi_importdirectory->value.IsObject()) {
                auto object = bi_importdirectory->value.GetObjectA();
                auto dd_reladdr = object.FindMember(DD_RELADDR);
                if (dd_reladdr != object.MemberEnd()) {
                    if (!dd_reladdr->value.IsNull() && dd_reladdr->value.IsInt()) {
                        if (dd_reladdr->value.GetInt()) {
                            bimportdirectory = true;
                        }
                    }
                }
            }
        }
        subsg->addElement(DD_IMPORT_DIRECTORY, bimportdirectory);
        bool bresdirectory = false;
        const auto bi_resdirectory = ddinfo.FindMember(DD_RESOURCE_DIRECTORY);
        if (bi_resdirectory != ddinfo.MemberEnd()) {
            if (!bi_resdirectory->value.IsNull() && bi_resdirectory->value.IsObject()) {
                auto object = bi_resdirectory->value.GetObjectA();
                auto dd_reladdr = object.FindMember(DD_RELADDR);
                if (dd_reladdr != object.MemberEnd()) {
                    if (!dd_reladdr->value.IsNull() && dd_reladdr->value.IsInt()) {
                        if (dd_reladdr->value.GetInt()) {
                            bresdirectory = true;
                        }
                    }
                }
            }
        }
        subsg->addElement(DD_RESOURCE_DIRECTORY, bresdirectory);
        bool bexceptiondirectory = false;
        const auto bi_exceptiondirectory = ddinfo.FindMember(DD_EXCEPTION_DIRECTORY);
        if (bi_exceptiondirectory != ddinfo.MemberEnd()) {
            if (!bi_exceptiondirectory->value.IsNull() && bi_exceptiondirectory->value.IsObject()) {
                auto object = bi_exceptiondirectory->value.GetObjectA();
                auto dd_reladdr = object.FindMember(DD_RELADDR);
                if (dd_reladdr != object.MemberEnd()) {
                    if (!dd_reladdr->value.IsNull() && dd_reladdr->value.IsInt()) {
                        if (dd_reladdr->value.GetInt()) {
                            bexceptiondirectory = true;
                        }
                    }
                }
            }
        }
        subsg->addElement(DD_EXCEPTION_DIRECTORY, bexceptiondirectory);
        bool bsecurityiondirectory = false;
        const auto bi_securityiondirectory = ddinfo.FindMember(DD_SECURITY_DIRECTORY);
        if (bi_securityiondirectory != ddinfo.MemberEnd()) {
            if (!bi_securityiondirectory->value.IsNull() && bi_securityiondirectory->value.IsObject()) {
                auto object = bi_securityiondirectory->value.GetObjectA();
                auto dd_reladdr = object.FindMember(DD_RELADDR);
                if (dd_reladdr != object.MemberEnd()) {
                    if (!dd_reladdr->value.IsNull() && dd_reladdr->value.IsInt()) {
                        if (dd_reladdr->value.GetInt()) {
                            bsecurityiondirectory = true;
                        }
                    }
                }
            }
        }
        subsg->addElement(DD_SECURITY_DIRECTORY, bsecurityiondirectory);
        bool bbaserelociondirectory = false;
        const auto bi_baserelociondirectory = ddinfo.FindMember(DD_BASERELOC_DIRECTORY);
        if (bi_baserelociondirectory != ddinfo.MemberEnd()) {
            if (!bi_baserelociondirectory->value.IsNull() && bi_baserelociondirectory->value.IsObject()) {
                auto object = bi_baserelociondirectory->value.GetObjectA();
                auto dd_reladdr = object.FindMember(DD_RELADDR);
                if (dd_reladdr != object.MemberEnd()) {
                    if (!dd_reladdr->value.IsNull() && dd_reladdr->value.IsInt()) {
                        if (dd_reladdr->value.GetInt()) {
                            bbaserelociondirectory = true;
                        }
                    }
                }
            }
        }
        subsg->addElement(DD_BASERELOC_DIRECTORY, bbaserelociondirectory);
        bool bdebugdirectory = false;
        const auto bi_debugdirectory = ddinfo.FindMember(DD_DEBUG_DIRECTORY);
        if (bi_debugdirectory != ddinfo.MemberEnd()) {
            if (!bi_debugdirectory->value.IsNull() && bi_debugdirectory->value.IsObject()) {
                auto object = bi_debugdirectory->value.GetObjectA();
                auto dd_reladdr = object.FindMember(DD_RELADDR);
                if (dd_reladdr != object.MemberEnd()) {
                    if (!dd_reladdr->value.IsNull() && dd_reladdr->value.IsInt()) {
                        if (dd_reladdr->value.GetInt()) {
                            bdebugdirectory = true;
                        }
                    }
                }
            }
        }
        subsg->addElement(DD_DEBUG_DIRECTORY, bdebugdirectory);
        bool barchdirectory = false;
        const auto bi_archdirectory = ddinfo.FindMember(DD_ARCHITECTURE_DIRECTORY);
        if (bi_archdirectory != ddinfo.MemberEnd()) {
            if (!bi_archdirectory->value.IsNull() && bi_archdirectory->value.IsObject()) {
                auto object = bi_archdirectory->value.GetObjectA();
                auto dd_reladdr = object.FindMember(DD_RELADDR);
                if (dd_reladdr != object.MemberEnd()) {
                    if (!dd_reladdr->value.IsNull() && dd_reladdr->value.IsInt()) {
                        if (dd_reladdr->value.GetInt()) {
                            barchdirectory = true;
                        }
                    }
                }
            }
        }
        subsg->addElement(DD_ARCHITECTURE_DIRECTORY, barchdirectory);
        bool bglobalptrdirectory = false;
        const auto bi_globalptrdirectory = ddinfo.FindMember(DD_GLOBALPTR_DIRECTORY);
        if (bi_globalptrdirectory != ddinfo.MemberEnd()) {
            if (!bi_globalptrdirectory->value.IsNull() && bi_globalptrdirectory->value.IsObject()) {
                auto object = bi_globalptrdirectory->value.GetObjectA();
                auto dd_reladdr = object.FindMember(DD_RELADDR);
                if (dd_reladdr != object.MemberEnd()) {
                    if (!dd_reladdr->value.IsNull() && dd_reladdr->value.IsInt()) {
                        if (dd_reladdr->value.GetInt()) {
                            bglobalptrdirectory = true;
                        }
                    }
                }
            }
        }
        subsg->addElement(DD_GLOBALPTR_DIRECTORY, bglobalptrdirectory);
        bool btlsdirectory = false;
        const auto bi_tlsdirectory = ddinfo.FindMember(DD_TLS_DIRECTORY);
        if (bi_tlsdirectory != ddinfo.MemberEnd()) {
            if (!bi_tlsdirectory->value.IsNull() && bi_tlsdirectory->value.IsObject()) {
                auto object = bi_tlsdirectory->value.GetObjectA();
                auto dd_reladdr = object.FindMember(DD_RELADDR);
                if (dd_reladdr != object.MemberEnd()) {
                    if (!dd_reladdr->value.IsNull() && dd_reladdr->value.IsInt()) {
                        if (dd_reladdr->value.GetInt()) {
                            btlsdirectory = true;
                        }
                    }
                }
            }
        }
        subsg->addElement(DD_TLS_DIRECTORY, btlsdirectory);
        bool bloadconfigdirectory = false;
        const auto bi_loadconfigdirectory = ddinfo.FindMember(DD_LOADCONFIG_DIRECTORY);
        if (bi_loadconfigdirectory != ddinfo.MemberEnd()) {
            if (!bi_loadconfigdirectory->value.IsNull() && bi_loadconfigdirectory->value.IsObject()) {
                auto object = bi_loadconfigdirectory->value.GetObjectA();
                auto dd_reladdr = object.FindMember(DD_RELADDR);
                if (dd_reladdr != object.MemberEnd()) {
                    if (!dd_reladdr->value.IsNull() && dd_reladdr->value.IsInt()) {
                        if (dd_reladdr->value.GetInt()) {
                            bloadconfigdirectory = true;
                        }
                    }
                }
            }
        }
        subsg->addElement(DD_LOADCONFIG_DIRECTORY, bloadconfigdirectory);
        bool bboundimportdirectory = false;
        const auto bi_boundimportdirectory = ddinfo.FindMember(DD_BOUNDIMPORT_DIRECTORY);
        if (bi_boundimportdirectory != ddinfo.MemberEnd()) {
            if (!bi_boundimportdirectory->value.IsNull() && bi_boundimportdirectory->value.IsObject()) {
                auto object = bi_boundimportdirectory->value.GetObjectA();
                auto dd_reladdr = object.FindMember(DD_RELADDR);
                if (dd_reladdr != object.MemberEnd()) {
                    if (!dd_reladdr->value.IsNull() && dd_reladdr->value.IsInt()) {
                        if (dd_reladdr->value.GetInt()) {
                            bboundimportdirectory = true;
                        }
                    }
                }
            }
        }
        subsg->addElement(DD_BOUNDIMPORT_DIRECTORY, bboundimportdirectory);
        bool biatdirectory = false;
        const auto bi_iatdirectory = ddinfo.FindMember(DD_IAT_DIRECTORY);
        if (bi_iatdirectory != ddinfo.MemberEnd()) {
            if (!bi_iatdirectory->value.IsNull() && bi_iatdirectory->value.IsObject()) {
                auto object = bi_iatdirectory->value.GetObjectA();
                auto dd_reladdr = object.FindMember(DD_RELADDR);
                if (dd_reladdr != object.MemberEnd()) {
                    if (!dd_reladdr->value.IsNull() && dd_reladdr->value.IsInt()) {
                        if (dd_reladdr->value.GetInt()) {
                            biatdirectory = true;
                        }
                    }
                }
            }
        }
        subsg->addElement(DD_IAT_DIRECTORY, biatdirectory);
        bool bdelayimportdirectory = false;
        const auto bi_delayimportdirectory = ddinfo.FindMember(DD_DELAYIMPORT_DIRECTORY);
        if (bi_delayimportdirectory != ddinfo.MemberEnd()) {
            if (!bi_delayimportdirectory->value.IsNull() && bi_delayimportdirectory->value.IsObject()) {
                auto object = bi_delayimportdirectory->value.GetObjectA();
                auto dd_reladdr = object.FindMember(DD_RELADDR);
                if (dd_reladdr != object.MemberEnd()) {
                    if (!dd_reladdr->value.IsNull() && dd_reladdr->value.IsInt()) {
                        if (dd_reladdr->value.GetInt()) {
                            bdelayimportdirectory = true;
                        }
                    }
                }
            }
        }
        subsg->addElement(DD_DELAYIMPORT_DIRECTORY, bdelayimportdirectory);
        bool bcomdesdirectory = false;
        const auto bi_comdesdirectory = ddinfo.FindMember(DD_COMDES_DIRECTORY);
        if (bi_comdesdirectory != ddinfo.MemberEnd()) {
            if (!bi_comdesdirectory->value.IsNull() && bi_comdesdirectory->value.IsObject()) {
                auto object = bi_comdesdirectory->value.GetObjectA();
                auto dd_reladdr = object.FindMember(DD_RELADDR);
                if (dd_reladdr != object.MemberEnd()) {
                    if (!dd_reladdr->value.IsNull() && dd_reladdr->value.IsInt()) {
                        if (dd_reladdr->value.GetInt()) {
                            bcomdesdirectory = true;
                        }
                    }
                }
            }
        }
        subsg->addElement(DD_COMDES_DIRECTORY, bcomdesdirectory);
        return sgobject->addElement(DD_DATADIRECTORY, std::move(subsg));
    }

    bool CTextCorpusManager::CommonParsePdbInfosInfo(ConstRapidObject& pdbinfo, std::shared_ptr<CSemanticGeneration> sgobject) const {
        if (!sgobject) return false;
        auto subsg = sgobject->makeSubSg();
        if (!subsg) return false;
        for (auto& pdb : pdbinfo) {
            std::string pdb_guid;
            pdb_guid = pdb.name.GetString();
            subsg->addElement(PS_GUID, pdb_guid);
            if (!pdb.value.IsNull() && pdb.value.IsObject()) {
                auto pdbobj = pdb.value.GetObjectA();
                std::string type_str;
                const auto bi_type = pdbobj.FindMember(PS_TYPE);
                if (bi_type != pdbobj.MemberEnd()) {
                    if (!bi_type->value.IsNull() && bi_type->value.IsString()) {
                        type_str = bi_type->value.GetString();
                        if (type_str.length()) {
                            type_str = StringToLower(type_str);
                        }
                    }
                }
                subsg->addElement(PS_TYPE, type_str);
                std::string path_str;
                const auto bi_path = pdbobj.FindMember(PS_PATH);
                if (bi_path != pdbobj.MemberEnd()) {
                    if (!bi_path->value.IsNull() && bi_path->value.IsString()) {
                        path_str = bi_path->value.GetString();
                        if (path_str.length()) {
                            auto path = fs::path(path_str);
                            path_str = path.filename().u8string();
                            if (path_str.length()) {
                                path_str = StringToLower(path_str);
                            }
                        }
                    }
                }
                subsg->addElement(PS_PATH, path_str);
            }
        }
        return sgobject->addElement(PS_PDBINFOS, std::move(subsg));
    }

    bool CTextCorpusManager::CommonParseEmbeddedpeInfo(ConstRapidObject& embeddedinfo, std::shared_ptr<CSemanticGeneration> sgobject) const {
        if (!sgobject) return false;
        const auto bi_offset = embeddedinfo.FindMember(ED_OFFSET);
        if (bi_offset != embeddedinfo.MemberEnd()) {
            return sgobject->addElement(PS_PDBINFOS, SI_TRUE);
        }
        return sgobject->addElement(PS_PDBINFOS, std::string(""));
    }

    bool CTextCorpusManager::CommonParseImportsInfo(ConstRapidObject& importsinfo, std::shared_ptr<CSemanticGeneration> sgobject) const {
        if (!sgobject) return false;
        std::vector <std::string> detected_l;
        auto subsg = sgobject->makeSubSg();
        if (!subsg) return false;
        for (auto& imports : importsinfo) {
            if (imports.value.IsNull() || !imports.value.IsObject())
                continue;
            std::stringstream imports_str;
            auto importins = imports.value.GetObjectA();
            const auto name = importins.FindMember(IT_NAME);
            std::string name_str;
            if (name != importins.MemberEnd()) {
                if (!name->value.IsNull() && name->value.IsString()) {
                    name_str = name->value.GetString();
                }
            }
            if (!name_str.length()) continue;
            name_str = StringToLower(name_str);
            std::string function_str;
            const auto it_functions = importins.FindMember(IT_FUNCTIONS);
            if (it_functions != importins.MemberEnd()) {
                if (!it_functions->value.IsNull() && it_functions->value.IsArray()) {
                    for (auto& item : it_functions->value.GetArray()) {
                        if (!item.IsNull() && item.IsString()) {
                            std::string api_str = item.GetString();
                            std::string detected;
                            if (CStrExtractor::extractApiCorpus(api_str, detected)) {
                                if (!function_str.length())
                                    function_str = StringToLower(detected);
                                else
                                    function_str += " " + detected;
                            }
                        }
                    }
                }
            }
            subsg->addElement(name_str, function_str);
        }
        return sgobject->addElement(IT_IMPORTS, std::move(subsg));
    }

    bool CTextCorpusManager::CommonParseExportsInfo(ConstRapidObject& exportsinfo, std::shared_ptr<CSemanticGeneration> sgobject) const {
        if (!sgobject) return false;
        std::vector <std::string> detected_l;
        auto subsg = sgobject->makeSubSg();
        if (!subsg) return false;
        std::stringstream exports_str;
        std::string dllname_str;
        const auto dllname = exportsinfo.FindMember(ET_DLL_NAME);
        if (dllname == exportsinfo.MemberEnd()) {
            return false;
        }
        if (!dllname->value.IsNull() && dllname->value.IsString()) {
            dllname_str = dllname->value.GetString();
        }
        if (!dllname_str.length())
            return false;
        const auto functions = exportsinfo.FindMember(ET_FUNCTIONS);
        if (functions == exportsinfo.MemberEnd())
            return true;
        std::string function_str;
        if (!functions->value.IsNull() && functions->value.IsArray()) {
            for (auto& item : functions->value.GetArray()) {
                if (!item.IsNull() && item.IsString()) {
                    std::string api_str = item.GetString();
                    std::string detected;
                    if (CStrExtractor::extractApiCorpus(api_str, detected)) {
                        if (!function_str.length())
                            function_str = detected;
                        else
                            function_str += " " + detected;
                    }
                }
            }
        }
        subsg->addElement(dllname_str, function_str);
        return sgobject->addElement(ET_EXPORTS, std::move(subsg));
    }

    bool CTextCorpusManager::CommonParseManifestInfo(ConstRapidObject& manifestinfo, std::shared_ptr<CSemanticGeneration> sgobject) const {
        if (!sgobject) return false;
        std::vector <std::string> detected_l;
        auto subsg = sgobject->makeSubSg();
        if (!subsg) return false;
        std::string assembly_str;
        const auto mf_assembly = manifestinfo.FindMember(MF_ASSEMBLY_IDENTITY);
        if (mf_assembly != manifestinfo.MemberEnd()) {
            if (!mf_assembly->value.IsNull() && mf_assembly->value.IsObject()) {
                std::string name_str;
                std::string type_str;
                auto assemblyobj = mf_assembly->value.GetObjectA();
                const auto mf_name = assemblyobj.FindMember(MF_NAME);
                if (mf_name != assemblyobj.MemberEnd()) {
                    if (!mf_name->value.IsNull() && mf_name->value.IsString()) {
                        if (CStrExtractor::detectedUSCorpus(mf_name->value.GetString(), detected_l)) {
                            for (auto& detect : detected_l) {
                                if (!name_str.length())
                                    name_str = detect;
                                else
                                    name_str += " " + detect;
                            }
                        }
                    }
                }
                assembly_str = name_str;
                const auto mf_type = assemblyobj.FindMember(MF_TYPE);
                if (mf_type != assemblyobj.MemberEnd()) {
                    if (!mf_type->value.IsNull() && mf_type->value.IsString()) {
                        type_str = mf_type->value.GetString();
                        if (type_str.length()) {
                            type_str = StringToLower(type_str);
                        }
                    }
                }
                if (type_str.length()) {
                    if (assembly_str.length())
                        assembly_str += " " + type_str;
                    else
                        assembly_str = type_str;
                }
            }
        }
        subsg->addElement(MF_ASSEMBLY_IDENTITY, assembly_str);
        std::string description_str;
        const auto mf_description = manifestinfo.FindMember(MF_DESCRIPTION);
        if (mf_description != manifestinfo.MemberEnd()) {
            if (!mf_description->value.IsNull() && mf_description->value.IsObject()) {
                std::string name_str;
                std::string type_str;
                auto descriptionobj = mf_description->value.GetObjectA();
                const auto mf_name = descriptionobj.FindMember(MF_NAME);
                if (mf_name != descriptionobj.MemberEnd()) {
                    if (!mf_name->value.IsNull() && mf_name->value.IsString()) {
                        if (CStrExtractor::detectedUSCorpus(mf_name->value.GetString(), detected_l)) {
                            for (auto& detect : detected_l) {
                                if (!name_str.length())
                                    name_str = detect;
                                else
                                    name_str += " " + detect;
                            }
                        }
                    }
                }
                description_str = name_str;
                const auto mf_type = descriptionobj.FindMember(MF_TYPE);
                if (mf_type != descriptionobj.MemberEnd()) {
                    if (!mf_type->value.IsNull() && mf_type->value.IsString()) {
                        type_str = mf_type->value.GetString();
                        if (type_str.length()) {
                            type_str = StringToLower(type_str);
                        }
                    }
                }
                if (type_str.length()) {
                    if (description_str.length())
                        description_str += " " + type_str;
                    else
                        description_str = type_str;
                }
            }
        }
        subsg->addElement(MF_DESCRIPTION, description_str);
        std::string dependent_str;
        const auto mf_dependent = manifestinfo.FindMember(MF_DEPENDENT_ASSEMBLY);
        if (mf_dependent != manifestinfo.MemberEnd()) {
            if (!mf_dependent->value.IsNull() && mf_dependent->value.IsObject()) {
                std::string name_str;
                std::string type_str;
                auto dependentobj = mf_dependent->value.GetObjectA();
                const auto mf_name = dependentobj.FindMember(MF_NAME);
                if (mf_name != dependentobj.MemberEnd()) {
                    if (!mf_name->value.IsNull() && mf_name->value.IsString()) {
                        if (CStrExtractor::detectedUSCorpus(mf_name->value.GetString(), detected_l)) {
                            for (auto& detect : detected_l) {
                                if (!name_str.length())
                                    name_str = detect;
                                else
                                    name_str += " " + detect;
                            }
                        }
                    }
                }
                dependent_str = name_str;
                const auto mf_type = dependentobj.FindMember(MF_TYPE);
                if (mf_type != dependentobj.MemberEnd()) {
                    if (!mf_type->value.IsNull() && mf_type->value.IsString()) {
                        type_str = mf_type->value.GetString();
                        if (type_str.length()) {
                            type_str = StringToLower(type_str);
                        }
                    }
                }
                if (type_str.length()) {
                    if (dependent_str.length())
                        dependent_str += " " + type_str;
                    else
                        dependent_str = type_str;
                }
            }
        }
        subsg->addElement(MF_DEPENDENT_ASSEMBLY, dependent_str);
        std::string level_str;
        const auto mf_level = manifestinfo.FindMember(MF_REQUESTED_EXECUTION_LEVEL);
        if (mf_level != manifestinfo.MemberEnd()) {
            if (!mf_level->value.IsNull() && mf_level->value.IsObject()) {
                auto levelobj = mf_level->value.GetObjectA();
                const auto mf_name = levelobj.FindMember(MF_LEVEL);
                if (mf_name != levelobj.MemberEnd()) {
                    if (!mf_name->value.IsNull() && mf_name->value.IsString()) {
                        if (CStrExtractor::detectedUSCorpus(mf_name->value.GetString(), detected_l)) {
                            for (auto& detect : detected_l) {
                                if (!level_str.length())
                                    level_str = detect;
                                else
                                    level_str += " " + detect;
                            }
                        }
                    }
                }
            }
        }
        subsg->addElement(MF_REQUESTED_EXECUTION_LEVEL, level_str);
        return sgobject->addElement(MF_MANIFEST, std::move(subsg));
    }

    bool CTextCorpusManager::CommonParseResourceInfo(ConstRapidObject& resourceinfo, std::shared_ptr<CSemanticGeneration> sgobject) const {
        if (!sgobject) return false;
        auto subsg = sgobject->makeSubSg();
        if (!subsg) return false;
        std::vector<std::string> detected_l;
        std::string commpanyname_str;
        const auto bi_commpanyname = resourceinfo.FindMember(RI_COMMPANYNAME);
        if (bi_commpanyname != resourceinfo.MemberEnd()) {
            if (!bi_commpanyname->value.IsNull() && bi_commpanyname->value.IsString()) {
                commpanyname_str = bi_commpanyname->value.GetString();
                if (commpanyname_str.length()) {
                    std::string des_str;
                    if (CStrExtractor::detectedUSCorpus(commpanyname_str, detected_l)) {
                        for (auto& detect : detected_l) {
                            if (!des_str.length())
                                des_str = detect;
                            else
                                des_str += " " + detect;
                        }
                    }
                    commpanyname_str = des_str;
                }
            }
        }
        subsg->addElement(RI_COMMPANYNAME, commpanyname_str);
        std::string description_str;
        const auto bi_description = resourceinfo.FindMember(RI_DESCRIPTION);
        if (bi_description != resourceinfo.MemberEnd()) {
            if (!bi_description->value.IsNull() && bi_description->value.IsString()) {
                description_str = bi_description->value.GetString();
                if (description_str.length()) {
                    std::string des_str;
                    if (CStrExtractor::detectedUSCorpus(description_str, detected_l)) {
                        for (auto& detect : detected_l) {
                            if (!des_str.length())
                                des_str = detect;
                            else
                                des_str += " " + detect;
                        }
                    }
                    description_str = des_str;
                }
            }
        }
        subsg->addElement(RI_DESCRIPTION, description_str);
        std::uint32_t file_version = 0;
        const auto bi_filever = resourceinfo.FindMember(RI_FILEVER);
        if (bi_filever != resourceinfo.MemberEnd()) {
            if (!bi_filever->value.IsNull() && bi_filever->value.IsString()) {
                std::string filever_str = bi_filever->value.GetString();
                if (filever_str.length()) {
                    file_version = 5;
                }
            }
        }
        subsg->addElement(RI_FILEVER, file_version);
        std::string internalname_str;
        const auto bi_internalname = resourceinfo.FindMember(RI_INTERNALNAME);
        if (bi_internalname != resourceinfo.MemberEnd()) {
            if (!bi_internalname->value.IsNull() && bi_internalname->value.IsString()) {
                internalname_str = bi_internalname->value.GetString();
                if (internalname_str.length()) {
                    std::string name_str;
                    if (CStrExtractor::detectedUSCorpus(internalname_str, detected_l)) {
                        for (auto& detect : detected_l) {
                            if (!name_str.length())
                                name_str = detect;
                            else
                                name_str += " " + detect;
                        }
                    }
                    internalname_str = name_str;
                }
            }
        }
        subsg->addElement(RI_INTERNALNAME, internalname_str);
        std::string originalname_str;
        const auto bi_originalname = resourceinfo.FindMember(RI_ORIGINALNAME);
        if (bi_originalname != resourceinfo.MemberEnd()) {
            if (!bi_originalname->value.IsNull() && bi_originalname->value.IsString()) {
                originalname_str = bi_originalname->value.GetString();
                if (originalname_str.length()) {
                    std::string name_str;
                    if (CStrExtractor::detectedUSCorpus(originalname_str, detected_l)) {
                        for (auto& detect : detected_l) {
                            if (!name_str.length())
                                name_str = detect;
                            else
                                name_str += " " + detect;
                        }
                    }
                    originalname_str = name_str;
                }
            }
        }
        subsg->addElement(RI_ORIGINALNAME, originalname_str);
        std::string productname_str;
        const auto bi_productname = resourceinfo.FindMember(RI_PRODUCTNAME);
        if (bi_productname != resourceinfo.MemberEnd()) {
            if (!bi_productname->value.IsNull() && bi_productname->value.IsString()) {
                productname_str = bi_productname->value.GetString();
                if (productname_str.length()) {
                    std::string name_str;
                    if (CStrExtractor::detectedUSCorpus(productname_str, detected_l)) {
                        for (auto& detect : detected_l) {
                            if (!name_str.length())
                                name_str = detect;
                            else
                                name_str += " " + detect;
                        }
                    }
                    productname_str = name_str;
                }
            }
        }
        subsg->addElement(RI_PRODUCTNAME, productname_str);
        std::uint32_t product_version = 0;
        const auto bi_productver = resourceinfo.FindMember(RI_PRODUCTVER);
        if (bi_productver != resourceinfo.MemberEnd()) {
            if (!bi_productver->value.IsNull() && bi_productver->value.IsString()) {
                std::string productver_str = bi_productver->value.GetString();
                if (productver_str.length()) {
                    product_version = 5;
                }
            }
        }
        subsg->addElement(RI_PRODUCTVER, product_version);
        std::string copyright_str;
        const auto bi_copyright = resourceinfo.FindMember(RI_COPYRIGHT);
        if (bi_copyright != resourceinfo.MemberEnd()) {
            if (!bi_copyright->value.IsNull() && bi_copyright->value.IsString()) {
                copyright_str = bi_copyright->value.GetString();
                if (copyright_str.length()) {
                    std::string name_str;
                    if (CStrExtractor::detectedUSCorpus(copyright_str, detected_l)) {
                        for (auto& detect : detected_l) {
                            if (!name_str.length())
                                name_str = detect;
                            else
                                name_str += " " + detect;
                        }
                    }
                    copyright_str = name_str;
                }
            }
        }
        subsg->addElement(RI_COPYRIGHT, copyright_str);
        std::string comments_str;
        const auto bi_comments = resourceinfo.FindMember(RI_COMMENTS);
        if (bi_comments != resourceinfo.MemberEnd()) {
            if (!bi_comments->value.IsNull() && bi_comments->value.IsString()) {
                comments_str = bi_comments->value.GetString();
                if (comments_str.length()) {
                    std::string name_str;
                    if (CStrExtractor::detectedUSCorpus(comments_str, detected_l)) {
                        for (auto& detect : detected_l) {
                            if (!name_str.length())
                                name_str = detect;
                            else
                                name_str += " " + detect;
                        }
                    }
                    comments_str = name_str;
                }
            }
        }
        subsg->addElement(RI_COMMENTS, comments_str);
        std::string trademarks_str;
        const auto bi_trademarks = resourceinfo.FindMember(RI_TRADEMARKS);
        if (bi_trademarks != resourceinfo.MemberEnd()) {
            if (!bi_trademarks->value.IsNull() && bi_trademarks->value.IsString()) {
                trademarks_str = bi_trademarks->value.GetString();
                if (trademarks_str.length()) {
                    std::string name_str;
                    if (CStrExtractor::detectedUSCorpus(trademarks_str, detected_l)) {
                        for (auto& detect : detected_l) {
                            if (!name_str.length())
                                name_str = detect;
                            else
                                name_str += " " + detect;
                        }
                    }
                    trademarks_str = name_str;
                }
            }
        }
        subsg->addElement(RI_TRADEMARKS, trademarks_str);
        std::uint32_t privatebuild = 0;
        const auto bi_privatebuild = resourceinfo.FindMember(RI_PRIVATEBUILD);
        if (bi_privatebuild != resourceinfo.MemberEnd()) {
            if (!bi_privatebuild->value.IsNull() && bi_privatebuild->value.IsString()) {
                std::string privatebuild_str = bi_privatebuild->value.GetString();
                if (privatebuild_str.length()) {
                    privatebuild = 5;
                }
            }
        }
        subsg->addElement(RI_PRIVATEBUILD, privatebuild);
        std::uint32_t specialbuild = 0;
        const auto bi_specialbuild = resourceinfo.FindMember(RI_SPECIALBUILD);
        if (bi_specialbuild != resourceinfo.MemberEnd()) {
            if (!bi_specialbuild->value.IsNull() && bi_specialbuild->value.IsString()) {
                std::string specialbuild_str = bi_specialbuild->value.GetString();
                if (specialbuild_str.length()) {
                    specialbuild = 5;
                }
            }
        }
        subsg->addElement(RI_SPECIALBUILD, specialbuild);
        return sgobject->addElement(RI_RESOURCEINFO, std::move(subsg));
    }

    bool CTextCorpusManager::CommonParseRichheaderInfo(ConstRapidObject& richheaderinfo, std::shared_ptr<CSemanticGeneration> sgobject) const {
        if (!sgobject) return false;
        bool brichheader = false;
        for (auto& richheader : richheaderinfo) {
            if (richheader.value.IsNull() || !richheader.value.IsObject())
                continue;
            brichheader = true;
        }
        return sgobject->addElement(RH_RICHHEADER, brichheader);
    }

    bool CTextCorpusManager::CommonParseSectionsInfo(ConstRapidObject& sectionsinfo, std::shared_ptr<CSemanticGeneration> sgobject) const {
        if (!sgobject) return false;
        std::vector <std::string> detected_l;
        auto subsg = sgobject->makeSubSg();
        if (!subsg) return false;
        for (auto& sec : sectionsinfo) {
            if (sec.value.IsNull() || !sec.value.IsObject())
                continue;
            std::string sec_name = sec.name.GetString();
            if (!sec_name.length())
                continue;
            std::vector<std::pair<std::string, std::any>> value_vec;
            bool has_data = false;
            bool has_code = false;
            bool data_only = false;
            bool read_only = false;
            bool is_bbs = false;
            bool is_debug = false;
            bool is_info = false;
            std::string entropy = PI_UNPACK;

            const auto section = sec.value.GetObjectA();
            const auto hasdata = section.FindMember(SS_HASDATA);
            if (hasdata != section.MemberEnd()) {
                if (!hasdata->value.IsNull() && hasdata->value.IsBool()) {
                    has_data = hasdata->value.GetBool();
                }
            }
            value_vec.push_back(std::pair(SS_HASDATA, has_data));
            const auto hascode = section.FindMember(SS_HASCODE);
            if (hascode != section.MemberEnd()) {
                if (!hascode->value.IsNull() && hascode->value.IsBool()) {
                    has_code = hascode->value.GetBool();
                }
            }
            value_vec.push_back(std::pair(SS_HASCODE, has_code));
            const auto dataonly = section.FindMember(SS_DATAONLY);
            if (dataonly != section.MemberEnd()) {
                if (!dataonly->value.IsNull() && dataonly->value.IsBool()) {
                    data_only = dataonly->value.GetBool();
                }
            }
            value_vec.push_back(std::pair(SS_DATAONLY, data_only));
            const auto readonly = section.FindMember(SS_READONLY);
            if (readonly != section.MemberEnd()) {
                if (!readonly->value.IsNull() && readonly->value.IsBool()) {
                    read_only = readonly->value.GetBool();
                }
            }
            value_vec.push_back(std::pair(SS_READONLY, read_only));
            const auto isbbs = section.FindMember(SS_BBS);
            if (isbbs != section.MemberEnd()) {
                if (!isbbs->value.IsNull() && isbbs->value.IsBool()) {
                    is_bbs = isbbs->value.GetBool();
                }
            }
            value_vec.push_back(std::pair(SS_BBS, is_bbs));
            const auto isdebug = section.FindMember(SS_DEBUG);
            if (isdebug != section.MemberEnd()) {
                if (!isdebug->value.IsNull() && isdebug->value.IsBool()) {
                    is_debug = isdebug->value.GetBool();
                }
            }
            value_vec.push_back(std::pair(SS_DEBUG, is_debug));
            const auto isinfo = section.FindMember(SS_INFO);
            if (isinfo != section.MemberEnd()) {
                if (!isinfo->value.IsNull() && isinfo->value.IsBool()) {
                    is_info = isinfo->value.GetBool();
                }
            }
            value_vec.push_back(std::pair(SS_INFO, is_info));
            const auto ep = section.FindMember(SS_ENTROPY);
            if (ep != section.MemberEnd()) {
                if (!ep->value.IsNull() && ep->value.IsDouble()) {
                    double entropyd = ep->value.GetDouble();
                    if (IsPackData(entropyd)) {
                        entropy = PI_PACK;
                    }
                    else if (MaybePackData(entropyd)) {
                        entropy = PI_MAYPACK;
                    }
                }
            }
            value_vec.push_back(std::pair(SS_ENTROPY, entropy));
            subsg->addElement(sec_name, value_vec);
        }
        return sgobject->addElement(SS_SECTIONS, std::move(subsg));
    }

    bool CTextCorpusManager::CommonParseStrfeatureInfo(ConstRapidObject& strfeatureinfo, std::shared_ptr<CSemanticGeneration> sgobject, std::uint32_t feature_nums) const {
        if (!sgobject) return false;
        std::vector <std::string> detected_l;
        auto subsg = sgobject->makeSubSg();
        if (!subsg) return false;
        std::uint64_t strings_count = 0;
        for (auto& strfeature : strfeatureinfo) {
            if (!strfeature.value.IsNull() && strfeature.value.IsObject()) {
                auto strfeatureobj = strfeature.value.GetObjectA();
                std::string secname = strfeature.name.GetString();
                if (!secname.length()) continue;
                secname = StringToLower(secname);
                const auto entropy = strfeatureobj.FindMember(SF_ENTROPY);
                if (entropy != strfeatureobj.MemberEnd()) {
                    if (!entropy->value.IsNull() && entropy->value.IsDouble()) {
                        double entropyval = entropy->value.GetDouble();
                        if (IsPackData(entropyval))
                            continue;
                    }
                }
                const auto insidestr = strfeatureobj.FindMember(SF_INSIDESTR);
                if (insidestr != strfeatureobj.MemberEnd()) {
                    if (!insidestr->value.IsNull() && insidestr->value.IsArray()) {
                        std::string feature_strings;
                        for (auto& item : insidestr->value.GetArray()) {
                            if (!item.IsNull() && item.IsString()) {
                                std::string str = item.GetString();
                                if (CStrExtractor::detectedInsideCorpus(str, detected_l)) {
                                    for (auto& detect : detected_l) {
                                        if (!feature_strings.length())
                                            feature_strings = detect;
                                        else
                                            feature_strings += " " + detect;
                                        strings_count++;
                                    }
                                }
                            }
                            if (strings_count > feature_nums)
                                break;
                        }
                        subsg->addElement(secname, feature_strings);
                        if (subsg->getelemcount() >= feature_nums) {
                            break;
                        }
                    }
                }
            }
        }
        return sgobject->addElement(SF_STRINGS_FEATURE, std::move(subsg));
    }

    bool CTextCorpusManager::CommonParseFlowGraph(ConstRapidObject& flowgraphobj, std::shared_ptr<CSemanticGeneration> sgobject, std::uint32_t feature_nums) const {
        if (!sgobject) return false;
        std::vector <std::string> detected_l;
        auto subsg = sgobject->makeSubSg();
        if (!subsg) return false;
        unsigned int flowcount = 0;
        const auto flowcountitem = flowgraphobj.FindMember(FI_APIFLOWCFGCOUNT);
        if (flowcountitem != flowgraphobj.MemberEnd()) {
            if (!flowcountitem->value.IsNull() && flowcountitem->value.IsInt()) {
                flowcount = flowcountitem->value.GetInt();
            }
        }
        if (!flowcount) return true;
        std::uint64_t strings_count = 0;
        const auto apiflowcfgarray = flowgraphobj.FindMember(FI_APIFLOWCFG);
        if (apiflowcfgarray != flowgraphobj.MemberEnd()) {
            if (!apiflowcfgarray->value.IsNull() && apiflowcfgarray->value.IsArray()) {
                std::string feature_strings;
                for (auto& item : apiflowcfgarray->value.GetArray()) {
                    if (!item.IsNull() && item.IsString()) {
                        std::string str = item.GetString();
                        if (CStrExtractor::detectedInsideCorpus(str, detected_l)) {
                            for (auto& detect : detected_l) {
                                if (!feature_strings.length())
                                    feature_strings = detect;
                                else
                                    feature_strings += " " + detect;
                                strings_count++;
                            }
                        }
                    }
                    if (strings_count > feature_nums)
                        break;
                }
                subsg->addElement(FI_APIFLOWCFG, feature_strings);
            }
        }

        return sgobject->addElement(FI_CFGFLOWGRAPH, std::move(subsg));
    }

    bool CTextCorpusManager::CommonParseIlcode(ConstRapidObject& ilcodeobj, std::shared_ptr<CSemanticGeneration> sgobject, std::uint32_t feature_nums) const {
        if (!sgobject) return false;
        std::vector <std::string> detected_l;
        auto subsg = sgobject->makeSubSg();
        if (!subsg) return false;
        unsigned int ilcodecount = 0;
        const auto ilcodecountitem = ilcodeobj.FindMember(FI_ILCODECOUNT);
        if (ilcodecountitem != ilcodeobj.MemberEnd()) {
            if (!ilcodecountitem->value.IsNull() && ilcodecountitem->value.IsInt()) {
                ilcodecount = ilcodecountitem->value.GetInt();
            }
        }
        if (!ilcodecount) return true;
        std::uint64_t strings_count = 0;
        const auto ilcodearray = ilcodeobj.FindMember(FI_ILCODE);
        if (ilcodearray != ilcodeobj.MemberEnd()) {
            if (!ilcodearray->value.IsNull() && ilcodearray->value.IsArray()) {
                std::string feature_strings;
                for (auto& item : ilcodearray->value.GetArray()) {
                    if (!item.IsNull() && item.IsString()) {
                        std::string str = item.GetString();
                        if (CStrExtractor::detectedInsideCorpus(str, detected_l)) {
                            for (auto& detect : detected_l) {
                                if (!feature_strings.length())
                                    feature_strings = detect;
                                else
                                    feature_strings += " " + detect;
                                strings_count++;
                            }
                        }
                    }
                    if (strings_count > feature_nums)
                        break;
                }
                subsg->addElement(FI_ILCODE, feature_strings);
            }
        }

        return sgobject->addElement(FI_CFGFLOWGRAPH, std::move(subsg));
    }

    bool CTextCorpusManager::CFastTextSG::addElement(const std::string& key, const std::string& value) {
        if (!key.length()) {
            return false;
        }
        if (value.length()) {
            m_dataset << getPrefix() << key << " " << SPECIAL_CORPUS_EQU << " " << value;
        }
        else {
            m_dataset << getPrefix() << key << " " << SPECIAL_CORPUS_EQU << " " << SPECIAL_CORPUS_UNK;
        }
        m_append = true;
        return true;
    }

    bool CTextCorpusManager::CFastTextSG::addElement(const std::string& key, bool value) {
        if (!key.length()) {
            return false;
        }
        if (value) {
            m_dataset << getPrefix() << key << " " << SPECIAL_CORPUS_EQU << " " << SI_TRUE;
        }
        else {
            m_dataset << getPrefix() << key << " " << SPECIAL_CORPUS_EQU << " " << SI_FALSE;
        }
        m_append = true;
        return true;
    }

    bool CTextCorpusManager::CFastTextSG::addElement(const std::string& key, std::uint32_t value) {
        if (!key.length()) {
            return false;
        }
        if (value) {
            m_dataset << getPrefix() << key << " " << SPECIAL_CORPUS_EQU << " " << SPECIAL_CORPUS_NUM;
        }
        else {
            m_dataset << getPrefix() << key << " " << SPECIAL_CORPUS_EQU << " " << SPECIAL_CORPUS_ZER;
        }
        m_append = true;
        return true;
    }

    bool CTextCorpusManager::CFastTextSG::addElement(const std::string& key, std::unique_ptr<CSemanticGeneration> value) {
        if (!value) {
            return false;
        }
        std::string dump = value->fulldump();
        m_dataset << getPrefix() << key << " " << SPECIAL_CORPUS_EQU << " " << dump;
        m_append = false;
        return true;
    }

    bool CTextCorpusManager::CFastTextSG::addElement(const std::string& key, const std::vector<std::pair<std::string, std::any>>& value_vec) {
        if (!key.length()) {
            return false;
        }
        if (!value_vec.size()) {
            return false;
        }
        m_dataset << getPrefix() << key << " " << SPECIAL_CORPUS_EQU;
        m_append = false;
        for (size_t count = 0; count < value_vec.size(); count++) {
            auto value = value_vec[count];
            if (!value.first.length())
                continue;
            if (!value.second.has_value()) {
                addElement(value.first, std::string(""));
            }
            else {
                if (value.second.type() == typeid(bool)) {
                    bool bvalue = std::any_cast<bool>(value.second);
                    addElement(value.first, bvalue);
                }
                else if (value.second.type() == typeid(std::uint32_t)) {
                    std::uint32_t uvalue = std::any_cast<std::uint32_t>(value.second);
                    addElement(value.first, uvalue);
                }
                else if (value.second.type() == typeid(std::string)) {
                    std::string svalue = std::any_cast<std::string>(value.second);
                    addElement(value.first, svalue);
                }
            }
        }
        if (!m_append) {
            m_dataset << " " << SPECIAL_CORPUS_UNK;
        }
        m_append = true;
        return true;
    }

    std::unique_ptr<CSemanticGeneration> CTextCorpusManager::CFastTextSG::makeSubSg() const {
        std::unique_ptr<CFastTextSG> fastsg = std::make_unique<CFastTextSG>();
        return fastsg;
    }

    std::string CTextCorpusManager::CFastTextSG::fulldump() const {
        if (m_dataset.str().length() > sizeof(SPECIAL_CORPUS_CLS)) {
            return std::string(m_dataset.str() + " " + SPECIAL_CORPUS_SEP);
        }
        else {
            return std::string(m_dataset.str() + " " + SPECIAL_CORPUS_EQU + " " + SPECIAL_CORPUS_UNK + " " + SPECIAL_CORPUS_SEP);
        }
    }

    std::string CTextCorpusManager::CFastTextSG::justdump() const {
        return {};
    }

    bool CTextCorpusManager::CCorpusExtractSG::AddCorpus(const std::string& word, mongocxx::collection& dict, bool blower) const {
        if (!m_valid)
            return false;
        try {
            if (!word.length()) return false;
            //if (word.compare("9-9c21586") == 0) {
            //    return false;
            //}
            bsoncxx::builder::stream::document doc{};
            if (blower) {
                doc << "Vocab" << StringToLower(word).c_str();
            }
            else {
                doc << "Vocab" << word.c_str();
            }

            auto result = dict.find_one(doc.view());
            if (!result.has_value()) {
                dict.insert_one(doc.view());
            }
        }
        catch (const std::exception& e) {
            std::cout << "Exception occurred during add corpus: " << e.what() << std::endl;
            return false;
        }
        return true;
    }

    void CTextCorpusManager::CCorpusExtractSG::ConstCorpusExtract(mongocxx::collection& dict) const {
        std::cout << "ConstCorpusExtract start... " << std::endl;
        SpecialCorpusExtract(dict);
        AddCorpus(BI_BASEINFO, dict, false);
        AddCorpus(BI_FILENAME, dict, false);
        AddCorpus(BI_FILEPATH, dict, false);
        AddCorpus(BI_FILETYPE, dict, false);
        AddCorpus(BI_EXE, dict, false);
        AddCorpus(BI_FILESIZE, dict, false);
        AddCorpus(BI_FILEATTR, dict, false);
        AddCorpus(BI_ARCHIVE, dict, false);
        AddCorpus(BI_COMPRESSED, dict, false);
        AddCorpus(BI_ENCRYPTED, dict, false);
        AddCorpus(BI_HIDDEN, dict, false);
        AddCorpus(BI_NORMAL, dict, false);
        AddCorpus(BI_READONLY, dict, false);
        AddCorpus(BI_SYSTEM, dict, false);
        AddCorpus(BI_TEMPORARY, dict, false);
        AddCorpus(VI_VERINFO, dict, false);
        AddCorpus(VI_DESCRIPTION, dict, false);
        AddCorpus(VI_FILEVER, dict, false);
        AddCorpus(VI_PRODUCTNAME, dict, false);
        AddCorpus(VI_PRODUCTVER, dict, false);
        AddCorpus(VI_COPYRIGHT, dict, false);
        AddCorpus(VI_VFT_APP, dict, false);
        AddCorpus(VI_VFT_DLL, dict, false);
        AddCorpus(VI_VFT_DRV, dict, false);
        AddCorpus(VI_VFT_FONT, dict, false);
        AddCorpus(VI_VFT_STATIC_LIB, dict, false);
        AddCorpus(VI_VFT_VXD, dict, false);
        AddCorpus(II_INSTALLINFO, dict, false);
        AddCorpus(II_SOFTNAME, dict, false);
        AddCorpus(II_SOFTVER, dict, false);
        AddCorpus(II_INSTALLLOC, dict, false);
        AddCorpus(II_PUBLISHER, dict, false);
        AddCorpus(II_PRODUCTID, dict, false);
        AddCorpus(SI_SIGNINFO, dict, false);
        AddCorpus(SI_TRUE, dict, false);
        AddCorpus(SI_FALSE, dict, false);
        AddCorpus(SI_BSIGNED, dict, false);
        AddCorpus(SI_BVERIFIED, dict, false);
        AddCorpus(SI_BCOUNTERSIGN, dict, false);
        AddCorpus(SI_BMUTILCERTS, dict, false);
        AddCorpus(SI_PRIMARYCERT, dict, false);
        AddCorpus(SI_ISSUER, dict, false);
        AddCorpus(SI_SUBJECT, dict, false);
        AddCorpus(SI_THUMBPRINT, dict, false);
        AddCorpus(NI_INSIDEINFO, dict, false);
        AddCorpus(PI_PEINFO, dict, false);
        AddCorpus(PI_EXETYPE, dict, false);
        AddCorpus(PI_IMPORTS, dict, false);
        AddCorpus(PI_EXPORTS, dict, false);
        AddCorpus(PI_RESOURCES, dict, false);
        AddCorpus(PI_SECURITY, dict, false);
        AddCorpus(PI_RELOC, dict, false);
        AddCorpus(PI_TLS, dict, false);
        AddCorpus(PI_BOUND_IMPORT, dict, false);
        AddCorpus(PI_DELAY_IMPORT, dict, false);
        AddCorpus(PI_DOTNET, dict, false);
        AddCorpus(PI_DEBUG, dict, false);
        AddCorpus(PI_SUBSYSTEM, dict, false);
        AddCorpus(PI_MINOR_OS_VERSION, dict, false);
        AddCorpus(PI_MAJOR_OS_VERSION, dict, false);
        AddCorpus(PI_MINOR_SUBSYSTEM_VERSION, dict, false);
        AddCorpus(PI_MAJOR_SUBSYSTEM_VERSION, dict, false);
        AddCorpus(PI_PEHEADER_START, dict, false);
        AddCorpus(PI_FILE_ALIGNMENT, dict, false);
        AddCorpus(PI_SIZE_OF_IMAGE, dict, false);
        AddCorpus(PI_ENTRYPOINT, dict, false);
        AddCorpus(PI_RVAS_AND_SIZES, dict, false);
        AddCorpus(PI_CHARACTERISTICS, dict, false);
        AddCorpus(PI_DLL_CHARACTERISTICS, dict, false);
        AddCorpus(PI_SIZE_OF_HEADERS, dict, false);
        AddCorpus(PI_SIZE_OF_OPTIONAL_HEADER, dict, false);
        AddCorpus(PI_PE_SIGNATURE, dict, false);
        AddCorpus(PI_MAGIC, dict, false);
        AddCorpus(PI_IMAGE_BASE, dict, false);
        AddCorpus(PI_HEAP_SIZE_COMMIT, dict, false);
        AddCorpus(PI_HEAP_SIZE_RESERVE, dict, false);
        AddCorpus(PI_STACK_SIZE_COMMIT, dict, false);
        AddCorpus(PI_STACK_SIZE_RESERVE, dict, false);
        AddCorpus(PI_CHECKSUM, dict, false);
        AddCorpus(PI_VERIFY_CHECKSUM, dict, false);
        AddCorpus(PI_TIME_DATE_STAMP, dict, false);
        AddCorpus(PI_MACHINE, dict, false);
        AddCorpus(PI_SIZEOF_NT_HEADER, dict, false);
        AddCorpus(PI_BASE_OF_CODE, dict, false);
        AddCorpus(PI_NUMBER_OF_SECTIONS, dict, false);
        AddCorpus(PI_SECTION_ALIGNMENT, dict, false);
        AddCorpus(PI_HAS_OVERLAY, dict, false);
        AddCorpus(PI_ENTROPY, dict, false);
        AddCorpus(PI_PACK, dict, false);
        AddCorpus(PI_MAYPACK, dict, false);
        AddCorpus(PI_UNPACK, dict, false);
        AddCorpus(TI_TOOLINFO, dict, false);
        AddCorpus(TI_COMPILER, dict, false);
        AddCorpus(TI_LINKER, dict, false);
        AddCorpus(TI_PACKER, dict, false);
        AddCorpus(TI_INSTALLER, dict, false);
        AddCorpus(DD_DATADIRECTORY, dict, false);
        AddCorpus(DD_EXPORT_DIRECTORY, dict, false);
        AddCorpus(DD_IMPORT_DIRECTORY, dict, false);
        AddCorpus(DD_RESOURCE_DIRECTORY, dict, false);
        AddCorpus(DD_EXCEPTION_DIRECTORY, dict, false);
        AddCorpus(DD_SECURITY_DIRECTORY, dict, false);
        AddCorpus(DD_BASERELOC_DIRECTORY, dict, false);
        AddCorpus(DD_DEBUG_DIRECTORY, dict, false);
        AddCorpus(DD_ARCHITECTURE_DIRECTORY, dict, false);
        AddCorpus(DD_GLOBALPTR_DIRECTORY, dict, false);
        AddCorpus(DD_TLS_DIRECTORY, dict, false);
        AddCorpus(DD_LOADCONFIG_DIRECTORY, dict, false);
        AddCorpus(DD_BOUNDIMPORT_DIRECTORY, dict, false);
        AddCorpus(DD_IAT_DIRECTORY, dict, false);
        AddCorpus(DD_DELAYIMPORT_DIRECTORY, dict, false);
        AddCorpus(DD_COMDES_DIRECTORY, dict, false);
        AddCorpus(DD_RELADDR, dict, false);
        AddCorpus(DD_SIZE, dict, false);
        AddCorpus(PS_PDBINFOS, dict, false);
        AddCorpus(PS_GUID, dict, false);
        AddCorpus(PS_TYPE, dict, false);
        AddCorpus(PS_TIMESTAMP, dict, false);
        AddCorpus(PS_AGE, dict, false);
        AddCorpus(PS_PATH, dict, false);
        AddCorpus(ED_EMBEDDEDPE, dict, false);
        AddCorpus(IT_IMPORTS, dict, false);
        AddCorpus(IT_NAME, dict, false);
        AddCorpus(IT_FUNCTIONS, dict, false);
        AddCorpus(ET_EXPORTS, dict, false);
        AddCorpus(ET_CHARACTERISTICS, dict, false);
        AddCorpus(ET_TIME_DATE_STAMP, dict, false);
        AddCorpus(ET_MAJOR_VERSION, dict, false);
        AddCorpus(ET_MINOR_VERSION, dict, false);
        AddCorpus(ET_DLL_NAME, dict, false);
        AddCorpus(ET_BASE, dict, false);
        AddCorpus(ET_NUMBER_OF_FUNCS, dict, false);
        AddCorpus(ET_NUMBER_OF_NAMES, dict, false);
        AddCorpus(ET_ADDR_OF_FUNCS, dict, false);
        AddCorpus(ET_ADDR_OF_NAMES, dict, false);
        AddCorpus(ET_ADDR_OF_NAMES_ORD, dict, false);
        AddCorpus(ET_FUNCTIONS, dict, false);
        AddCorpus(MN_MAINICON, dict, false);
        AddCorpus(MF_MANIFEST, dict, false);
        AddCorpus(MF_DEPENDENT_ASSEMBLY, dict, false);
        AddCorpus(MF_NAME, dict, false);
        AddCorpus(MF_VERSION, dict, false);
        AddCorpus(MF_TYPE, dict, false);
        AddCorpus(MF_REQUESTED_EXECUTION_LEVEL, dict, false);
        AddCorpus(MF_LEVEL, dict, false);
        AddCorpus(MF_UIACCESS, dict, false);
        AddCorpus(RH_RICHHEADER, dict, false);
        AddCorpus(RH_PRODUCTID, dict, false);
        AddCorpus(RH_PRODUCTBUILD, dict, false);
        AddCorpus(RH_NUMBEROFUSES, dict, false);
        AddCorpus(RH_PRODUCTNAME, dict, false);
        AddCorpus(RH_VSNAME, dict, false);
        AddCorpus(SS_SECTIONS, dict, false);
        AddCorpus(SS_SECNAME, dict, false);
        AddCorpus(SS_INDEX, dict, false);
        AddCorpus(SS_TYPE, dict, false);
        AddCorpus(SS_CHARACTERISTICS, dict, false);
        AddCorpus(SS_RAW_DATA_POINT, dict, false);
        AddCorpus(SS_RAW_DATA_SIZE, dict, false);
        AddCorpus(SS_VIRTUAL_SIZE, dict, false);
        AddCorpus(SS_HASDATA, dict, false);
        AddCorpus(SS_HASCODE, dict, false);
        AddCorpus(SS_DATAONLY, dict, false);
        AddCorpus(SS_READONLY, dict, false);
        AddCorpus(SS_BBS, dict, false);
        AddCorpus(SS_DEBUG, dict, false);
        AddCorpus(SS_INFO, dict, false);
        AddCorpus(SS_ENTROPY, dict, false);
        AddCorpus(SF_STRINGS_FEATURE, dict, false);
        AddCorpus(SF_SECNAME, dict, false);
        AddCorpus(SF_INSIDESTR, dict, false);
        return;
    }

    void CTextCorpusManager::CCorpusExtractSG::SpecialCorpusExtract(mongocxx::collection& dict) const {
        for (auto& vocab : _special_vocab) {
            AddCorpus(vocab, dict, false);
        }
        return;
    }

    bool CTextCorpusManager::CCorpusExtractSG::initDict(const mongocxx::database& db) {
        if (m_valid) return false;
        if (!db) return false;
        m_textcorpus_dict = db[_mongodb_textcorpus_dict];
        m_valid = true;
        ConstCorpusExtract(m_textcorpus_dict);
        return m_valid;
    }

    bool CTextCorpusManager::CCorpusExtractSG::initDict(const mongocxx::collection& dict) {
        if (m_valid) return false;
        if (!dict) return false;
        m_textcorpus_dict = dict;
        m_valid = true;
        return m_valid;
    }

    bool CTextCorpusManager::CCorpusExtractSG::addElement(const std::string& key, const std::string& value) {
        if (!m_valid) {
            return false;
        }
        if (key.length()) {
            AddCorpus(key, m_textcorpus_dict);
        }
        if (value.length()) {
            std::string substring;
            std::istringstream iss(value);
            while (std::getline(iss, substring, ' ')) {
                AddCorpus(substring, m_textcorpus_dict);
            }
        }
        return true;
    }

    bool CTextCorpusManager::CCorpusExtractSG::addElement(const std::string& key, bool value) {
        if (!m_valid) {
            return false;
        }
        return true;
    }

    bool CTextCorpusManager::CCorpusExtractSG::addElement(const std::string& key, std::uint32_t value) {
        if (!m_valid) {
            return false;
        }
        return true;
    }

    bool CTextCorpusManager::CCorpusExtractSG::addElement(const std::string& key, std::unique_ptr<CSemanticGeneration> value) {
        if (!m_valid) {
            return false;
        }
        return true;
    }

    bool CTextCorpusManager::CCorpusExtractSG::addElement(const std::string& key, const std::vector<std::pair<std::string, std::any>>& value_vec) {
        if (!m_valid) {
            return false;
        }
        return true;
    }

    std::unique_ptr<CSemanticGeneration> CTextCorpusManager::CCorpusExtractSG::makeSubSg() const {
        if (!m_valid) {
            return nullptr;
        }
        std::unique_ptr<CCorpusExtractSG> cesg = std::make_unique<CCorpusExtractSG>();
        cesg->initDict(m_textcorpus_dict);
        return cesg;
    }

    std::string CTextCorpusManager::CCorpusExtractSG::fulldump() const {
        return {};
    }

    std::string CTextCorpusManager::CCorpusExtractSG::justdump() const {
        return {};
    }

    bool CTextCorpusManager::CTransformersSG::addElement(const std::string& key, const std::string& value) {
        auto count_words_in_line = [&](const std::string& input) -> std::uint32_t {
            if (input.empty()) {
                return 0;
            }
            std::uint32_t word_nums = 0;
            std::istringstream iss(input);
            std::string word;
            while (iss >> word) {
                word_nums++;
            }
            return word_nums - 1;
        };
        if (value.length()) {
            m_dataset << " " << value;
            m_elementcount += count_words_in_line(value);
        }
        return true;
    }

    bool CTextCorpusManager::CTransformersSG::addElement(const std::string& key, bool value) {
        if (value) {
            m_dataset << " " << SI_TRUE;
        }
        else {
            m_dataset << " " << SI_FALSE;
        }
        m_elementcount++;
        return true;
    }

    bool CTextCorpusManager::CTransformersSG::addElement(const std::string& key, std::uint32_t value) {
        if (value) {
            m_dataset << " " << SPECIAL_CORPUS_NUM;
        }
        else {
            m_dataset << " " << SPECIAL_CORPUS_ZER;
        }
        m_elementcount++;
        return true;
    }

    bool CTextCorpusManager::CTransformersSG::addElement(const std::string& key, std::unique_ptr<CSemanticGeneration> value) {
        if (!value) {
            return false;
        }
        std::string dump = value->fulldump();
        if (m_dataset.str().empty()) {
            m_dataset << dump;
            m_elementcount += value->getelemcount();
        }
        else {
            m_dataset << " " << SPECIAL_CORPUS_SEP << dump;
            m_elementcount += value->getelemcount() + 1;
        }

        return true;
    }

    bool CTextCorpusManager::CTransformersSG::addElement(const std::string& key, const std::vector<std::pair<std::string, std::any>>& value_vec) {
        if (!value_vec.size()) {
            return false;
        }
        for (size_t count = 0; count < value_vec.size(); count++) {
            auto value = value_vec[count];
            if (!value.first.length())
                continue;
            if (value.second.has_value()) {
                if (value.second.type() == typeid(bool)) {
                    bool bvalue = std::any_cast<bool>(value.second);
                    addElement(value.first, bvalue);
                }
                else if (value.second.type() == typeid(std::uint32_t)) {
                    std::uint32_t uvalue = std::any_cast<std::uint32_t>(value.second);
                    addElement(value.first, uvalue);
                }
                else if (value.second.type() == typeid(std::string)) {
                    std::string svalue = std::any_cast<std::string>(value.second);
                    addElement(value.first, svalue);
                }
            }
        }
        return true;
    }

    std::unique_ptr<CSemanticGeneration> CTextCorpusManager::CTransformersSG::makeSubSg() const {
        std::unique_ptr<CTransformersSG> tfssg = std::make_unique<CTransformersSG>();
        return tfssg;
    }

    std::string CTextCorpusManager::CTransformersSG::fulldump() const {
        return std::string(m_dataset.str());
    }

    std::string CTextCorpusManager::CTransformersSG::justdump() const {
        return {};
    }

} // namespace cchips
