#pragma once
#include <string>
#include <vector>
#include <unicode/ucnv.h>
#include <unicode/utypes.h>
#include <unicode/ucsdet.h>
#include <unicode/ustream.h>
#include <unicode/brkiter.h>
#include <unicode/regex.h>

namespace cchips {

    class CStrExtractor {
    private:
#define _minimum_corpus_length 5
#define _maximum_depths 100
#define _maximum_string_length 10000

    public:
        CStrExtractor() = delete;
        ~CStrExtractor() = delete;

#define SPECIAL_CORPUS_PAD "#PAD#"
#define SPECIAL_CORPUS_UNK "#UNK#"
#define SPECIAL_CORPUS_SEP "#SEP#"
#define SPECIAL_CORPUS_CLS "#CLS#"
#define SPECIAL_CORPUS_MSK "#MASK#"
#define SPECIAL_CORPUS_EQU "#EQU#"
#define SPECIAL_CORPUS_NUM "#NUM#"
#define SPECIAL_CORPUS_ZER "#ZERO#"
#define SPECIAL_CORPUS_LNK "#LNK#"
#define SPECIAL_CORPUS_COMMA "#COMMA#"
#define SPECIAL_CORPUS_APL "#@#"
#define SPECIAL_CORPUS_RES1 "#RES1#"
#define SPECIAL_CORPUS_RES2 "#RES2#"
#define SPECIAL_CORPUS_RES3 "#RES3#"
#define SPECIAL_CORPUS_RES4 "#RES4#"
#define SPECIAL_CORPUS_RES5 "#RES5#"
#define SPECIAL_CORPUS_RES6 "#RES6#"
#define SPECIAL_CORPUS_RES7 "#RES7#"
#define SPECIAL_CORPUS_RES8 "#RES8#"
#define SPECIAL_CORPUS_RES9 "#RES9#"
#define SPECIAL_CORPUS_RES10 "#RES10#"
#define SPECIAL_CORPUS_RES11 "#RES11#"
#define SPECIAL_CORPUS_RES12 "#RES12#"
#define SPECIAL_CORPUS_RES13 "#RES13#"
#define SPECIAL_CORPUS_RES14 "#RES14#"
#define SPECIAL_CORPUS_RES15 "#RES15#"
#define SPECIAL_CORPUS_RES16 "#RES16#"
#define SPECIAL_CORPUS_RES17 "#RES17#"
#define SPECIAL_CORPUS_RES18 "#RES18#"
#define SPECIAL_CORPUS_RES19 "#RES19#"
#define SPECIAL_CORPUS_RES20 "#RES20#"
#define SPECIAL_CORPUS_NUM1 "#1#"
#define SPECIAL_CORPUS_NUM2 "#2#"
#define SPECIAL_CORPUS_NUM3 "#3#"
#define SPECIAL_CORPUS_NUM4 "#4#"
#define SPECIAL_CORPUS_NUM5 "#5#"
#define SPECIAL_CORPUS_NUM6 "#6#"
#define SPECIAL_CORPUS_NUM7 "#7#"
#define SPECIAL_CORPUS_NUM8 "#8#"
#define SPECIAL_CORPUS_NUM9 "#9#"
#define SPECIAL_CORPUS_NUM0 "#0#"

        static bool Initialize() {
            UErrorCode status = U_ZERO_ERROR;
            _usword_pattern.reset(icu::RegexPattern::compile("[a-zA-Z-_]+", 0, status));
            if (!_usword_pattern)
                return false;
            if (U_FAILURE(status))
                return false;
            _vsword_pattern.reset(icu::RegexPattern::compile("[a-zA-Z0-9-_+]+", 0, status));
            if (!_vsword_pattern)
                return false;
            if (U_FAILURE(status))
                return false;
            _url_pattern.reset(icu::RegexPattern::compile("(https?|ftp|file):\/\/[-A-Za-z0-9+&@#\/%?=~_|!:,.;]+[-A-Za-z0-9+&@#\/%=~_|]", 0, status));
            if (!_url_pattern)
                return false;
            if (U_FAILURE(status))
                return false;
            _email_pattern.reset(icu::RegexPattern::compile("[A-Za-z0-9._%+-]+@[A-Za-z0-9-]+\.[A-Za-z]{2,}", 0, status));
            if (!_email_pattern)
                return false;
            if (U_FAILURE(status))
                return false;
            _ipv4_pattern.reset(icu::RegexPattern::compile("(?:(?:1[0-9][0-9]\.)|(?:2[0-4][0-9]\.)|(?:25[0-5]\.)|(?:[1-9][0-9]\.)|(?:[0-9]\.)){3}(?:(?:1[0-9][0-9])|(?:2[0-4][0-9])|(?:25[0-5])|(?:[1-9][0-9])|(?:[0-9]))", 0, status));
            if (!_ipv4_pattern)
                return false;
            if (U_FAILURE(status))
                return false;
            _guid_pattern.reset(icu::RegexPattern::compile("[0-9a-fA-F]{8}\-[0-9a-fA-F]{4}\-[0-9a-fA-F]{4}\-[0-9a-fA-F]{4}\-[0-9a-fA-F]{12}", 0, status));
            if (!_guid_pattern)
                return false;
            if (U_FAILURE(status))
                return false;
            return true;
        }

        static bool detectedStringEncoding(const std::string& str, std::string& detected) {
            UCharsetDetector* csd;
            const UCharsetMatch** csm;
            std::int32_t match, matchCount = 0;
            detected.clear();
            if (!str.length()) return false;
            std::uint32_t length = str.length() > _maximum_string_length ? _maximum_string_length : str.length();
            UErrorCode status = U_ZERO_ERROR;

            csd = ucsdet_open(&status);
            if (status != U_ZERO_ERROR)
                return false;

            ucsdet_setText(csd, str.c_str(), length, &status);
            if (status != U_ZERO_ERROR)
                return false;

            csm = ucsdet_detectAll(csd, &matchCount, &status);
            if (status != U_ZERO_ERROR)
                return false;

            for (match = 0; match < matchCount; match += 1) {
                const char* name = ucsdet_getName(csm[match], &status);
                const char* lang = ucsdet_getLanguage(csm[match], &status);
                std::int32_t confidence = ucsdet_getConfidence(csm[match], &status);

                if (lang == NULL || strlen(lang) == 0)
                    lang = "**";

                //printf("%s (%s) %d\n", name, lang, confidence);
            }

            if (matchCount > 0) {
                detected = ucsdet_getName(csm[0], &status);
                if (status != U_ZERO_ERROR)
                    return false;
            }
            //printf("charset = %s\n\n", detected.c_str());
            ucsdet_close(csd);
            return true;
        }

        static bool detectedUSCorpus(const std::string_view str, std::vector <std::string>& detected_list, std::uint32_t _min_corpus_lenth = 3) {
            detected_list.clear();
            UErrorCode status = U_ZERO_ERROR;
            if (!str.length()) return false;
            std::uint32_t length = str.length() > _maximum_string_length ? _maximum_string_length : str.length();
            if (!_usword_pattern)
                return false;
            icu::UnicodeString findString(str.data(), length);
            std::unique_ptr<icu::RegexMatcher> regexFilter = nullptr;
            regexFilter.reset(_usword_pattern->matcher(findString, status));
            if (U_FAILURE(status)) {
                return false;
            }
            if (!regexFilter) {
                return false;
            }
            std::uint32_t pos = 0;
            icu::UnicodeString resultString;
            while (regexFilter->find(pos, status)) {
                resultString = regexFilter->group(status);
                if (resultString.length() >= _min_corpus_lenth) {
                    std::string detected;
                    resultString.toLower();
                    resultString.toUTF8String(detected);
                    detected_list.push_back(detected);
                }
                pos = regexFilter->end64(status);
            }
            return true;
        }

        static bool extractApiCorpus(const std::string_view api, std::string& detected) {
            detected.clear();
            if (!api.length()) return false;
            std::vector <std::string> detected_l;
            if (detectedVSCorpus(api, detected_l)) {
                if (detected_l.size() == 1) {
                    detected = detected_l[0];
                    if (detected.length()) {
                        if (detected[detected.length() - 1] == 'w' || detected[detected.length() - 1] == 'a') {
                            detected = detected.substr(0, detected.length() - 1);
                        }
                    }
                }
                else {
                    for (auto& it : detected_l) {
                        if (!it.length())continue;
                        if (detected.length()) {
                            detected += SPECIAL_CORPUS_APL;
                            detected += it;
                        }
                        else {
                            detected = it;
                        }
                    }
                }
            }
            if (detected.length())
                return true;
            return false;
        }

        static bool detectedVSCorpus(const std::string_view str, std::vector <std::string>& detected_list) {
            detected_list.clear();
            if (!str.length()) return false;
            std::uint32_t length = str.length() > _maximum_string_length ? _maximum_string_length : str.length();
            UErrorCode status = U_ZERO_ERROR;
            if (!_vsword_pattern)
                return false;
            icu::UnicodeString findString(str.data(), length);
            std::unique_ptr<icu::RegexMatcher> regexFilter = nullptr;
            regexFilter.reset(_vsword_pattern->matcher(findString, status));
            if (U_FAILURE(status)) {
                return false;
            }
            if (!regexFilter) {
                return false;
            }
            std::uint32_t pos = 0;
            icu::UnicodeString resultString;
            while (regexFilter->find(pos, status)) {
                resultString = regexFilter->group(status);
                std::string detected;
                resultString.toLower();
                resultString.toUTF8String(detected);
                detected_list.push_back(detected);
                pos = regexFilter->end64(status);
            }
            return true;
        }

        static bool detectedMatchCorpus(std::unique_ptr<icu::RegexPattern>& pattern, const std::string_view str, std::string& findstr, std::string& remainstr) {
            UErrorCode status = U_ZERO_ERROR;
            if (!str.length())
                return false;
            std::uint32_t length = str.length() > _maximum_string_length ? _maximum_string_length: str.length();
            if (!pattern)
                return false;
            icu::UnicodeString findString(str.data(), length);
            std::unique_ptr<icu::RegexMatcher> regexFilter = nullptr;
            regexFilter.reset(pattern->matcher(findString, status));
            if (U_FAILURE(status)) {
                return false;
            }
            if (!regexFilter) {
                return false;
            }
            std::uint32_t start = 0;
            std::uint32_t end = 0;
            std::string star_str;
            std::string tail_str;
            icu::UnicodeString resultString;
            if (regexFilter->find(0, status)) {
                resultString = regexFilter->group(status);
                resultString.toLower();
                resultString.toUTF8String(findstr);
                start = regexFilter->start(status);
                end = regexFilter->end(status);
                if (start != 0) {
                    star_str = str.substr(0, start);
                }
                if (end < str.length()) {
                    tail_str = str.substr(end, length - end);
                }
                remainstr = star_str + tail_str;
                return true;
            }
            return false;
        }

        static bool detectedUrlCorpus(const std::string_view str, std::string& urlstr, std::string& remainstr) {
            return detectedMatchCorpus(_url_pattern, str, urlstr, remainstr);
        }

        static bool detectedIpv4Corpus(const std::string_view str, std::string& ipv4str, std::string& remainstr) {
            return detectedMatchCorpus(_ipv4_pattern, str, ipv4str, remainstr);
        }

        static bool detectedEmailCorpus(const std::string_view str, std::string& emailstr, std::string& remainstr) {
            return detectedMatchCorpus(_email_pattern, str, emailstr, remainstr);
        }

        static bool detectedGuidCorpus(const std::string_view str, std::string& emailstr, std::string& remainstr) {
            return detectedMatchCorpus(_guid_pattern, str, emailstr, remainstr);
        }

        static bool detectedInsideCorpus(const std::string_view str, std::vector <std::string>& detected_list, std::uint32_t curdepth = 0) {
            detected_list.clear();
            if (!str.length()) return false;
            if (curdepth >= _maximum_depths) {
                return false;
            }
            std::string hitstr, remainstr;
            std::vector<std::string> detected_l;
            if (detectedUrlCorpus(str, hitstr, remainstr) || detectedEmailCorpus(str, hitstr, remainstr) || detectedGuidCorpus(str, hitstr, remainstr) || detectedIpv4Corpus(str, hitstr, remainstr)) {
                if (hitstr.length()) {
                    detected_list.push_back(hitstr);
                }
                if (remainstr.length()) {
                    if (detectedInsideCorpus(remainstr, detected_l, curdepth + 1)) {
                        detected_list.insert(detected_list.end(), detected_l.begin(), detected_l.end());
                    }
                }
                return true;
            }
            return detectedUSCorpus(str, detected_list, _minimum_corpus_length);
        }
    private:
        static std::unique_ptr<icu::RegexPattern> _usword_pattern;
        static std::unique_ptr<icu::RegexPattern> _vsword_pattern;
        static std::unique_ptr<icu::RegexPattern> _url_pattern;
        static std::unique_ptr<icu::RegexPattern> _ipv4_pattern;
        static std::unique_ptr<icu::RegexPattern> _email_pattern;
        static std::unique_ptr<icu::RegexPattern> _guid_pattern;
    };
} // namespace cchips
