#include "InstallInfoBuilder.h"

namespace cchips {
    const std::pair<HKEY, std::string> CInstalledSoftInfo::_hkcu_install_soft_register_info{ HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall" };
    const std::pair<HKEY, std::string> CInstalledSoftInfo::_hklm_install_soft_register_info{ HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall" };
    const std::pair<HKEY, std::string> CInstalledSoftInfo::_hklm_wow64_install_soft_register_info{ HKEY_LOCAL_MACHINE, "SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall" };

    const std::tuple<HKEY, std::string, std::string> CProgramsInfo::_hkcu_start_enum_info = { HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders", "Programs" };
    const std::tuple<HKEY, std::string, std::string> CProgramsInfo::_hklm_common_start_enum_info = { HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders", "Common Programs" };
} // namespace cchips
