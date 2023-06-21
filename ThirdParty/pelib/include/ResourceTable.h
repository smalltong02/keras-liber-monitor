#pragma once
#include "PeLib.h"

namespace cchips {

    class PeFormat;

    class ResourceTree
    {
    private:
        std::vector<std::vector<std::size_t>> tree; ///< resource tree structure
    public:
        ResourceTree() {
            tree.push_back(std::vector<std::size_t>());
            tree[0].push_back(0);
        }

        /// @name Queries
        /// @{
        bool isValidTree() const;
        /// @}

        /// @name Getters
        /// @{
        std::size_t getNumberOfLevels() const { return tree.size(); }
        std::size_t getNumberOfLevelsWithoutRoot() const { return tree.empty() ? 0 : tree.size() - 1; }
        std::size_t getNumberOfNodesInLevel(std::size_t level) const { return level < tree.size() ? tree[level].size() : 0; }
        std::size_t getNumberOfLeafs() const { return getNumberOfNodesInLevel(tree.size() - 1); }
        /// @}

        /// @name Other methods
        /// @{
        void addNode(std::size_t level, std::size_t childs);
        void dump(std::string &dumpTree) const { return; }
        /// @}
    };

    class Resource
    {
    private:
        std::string name;                  ///< resource name
        std::string type;                  ///< resource type
        std::string language;              ///< resource language
        std::string_view loadedBytes;      ///< reference to resource data
        std::vector<unsigned char> bytes;  ///resource data
        std::size_t offset = 0;            ///< offset in file
        std::size_t size = 0;              ///< size in file
        std::size_t nameId = 0;            ///< resource name identifier
        std::size_t typeId = 0;            ///< resource type identifier
        std::size_t languageId = 0;        ///< resource language identifier
        std::size_t sublanguageId = 0;     ///< resource sublanguage identifier
        bool nameIdIsValid = false;        ///< @c true if name ID is valid
        bool typeIdIsValid = false;        ///< @c true if type ID is valid
        bool languageIdIsValid = false;    ///< @c true if language ID is valid
        bool sublanguageIdIsValid = false; ///< @c true if sublanguage ID is valid
        bool loaded = false;               ///< @c true if content of resource was successfully loaded from input file
    public:
        /// @name Getters
        /// @{
        std::string getName() const { return name; }
        std::string getType() const { return type; }
        std::string getLanguage() const { return language; }
        const std::string_view getBytes(std::size_t sOffset = 0, std::size_t sSize = 0) const;
        std::size_t getOffset() const { return offset; }
        std::size_t getSizeInFile() const { return size; }
        std::size_t getLoadedSize() const { return bytes.size(); }
        bool getNameId(std::size_t &rId) const;
        bool getTypeId(std::size_t &rId) const;
        bool getLanguageId(std::size_t &rId) const;
        bool getSublanguageId(std::size_t &rId) const;
        /// @}

        /// @name Getters of resource content
        /// @{
        bool getBits(std::string &sResult) const;
        bool getBytes(std::vector<unsigned char> &sResult, std::size_t sOffset = 0, std::size_t sSize = 0) const;
        bool getString(std::string &sResult, std::size_t sOffset = 0, std::size_t sSize = 0) const;
        bool getHexBytes(std::string &sResult) const;
        /// @}

        /// @name Setters
        /// @{
        void setName(std::string rName) { name = rName; }
        void setType(std::string rType) { type = rType; }
        void setLanguage(std::string rLan) { language = rLan; }
        void setOffset(std::size_t rOffset) { offset = rOffset; }
        void setSizeInFile(std::size_t rSize) { size = rSize; }
        void setNameId(std::size_t rId) {
            nameId = rId;
            nameIdIsValid = true;
        }
        void setTypeId(std::size_t rId) {
            typeId = rId;
            typeIdIsValid = true;
        }
        void setLanguageId(std::size_t rId) {
            languageId = rId;
            languageIdIsValid = true;
        }
        void setSublanguageId(std::size_t rId) {
            sublanguageId = rId;
            sublanguageIdIsValid = true;
        }
        /// @}

        /// @name Other methods
        /// @{
        bool isLoaded() const { return loaded; }
        bool hasValidName() const { return !name.empty(); }
        bool hasValidId() const { return nameIdIsValid; }
        void invalidateNameId() { nameIdIsValid = false; }
        void invalidateTypeId() { typeIdIsValid = false; }
        void invalidateLanguageId() { languageIdIsValid = false; }
        void invalidateSublanguageId() { sublanguageIdIsValid = false; }
        void load(const PeFormat *rOwner);
        bool hasEmptyName() const { return name.empty(); }
        bool hasEmptyType() const { return type.empty(); }
        bool hasEmptyLanguage() const { return language.empty(); }
        /// @}
    };

    class ResourceIcon : public Resource
    {
    private:
        std::uint16_t width = 0;       ///< icon width
        std::uint16_t height = 0;      ///< icon height
        std::size_t iconSize = 0;      ///< icon size in file
        std::uint8_t colorCount = 0;   ///< icon color count
        std::uint16_t planes = 0;      ///< icon planes
        std::uint16_t bitCount = 0;    ///< icon bit count
        std::size_t iconGroup = 0;     ///< icon group the icon belongs to
        bool loadedProperties = false; ///< @c true if properties were successfully loaded from icon group resource
        bool validColorCount = false;  ///< @c true if color count has a valid value

    public:
        /// @name Getters
        /// @{
        std::uint16_t getWidth() const { return width; }
        std::uint16_t getHeight() const { return height; }
        std::size_t getIconSize() const { return iconSize; }
        std::uint8_t getColorCount() const { return colorCount; }
        std::uint16_t getPlanes() const { return planes; }
        std::uint16_t getBitCount() const { return bitCount; }
        std::size_t getIconGroup() const { return iconGroup; }
        /// @}

        /// @name Setters
        /// @{
        void setWidth(std::uint16_t iWidth) {
            if (width == 0)
            {
                width = 256;
            }

            width = iWidth;
        }
        void setHeight(std::uint16_t iHeight) {
            if (height == 0)
            {
                height = 256;
            }

            height = iHeight;
        }
        void setIconSize(std::size_t iSize) { iconSize = iSize; }
        void setColorCount(std::uint8_t iColorCount) { colorCount = iColorCount; }
        void setPlanes(std::uint16_t iPlanes) { planes = iPlanes; }
        void setBitCount(std::uint16_t iBitCount) { bitCount = iBitCount; }
        void setIconGroup(std::size_t iGroup) { iconGroup = iGroup; }
        void setLoadedProperties() { loadedProperties = true; }
        void setValidColorCount() { validColorCount = true; }
        /// @}

        /// @name Other methods
        /// @{
        bool hasLoadedProperties() const { return loadedProperties; }
        bool hasValidColorCount() const { return validColorCount; }
        /// @}
    };

    class ResourceIconGroup : public Resource
    {
    private:
        std::vector<ResourceIcon *> icons;  ///< stored icons
        size_t iconGroupID = 0;             ///< icon group id

        /// @name Auxiliary methods
        /// @{
        std::size_t getEntryOffset(std::size_t eIndex) const { return 6 + eIndex * 14; }
        /// @}
        static bool ResourceIconGroup::iconCompare(const ResourceIcon *i1, const ResourceIcon *i2);

        static const std::pair<uint16_t, uint16_t> iconPriorities[16];
    public:
        /// @name Getters
        /// @{
        std::size_t getNumberOfIcons() const { return icons.size(); }
        const ResourceIcon *getIcon(std::size_t iIndex) const {
            if (iIndex >= icons.size())
            {
                return nullptr;
            }

            return icons[iIndex];
        }
        const ResourceIcon *getPriorIcon() const;
        std::size_t getIconGroupID() const { return iconGroupID; }
        /// @}

        /// @name Getters of icon group content
        /// @{
        bool getNumberOfEntries(std::size_t &nEntries) const {
            std::vector<unsigned char> bytes;

            if (!getBytes(bytes, 4, 2))
            {
                return false;
            }

            nEntries = bytes[1] << 8 | bytes[0];

            return true;
        }

        bool getEntryNameID(std::size_t eIndex, std::size_t &nameID) const {
            std::vector<unsigned char> bytes;

            if (!getBytes(bytes, getEntryOffset(eIndex) + 12, 2))
            {
                return false;
            }

            nameID = bytes[1] << 8 | bytes[0];

            return true;
        }
        bool getEntryWidth(std::size_t eIndex, std::uint16_t &width) const {
            std::vector<unsigned char> bytes;

            if (!getBytes(bytes, getEntryOffset(eIndex) + 0, 1))
            {
                return false;
            }

            width = bytes[0];

            return true;
        }
        bool getEntryHeight(std::size_t eIndex, std::uint16_t &height) const {
            std::vector<unsigned char> bytes;

            if (!getBytes(bytes, getEntryOffset(eIndex) + 1, 1))
            {
                return false;
            }

            height = bytes[0];

            return true;
        }
        bool getEntryIconSize(std::size_t eIndex, std::size_t &iconSize) const {
            std::vector<unsigned char> bytes;

            if (!getBytes(bytes, getEntryOffset(eIndex) + 8, 4))
            {
                return false;
            }

            iconSize = bytes[3] << 24 | bytes[2] << 16 | bytes[1] << 8 | bytes[0];

            return true;
        }
        bool getEntryColorCount(std::size_t eIndex, std::uint8_t &colorCount) const {
            std::vector<unsigned char> bytes;

            if (!getBytes(bytes, getEntryOffset(eIndex) + 2, 1))
            {
                return false;
            }

            colorCount = bytes[0];

            return true;
        }
        bool getEntryPlanes(std::size_t eIndex, std::uint16_t &planes) const {
            std::vector<unsigned char> bytes;

            if (!getBytes(bytes, getEntryOffset(eIndex) + 4, 2))
            {
                return false;
            }

            planes = bytes[1] << 8 | bytes[0];

            return true;
        }
        bool getEntryBitCount(std::size_t eIndex, std::uint16_t &bitCount) const {
            std::vector<unsigned char> bytes;

            if (!getBytes(bytes, getEntryOffset(eIndex) + 6, 2))
            {
                return false;
            }

            bitCount = bytes[1] << 8 | bytes[0];

            return true;
        }
        /// @}

        /// @name Setters
        /// @{
        void setIconGroupID(std::size_t id) { iconGroupID = id; }
        /// @}

        /// @name Other methods
        /// @{
        bool hasIcons() const { return !icons.empty(); }
        void addIcon(ResourceIcon *icon) { icons.push_back(icon); }
        /// @}
    };

    class ResourceTable
    {
    private:
        using resourcesIterator = std::vector<std::unique_ptr<Resource>>::const_iterator;
        std::vector<std::unique_ptr<Resource>> table;                ///< stored resources
        std::vector<Resource *> resourceVersions;             ///< version info resources
        std::vector<ResourceIconGroup *> iconGroups;                 ///< icon groups
        std::vector<ResourceIcon *> icons;                           ///< icons
        std::vector<std::pair<std::string, std::string>> languages;  ///< supported languages, LCID and code page
        std::vector<std::pair<std::string, std::string>> strings;    ///< version info strings

        bool parseVersionInfo(const std::vector<std::uint8_t> &bytes);
        bool parseVersionInfoChild(const std::vector<std::uint8_t> &bytes, std::size_t &offset);
        bool parseVarFileInfoChild(const std::vector<std::uint8_t> &bytes, std::size_t &offset);
        bool parseStringFileInfoChild(const std::vector<std::uint8_t> &bytes, std::size_t &offset);
        bool parseVarString(const std::vector<std::uint8_t> &bytes, std::size_t &offset);

        static const std::size_t VI_KEY_SIZE = 32;               ///< unicode "VS_VERSION_INFO"
        static const std::size_t VFI_KEY_SIZE = 24;              ///< unicode "VarFileInfo"
        static const std::size_t SFI_KEY_SIZE = 30;              ///< unicode "StringFileInfo"
        static const std::size_t VAR_KEY_SIZE = 24;              ///< unicode "Translation"
        static const std::size_t STRTAB_KEY_SIZE = 18;           ///< 8 unicode hex digits
        static const std::uint32_t FFI_SIGNATURE = 0xFEEF04BD;   ///< fixed file info signature
    public:
        struct VersionInfoHeader
        {
            std::uint16_t length;                       ///< length of whole structure
            std::uint16_t valueLength;                  ///< length of following structure
            std::uint16_t type;                         ///< type of data

            static std::size_t structSize()
            {
                return sizeof(length) + sizeof(valueLength) + sizeof(type);
            }
        };
        struct FixedFileInfo
        {
            std::uint32_t signature;                   ///< signature FFI_SIGNATURE
            std::uint16_t strucVersionMaj;             ///< binary major version number
            std::uint16_t strucVersionMin;             ///< binary minor version number
            std::uint32_t fileVersionMaj;              ///< file major version number
            std::uint32_t fileVersionMin;              ///< file minor version number
            std::uint64_t productVersion;              ///< product version number
            std::uint32_t fileFlagsMask;               ///< validity mask of fileFalgs member
            std::uint32_t fileFlags;                   ///< file flags
            std::uint32_t fileOS;                      ///< target operating system
            std::uint32_t fileType;                    ///< type of file
            std::uint32_t fileSubtype;                 ///< subtype of file
            std::uint64_t timestamp;                   ///< timestamp

            static std::size_t structSize()
            {
                return
                    sizeof(signature) + sizeof(strucVersionMaj) + sizeof(strucVersionMin) + sizeof(fileVersionMaj) +
                    sizeof(fileVersionMin) + sizeof(productVersion) + sizeof(fileFlagsMask) + sizeof(fileFlags) +
                    sizeof(fileOS) + sizeof(fileType) + sizeof(fileSubtype) + sizeof(timestamp);
            }
        };
        /// @name Getters
        /// @{
        std::size_t getNumberOfResources() const { return table.size(); }
        std::size_t getNumberOfLanguages() const { return languages.size(); }
        std::size_t getNumberOfStrings() const { return strings.size(); }
        std::size_t getSizeInFile() const {
            std::size_t sum = 0;

            for (const auto &r : table)
            {
                sum += r->getSizeInFile();
            }

            return sum;
        }
        std::size_t getLoadedSize() const {
            std::size_t sum = 0;

            for (const auto &r : table)
            {
                sum += r->getLoadedSize();
            }

            return sum;
        }
        const Resource* getResource(std::size_t rIndex) const { return (rIndex < getNumberOfResources()) ? table[rIndex].get() : nullptr; }
        const std::pair<std::string, std::string>* getLanguage(std::size_t rIndex) const { return (rIndex < getNumberOfLanguages()) ? &languages[rIndex] : nullptr; }
        const std::pair<std::string, std::string>* getString(std::size_t rIndex) const { return (rIndex < getNumberOfStrings()) ? &strings[rIndex] : nullptr; }
        const Resource* getResourceWithName(const std::string &rName) const {
            for (const auto &r : table)
            {
                if (r->getName() == rName)
                {
                    return r.get();
                }
            }

            return nullptr;
        }
        const Resource* getResourceWithName(std::size_t rId) const {
            std::size_t tmpId;

            for (const auto &r : table)
            {
                if (r->getNameId(tmpId) && tmpId == rId)
                {
                    return r.get();
                }
            }

            return nullptr;
        }
        const Resource* getResourceWithType(const std::string &rType) const {
            for (const auto &r : table)
            {
                if (r->getType() == rType)
                {
                    return r.get();
                }
            }

            return nullptr;
        }
        const Resource* getResourceWithType(std::size_t rId) const {
            std::size_t tmpId;

            for (const auto &r : table)
            {
                if (r->getTypeId(tmpId) && tmpId == rId)
                {
                    return r.get();
                }
            }

            return nullptr;
        }
        const Resource* getResourceWithLanguage(const std::string &rLan) const {
            for (const auto &r : table)
            {
                if (r->getLanguage() == rLan)
                {
                    return r.get();
                }
            }

            return nullptr;
        }
        const Resource* getResourceWithLanguage(std::size_t rId) const {
            std::size_t tmpId;

            for (const auto &r : table)
            {
                if (r->getLanguageId(tmpId) && tmpId == rId)
                {
                    return r.get();
                }
            }

            return nullptr;
        }
        const ResourceIconGroup* getPriorResourceIconGroup() const {
            for (const auto group : iconGroups)
            {
                if (group->getIconGroupID() == 0)
                {
                    return group;
                }
            }

            return nullptr;
        }
        /// @}

        /// @name Iterators
        /// @{
        resourcesIterator begin() const { return table.begin(); }
        resourcesIterator end() const { return table.end(); }
        /// @}

        /// @name Other methods
        /// @{
        void parseVersionInfoResources();
        void clear() { table.clear(); }
        void addResource(std::unique_ptr<Resource>&& newResource) { table.push_back(std::move(newResource)); }
        void addResourceVersion(Resource *ver) { resourceVersions.push_back(ver); }
        void addResourceIcon(ResourceIcon *icon) { icons.push_back(icon); }
        void addResourceIconGroup(ResourceIconGroup *iGroup) { iconGroups.push_back(iGroup); }
        void linkResourceIconGroups();
        bool hasResources() const { return !table.empty(); }
        bool hasResourceWithName(const std::string &rName) const { return getResourceWithName(rName); }
        bool hasResourceWithName(std::size_t rId) const { return getResourceWithName(rId); }
        bool hasResourceWithType(const std::string &rType) const { return getResourceWithType(rType); }
        bool hasResourceWithType(std::size_t rId) const { return getResourceWithType(rId); }
        bool hasResourceWithLanguage(const std::string &rLan) const { return getResourceWithLanguage(rLan); }
        bool hasResourceWithLanguage(std::size_t rId) const { return getResourceWithLanguage(rId); }
        void dump(std::string &dumpTable) const { return; }
        /// @}
    };
} // namespace cchips
