#include "extractor.h"

namespace cchips {

    std::unique_ptr<icu::RegexPattern> CStrExtractor::_usword_pattern = nullptr;
    std::unique_ptr<icu::RegexPattern> CStrExtractor::_vsword_pattern = nullptr;
    std::unique_ptr<icu::RegexPattern> CStrExtractor::_url_pattern = nullptr;
    std::unique_ptr<icu::RegexPattern> CStrExtractor::_ipv4_pattern = nullptr;
    std::unique_ptr<icu::RegexPattern> CStrExtractor::_email_pattern = nullptr;
    std::unique_ptr<icu::RegexPattern> CStrExtractor::_guid_pattern = nullptr;
} // namespace cchips
