#pragma once
#include "ExportTable.h"

namespace cchips {

    bool Export::getOrdinalNumber(unsigned long long &exportOrdinalNumber) const
    {
        if (ordinalNumberIsValid)
        {
            exportOrdinalNumber = ordinalNumber;
        }

        return ordinalNumberIsValid;
    }

    const Export* ExportTable::getExport(const std::string &name) const
    {
        for (const auto &e : exports)
        {
            if (e.getName() == name)
            {
                return &e;
            }
        }

        return nullptr;
    }

    const Export* ExportTable::getExportOnAddress(unsigned long long address) const
    {
        for (const auto &e : exports)
        {
            if (e.getAddress() == address)
            {
                return &e;
            }
        }

        return nullptr;
    }
} // namespace cchips
