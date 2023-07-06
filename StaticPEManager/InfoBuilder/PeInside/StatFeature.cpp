#include "StatFeature.h"

namespace cchips {
    const uint32_t CStatFeatureBuilder::_hexstring_min_length = 16;

    inline bool IsUnicode(uint16_t hw) { return ((hw >= 0x0020 && hw <= 0x007E)); }
    inline bool IsUnicodeHex(uint16_t hw) { return ((hw >= 0x0030 && hw <= 0x0039) || (hw >= 0x0061 && hw <= 0x0066) || (hw >= 0x0041 && hw <= 0x0046)); }

    bool CStatFeatureBuilder::Scan(std::unique_ptr<cchips::PeFormat>&pe_format, std::unique_ptr<cchips::CRapidJsonWrapper>&json_result)
    {
        if (!pe_format || !json_result)
            return false;
        //try {
        //    // hex numbers in PE
        //    uint32_t longest_hex_length = 0;
        //    uint32_t no_hex_strings = 0;
        //    uint32_t temp_hex_length = 0;

        //    // ascii in PE
        //    uint32_t longest_ascii_length = 0;
        //    uint32_t no_ascii_strings = 0;
        //    uint32_t temp_ascii_length = 0;

        //    // unicode
        //    uint16_t hw = 0;
        //    size_t uni_count = 0;
        //    size_t uni_start = 0;
        //    size_t uni_start_hex = 0;
        //    uint32_t longest_hex_length_u = 0;
        //    uint32_t no_hex_strings_u = 0;
        //    uint32_t temp_hex_length_u = 0;

        //    // unicode ascii in PE
        //    uint32_t longest_ascii_length_u = 0;
        //    uint32_t no_ascii_strings_u = 0;
        //    uint32_t temp_ascii_length_u = 0;

        //    unsigned long byte_counter = 0;

        //    unsigned long long nsections = pe_format->getDeclaredNumberOfSections();
        //    for (int index = 0; index < nsections; index++) {
        //        const std::shared_ptr<cchips::PeCoffSection> section = pe_format->getPeSection(index);
        //        if (section) {
        //            const auto bytes = section->getBytes(0, section->getLoadedSize());
        //            if (bytes.size() <= 4)
        //                continue;
        //            for (size_t i = 0; (i + 4) < bytes.size(); ++i) {
        //                // -------------------- ascii ---------------------------
        //                // skip 4 redundant character
        //                if (bytes[i] == bytes[i + 1] && bytes[i + 1] == bytes[i + 2] && bytes[i + 2] == bytes[i + 3]) {
        //                    i += 4;
        //                    continue;
        //                }
        //                // skip PADDING string
        //                if (bytes[i] == 'P' && bytes[i + 1] == 'A' && bytes[i + 2] == 'D') {
        //                    i += 3;
        //                    continue;
        //                }

        //                // hex number check
        //                if ((bytes[i] >= '0' && bytes[i] <= '9') ||
        //                    (bytes[i] >= 'a' && bytes[i] <= 'f') ||
        //                    (bytes[i] >= 'A' && bytes[i] <= 'F')
        //                    ) { // is hex number?
        //                    temp_hex_length++;
        //                }
        //                else {
        //                    if (temp_hex_length > longest_hex_length) {
        //                        longest_hex_length = temp_hex_length;
        //                    }
        //                    if (temp_hex_length >= _hexstring_min_length) {
        //                        no_hex_strings++;
        //                    }
        //                    temp_hex_length = 0;
        //                }

        //                // ascii a-z A-Z 0-9
        //                if ((bytes[i] >= '0' && bytes[i] <= '9') ||
        //                    (bytes[i] >= 'a' && bytes[i] <= 'z') ||
        //                    (bytes[i] >= 'A' && bytes[i] <= 'Z')
        //                    ) { // is hex number?
        //                    temp_ascii_length++;
        //                }
        //                else {
        //                    if (temp_ascii_length > longest_ascii_length) {
        //                        longest_ascii_length = temp_ascii_length;
        //                    }
        //                    if (temp_ascii_length >= _hexstring_min_length) {
        //                        no_ascii_strings++;
        //                    }
        //                    temp_ascii_length = 0;
        //                }
        //                // --------------------- unicode -------------------
        //                hw = (hw >> 8) | (bytes[i] << 8);
        //                if (i < 1)
        //                    continue;
        //                if (IsUnicode(hw) && temp_ascii_length_u == 0)
        //                {
        //                    uni_start = i - 1;
        //                    temp_ascii_length_u++;
        //                }
        //                // read and examine two bytes a time after first unicode hw
        //                else if (temp_ascii_length_u > 0 && (i - uni_start) % 2 == 1)
        //                {
        //                    if (IsUnicode(hw))
        //                    {
        //                        temp_ascii_length_u++;
        //                    }
        //                    else
        //                    {
        //                        if (temp_ascii_length_u > longest_ascii_length_u) {
        //                            longest_ascii_length_u = temp_ascii_length_u;
        //                        }
        //                        if (temp_ascii_length_u >= _hexstring_min_length) {
        //                            no_ascii_strings_u++;
        //                        }
        //                        temp_ascii_length_u = 0;
        //                    }
        //                } // unicode

        //                // is unicode hex
        //                if (IsUnicodeHex(hw) && temp_hex_length_u == 0)
        //                {
        //                    uni_start_hex = i - 1;
        //                    temp_hex_length_u++;
        //                }
        //                // read and examine two bytes a time after first unicode hw
        //                else if (temp_hex_length_u > 0 && (i - uni_start_hex) % 2 == 1)
        //                {
        //                    if (IsUnicodeHex(hw))
        //                    {
        //                        temp_hex_length_u++;
        //                    }
        //                    else
        //                    {
        //                        if (temp_hex_length_u > longest_hex_length_u) {
        //                            longest_hex_length_u = temp_hex_length_u;
        //                        }
        //                        if (temp_hex_length_u >= _hexstring_min_length) {
        //                            no_hex_strings_u++;
        //                        }
        //                        temp_hex_length_u = 0;
        //                    }
        //                } // unicode
        //            } // for loop over data i
        //            byte_counter = byte_counter + bytes.size();
        //            if (byte_counter > 0x500000) // almost 5 MB 
        //                break;
        //        }
        //    }
        //    return true;
        //}
        //catch (const std::exception& e)
        //{
        //}
        return false;
    }
} // namespace cchips
