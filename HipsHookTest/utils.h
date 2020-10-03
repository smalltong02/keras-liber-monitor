#pragma once
#include <stdio.h>
#include <windows.h>
#include <string>
#include <vector>
#include <variant>
#include "tydefine.h"

class CStringCommonTestI
{
public:
    CStringCommonTestI() : m_bunicode(false) {}
    CStringCommonTestI(bool bunicode) : m_bunicode(bunicode) {}
    ~CStringCommonTestI() = default;

#define STRING_COMMON_DEFAULT_LENGTH 1024
#define STRING_COMMON_DEFAULT_SUPER_LENGTH (1024 * 16)
public:
    std::variant<const std::string, const std::wstring> NormalString(std::variant<const std::string, const std::wstring> prefix_str, size_t str_len = 0);
    std::variant<const std::string, const std::wstring> NullString(size_t str_len = 0);
    std::variant<const std::string, const std::wstring> InvalidString(std::variant<const std::string, const std::wstring> prefix_str);
    std::variant<const std::string, const std::wstring> SuperLongString(std::variant<const std::string, const std::wstring> prefix_str);
    std::vector<char> SuperLongData();
private:
    bool m_bunicode;
};

