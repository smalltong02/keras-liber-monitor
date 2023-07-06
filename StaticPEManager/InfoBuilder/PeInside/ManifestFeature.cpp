#include "ManifestFeature.h"
#include "tinyxml2.h"

namespace cchips {
    bool CManifestFeatureBuilder::Scan(std::unique_ptr<cchips::PeFormat>& pe_format, std::unique_ptr<cchips::CRapidJsonWrapper>& json_result)
    {
        if (!pe_format || !json_result)
            return false;
        try {
            const auto manifestres = pe_format->getManifestResource();
            if (!manifestres)
                return true;
            std::string_view bytes = manifestres->getBytes(0, manifestres->getSizeInFile());
            if (!bytes.size())
                return true;
            auto& allocator = json_result->GetAllocator();
            std::unique_ptr<cchips::RapidValue> vmanifest = std::make_unique<cchips::RapidValue>();
            if (!vmanifest) return false;
            vmanifest->SetObject();
            tinyxml2::XMLDocument xmldoc;
            if (xmldoc.Parse(bytes.data(), bytes.size()) == tinyxml2::XML_SUCCESS) {
                tinyxml2::XMLElement* assemblyElement = xmldoc.FirstChildElement("assembly");
                if (assemblyElement) {
                    tinyxml2::XMLElement* identityElement = assemblyElement->FirstChildElement("assemblyIdentity");
                    if (identityElement) {
                        cchips::RapidValue velement;
                        velement.SetObject();
                        const char* name = identityElement->Attribute("name");
                        const char* version = identityElement->Attribute("version");
                        const char* type = identityElement->Attribute("type");
                        if (name) {
                            velement.AddMember("name", RapidValue(name, allocator), allocator);
                        }
                        if (version) {
                            velement.AddMember("version", RapidValue(version, allocator), allocator);
                        }
                        if (type) {
                            velement.AddMember("type", RapidValue(type, allocator), allocator);
                        }
                        vmanifest->AddMember("assembly identity", velement, allocator);
                    }
                    tinyxml2::XMLElement* descriptionElement = assemblyElement->FirstChildElement("description");
                    if (descriptionElement) {
                        const char* description = descriptionElement->GetText();
                        if (description) {
                            vmanifest->AddMember("description", RapidValue(description, allocator), allocator);
                        }
                    }
                    tinyxml2::XMLElement* dependencyElement = assemblyElement->FirstChildElement("dependency");
                    if (dependencyElement) {
                        tinyxml2::XMLElement* dependentAssemblyElement = dependencyElement->FirstChildElement("dependentAssembly");
                        if (dependentAssemblyElement) {
                            tinyxml2::XMLElement* assemblyIdentityElement = dependentAssemblyElement->FirstChildElement("assemblyIdentity");
                            if (assemblyIdentityElement) {
                                cchips::RapidValue velement;
                                velement.SetObject();
                                const char* name = assemblyIdentityElement->Attribute("name");
                                const char* version = assemblyIdentityElement->Attribute("version");
                                const char* type = assemblyIdentityElement->Attribute("type");
                                if (name) {
                                    velement.AddMember("name", RapidValue(name, allocator), allocator);
                                }
                                if (version) {
                                    velement.AddMember("version", RapidValue(version, allocator), allocator);
                                }
                                if (type) {
                                    velement.AddMember("type", RapidValue(type, allocator), allocator);
                                }
                                vmanifest->AddMember("dependent assembly", velement, allocator);
                            }
                        }
                    }
                    tinyxml2::XMLElement* trustInfoElement = assemblyElement->FirstChildElement("trustInfo");
                    if (trustInfoElement) {
                        tinyxml2::XMLElement* securityElement = trustInfoElement->FirstChildElement("security");
                        if (securityElement) {
                            tinyxml2::XMLElement* requestedExecutionLevelElement = securityElement->FirstChildElement("requestedPrivileges")->FirstChildElement("requestedExecutionLevel");
                            if (requestedExecutionLevelElement) {
                                cchips::RapidValue velement;
                                velement.SetObject();
                                const char* level = requestedExecutionLevelElement->Attribute("level");
                                const char* uiAccess = requestedExecutionLevelElement->Attribute("uiAccess");
                                if (level) {
                                    velement.AddMember("level", RapidValue(level, allocator), allocator);
                                }
                                if (uiAccess) {
                                    velement.AddMember("uiaccess", RapidValue(uiAccess, allocator), allocator);
                                }
                                vmanifest->AddMember("requested execution level", velement, allocator);
                            }
                        }
                    }
                }
            }
            return json_result->AddTopMember("manifest", std::move(vmanifest));
        }
        catch (const std::exception& e)
        {
        }
        return false;
    }
} // namespace cchips
