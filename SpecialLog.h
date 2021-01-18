#pragma once
#include <string>
#include <vector>
#include <mutex>
#include <regex>
#include <iostream>
#include <type_traits>
#include <locale>
#include <codecvt>

extern std::wstring to_wide_string(const std::string& input);
extern std::string to_byte_string(const std::wstring& input);

namespace cchips {
    namespace special_log {

        template<typename E,
            typename TR = std::char_traits<E>,
            typename AL = std::allocator<E>>
            inline bool end_with(const std::basic_string<E, TR, AL>& src, const std::basic_string<E, TR, AL>& suffix) {
            if (src.size() < suffix.size()) {
                return false;
            }
            return src.substr(src.size() - suffix.size()) == suffix;
        }
        inline bool end_with(const std::wstring& src, const std::string& suffix) {
            return end_with(src, to_wide_string(suffix));
        }
        template<typename E,
            typename TR = std::char_traits<E>,
            typename AL = std::allocator<E>>
            inline bool end_with(const std::basic_string<E, TR, AL>& src, const E* suffix) {
            return end_with(src, std::basic_string<E, TR, AL>(suffix));
        }
        template<typename E,
            typename TR = std::char_traits<E>,
            typename AL = std::allocator<E>>
            inline bool start_with(const std::basic_string<E, TR, AL>& src, const std::basic_string<E, TR, AL>& prefix) {
            return src.substr(0, prefix.size()) == prefix;
        }
        inline bool start_with(const std::wstring& src, const std::string& suffix) {
            return start_with(src, to_wide_string(suffix));
        }
        template<typename E,
            typename TR = std::char_traits<E>,
            typename AL = std::allocator<E>>
            inline bool start_with(const std::basic_string<E, TR, AL>& src, const E* suffix) {
            return start_with(src, std::basic_string<E, TR, AL>(suffix));
        }
        template<typename E,
            typename TR = std::char_traits<E>,
            typename AL = std::allocator<E>,
            typename _str_type = std::basic_string<E, TR, AL>>
            std::vector<_str_type> split(const std::basic_string<E, TR, AL>& in, const std::basic_string<E, TR, AL>& delim) {
            std::basic_regex<E> re{ delim };
            return std::vector<_str_type> {
                std::regex_token_iterator<typename _str_type::const_iterator>(in.begin(), in.end(), re, -1),
                    std::regex_token_iterator<typename _str_type::const_iterator>()
            };
        }
        inline std::vector<std::wstring> split(const std::wstring& in, const std::string& delim) {
            return split(in, to_wide_string(delim));
        }
        template<typename E,
            typename TR = std::char_traits<E>,
            typename AL = std::allocator<E>,
            typename _str_type = std::basic_string<E, TR, AL>>
            std::vector<_str_type> split(const std::basic_string<E, TR, AL>& in, const E* delim) {
            return split(in, _str_type(delim));
        }
        inline std::vector<std::string> split(const char* in, const char* delim) {
            std::regex re{ delim };
            return std::vector<std::string> {
                std::cregex_token_iterator(in, in + strlen(in), re, -1),
                    std::cregex_token_iterator()
            };
        }
        inline std::vector<std::wstring> split(const wchar_t* in, const wchar_t* delim) {
            std::wregex re{ delim };
            return std::vector<std::wstring> {
                std::wcregex_token_iterator(in, in + wcslen(in), re, -1),
                    std::wcregex_token_iterator()
            };
        }

        template<typename E,
            typename TR = std::char_traits<E>,
            typename T>
            typename std::enable_if<!std::is_pointer<T>::value>::type
            _value_output_stream(std::basic_ostream<E, TR>& stream, const T& value) {
            stream << value;
        }

        template<typename E,
            typename TR = std::char_traits<E>,
            typename T>
            typename std::enable_if<std::is_pointer<T>::value>::type
            _value_output_stream(std::basic_ostream<E, TR>& stream, const T& value) {
            if (nullptr == value) {
                stream << "nullptr";
            }
            else {
                stream << value;
            }
        }

        inline void _value_output_stream(std::wostream&stream, const std::string& value) {
            stream << to_wide_string(value);
        }
        inline void _value_output_stream(std::ostream&stream, const std::wstring& value) {
            stream << to_byte_string(value);
        }
        inline void _value_output_stream(std::ostream&stream, const wchar_t* value) {
            if (nullptr == value) {
                stream << "nullptr";
            }
            else {
                stream << to_byte_string(value);
            }
        }
        template<typename E,
            typename TR = std::char_traits<E>,
            typename AL = std::allocator<E>>
            void _sm_log_output(std::basic_ostream<E, TR>& stream, const std::vector<std::basic_string<E, TR, AL>>& format, int& idx) {
        }
        template<typename E,
            typename TR = std::char_traits<E>,
            typename AL = std::allocator<E>,
            typename T, typename ...Args>
            typename void _sm_log_output(std::basic_ostream<E, TR>& stream, const std::vector<std::basic_string<E, TR, AL>>& format, int& idx, const T& first, Args...rest) {
            if (idx < format.size()) {
                _value_output_stream(stream, format[idx]);
                if (idx < format.size() - 1) {
                    _value_output_stream(stream, first);
                }
                _sm_log_output(stream, format, ++idx, rest...);
            }
        }
        template<typename E,
            typename TR = std::char_traits<E>,
            typename AL = std::allocator<E>,
            typename _str_type = std::basic_string<E, TR, AL>,
            typename ...Args>
            void sm_log(std::basic_ostream<E, TR>& stream, const char* file, int line, const std::basic_string<E, TR, AL>& format, Args...args) {
            const static std::string delim("{}");
            static std::once_flag oc;
            std::call_once(oc, [] {
#ifdef _MSC_VER
                std::locale loc(std::locale(), "", LC_CTYPE);
                std::wcout.imbue(loc);
                std::wcerr.imbue(loc);
                std::wclog.imbue(loc);
#elif defined(__GNUC__)
                std::locale::global(std::locale(""));
#endif
                });
            auto vf = split(format, std::string("\\{\\}"));
            if (end_with(format, delim)) {
                vf.push_back(_str_type());
            }
            std::string fn(file);
            auto pos = fn.find_last_of("\\/");
            int index = 0;
            stream << "[" << (pos != std::string::npos ? fn.substr(pos + 1) : fn).c_str() << ":" << line << "]:";
            _sm_log_output(stream, vf, index, args...);
            for (; index < vf.size(); ++index) {
                stream << vf[index];
                if (index < vf.size() - 1) {
                    stream << "{}";
                }
            }
            //		stream << std::endl;
        }
        template<typename E,
            typename TR = std::char_traits<E>,
            typename AL = std::allocator<E>,
            typename ...Args>
            void sm_log(std::basic_ostream<E, TR>& stream, const char* file, int line, const E* format, Args...args) {
            sm_log(stream, file, line, std::basic_string<E, TR, AL>(format), args...);
        }
        template<typename ...Args>
        void sm_log(std::wostream& stream, const char* file, int line, const char* format, Args...args) {
            sm_log(stream, file, line, to_wide_string(format), args...);
        }

        template<typename E,
            typename TR = std::char_traits<E>,
            typename AL = std::allocator<E>,
            typename _str_type = std::basic_string<E, TR, AL>,
            typename ...Args>
            void sm_simple_log(std::basic_ostream<E, TR>& stream, const std::basic_string<E, TR, AL>& format, Args...args) {
            const static std::string delim("{}");
            static std::once_flag oc;
            std::call_once(oc, [] {
#ifdef _MSC_VER
                std::locale loc(std::locale(), "", LC_CTYPE);
                std::wcout.imbue(loc);
                std::wcerr.imbue(loc);
                std::wclog.imbue(loc);
#elif defined(__GNUC__)
                std::locale::global(std::locale(""));
#endif
                });
            auto vf = split(format, std::string("\\{\\}"));
            if (end_with(format, delim)) {
                vf.push_back(_str_type());
            }
            int index = 0;
            _sm_log_output(stream, vf, index, args...);
            for (; index < vf.size(); ++index) {
                stream << vf[index];
                if (index < vf.size() - 1) {
                    stream << "{}";
                }
            }
            //		stream << std::endl;
        }
        template<typename E,
            typename TR = std::char_traits<E>,
            typename AL = std::allocator<E>,
            typename ...Args>
            void sm_simple_log(std::basic_ostream<E, TR>& stream, const E* format, Args...args) {
            sm_simple_log(stream, std::basic_string<E, TR, AL>(format), args...);
        }
        template<typename ...Args>
        void sm_simple_log(std::wostream& stream, const char* format, Args...args) {
            sm_simple_log(stream, to_wide_string(format), args...);
        }

#define SAMPLE_LOG_STREAM(stream,format,...) cchips::special_log::sm_log(stream,__FILE__,__LINE__,format, ##__VA_ARGS__)
    } // namespace special_log
} // namespace cchips