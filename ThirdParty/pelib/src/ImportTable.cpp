#pragma once
#include "SharedFunc.h"
#include "ImportTable.h"

namespace cchips {

    std::size_t ImportTable::getNumberOfImportsInLibrary(std::size_t libraryIndex) const
    {
        std::size_t result = 0;
        if (libraryIndex < libraries.size())
        {
            for (const auto &imp : imports)
            {
                if (imp->getLibraryIndex() == libraryIndex)
                {
                    ++result;
                }
            }
        }

        return result;
    }

    std::size_t ImportTable::getNumberOfImportsInLibrary(const std::string &name) const
    {
        std::size_t result = 0;

        for (std::size_t i = 0, e = getNumberOfLibraries(); i < e; ++i)
        {
            if (libraries[i] == name)
            {
                result += getNumberOfImportsInLibrary(i);
            }
        }

        return result;
    }

    std::size_t ImportTable::getNumberOfImportsInLibraryCaseInsensitive(const std::string &name) const
    {
        std::size_t result = 0;

        for (std::size_t i = 0, e = getNumberOfLibraries(); i < e; ++i)
        {
            if (areEqualCaseInsensitive(libraries[i], name))
            {
                result += getNumberOfImportsInLibrary(i);
            }
        }

        return result;
    }

    bool ImportTable::hasLibraryCaseInsensitive(const std::string &name) const
    {
        for (const auto &item : libraries)
        {
            if (areEqualCaseInsensitive(item, name))
            {
                return true;
            }
        }

        return false;
    }

    void ImportTable::clear()
    {
        libraries.clear();
        imports.clear();
    }

    const Import* ImportTable::getImport(const std::string &name) const
    {
        for (const auto &i : imports)
        {
            if (i->getName() == name)
            {
                return i.get();
            }
        }

        return nullptr;
    }

    const Import* ImportTable::getImportOnAddress(unsigned long long address) const
    {
        for (const auto &i : imports)
        {
            if (i->getAddress() == address)
            {
                return i.get();
            }
        }

        return nullptr;
    }
} // namespace cchips
