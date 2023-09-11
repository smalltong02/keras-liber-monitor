#pragma once
#include "InfoBuilder.h"
#include <ShObjIdl.h>
#include <ShlGuid.h>
#include "peinside\stringutils.h"

namespace cchips {
    class CRegister {

    public:
        class RegistryError
            : public std::runtime_error
        {
        public:
            RegistryError(const char* message, LONG errorCode)
                : std::runtime_error{ message }
                , m_errorCode{ errorCode }
            {}
            LONG code() const noexcept
            {
                return m_errorCode;
            }
        private:
            LONG m_errorCode;
        };

        CRegister(HKEY hkey) : m_hkey(hkey) {}
        CRegister(HKEY hrootkey, const std::string& subkey) {
            HKEY hresult = nullptr;
            LONG bret = ::RegOpenKeyEx(hrootkey, subkey.c_str(), 0, KEY_ALL_ACCESS, &hresult);
            if (bret == ERROR_SUCCESS)
                m_hkey = hresult;
        }
        ~CRegister() {
            if (m_hkey)
                RegCloseKey(m_hkey);
        }
        bool Valid() const { return m_hkey != nullptr; }

        DWORD RegGetDword(const std::string& subKey, const std::string& value) {
            if (!Valid()) {
                return {};
            }

            DWORD data{};
            DWORD dataSize = sizeof(data);

            LONG retCode = ::RegGetValue(
                m_hkey,
                subKey.c_str(),
                value.c_str(),
                RRF_RT_REG_DWORD,
                nullptr,
                &data,
                &dataSize
            );
            if (retCode != ERROR_SUCCESS)
            {
                return {};
            }
            return data;
        }

        std::string RegGetString(const std::string& subKey, const std::string& value) {
            if (!Valid()) {
                return {};
            }

            DWORD dataSize{};

            LONG retCode = ::RegGetValue(
                m_hkey,
                subKey.c_str(),
                value.c_str(),
                RRF_RT_REG_SZ,
                nullptr,
                nullptr,
                &dataSize
            );
            if (retCode != ERROR_SUCCESS)
            {
                return {};
            }

            std::string data;
            data.resize(dataSize);
            retCode = ::RegGetValue(
                m_hkey,
                subKey.c_str(),
                value.c_str(),
                RRF_RT_REG_SZ,
                nullptr,
                &data[0],
                &dataSize
            );
            if (retCode != ERROR_SUCCESS)
            {
                return {};
            }
            DWORD stringLengthInChars = dataSize;
            stringLengthInChars--; // Exclude the NUL written by the Win32 API
            data.resize(stringLengthInChars);
            return data;
        }

        std::vector<std::string> RegGetMultiString(const std::string& subKey, const std::string& value) {
            if (!Valid()) {
                return {};
            }

            DWORD dataSize{};

            LONG retCode = ::RegGetValue(
                m_hkey,
                subKey.c_str(),
                value.c_str(),
                RRF_RT_REG_MULTI_SZ,
                nullptr,
                nullptr,
                &dataSize
            );
            if (retCode != ERROR_SUCCESS)
            {
                return {};
            }

            std::vector<char> data;
            data.resize(dataSize);
            retCode = ::RegGetValue(
                m_hkey,
                subKey.c_str(),
                value.c_str(),
                RRF_RT_REG_MULTI_SZ,
                nullptr,
                &data[0],
                &dataSize
            );
            if (retCode != ERROR_SUCCESS)
            {
                return {};
            }

            data.resize(dataSize);
            // Parse the double-NUL-terminated string into a vector<wstring>
            std::vector<std::string> result;
            const char* currStringPtr = &data[0];
            while (*currStringPtr != L'\0')
            {
                // Current string is NUL-terminated, so get its length with wcslen
                const size_t currStringLength = strlen(currStringPtr);
                // Add current string to result vector
                result.push_back(std::string{ currStringPtr, currStringLength });
                // Move to the next string
                currStringPtr += currStringLength + 1;
            }
            return result;
        }

        std::vector<std::pair<std::string, DWORD>> RegEnumValues() {
            if (!Valid()) {
                return {};
            }

            DWORD valueCount{};
            DWORD maxValueNameLen{};

            LONG retCode = ::RegQueryInfoKey(
                m_hkey,
                nullptr,    // No user-defined class
                nullptr,    // No user-defined class size
                nullptr,    // Reserved
                nullptr,    // No subkey count
                nullptr,    // No subkey max length
                nullptr,    // No subkey class length
                &valueCount,
                &maxValueNameLen,
                nullptr,    // No max value length
                nullptr,    // No security descriptor
                nullptr     // No last write time
            );
            if (retCode != ERROR_SUCCESS)
            {
                return {};
            }
            maxValueNameLen++;

            auto nameBuffer = std::make_unique<char[]>(maxValueNameLen);
            std::vector<std::pair<std::string, DWORD>> values;
            for (DWORD index = 0; index < valueCount; index++)
            {
                DWORD valueNameLen = maxValueNameLen;
                DWORD valueType{};
                retCode = ::RegEnumValue(
                    m_hkey,
                    index,
                    nameBuffer.get(),
                    &valueNameLen,
                    nullptr,    // Reserved
                    &valueType,
                    nullptr,    // Not interested in data
                    nullptr     // Not interested in data size
                );
                if (retCode != ERROR_SUCCESS)
                {
                    return {};
                }

                values.push_back(std::make_pair(
                    std::string{ nameBuffer.get(), valueNameLen },
                    valueType
                ));
                return values;
            }
        }

        std::vector<std::string> RegEnumKeys() {

            if (!Valid()) {
                return {};
            }

            LONG retCode = ERROR_SUCCESS;
#define MAX_KEY_NAME_LEN 255

            auto nameBuffer = std::make_unique<char[]>(MAX_KEY_NAME_LEN);
            std::vector<std::string> values;
            for (DWORD index = 0; retCode == ERROR_SUCCESS; index++)
            {
                DWORD valueNameLen = MAX_KEY_NAME_LEN;
                retCode = ::RegEnumKeyEx(
                    m_hkey,
                    index,
                    nameBuffer.get(),
                    &valueNameLen,
                    0,
                    nullptr,
                    nullptr,
                    nullptr
                );
                if (retCode == ERROR_NO_MORE_ITEMS)
                {
                    break;
                }
                if (retCode != ERROR_SUCCESS)
                {
                    return {};
                }

                values.push_back(std::string{ nameBuffer.get(), valueNameLen });
            }
            return values;
        }

    private:
        HKEY m_hkey = nullptr;
    };

    class CInstalledSoftInfo
    {
    public:
        struct _SoftInfo {
            std::string soft_name;
            std::string soft_version;
            std::string install_location;
            std::string product_id;
            std::string publisher;
            std::string install_source;
            std::string uninstall_location;
        };

        CInstalledSoftInfo() {
            Initialize();
        }

        bool Initialize() {
            auto key_string_func = [](std::string& key_str, const std::string& src_str) {
                if (key_str.length()) return;
                if (!src_str.length()) return;
                key_str = src_str;
                return;
            };
            auto collect_info_func = [&](const std::pair<HKEY, std::string>& register_info_def) {
                CRegister register_info(register_info_def.first, register_info_def.second);
                if (!register_info.Valid()) return;

                try {
                    std::vector<std::string> key_list = register_info.RegEnumKeys();
                    for (auto key : key_list) {
                        _SoftInfo info{};
                        std::string key_str{};
                        info.product_id = key;
                        info.soft_name = register_info.RegGetString(key, "DisplayName");
                        info.soft_version = register_info.RegGetString(key, "DisplayVersion");
                        info.install_location = register_info.RegGetString(key, "InstallLocation");
                        key_string_func(key_str, info.install_location);
                        if (!key_str.length()) continue;
                        info.publisher = register_info.RegGetString(key, "Publisher");
                        info.install_source = register_info.RegGetString(key, "InstallSource");
                        info.uninstall_location = register_info.RegGetString(key, "UninstallString");

                        if (contain_invalid_utf8char(info.soft_name)) {
                            info.soft_name = stringto_hexstring(info.soft_name);
                        }
                        if (contain_invalid_utf8char(info.soft_version)) {
                            info.soft_version = stringto_hexstring(info.soft_version);
                        }
                        if (contain_invalid_utf8char(info.install_location)) {
                            info.install_location = stringto_hexstring(info.install_location);
                        }
                        if (contain_invalid_utf8char(info.publisher)) {
                            info.publisher = stringto_hexstring(info.publisher);
                        }
                        if (contain_invalid_utf8char(info.install_source)) {
                            info.install_source = stringto_hexstring(info.install_source);
                        }
                        if (contain_invalid_utf8char(info.uninstall_location)) {
                            info.uninstall_location = stringto_hexstring(info.uninstall_location);
                        }

                        if (key_str.at(key_str.length() - 1) != '\\')
                            key_str.append("\\");
                        key_str = StringToLower(key_str);
                        std::string md5_str = GetMd5((unsigned char*)key_str.c_str(), (unsigned int)key_str.size());
                        if (md5_str.length()) {
                            //if (bpatch) {
                            //    m_systempatches_list.emplace(md5_str, info);
                            //}
                            //else {
                            m_softinfo_list.emplace(md5_str, info);
                            //}
                        }
                    }
                }
                catch (const std::exception& e)
                {
                    //std::cout << "error(" << e.code() << "): " << e.what() << std::endl;
                }
                return;
            };

            collect_info_func(_hkcu_install_soft_register_info);
            collect_info_func(_hklm_install_soft_register_info);
            collect_info_func(_hklm_wow64_install_soft_register_info);
            return true;
        }

        const std::map<std::string, _SoftInfo>& GetSoftInfoList() { return m_softinfo_list; }

        static const std::pair<HKEY, std::string> _hkcu_install_soft_register_info;
        static const std::pair<HKEY, std::string> _hklm_install_soft_register_info;
        static const std::pair<HKEY, std::string> _hklm_wow64_install_soft_register_info;
    private:
        std::map<std::string, _SoftInfo> m_softinfo_list;
        //std::map<std::string, _SoftInfo> m_systempatches_list;
    };

    class CTraverseLnkFolder
    {
    public:
        using iterator = std::map<std::string, std::string>::iterator;
        using const_iterator = std::map<std::string, std::string>::const_iterator;

        CTraverseLnkFolder(const std::string& lnk_folder) : m_lnk_folder(lnk_folder) {
            CoInitialize(nullptr);
            Initialize();
        }
        ~CTraverseLnkFolder() {
            if (m_pisl)
                m_pisl->Release();
            CoUninitialize();
        }

        iterator        begin() { return m_lnk_info_list.begin(); }
        const_iterator  begin() const { return m_lnk_info_list.begin(); }
        iterator        end() { return m_lnk_info_list.end(); }
        const_iterator  end() const { return m_lnk_info_list.end(); }

    private:
        bool Initialize() {
            HRESULT hr;
            hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&m_pisl);
            if (!SUCCEEDED(hr)) {
                return false;
            }
            hr = m_pisl->QueryInterface(IID_IPersistFile, (LPVOID*)&m_pipf);
            if (!SUCCEEDED(hr)) {
                return false;
            }
            return TraverseFolder(m_lnk_folder);
        }

        bool TraverseFolder(const std::string& dir_path) {
            if (!dir_path.length())
                return false;
            if (!m_pisl || !m_pipf)
                return false;
            HRESULT hr;
    #define _max_lnk_path 1024
            char* lnk_path = new char[_max_lnk_path];
            try {
                for (auto& de : fs::directory_iterator(dir_path)) {
                    auto file_path = de.path();
                    auto file_name = file_path.filename();
                    if (de.is_directory()) {
                        TraverseFolder(file_path.string());
                    }
                    else {
                        try {
                            hr = m_pipf->Load(file_path.c_str(), STGM_READ);

                            if (SUCCEEDED(hr)) {
                                WIN32_FIND_DATA find_data{};
                                hr = m_pisl->GetPath(lnk_path, MAX_PATH, &find_data, SLGP_UNCPRIORITY | SLGP_RAWPATH);
                                if (SUCCEEDED(hr)) {
                                    std::string str = StringToLower(find_data.cFileName);
                                    if (str.length()) {
                                        auto find = m_lnk_info_list.find(str);
                                        if (find == m_lnk_info_list.end()) {
                                            m_lnk_info_list[str] = lnk_path;
                                        }
                                    }
                                }
                            }
                        }
                        catch (std::system_error& e)
                        {
                            //std::cout << "error(" << e.code() << "): " << e.what() << std::endl;
                        }
                    }
                }
            }
            catch (const std::exception& e)
            {
                //std::cout << "error(" << e.code() << "): " << e.what() << std::endl;
            }
            return true;
        }

        IShellLink* m_pisl = nullptr;
        IPersistFile* m_pipf = nullptr;
        std::map<std::string, std::string> m_lnk_info_list;
        std::string m_lnk_folder;
    };

    class CProgramsInfo
    {
    public:
        using iterator = std::map<std::string, std::string>::iterator;
        using const_iterator = std::map<std::string, std::string>::const_iterator;

        CProgramsInfo() {
            Initialize();
        }

        iterator        begin() { return m_program_list.begin(); }
        const_iterator  begin() const { return m_program_list.begin(); }
        iterator        end() { return m_program_list.end(); }
        const_iterator  end() const { return m_program_list.end(); }
        iterator       find(const std::string& key) { return m_program_list.find(key); }
        const_iterator find(const std::string& key) const { return m_program_list.find(key); }

        static const std::tuple<HKEY, std::string, std::string> _hkcu_start_enum_info;
        static const std::tuple<HKEY, std::string, std::string> _hklm_common_start_enum_info;

    private:
        bool Initialize() {
            auto key_string_func = [](std::string& key_str, const std::string& src_str) {
                if (key_str.length()) return;
                if (!src_str.length()) return;
                key_str = src_str;
            };
            auto collect_info_func = [&](const std::tuple<HKEY, std::string, std::string>& register_info_def) {
                CRegister register_info(std::get<0>(register_info_def));
                if (!register_info.Valid()) return;

                try {
                    std::string program_path = register_info.RegGetString(std::get<1>(register_info_def), std::get<2>(register_info_def));
                    CTraverseLnkFolder lnk_folder(program_path);
                    for (auto& lnk : lnk_folder) {
                        auto find = m_program_list.find(lnk.first);
                        if (find != m_program_list.end())
                            continue;
                        m_program_list[lnk.first] = lnk.second;
                    }
                }
                catch (std::system_error& e)
                {
                    //std::cout << "error(" << e.code() << "): " << e.what() << std::endl;
                }
                return;
            };

            collect_info_func(_hkcu_start_enum_info);
            collect_info_func(_hklm_common_start_enum_info);
            return true;
        }

        std::map<std::string, std::string> m_program_list;
    };

    class CInstallInfoBuilder : public CInfoBuilder
    {
    public:
        CInstallInfoBuilder() { UpdateInfoType(CJsonOptions::_info_type::info_type_install); }
        ~CInstallInfoBuilder() = default;

        bool Initialize(std::unique_ptr<CJsonOptions>& json_options) { 
            if (!json_options)
                return false;
            if (!json_options->GetOptionsInfo(GetInfoType(), std::pair<unsigned char*, size_t>(reinterpret_cast<unsigned char*>(&m_options_info), sizeof(m_options_info))))
                return false;
            return true;
        }
        bool Scan(const std::string& file_buf, CFileInfo& file_info) {
            std::unique_ptr<cchips::CRapidJsonWrapper> json_result;
            json_result = std::make_unique<cchips::CRapidJsonWrapper>("{}");
            if (!json_result)
                return false;
            file_info.SetJsonInstallInfo(std::move(json_result));
            return true;
        }
        bool Scan(fs::path& file_path, CFileInfo& file_info) { 
            try {
                std::string parent_path = file_path.parent_path().string();
                if (parent_path.at(parent_path.length() - 1) != '\\')
                    parent_path.append("\\");
                std::unique_ptr<cchips::CRapidJsonWrapper> json_result;
                json_result = std::make_unique<cchips::CRapidJsonWrapper>("{}");
                if (!json_result)
                    return false;
                parent_path = StringToLower(parent_path);
                std::string md5_str = GetMd5((unsigned char*)parent_path.c_str(), (unsigned int)parent_path.size());
                if (md5_str.length()) {
                    auto& installinfo_list = m_installed_info.GetSoftInfoList();
                    if (auto installinfo = installinfo_list.find(md5_str); installinfo != installinfo_list.end()) {
                        if (installinfo->second.soft_name.length() && m_options_info.bname) {
                            json_result->AddTopMember("softwarename", cchips::RapidValue(installinfo->second.soft_name.c_str(), json_result->GetAllocator()));
                        }
                        if (installinfo->second.soft_version.length() && m_options_info.bversion) {
                            json_result->AddTopMember("softwareversion", cchips::RapidValue(installinfo->second.soft_version.c_str(), json_result->GetAllocator()));
                        }
                        if (installinfo->second.install_location.length() && m_options_info.blocation) {
                            json_result->AddTopMember("installlocation", cchips::RapidValue(installinfo->second.install_location.c_str(), json_result->GetAllocator()));
                        }

                        if (installinfo->second.publisher.length() && m_options_info.bpublisher) {
                            json_result->AddTopMember("publisher", cchips::RapidValue(installinfo->second.publisher.c_str(), json_result->GetAllocator()));
                        }
                        if (installinfo->second.product_id.length() && m_options_info.bpid) {
                            json_result->AddTopMember("productid", cchips::RapidValue(installinfo->second.product_id.c_str(), json_result->GetAllocator()));
                        }
                    }
                }
                file_info.SetJsonInstallInfo(std::move(json_result));
                return true;
            }
            catch (const std::exception& e)
            {
            }
            return false;
        }

    private:
        CJsonOptions::_install_info m_options_info;
        CInstalledSoftInfo m_installed_info;
    };
} // namespace cchips
