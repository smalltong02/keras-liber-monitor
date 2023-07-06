#include "StringsFeature.h"

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
            const unsigned char* string_buf = pe_format->getLoadedBytesData();
            size_t buf_size = pe_format->getLoadedFileLength();
            if (!string_buf || !buf_size)
                return false;

            auto& allocator = json_result->GetAllocator();
            std::unique_ptr<cchips::RapidValue> inside_strings = std::make_unique<cchips::RapidValue>();
            if (!inside_strings) return false;
            inside_strings->SetArray();

            for (unsigned long i = 0; i < buf_size; i++)
            {
                byte = uint8_t(string_buf[i]);
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
                        std::string str((const char*)string_buf + ascii_start, ascii_count);

                        inside_strings->PushBack(cchips::RapidValue(str.c_str(), allocator), allocator);
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
                            inside_strings->PushBack(cchips::RapidValue(str.c_str(), allocator), allocator);
                            //log_output("unicode str: %s\n", str.c_str());
                        }
                        uni_count = 0;
                        temp_wstr.clear();
                    }
                }
            }
            return json_result->AddTopMember("inside_strings", std::move(inside_strings));
        }
        catch (const std::exception& e)
        {
        }
        return false;
    }
} // namespace cchips
