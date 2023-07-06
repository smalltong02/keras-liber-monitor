#include "ImporttableFeature.h"
#include "..\utils.h"

namespace cchips {
    bool CImporttableFeatureBuilder::Initialize()
    {
        return true;
    }

    bool CImporttableFeatureBuilder::Scan(std::unique_ptr<cchips::PeFormat>& pe_format, std::unique_ptr<cchips::CRapidJsonWrapper>& json_result)
    {
        if (!pe_format || !json_result)
            return false;

        try {
            if (pe_format->hasImportsTable()) {
                auto& allocator = json_result->GetAllocator();
                auto& import_tables = pe_format->getImportTable();
                if (!import_tables)
                    return false;

                const std::string delimiter = ".";
                std::unique_ptr<cchips::RapidValue> vimports = std::make_unique<cchips::RapidValue>();
                if (!vimports) return false;
                vimports->SetObject();

                for (int library_index = 0; library_index < import_tables->getNumberOfLibraries(); library_index++) {

                    cchips::RapidValue vdllinfo;
                    vdllinfo.SetObject();
                    std::string dll_name = import_tables->getLibrary(library_index);
                    dll_name = StringToLower(dll_name);
                    std::string dll_basename = dll_name;
                    int separator_index = dll_name.find(delimiter);
                    if (separator_index != dll_name.size() - 1)
                    {
                        dll_basename = dll_basename.substr(0, separator_index);
                    }
                    vdllinfo.AddMember("name", cchips::RapidValue(dll_name.c_str(), dll_name.length(), allocator), allocator);
                    cchips::RapidValue vfunctions;
                    vfunctions.SetArray();
                    for (auto& entry : *import_tables) {

                        if (entry->getLibraryIndex() == library_index) {
                            if (entry->getName().length()) {
                                vfunctions.PushBack(cchips::RapidValue(entry->getName().c_str(), allocator), allocator);
                            }
                            else {
                                unsigned long long ordinal = 0;
                                entry->getOrdinalNumber(ordinal);
                                std::string ordinal_str = std::to_string(ordinal);
                                vfunctions.PushBack(cchips::RapidValue(ordinal_str.c_str(), allocator), allocator);
                            }
                        }
                    }
                    vdllinfo.AddMember("functions", vfunctions, allocator);
                    vimports->AddMember(cchips::RapidValue(dll_basename.c_str(), dll_basename.length(), allocator), vdllinfo, allocator);
                }
                return json_result->AddTopMember("imports", std::move(vimports));
            }
        }
        catch (const std::exception& e)
        {
        }
        return false;
    }
} // namespace cchips
