#include "StatisticianManager.h"
#include "JsonDefine.h"

namespace cchips {

    bool CStatisticianManager::GeneratingDupappsDatasets(std::unique_ptr<CRapidJsonWrapper> wrapper, std::vector<std::string>& datasets) {
        if (!wrapper) return false;
        if (auto anyvalue(wrapper->GetMember(std::vector<std::string>{DUPCOUNTS}));
            !anyvalue.has_value() || anyvalue.type() != typeid(int)) {
            return false;
        }

        if (auto anyvalue(wrapper->GetMember(DUPLISTS));
            anyvalue.has_value() && anyvalue.type() == typeid(ConstRapidObject))
        {
            auto duplistobj = std::any_cast<ConstRapidObject>(anyvalue);
            for (auto& it : duplistobj) {
                if (!it.value.IsObject()) continue;
                if (!it.name.IsString()) continue;
                std::string appname = it.name.GetString();
                for (auto& member : it.value.GetObjectA()) {
                    if (_stricmp(member.name.GetString(), DUPCOUNTS) == 0) {
                        if (!member.value.IsInt()) continue;
                        int counts = member.value.GetInt();
                    }
                    else if (_stricmp(member.name.GetString(), DUPPATHS) == 0) {
                        if (!member.value.IsArray()) continue;
                        fs::path first_label;
                        for (auto& item : member.value.GetArray()) {
                            if (!item.IsNull() && item.IsString()) {
                                if (first_label.empty()) {
                                    first_label = RemovePrefix(item.GetString());
                                    if (!first_label.empty()) {
                                        auto it = first_label.begin();
                                        if (it != first_label.end()) {
                                            first_label = *it;
                                        }
                                    }
                                }
                                else {
                                    fs::path label = RemovePrefix(item.GetString());
                                    if (!label.empty()) {
                                        auto it = label.begin();
                                        if (it != label.end()) {
                                            label = *it;
                                        }
                                    }
                                    if (first_label != label) {
                                        datasets.push_back(appname);
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        return true;
    }

} // namespace cchips
