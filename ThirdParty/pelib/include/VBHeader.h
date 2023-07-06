#ifndef FILEFORMAT_TYPES_VISUAL_BASIC_VISUAL_BASIC_STRUCTURES_H
#define FILEFORMAT_TYPES_VISUAL_BASIC_VISUAL_BASIC_STRUCTURES_H

#include <string>
#include "..\StaticPEManager\InfoBuilder\PeInside\stringutils.h"

namespace cchips {

	constexpr std::size_t VBHEADER_SIGNATURE = 0x21354256;
	constexpr std::size_t VB_MAX_STRING_LEN = 100;

	enum class VBExternTableEntryType
	{
		internal = 0x6,
		external = 0x7
	};

	struct VBHeader
	{
		std::uint32_t signature;                 ///< "VB5!" signature
		std::uint16_t runtimeBuild;              ///< runtime flag
		std::uint8_t languageDLL[14];            ///< language DLL
		std::uint8_t backupLanguageDLL[14];      ///< backup language DLL
		std::uint16_t runtimeDLLVersion;         ///< version of the runtime DLL
		std::uint32_t LCID1;                     ///< primary LCID
		std::uint32_t LCID2;                     ///< backup LCID
		std::uint32_t subMainAddr;               ///< sub main code address
		std::uint32_t projectInfoAddr;           ///< ProjectInfo address
		std::uint32_t MDLIntObjsFlags;           ///< VB controll flags for IDs < 32
		std::uint32_t MDLIntObjsFlags2;          ///< VB controll flags for IDs > 32
		std::uint32_t threadFlags;               ///< thread flags
		std::uint32_t nThreads;                  ///< number of threads to support in pool
		std::uint16_t nForms;                    ///< number of forms in this application
		std::uint16_t nExternals;                ///< number of external OCX components
		std::uint32_t nThunks;                   ///< number of thunks to create
		std::uint32_t GUITableAddr;              ///< GUITable address
		std::uint32_t externalTableAddr;         ///< ExternalTable address
		std::uint32_t COMRegisterDataAddr;       ///< ComRegisterData address
		std::uint32_t projExeNameOffset;         ///< offset to the string containing EXE filename
		std::uint32_t projDescOffset;            ///< offset to the string containing project's description
		std::uint32_t helpFileOffset;            ///< offset to the string containing name of the Help file
		std::uint32_t projNameOffset;            ///< offset to the string containing project's name

		static std::size_t structureSize()
		{
			return
				sizeof(signature) + sizeof(runtimeBuild) + sizeof(languageDLL) +
				sizeof(backupLanguageDLL) + sizeof(runtimeDLLVersion) + sizeof(LCID1) +
				sizeof(LCID2) + sizeof(subMainAddr) + sizeof(projectInfoAddr) +
				sizeof(MDLIntObjsFlags) + sizeof(MDLIntObjsFlags2) + sizeof(threadFlags) +
				sizeof(nThreads) + sizeof(nForms) + sizeof(nExternals) +
				sizeof(nThunks) + sizeof(GUITableAddr) + sizeof(externalTableAddr) +
				sizeof(COMRegisterDataAddr) + sizeof(projExeNameOffset) + sizeof(projDescOffset) +
				sizeof(helpFileOffset) + sizeof(projNameOffset);
		}
	};

	struct VBProjInfo
	{
		std::uint32_t version;                        ///< 5.00 in Hex (0x1F4), version
		std::uint32_t objectTableAddr;                ///< Object table address
		std::uint32_t null;                           ///< unused value after compilation
		std::uint32_t codeStartAddr;                  ///< etart of code address
		std::uint32_t codeEndAddr;                    ///< end of code address
		std::uint32_t dataSize;                       ///< size of VB object structures
		std::uint32_t threadSpaceAddr;                ///< eddress of address of thread object
		std::uint32_t exHandlerAddr;                  ///< VBA exception handler address
		std::uint32_t nativeCodeAddr;                 ///< .DATA section address
		std::uint8_t pathInformation[528];            ///< path and id string, <SP6
		std::uint32_t externalTableAddr;              ///< External table address
		std::uint32_t nExternals;                     ///< number of external OCX components

		static std::size_t structureSize()
		{
			return
				sizeof(version) + sizeof(objectTableAddr) + sizeof(null)
				+ sizeof(codeStartAddr) + sizeof(codeEndAddr) + sizeof(dataSize)
				+ sizeof(threadSpaceAddr) + sizeof(exHandlerAddr) + sizeof(nativeCodeAddr)
				+ sizeof(pathInformation) + sizeof(externalTableAddr) + sizeof(nExternals);
		}
	};

	struct VBObjectTable
	{
		std::uint32_t null1;                          ///< null
		std::uint32_t execCOMAddr;                    ///< VB Project Exec COM address
		std::uint32_t projecInfo2Addr;                ///< Project Info 2 address
		std::uint32_t reserved;                       ///< reserved
		std::uint32_t null2;                          ///< null
		std::uint32_t projectObjectAddr;              ///< In-memory data address
		std::uint8_t objectGUID[16];                  ///< Object table GUID
		std::uint16_t flagsCompileState;              ///< internal compilation flags
		std::uint16_t nObjects;                       ///< number of objects
		std::uint16_t nCompiledObjects;               ///< number of compiled objects
		std::uint16_t nUsedObjects;                   ///< number of used objects
		std::uint32_t objectDescriptorsAddr;          ///< Object descriptos address
		std::uint32_t IDE1;                           ///< IDE1 internals
		std::uint32_t IDE2;                           ///< IDE2 internals
		std::uint32_t IDE3;                           ///< IDE3 internals
		std::uint32_t projectNameAddr;                ///< project name address
		std::uint32_t LCID1;                          ///< LCID
		std::uint32_t LCID2;                          ///< backup LCID
		std::uint32_t IDE4;                           ///< IDE4 internals
		std::uint32_t templateVesion;                 ///< template version

		static std::size_t structureSize()
		{
			return
				sizeof(null1) + sizeof(execCOMAddr) + sizeof(projecInfo2Addr) + sizeof(reserved) +
				sizeof(null2) + sizeof(projectObjectAddr) + sizeof(objectGUID) + sizeof(flagsCompileState) +
				sizeof(nObjects) + sizeof(nCompiledObjects) + sizeof(nUsedObjects) +
				sizeof(objectDescriptorsAddr) + sizeof(IDE1) + sizeof(IDE2) + sizeof(IDE3) +
				sizeof(projectNameAddr) + sizeof(LCID1) + sizeof(LCID2) + sizeof(IDE4) +
				sizeof(templateVesion);
		}
	};

	struct VBPublicObjectDescriptor
	{
		std::uint32_t objectInfoAddr;                 ///< Object info of this object address
		std::uint32_t reserved;                       ///< reserved
		std::uint32_t publicBytesAddr;                ///< public integers address
		std::uint32_t staticBytesAddr;                ///< static integers address
		std::uint32_t modulePublicAddr;               ///< public DATA section address
		std::uint32_t moduleStaticAddr;               ///< static DATA section address
		std::uint32_t objectNameAddr;                 ///< object name address
		std::uint32_t nMethods;                       ///< number of methods
		std::uint32_t methodNamesAddr;                ///< method names array address
		std::uint32_t staticVarsCopyAddr;             ///< static variables copy destionation offset
		std::uint32_t objectType;                     ///< object type flags
		std::uint32_t null;                           ///< null

		static std::size_t structureSize()
		{
			return
				sizeof(objectInfoAddr) + sizeof(reserved) + sizeof(publicBytesAddr) +
				sizeof(staticBytesAddr) + sizeof(modulePublicAddr) + sizeof(moduleStaticAddr) +
				sizeof(objectNameAddr) + sizeof(nMethods) + sizeof(methodNamesAddr) +
				sizeof(staticVarsCopyAddr) + sizeof(objectType) + sizeof(null);
		}
	};

	struct VBExternTableEntry
	{
		std::uint32_t type;                      ///< import type
		std::uint32_t importDataAddr;            ///< import data address

		static std::size_t structureSize()
		{
			return sizeof(type) + sizeof(importDataAddr);
		}
	};

	struct VBExternTableEntryData
	{
		std::uint32_t moduleNameAddr;            ///< mode name address
		std::uint32_t apiNameAddr;               ///< api name address

		static std::size_t structureSize()
		{
			return sizeof(moduleNameAddr) + sizeof(apiNameAddr);
		}
	};

	struct VBCOMRData
	{
		std::uint32_t regInfoOffset;             ///< Registration Info offset
		std::uint32_t projNameOffset;            ///< Project/TypeLib name offset
		std::uint32_t helpFileOffset;            ///< Help file offset
		std::uint32_t projDescOffset;            ///< Project description offset
		std::uint8_t projCLSID[16];              ///< Project/TypeLib CLSID
		std::uint32_t projTlbLCID;               ///< TypeLib library LCID
		std::uint16_t unknown;                   ///< unknown
		std::uint16_t tlbVerMajor;               ///< TypeLib major version
		std::uint16_t tlbVerMinor;               ///< TypeLib minor version

		static std::size_t structureSize()
		{
			return
				sizeof(regInfoOffset) + sizeof(projNameOffset) + sizeof(helpFileOffset) +
				sizeof(projDescOffset) + sizeof(projCLSID) + sizeof(projTlbLCID) +
				sizeof(unknown) + sizeof(tlbVerMajor) + sizeof(tlbVerMinor);
		}
	};

	struct VBCOMRInfo
	{
		std::uint32_t ifInfoOffset;              ///< Offset to COM Interface Info
		std::uint32_t objNameOffset;             ///< Offset to object name
		std::uint32_t objDescOffset;             ///< Offset to object description
		std::uint32_t instancing;                ///< Instancing mode
		std::uint32_t objID;                     ///< Object ID within project
		std::uint8_t objCLSID[16];               ///< Object CLSID
		std::uint32_t isInterfaceFlag;           ///< Specifies whether Interface CLSID is valid
		std::uint32_t ifCLSIDOffset;             ///< Interface CLSID
		std::uint32_t eventCLSIDOffset;          ///< Event CLSID
		std::uint32_t hasEvents;                 ///< Specifies whether Event CLSID is valid
		std::uint32_t olemicsFlags;              ///< Status
		std::uint8_t classType;                  ///< Class Type
		std::uint8_t objectType;                 ///< Object Type
		std::uint16_t toolboxBitmap32;           ///< Control Bitmap ID in toobox
		std::uint16_t defaultIcon;               ///< Minimized icon of control window
		std::uint16_t isDesignerFlag;            ///< Specifies whether Designed Data offset is valid
		std::uint32_t designerDataOffset;        ///< Offset to Designed Data

		static std::size_t structureSize()
		{
			return
				sizeof(ifInfoOffset) + sizeof(objNameOffset) + sizeof(objDescOffset) +
				sizeof(instancing) + sizeof(objID) + sizeof(objCLSID) + sizeof(isInterfaceFlag) +
				sizeof(ifCLSIDOffset) + sizeof(eventCLSIDOffset) + sizeof(hasEvents) +
				sizeof(olemicsFlags) + sizeof(classType) + sizeof(objectType) +
				sizeof(toolboxBitmap32) + sizeof(defaultIcon) + sizeof(isDesignerFlag) +
				sizeof(designerDataOffset);
		}
	};

	class VisualBasicObject
	{
	private:
		std::string name;
		std::vector<std::string> methods;
	public:
		/// @name Getters
		/// @{
		const std::string& getName() const { return name; }
		const std::vector<std::string>& getMethods() const { return methods; }
		std::size_t getNumberOfMethods() const { return methods.size(); }
		/// @}

		/// @name Setters
		/// @{
		void setName(const std::string& n) { name = n; }
		/// @}

		/// @name Other methods
		/// @{
		void addMethod(const std::string& method) { methods.push_back(method); }
		/// @}
	};

	class VisualBasicExtern
	{
	private:
		std::string moduleName;
		std::string apiName;
	public:
		/// @name Getters
		/// @{
		const std::string& getModuleName() const { return moduleName; }
		const std::string& getApiName() const { return apiName; }
		/// @}

		/// @name Setters
		/// @{
		void setModuleName(const std::string& mName) { moduleName = mName; }
		void setApiName(const std::string& aName) { apiName = aName; }
		/// @}
	};

	class VisualBasicInfo
	{
	private:
		// VB Header
		std::string languageDLL;
		std::string backupLanguageDLL;
		std::string projectExeName;
		std::string projectDescription;
		std::string projectHelpFile;
		std::string projectName;
		std::uint32_t languageDLLPrimaryLCID = 0;
		std::uint32_t languageDLLSecondaryLCID = 0;

		// VB Proj Info
		std::string projectPath;

		// VB Object table
		std::string objectTableGUID;
		std::uint32_t projectPrimaryLCID = 0;
		std::uint32_t projectSecondaryLCID = 0;
		std::vector<std::unique_ptr<VisualBasicObject>> objects;

		// VB extern table
		std::vector<std::unique_ptr<VisualBasicExtern>> externs;

		// COM register data
		std::string typeLibCLSID;
		std::uint32_t typeLibLCID = 0;
		std::uint16_t typeLibMajorVersion = 0;
		std::uint16_t typeLibMinorVersion = 0;

		// COM register info
		std::string COMObjectName;
		std::string COMObjectDescription;
		std::string COMObjectCLSID;
		std::string COMObjectInterfaceCLSID;
		std::string COMObjectEventsCLSID;
		std::string COMObjectType;

		// others
		std::string externTableHashCrc32;
		std::string externTableHashMd5;
		std::string externTableHashSha256;
		std::string objectTableHashCrc32;
		std::string objectTableHashMd5;
		std::string objectTableHashSha256;

		bool validLanguageDLLPrimaryLCID = false;
		bool validLanguageDLLSecondaryLCID = false;
		bool validProjectPrimaryLCID = false;
		bool validProjectSecondaryLCID = false;
		bool validTypeLibLCID = false;
		bool validTypeLibMajorVersion = false;
		bool validTypeLibMinorVersion = false;
		bool pcodeFlag = false;

		std::string guidToStr(const std::uint8_t data[16]) {
			std::string r1, r2, r3, r4, r5;
			bytesToHexString(data, 16, r1, 0, 4);
			bytesToHexString(data, 16, r2, 4, 2);
			bytesToHexString(data, 16, r3, 6, 2);
			bytesToHexString(data, 16, r4, 8, 2);
			bytesToHexString(data, 16, r5, 10, 6);

			return r1 + "-" + r2 + "-" + r3 + "-" + r4 + "-" + r5;
		}

	public:
		/// @name Getters
		/// @{
		const std::string& getLanguageDLL() const { return languageDLL; }
		const std::string& getBackupLanguageDLL() const { return backupLanguageDLL; }
		const std::string& getProjectExeName() const { return projectExeName; }
		const std::string& getProjectDescription() const { return projectDescription; }
		const std::string& getProjectHelpFile() const { return projectHelpFile; }
		const std::string& getProjectName() const { return projectName; }
		bool getLanguageDLLPrimaryLCID(std::uint32_t& res) const {
			if (!validLanguageDLLPrimaryLCID)
			{
				return false;
			}
			res = languageDLLPrimaryLCID;
			return true;
		}
		bool getLanguageDLLSecondaryLCID(std::uint32_t& res) const {
			if (!validLanguageDLLSecondaryLCID)
			{
				return false;
			}
			res = languageDLLSecondaryLCID;
			return true;
		}
		const std::string& getProjectPath() const { return projectPath; }
		bool getProjectPrimaryLCID(std::uint32_t& res) const {
			if (!validProjectPrimaryLCID)
			{
				return false;
			}
			res = projectPrimaryLCID;
			return true;
		}
		bool getProjectSecondaryLCID(std::uint32_t& res) const {
			if (!validProjectSecondaryLCID)
			{
				return false;
			}
			res = projectSecondaryLCID;
			return true;
		}
		const std::vector<std::unique_ptr<VisualBasicObject>>& getObjects() const { return objects; }
		const std::vector<std::unique_ptr<VisualBasicExtern>>& getExterns() const { return externs; }
		const VisualBasicObject* getObject(std::size_t position) const { return (position < objects.size()) ? objects[position].get() : nullptr; }
		const VisualBasicExtern* getExtern(std::size_t position) const { return (position < externs.size()) ? externs[position].get() : nullptr; }
		std::size_t getNumberOfObjects() const { return objects.size(); }
		std::size_t getNumberOfExterns() const { return externs.size(); }
		const std::string& getObjectTableGUID() const { return objectTableGUID; }
		const std::string& getTypeLibCLSID() const { return typeLibCLSID; }
		bool getTypeLibLCID(std::uint32_t& res) const {
			if (!validTypeLibLCID)
			{
				return false;
			}
			res = typeLibLCID;
			return true;
		}
		bool getTypeLibMajorVersion(std::uint16_t& res) const {
			if (!validTypeLibMajorVersion)
			{
				return false;
			}
			res = typeLibMajorVersion;
			return true;
		}
		bool getTypeLibMinorVersion(std::uint16_t& res) const {
			if (!validTypeLibMinorVersion)
			{
				return false;
			}
			res = typeLibMinorVersion;
			return true;
		}
		const std::string& getCOMObjectName() const { return COMObjectName; }
		const std::string& getCOMObjectDescription() const { return COMObjectDescription; }
		const std::string& getCOMObjectCLSID() const { return COMObjectCLSID; }
		const std::string& getCOMObjectInterfaceCLSID() const { return COMObjectInterfaceCLSID; }
		const std::string& getCOMObjectEventsCLSID() const { return COMObjectEventsCLSID; }
		const std::string& getCOMObjectType() const { return COMObjectType; }
		const std::string& getExternTableHashCrc32() const { return externTableHashCrc32; }
		const std::string& getExternTableHashMd5() const { return externTableHashMd5; }
		const std::string& getExternTableHashSha256() const { return externTableHashSha256; }
		const std::string& getObjectTableHashCrc32() const { return objectTableHashCrc32; }
		const std::string& getObjectTableHashMd5() const { return objectTableHashMd5; }
		const std::string& getObjectTableHashSha256() const { return objectTableHashSha256; }
		/// @}

		/// @name Setters
		/// @{
		void setLanguageDLL(const std::string& lDll) { languageDLL = lDll; }
		void setBackupLanguageDLL(const std::string& blDll) { backupLanguageDLL = blDll; }
		void setProjectExeName(const std::string& exeName) { projectExeName = exeName; }
		void setProjectDescription(const std::string& desc) { projectDescription = desc; }
		void setProjectHelpFile(const std::string& helpFile) { projectHelpFile = helpFile; }
		void setProjectName(const std::string& name) { projectName = name; }
		void setLanguageDLLPrimaryLCID(std::uint32_t lDllPrimLCID) {
			languageDLLPrimaryLCID = lDllPrimLCID;
			validLanguageDLLPrimaryLCID = true;
		}
		void setLanguageDLLSecondaryLCID(std::uint32_t lDllSecLCID) {
			languageDLLSecondaryLCID = lDllSecLCID;
			validLanguageDLLSecondaryLCID = true;
		}
		void setProjectPath(const std::string& path) {
			const std::string prefix = "*\\A";

			if (prefix.size() > path.size())
			{
				projectPath = path;
			}
			else
			{
				auto res = std::mismatch(prefix.begin(), prefix.end(), path.begin());
				if (res.first == prefix.end())
				{
					projectPath = path.substr(prefix.size(), path.size() - prefix.size());
				}
				else
				{
					projectPath = path;
				}
			}
		}
		void setProjectPrimaryLCID(std::uint32_t primLCID) {
			projectPrimaryLCID = primLCID;
			validProjectPrimaryLCID = true;
		}
		void setProjectSecondaryLCID(std::uint32_t secLCID) {
			projectSecondaryLCID = secLCID;
			validProjectSecondaryLCID = true;
		}
		void setTypeLibCLSID(const std::uint8_t data[16]) { typeLibCLSID = guidToStr(data); }
		void setTypeLibLCID(std::uint32_t tlbLCID) {
			typeLibLCID = tlbLCID;
			validTypeLibLCID = true;
		}
		void setPcode(bool set) { pcodeFlag = set; }
		void setObjectTableGUID(const std::uint8_t data[16]) { objectTableGUID = guidToStr(data); }
		void setTypeLibMajorVersion(std::uint16_t majVer) {
			typeLibMajorVersion = majVer;
			validTypeLibMajorVersion = true;
		}
		void setTypeLibMinorVersion(std::uint16_t minVer) {
			typeLibMinorVersion = minVer;
			validTypeLibMinorVersion = true;
		}
		void setCOMObjectName(const std::string& name) { COMObjectName = name; }
		void setCOMObjectDescription(const std::string& description) { COMObjectDescription = description; }
		void setCOMObjectCLSID(const std::uint8_t data[16]) { COMObjectCLSID = guidToStr(data); }
		void setCOMObjectInterfaceCLSID(const std::uint8_t data[16]) { COMObjectInterfaceCLSID = guidToStr(data); }
		void setCOMObjectEventsCLSID(const std::uint8_t data[16]) { COMObjectEventsCLSID = guidToStr(data); }
		void setCOMObjectType(std::uint8_t type) {
			switch (type)
			{
			case 0x2: COMObjectType = "Designer"; break;
			case 0x10: COMObjectType = "ClassModule"; break;
			case 0x20: COMObjectType = "ActiveXUserControl"; break;
			case 0x80: COMObjectType = "UserDocument"; break;
			default: COMObjectType = "unknown"; break;
			}
		}
		/// @}

		/// @name Other methods
		/// @{
		void addObject(std::unique_ptr<VisualBasicObject>&& obj) { objects.push_back(std::move(obj)); }
		void addExtern(std::unique_ptr<VisualBasicExtern>&& ext) { externs.push_back(std::move(ext)); }
		bool hasProjectName() const { return !projectName.empty(); }
		bool hasProjectDescription() const { return !projectDescription.empty(); }
		bool hasProjectHelpFile() const { return !projectHelpFile.empty(); }
		bool isPcode() const { return pcodeFlag; }
		void computeExternTableHashes() {
			std::vector<std::uint8_t> hashBytes;

			for (const auto& ext : externs)
			{
				auto moduleName = toLower(ext->getModuleName());
				auto apiName = toLower(ext->getApiName());

				if (apiName.empty() || moduleName.empty())
				{
					continue;
				}

				if (!hashBytes.empty())
				{
					hashBytes.push_back(static_cast<unsigned char>(','));
				}

				for (const auto c : std::string(apiName + "." + moduleName))
				{
					hashBytes.push_back(static_cast<unsigned char>(c));
				}
			}

			//externTableHashCrc32 = getCrc32(hashBytes.data(), hashBytes.size());
			//externTableHashMd5 = getMd5(hashBytes.data(), hashBytes.size());
			//externTableHashSha256 = getSha256(hashBytes.data(), hashBytes.size());
		}
		void computeObjectTableHashes() {
			std::vector<std::uint8_t> hashBytes;

			for (const auto& obj : objects)
			{
				auto objName = toLower(obj->getName());
				if (objName.empty())
				{
					continue;
				}

				std::string methods = "";
				for (const auto& method : obj->getMethods())
				{
					if (!methods.empty())
					{
						methods.push_back('.');
					}

					methods += method;
				}

				if (!hashBytes.empty())
				{
					hashBytes.push_back(static_cast<unsigned char>(','));
				}

				for (const auto c : std::string(objName + "." + methods))
				{
					hashBytes.push_back(static_cast<unsigned char>(c));
				}
			}

			//objectTableHashCrc32 = getCrc32(hashBytes.data(), hashBytes.size());
			//objectTableHashMd5 = getMd5(hashBytes.data(), hashBytes.size());
			//objectTableHashSha256 = getSha256(hashBytes.data(), hashBytes.size());
		}
		/// @}
	};
} // namespace cchips

#endif
