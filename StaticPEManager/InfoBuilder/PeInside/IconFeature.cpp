#include "IconFeature.h"
namespace cchips {

    bool CIconFeatureBuilder::Scan(std::unique_ptr<cchips::PeFormat>& pe_format, std::unique_ptr<cchips::CRapidJsonWrapper>& json_result)
    {
        if (!pe_format || !json_result)
            return false;
        try {
            if (pe_format->hasResources()) {
                if (pe_format->getResourceTable()) {
                    auto& allocator = json_result->GetAllocator();
                    const auto& group = pe_format->getResourceTable()->getPriorResourceIconGroup();
                    if (group) {
                        const auto& icon = group->getPriorIcon();
                        if (icon) {
                            std::unique_ptr<cchips::RapidValue> vicon = std::make_unique<cchips::RapidValue>();
                            if (!vicon) return false;
                            vicon->SetObject();
                            auto data = icon->getBytes(0, icon->getIconSize());
                            if (icon->hasValidName()) {
                                vicon->AddMember("name", cchips::RapidValue(icon->getName().c_str(), allocator), allocator);
                            }
                            else if (group->hasValidName()) {
                                vicon->AddMember("name", cchips::RapidValue(group->getName().c_str(), allocator), allocator);
                            }
                            if (icon->hasValidId()) {
                                size_t id = 0;
                                icon->getNameId(id);
                                vicon->AddMember("id", cchips::RapidValue(id), allocator);
                            }
                            if (data.length()) {
                                vicon->AddMember("md5", cchips::RapidValue(GetMd5((unsigned char*)&data[0], data.length()).c_str(), allocator), allocator);
                            }
                            vicon->AddMember("height", cchips::RapidValue(icon->getHeight()), allocator);
                            vicon->AddMember("width", cchips::RapidValue(icon->getWidth()), allocator);
                            vicon->AddMember("planes", cchips::RapidValue(icon->getPlanes()), allocator);
                            vicon->AddMember("size", cchips::RapidValue(icon->getIconSize()), allocator);
                            vicon->AddMember("bitcount", cchips::RapidValue(icon->getBitCount()), allocator);
                            json_result->AddTopMember("mainicon", std::move(vicon));
                        }
                    }
                    else {
                        const auto& icon = pe_format->getResourceTable()->getPriorResourceIcon();
                        if (icon) {
                            std::unique_ptr<cchips::RapidValue> vicon = std::make_unique<cchips::RapidValue>();
                            if (!vicon) return false;
                            vicon->SetObject();
                            auto data = icon->getBytes(0, icon->getIconSize());
                            if (icon->hasValidName()) {
                                vicon->AddMember("name", cchips::RapidValue(icon->getName().c_str(), allocator), allocator);
                            }
                            if (icon->hasValidId()) {
                                size_t id = 0;
                                icon->getNameId(id);
                                vicon->AddMember("id", cchips::RapidValue(id), allocator);
                            }
                            if (data.length()) {
                                vicon->AddMember("md5", cchips::RapidValue(GetMd5((unsigned char*)&data[0], data.length()).c_str(), allocator), allocator);
                            }
                            vicon->AddMember("height", cchips::RapidValue(icon->getHeight()), allocator);
                            vicon->AddMember("width", cchips::RapidValue(icon->getWidth()), allocator);
                            vicon->AddMember("planes", cchips::RapidValue(icon->getPlanes()), allocator);
                            vicon->AddMember("size", cchips::RapidValue(icon->getIconSize()), allocator);
                            vicon->AddMember("bitcount", cchips::RapidValue(icon->getBitCount()), allocator);
                            json_result->AddTopMember("mainicon", std::move(vicon));
                        }
                    }
                }
            }
            return true;
        }
        catch (const std::exception& e)
        {
        }
        return false;
    }
} // namespace cchips
