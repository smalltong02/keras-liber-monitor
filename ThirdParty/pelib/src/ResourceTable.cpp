#pragma once
#include "SharedFunc.h"
#include "ResourceTable.h"
#include "PeFormat.h"

namespace cchips {

    bool ResourceTree::isValidTree() const
    {
        if (!getNumberOfLevelsWithoutRoot())
        {
            return tree.size() == 1 && tree[0].size() == 1 && tree[0][0] == 0;
        }

        for (std::size_t i = 0, e = getNumberOfLevelsWithoutRoot(); i < e; ++i)
        {
            std::size_t sumOfChildrens = 0;

            for (const auto &node : tree[i])
            {
                sumOfChildrens += node;
            }

            if (sumOfChildrens != tree[i + 1].size())
            {
                return false;
            }
        }

        return std::all_of(tree[tree.size() - 1].begin(), tree[tree.size() - 1].end(),
            [](const auto &leaf)
            {
                return !leaf;
            }
        );
    }

    void ResourceTree::addNode(std::size_t level, std::size_t childs)
    {
        for (std::size_t i = tree.size(); i < level + 1; ++i)
        {
            tree.push_back(std::vector<std::size_t>());
        }

        if (!level)
        {
            tree[0][0] = childs;
        }
        else
        {
            tree[level].push_back(childs);
        }
    }

    const std::string_view Resource::getBytes(std::size_t sOffset, std::size_t sSize) const
    {
        if (sOffset >= loadedBytes.size())
        {
            return {};
        }

        return std::string_view(loadedBytes.data() + sOffset, getRealSizeInRegion(sOffset, sSize, loadedBytes.size()));
    }

    bool Resource::getNameId(std::size_t &rId) const
    {
        if (nameIdIsValid)
        {
            rId = nameId;
        }

        return nameIdIsValid;
    }

    bool Resource::getTypeId(std::size_t &rId) const
    {
        if (typeIdIsValid)
        {
            rId = typeId;
        }

        return typeIdIsValid;
    }

    bool Resource::getLanguageId(std::size_t &rId) const
    {
        if (languageIdIsValid)
        {
            rId = languageId;
        }

        return languageIdIsValid;
    }

    bool Resource::getSublanguageId(std::size_t &rId) const
    {
        if (sublanguageIdIsValid)
        {
            rId = sublanguageId;
        }

        return sublanguageIdIsValid;
    }

    bool Resource::getBits(std::string &sResult) const
    {
        sResult = bytesToBits(loadedBytes.data(), loadedBytes.size());
        return loaded;
    }

    bool Resource::getBytes(std::vector<unsigned char> &sResult, std::size_t sOffset, std::size_t sSize) const
    {
        if (!loaded || sOffset >= loadedBytes.size())
        {
            return false;
        }

        sSize = getRealSizeInRegion(sOffset, sSize, loadedBytes.size());
        sResult.reserve(sSize);
        sResult.assign(loadedBytes.begin() + sOffset, loadedBytes.begin() + sOffset + sSize);
        return loaded;
    }

    bool Resource::getString(std::string &sResult, std::size_t sOffset, std::size_t sSize) const
    {
        if (sOffset >= loadedBytes.size())
        {
            return false;
        }

        bytesToString(loadedBytes.data(), loadedBytes.size(), sResult, sOffset, sSize);
        return loaded;
    }

    bool Resource::getHexBytes(std::string &sResult) const
    {
        bytesToHexString(loadedBytes.data(), loadedBytes.size(), sResult);
        return loaded;
    }

    void Resource::load(const PeFormat *rOwner)
    {
        if (!size || !rOwner || offset >= rOwner->getLoadedFileLength())
        {
            loadedBytes = "";
            loaded = false;
            return;
        }

        const auto* origBytes = rOwner->getLoadedBytesData() + offset;
        std::string_view bytes_view = std::string_view(reinterpret_cast<const char*>(origBytes), (std::min)(size, rOwner->getLoadedFileLength() - offset));
        std::copy(bytes_view.begin(), bytes_view.end(), std::back_inserter(bytes));
        loadedBytes = std::string_view(reinterpret_cast<const char*>(bytes.data()), bytes.size());
        loaded = true;
    }

    const std::pair<uint16_t, uint16_t> ResourceIconGroup::iconPriorities[16] =
    {
        {32, 32},
        {24, 32},
        {48, 32},
        {32, 8},
        {16, 32},
        {64, 32},
        {24, 8},
        {48, 8},
        {16, 8},
        {64, 8},
        {96, 32},
        {96, 8},
        {128, 32},
        {128, 8},
        {256, 32},
        {256, 8}
    };

    bool ResourceIconGroup::iconCompare(const ResourceIcon *i1, const ResourceIcon *i2)
    {
        auto i1Width = i1->getWidth();
        auto i1BitCount = i1->getBitCount();
        auto i2Width = i2->getWidth();
        auto i2Height = i2->getHeight();
        auto i2BitCount = i2->getBitCount();

        if (i2Width != i2Height)
        {
            return false;
        }

        for (const auto &p : iconPriorities)
        {
            if (p.first == i1Width && p.second == i1BitCount)
            {
                return false;
            }

            if (p.first == i2Width && p.second == i2BitCount)
            {
                return true;
            }
        }

        return false;
    }

    const ResourceIcon * ResourceIconGroup::getPriorIcon() const {
        auto result = std::max_element(icons.begin(), icons.end(), iconCompare);

        if (result == icons.end())
        {
            return nullptr;
        }

        return *result;
    }

    void ResourceTable::parseVersionInfoResources()
    {
        std::vector<std::uint8_t> bytes;

        for (auto ver : resourceVersions)
        {
            if (!ver->getBytes(bytes))
            {
                continue;
            }
            parseVersionInfo(bytes);
        }
    }

    bool ResourceTable::parseVersionInfo(const std::vector<std::uint8_t> &bytes)
    {
        VersionInfoHeader vih;
        if (bytes.size() < vih.structSize())
        {
            return false;
        }

        std::size_t offset = 0;

        DynamicBuffer structContent(bytes);
        vih.length = structContent.read<std::uint16_t>(offset); offset += sizeof(vih.length);
        vih.valueLength = structContent.read<std::uint16_t>(offset); offset += sizeof(vih.valueLength);
        vih.type = structContent.read<std::uint16_t>(offset); offset += sizeof(vih.type);

        std::string key = unicodeToAscii(&bytes.data()[offset], bytes.size() - offset);
        if (key != "VS_VERSION_INFO")
        {
            return false;
        }

        offset += VI_KEY_SIZE;
        offset = alignUp(offset, sizeof(std::uint32_t));

        FixedFileInfo ffi;
        if (vih.valueLength == ffi.structSize())
        {
            if (bytes.size() < offset + ffi.structSize())
            {
                return false;
            }

            ffi.signature = structContent.read<std::uint32_t>(offset); offset += sizeof(ffi.signature);
            ffi.strucVersionMin = structContent.read<std::uint16_t>(offset); offset += sizeof(ffi.strucVersionMin);
            ffi.strucVersionMaj = structContent.read<std::uint16_t>(offset); offset += sizeof(ffi.strucVersionMaj);
            std::uint32_t t1 = structContent.read<std::uint32_t>(offset);
            ffi.fileVersionMaj = t1 >> 16; offset += sizeof(ffi.fileVersionMaj);
            ffi.fileVersionMin = t1 & 0xFFFF; offset += sizeof(ffi.fileVersionMin);
            std::uint64_t t2 = structContent.read<std::uint64_t>(offset);
            ffi.productVersion = t2 >> 16; offset += sizeof(ffi.productVersion);
            ffi.fileFlagsMask = structContent.read<std::uint32_t>(offset); offset += sizeof(ffi.fileFlagsMask);
            ffi.fileFlags = structContent.read<std::uint32_t>(offset); offset += sizeof(ffi.fileFlags);
            ffi.fileOS = structContent.read<std::uint32_t>(offset); offset += sizeof(ffi.fileOS);
            ffi.fileType = structContent.read<std::uint32_t>(offset); offset += sizeof(ffi.fileType);
            ffi.fileSubtype = structContent.read<std::uint32_t>(offset); offset += sizeof(ffi.fileSubtype);
            ffi.timestamp = structContent.read<std::uint64_t>(offset); offset += sizeof(ffi.timestamp);

            if (ffi.signature != FFI_SIGNATURE)
            {
                return false;
            }
        }

        else if (vih.valueLength != 0)
        {
            return false;
        }

        offset = alignUp(offset, sizeof(std::uint32_t));
        while (offset < vih.length)
        {
            if (!parseVersionInfoChild(bytes, offset))
            {
                return false;
            }
        }

        return true;
    }

    bool ResourceTable::parseVersionInfoChild(const std::vector<std::uint8_t> &bytes, std::size_t &offset)
    {
        std::size_t origOffset = offset;
        VersionInfoHeader chh;
        if (bytes.size() < offset + chh.structSize())
        {
            return false;
        }

        DynamicBuffer structContent(bytes);
        chh.length = structContent.read<std::uint16_t>(offset); offset += sizeof(chh.length);
        chh.valueLength = structContent.read<std::uint16_t>(offset); offset += sizeof(chh.valueLength);
        chh.type = structContent.read<std::uint16_t>(offset); offset += sizeof(chh.type);

        std::string key = unicodeToAscii(&bytes.data()[offset], bytes.size() - offset);

        if (key == "VarFileInfo")
        {
            offset += VFI_KEY_SIZE;
            offset = alignUp(offset, sizeof(std::uint32_t));

            for (std::size_t targetOffset = origOffset + chh.length; offset < targetOffset; )
            {
                if (!parseVarFileInfoChild(bytes, offset))
                {
                    return false;
                }
            }
        }
        else if (key == "StringFileInfo")
        {
            offset += SFI_KEY_SIZE;
            offset = alignUp(offset, sizeof(std::uint32_t));

            for (std::size_t targetOffset = origOffset + chh.length; offset < targetOffset; )
            {
                if (!parseStringFileInfoChild(bytes, offset))
                {
                    return false;
                }
            }
        }
        else
        {
            return false;
        }

        offset = alignUp(offset, sizeof(std::uint32_t));
        return true;
    }

    bool ResourceTable::parseVarFileInfoChild(const std::vector<std::uint8_t> &bytes, std::size_t &offset)
    {
        VersionInfoHeader var;
        if (bytes.size() < offset + var.structSize())
        {
            return false;
        }

        DynamicBuffer structContent(bytes);
        var.length = structContent.read<std::uint16_t>(offset); offset += sizeof(var.length);
        var.valueLength = structContent.read<std::uint16_t>(offset); offset += sizeof(var.valueLength);
        var.type = structContent.read<std::uint16_t>(offset); offset += sizeof(var.type);

        std::string key = unicodeToAscii(&bytes.data()[offset], bytes.size() - offset);
        if (key != "Translation")
        {
            return false;
        }

        offset += VAR_KEY_SIZE;
        offset = alignUp(offset, sizeof(std::uint32_t));
        if (bytes.size() < offset + var.valueLength || var.valueLength % sizeof(std::uint32_t))
        {
            return false;
        }

        for (std::size_t targetOffset = offset + var.valueLength; offset < targetOffset; )
        {
            std::uint32_t lang = structContent.read<uint32_t>(offset); offset += sizeof(lang);
            std::uint16_t lcid = lang & 0xFFFF;
            std::uint16_t codePage = lang >> 16;
            languages.emplace_back(std::make_pair(lcidToStr(lcid), codePageToStr(codePage)));
        }

        offset = alignUp(offset, sizeof(std::uint32_t));
        return true;
    }

    bool ResourceTable::parseStringFileInfoChild(const std::vector<std::uint8_t> &bytes, std::size_t &offset)
    {
        std::size_t origOffset = offset;
        VersionInfoHeader sfih;
        if (bytes.size() < offset + sfih.structSize())
        {
            return false;
        }

        DynamicBuffer structContent(bytes);
        sfih.length = structContent.read<std::uint16_t>(offset); offset += sizeof(sfih.length);
        sfih.valueLength = structContent.read<std::uint16_t>(offset); offset += sizeof(sfih.valueLength);
        sfih.type = structContent.read<std::uint16_t>(offset); offset += sizeof(sfih.type);

        std::size_t nRead;
        std::string key = unicodeToAscii(&bytes.data()[offset], bytes.size() - offset, nRead);
        if (nRead != STRTAB_KEY_SIZE)
        {
            return false;
        }

        offset += STRTAB_KEY_SIZE;
        offset = alignUp(offset, sizeof(std::uint32_t));

        for (std::size_t targetOffset = origOffset + sfih.length; offset < targetOffset; )
        {
            if (!parseVarString(bytes, offset))
            {
                return false;
            }
        }

        offset = alignUp(offset, sizeof(std::uint32_t));
        return true;
    }

    bool ResourceTable::parseVarString(const std::vector<std::uint8_t> &bytes, std::size_t &offset)
    {
        std::size_t origOffset = offset;
        VersionInfoHeader str;
        if (bytes.size() < offset + str.structSize())
        {
            return false;
        }

        DynamicBuffer structContent(bytes);
        str.length = structContent.read<std::uint16_t>(offset); offset += sizeof(str.length);
        str.valueLength = structContent.read<std::uint16_t>(offset); offset += sizeof(str.valueLength);
        str.type = structContent.read<std::uint16_t>(offset); offset += sizeof(str.type);

        if (bytes.size() < origOffset + str.length || str.length < str.structSize())
        {
            return false;
        }

        std::size_t targetOffset = alignUp(origOffset + str.length, sizeof(std::uint32_t));
        if (offset > targetOffset)
        {
            return false;
        }

        std::size_t nToRead = targetOffset - offset;
        std::size_t nRead;
        std::string name = unicodeToAscii(&bytes.data()[offset], nToRead, nRead);
        offset += nRead;
        offset = alignUp(offset, sizeof(std::uint32_t));
        if (offset > targetOffset)
        {
            return false;
        }

        nToRead = targetOffset - offset;
        std::string value;
        if (nToRead > 0)
            value = unicodeToAscii(&bytes.data()[offset], nToRead, nRead);

        offset = targetOffset;
        strings.emplace_back(std::make_pair(name, value));
        return true;
    }

    void ResourceTable::linkResourceIconGroups()
    {
        for (auto iconGroup : iconGroups)
        {
            std::size_t numberOfEntries;
            if (!iconGroup->getNumberOfEntries(numberOfEntries))
            {
                continue;
            }

            for (std::size_t eIndex = 0; eIndex < numberOfEntries; eIndex++)
            {
                std::size_t entryNameID;
                if (!iconGroup->getEntryNameID(eIndex, entryNameID))
                {
                    continue;
                }

                for (auto icon : icons)
                {
                    std::size_t iconNameID, iconSize;
                    unsigned short width, height;
                    uint16_t planes, bitCount;
                    uint8_t colorCount;
                    if (!icon->getNameId(iconNameID) || iconNameID != entryNameID
                        || !iconGroup->getEntryWidth(eIndex, width) || !iconGroup->getEntryHeight(eIndex, height)
                        || !iconGroup->getEntryIconSize(eIndex, iconSize) || !iconGroup->getEntryColorCount(eIndex, colorCount)
                        || !iconGroup->getEntryPlanes(eIndex, planes) || !iconGroup->getEntryBitCount(eIndex, bitCount))
                    {
                        continue;
                    }

                    icon->setWidth(width);
                    icon->setHeight(height);
                    icon->setIconSize(iconSize);
                    icon->setColorCount(colorCount);
                    icon->setPlanes(planes);
                    icon->setBitCount(bitCount);
                    icon->setIconGroup(iconGroup->getIconGroupID());
                    icon->setLoadedProperties();

                    if (colorCount == 1 << (bitCount * planes))
                    {
                        icon->setValidColorCount();
                    }

                    iconGroup->addIcon(icon);
                }
            }
        }
    }
} // namespace cchips
