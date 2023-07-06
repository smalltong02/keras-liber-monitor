#pragma once
#include "JsonOptions.h"

namespace cchips {

    class CDiskDrive {
    public:
        using iterator = std::vector<std::string>::iterator;
        using const_iterator = std::vector<std::string>::const_iterator;

        CDiskDrive() {
            DWORD drives_len = GetLogicalDriveStrings(0, nullptr);
            if (drives_len) {
                std::vector<char> drive_vector(drives_len);
                drives_len = GetLogicalDriveStrings(drives_len, &drive_vector[0]);
                if (drives_len <= drive_vector.size()) {
                    for (DWORD offset = 0; offset < drives_len;) {
                        if (drive_vector[offset] != '\0') {
                            int lenth = static_cast<int>(strlen(&drive_vector[offset]));
                            m_drive_list.push_back(&drive_vector[offset]);
                            offset += lenth + 1;
                            continue;
                        }
                        offset++;
                    }
                }
            }
        }

        unsigned int GetDriveCount() const { return static_cast<unsigned int>(m_drive_list.size()); }
        std::string GetDrive(unsigned int index) const {
            if (index >= m_drive_list.size())
                return {};
            return m_drive_list[index];
        }

        iterator        begin() { return m_drive_list.begin(); }
        const_iterator  begin() const { return m_drive_list.begin(); }
        iterator        end() { return m_drive_list.end(); }
        const_iterator  end() const { return m_drive_list.end(); }

    private:
        std::vector<std::string> m_drive_list;
    };

    class CTraverseDriveEngine
    {
    public:
        using fi_callback = std::function<bool(const std::string&, const std::string&)>;

        static CTraverseDriveEngine& GetInstance()
        {
            static CTraverseDriveEngine m_instance;
            return m_instance;
        }

        void AddFiCb(fi_callback callback) {
            m_fi_cb_list.push_back(callback);
            return;
        }

        bool TraverseDrive(unsigned int drive_idx, const std::string& output_path) {
            if (drive_idx >= m_disk_drive.GetDriveCount())
                return false;
            return TraverseDrive(m_disk_drive.GetDrive(drive_idx), output_path);
        }

        bool TraverseDrive(const std::string& drive, const std::string& output_path) {
            m_ndirectory_totals = 0;
            m_nfile_totals = 0;
            return TraverseDirectory(drive, output_path);
        }

        void TraverseAllDrive(const std::string& output_path) {
            m_ndirectory_totals = 0;
            m_nfile_totals = 0;
            for (auto& disk : m_disk_drive) {
                TraverseDirectory(disk, output_path);
            }
            return;
        }

        unsigned long long GetDirectoryTotals() const { return m_ndirectory_totals; }
        unsigned long long GetFileTotals() const { return m_nfile_totals; }
    private:
        CTraverseDriveEngine() {
        }
        ~CTraverseDriveEngine() {
        }

        bool TraverseDirectory(const std::string& dir_path, const std::string& output_path) {
            if (dir_path.empty()) return false;
            info_log("search directory: %s ...", dir_path.c_str());
            try {
                for (auto& de : fs::directory_iterator(dir_path)) {
                    auto file_path = de.path();
                    auto file_name = file_path.filename();
                    if (de.is_directory()) {
                        TraverseDirectory(file_path.string(), output_path);
                        m_ndirectory_totals++;
                    }
                    else {
                        for (auto& cb : m_fi_cb_list) {
                            if (cb(file_path.string(), output_path)) {
                                m_nfile_totals++;
                            }
                        }
                    }
                }
            }
            catch (const std::exception& e)
            {
                return false;
            }
            return true;
        }

        std::vector<fi_callback> m_fi_cb_list;
        unsigned long long m_ndirectory_totals = 0;
        unsigned long long m_nfile_totals = 0;
        CDiskDrive m_disk_drive;
    };
} // namespace cchips