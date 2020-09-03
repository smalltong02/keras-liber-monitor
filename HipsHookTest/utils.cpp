#include "stdafx.h"
#include "utils.h"
#include <random>

std::variant<const std::string, const std::wstring> 
CStringCommonTestI::NormalString(std::variant<const std::string, const std::wstring> prefix_str, size_t str_len)
{
    if (m_bunicode)
    {
        std::wstring unicode_str;
        const std::wstring pre_str = std::get<const std::wstring>(prefix_str);
        if (pre_str.length())
        {
            if (str_len == 0 || str_len <= pre_str.length())
            {
                unicode_str = pre_str;
            }
            else
            {
                std::mt19937 generator{ std::random_device{}() };
                std::uniform_int_distribution<int> distribution(L'a', L'z');
                size_t generate_len = str_len;
                unicode_str = std::wstring(generate_len, L'\0');
                for (auto& c : unicode_str)
                    c = distribution(generator);
                memcpy(&unicode_str[0], &pre_str[0], sizeof(wchar_t)*pre_str.length());
            }
        }
        else
        {
            size_t string_len = str_len;
            if (string_len == 0)
                string_len = STRING_COMMON_DEFAULT_LENGTH;
            std::mt19937 generator{ std::random_device{}() };
            std::uniform_int_distribution<int> distribution(L'a', L'z');
            size_t generate_len = string_len;
            unicode_str = std::wstring(generate_len, L'\0');
            for (auto& c : unicode_str)
                c = distribution(generator);
        }
        return unicode_str;
    }
    else
    {
        std::string ansi_str;
        const std::string pre_str = std::get<const std::string>(prefix_str);
        if (pre_str.length())
        {
            if (str_len == 0 || str_len <= pre_str.length())
            {
                ansi_str = pre_str;
            }
            else
            {
                std::mt19937 generator{ std::random_device{}() };
                std::uniform_int_distribution<int> distribution('a', 'z');
                size_t generate_len = str_len;
                ansi_str = std::string(generate_len, '\0');
                for (auto& c : ansi_str)
                    c = distribution(generator);
                memcpy(&ansi_str[0], &pre_str[0], pre_str.length());
            }
        }
        else
        {
            size_t string_len = str_len;
            if (string_len == 0)
                string_len = STRING_COMMON_DEFAULT_LENGTH;
            std::mt19937 generator{ std::random_device{}() };
            std::uniform_int_distribution<int> distribution('a', 'z');
            size_t generate_len = string_len;
            ansi_str = std::string(generate_len, '\0');
            for (auto& c : ansi_str)
                c = distribution(generator);
        }
        return ansi_str;
    }
}

std::variant<const std::string, const std::wstring> CStringCommonTestI::NullString(size_t str_len)
{
    if (m_bunicode)
    {
        std::wstring unicode_str;
        size_t string_len = str_len;
        if (string_len == 0)
            string_len = STRING_COMMON_DEFAULT_LENGTH;
        unicode_str = std::wstring(string_len, L'\0');
        return unicode_str;
    }
    else
    {
        std::string ansi_str;
        size_t string_len = str_len;
        if (string_len == 0)
            string_len = STRING_COMMON_DEFAULT_LENGTH;
        ansi_str = std::string(string_len, '\0');
        return ansi_str;
    }
}

std::variant<const std::string, const std::wstring> CStringCommonTestI::InvalidString(std::variant<const std::string, const std::wstring> prefix_str)
{
    return NormalString(prefix_str, STRING_COMMON_DEFAULT_LENGTH);
}

std::variant<const std::string, const std::wstring> CStringCommonTestI::SuperLongString(std::variant<const std::string, const std::wstring> prefix_str)
{
    return NormalString(prefix_str, STRING_COMMON_DEFAULT_SUPER_LENGTH);
}

std::vector<char> CStringCommonTestI::SuperLongData()
{
    std::vector<char> data_str;
    size_t data_length = STRING_COMMON_DEFAULT_SUPER_LENGTH;
    std::mt19937 generator{ std::random_device{}() };
    std::uniform_int_distribution<int> distribution(1, 255);
    size_t generate_len = data_length;
    data_str = std::vector<char>(generate_len, '\0');
    for (auto& c : data_str)
        c = distribution(generator);
    data_str[generate_len - 1] = '\0';
    data_str[generate_len - 2] = '\0';
    return data_str;
}
