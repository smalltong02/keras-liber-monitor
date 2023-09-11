#include "StringsFeature.h"
#include "utf8.h"

namespace cchips {
    const size_t kShortestRun = 5;
    const size_t kMaxLen = 16384;
    const size_t kLongStrSize = 2048;
    const size_t kMaxStrCount = 15000;
    const size_t kMaxUrlCount = 1000;

    bool CStringsFeatureBuilder::Initialize()
    {
        return true;
    }

    bool CStringsFeatureBuilder::Scan(std::unique_ptr<cchips::PeFormat>& pe_format, std::unique_ptr<cchips::CRapidJsonWrapper>& json_result)
    {
        if (!pe_format || !json_result)
            return false;
    
        try {
            uint8_t byte = 0;
            uint16_t hw = 0;
            size_t ascii_count = 0;
            size_t uni_count = 0;
            unsigned long ascii_start = 0;
            unsigned long uni_start = 0;
            std::wstring temp_wstr;
            auto& allocator = json_result->GetAllocator();

            std::unique_ptr<cchips::RapidValue> vfeature = std::make_unique<cchips::RapidValue>();
            if (!vfeature) return false;
            vfeature->SetObject();
            auto& importtable = pe_format->getImportTable();
            auto& exporttable = pe_format->getExportTable();
            auto& res = pe_format->getResSection();
            const auto secCounter = pe_format->getDeclaredNumberOfSections();
            std::vector<std::shared_ptr<cchips::PeCoffSection>> toppriority;
            std::vector<std::shared_ptr<cchips::PeCoffSection>> secpriority;
            std::vector<std::shared_ptr<cchips::PeCoffSection>> lastpriority;
            for (std::size_t i = 0; i < secCounter; ++i)
            {
                const auto fsec = pe_format->getPeSection(i);
                if (!UnporcessSection(fsec, res))
                {
                    if (fsec->isDataOnly() && fsec->isReadOnly()) {
                        toppriority.push_back(fsec);
                        continue;
                    }
                    if (fsec->isCode() && fsec->isReadOnly()) {
                        secpriority.push_back(fsec);
                        continue;
                    }
                    lastpriority.push_back(fsec);
                }
            }
            std::vector<std::shared_ptr<cchips::PeCoffSection>> mergedvector;
            mergedvector.reserve(toppriority.size() + secpriority.size() + lastpriority.size());
            mergedvector.insert(mergedvector.end(), toppriority.begin(), toppriority.end());
            mergedvector.insert(mergedvector.end(), secpriority.begin(), secpriority.end());
            mergedvector.insert(mergedvector.end(), lastpriority.begin(), lastpriority.end());

            for (auto& fsec : mergedvector) {
                if (!UnporcessSection(fsec, res))
                {
                    cchips::RapidValue vsection;
                    vsection.SetObject();
                    vsection.AddMember("sec_no", cchips::RapidValue((std::uint32_t)fsec->getIndex()), allocator);
                    vsection.AddMember("sec_name", cchips::RapidValue(fsec->getName().c_str(), allocator), allocator);
                    vsection.AddMember("characteristics", cchips::RapidValue(fsec->getPeCoffFlags()), allocator);
                    unsigned long long sizeinmemory = 0;
                    fsec->getSizeInMemory(sizeinmemory);
                    std::string_view bytes = fsec->getBytes(0, sizeinmemory);
                    vsection.AddMember("entropy", RapidValue(getEntropy(bytes)), allocator);
                    char* address = (char*)fsec->getAddress();
                    auto addr_size = fsec->getSizeInFile();
                    if (address && addr_size) {

                        cchips::RapidValue inside_strings;
                        inside_strings.SetArray();

                        for (unsigned long i = 0; i < addr_size; i++)
                        {
                            byte = uint8_t(address[i]);
                            if (IsAscii(byte))
                            {
                                if (ascii_count == 0)
                                    ascii_start = i;
                                ascii_count += 1;
                            }
                            else
                            {
                                if (ascii_count >= kShortestRun)
                                {
                                    std::string str((const char*)address + ascii_start, ascii_count);
                                    if (contain_invalid_utf8char(str)) {
                                        str = stringto_hexstring(str);
                                    }
                                    do {
                                        if (importtable && importtable->hasImport(str))
                                            break;
                                        if (exporttable && exporttable->hasExport(str))
                                            break;
                                        inside_strings.PushBack(cchips::RapidValue(str.c_str(), allocator), allocator);
                                    } while (0);
                                    //log_output("ascii str: %s\n", str.c_str());
                                }
                                ascii_count = 0;
                            }

                            // --------------------- check unicode -------------------
                            hw = (hw >> 8) | (byte << 8);
                            if (i < 1)
                                continue;
                            if (IsUnicode(hw) && uni_count == 0)
                            {
                                uni_start = i - 1;
                                uni_count++;
                                temp_wstr.push_back((wchar_t)hw);
                            }
                            // read and examine two bytes a time after first unicode hw
                            else if (uni_count > 0 && (i - uni_start) % 2 == 1)
                            {
                                if (IsUnicode(hw))
                                {
                                    uni_count++;
                                    if (uni_count <= kMaxLen)
                                        temp_wstr.push_back(hw);
                                }
                                else
                                {
                                    if (uni_count >= kShortestRun)
                                    {
                                        std::string str = to_byte_string(temp_wstr);
                                        if (contain_invalid_utf8char(str)) {
                                            str = stringto_hexstring(str);
                                        }
                                        do {
                                            if (importtable && importtable->hasImport(str))
                                                break;
                                            if (exporttable && exporttable->hasExport(str))
                                                break;
                                            inside_strings.PushBack(cchips::RapidValue(str.c_str(), allocator), allocator);
                                        } while (0);
                                        //log_output("unicode str: %s\n", str.c_str());
                                    }
                                    uni_count = 0;
                                    temp_wstr.clear();
                                }
                            }
                        }
                        vsection.AddMember("insidestr", inside_strings, allocator);
                    }
                    vfeature->AddMember(cchips::RapidValue(fsec->getName().c_str(), allocator), vsection, allocator);
                }
                byte = 0;
                hw = 0;
                ascii_count = 0;
                uni_count = 0;
                ascii_start = 0;
                uni_start = 0;
                temp_wstr.clear();
            }
            json_result->AddTopMember("strings_feature", std::move(vfeature));
        }
        catch (const std::exception& e)
        {
        }
        return false;
    }

    bool CStringsFeatureBuilder::UnporcessSection(const std::shared_ptr<PeCoffSection> sec, const std::shared_ptr<PeCoffSection> res) const {
        if (!sec)
            return true;
        if (res) {
            if (sec->getAddress() == res->getAddress()) {
                return true;
            }
        }
        if (sec->isBss() ||
            sec->isInfo() ||
            sec->isUndefined())
            return true;
        return false;
    }
} // namespace cchips
