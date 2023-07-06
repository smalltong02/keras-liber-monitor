#ifndef FILEFORMAT_TYPES_DOTNET_HEADERS_CLR_HEADER_H
#define _FILEFORMAT_TYPES_DOTNET_HEADERS_CLR_HEADER_H

#include <cstdint>

namespace cchips {

	/**
		* CLR header
		*/
	class CLRHeader
	{
	private:
		std::uint64_t headerSize;
		std::uint64_t majorRuntimeVersion;
		std::uint64_t minorRuntimeVersion;
		std::uint64_t metadataDirectoryAddress;
		std::uint64_t metadataDirectorySize;
		std::uint64_t flags;
		std::uint64_t entryPointToken;
		std::uint64_t resourcesAddress;
		std::uint64_t resourcesSize;
		std::uint64_t strongNameSignatureAddress;
		std::uint64_t strongNameSignatureSize;
		std::uint64_t codeManagerTableAddress;
		std::uint64_t codeManagerTableSize;
		std::uint64_t vTableFixupsDirectoryAddress;
		std::uint64_t vTableFixupsDirectorySize;
		std::uint64_t exportAddressTableAddress;
		std::uint64_t exportAddressTableSize;
		std::uint64_t precompileHeaderAddress;
		std::uint64_t precompileHeaderSize;
	public:
		/// @name Getters
		/// @{
		std::uint64_t getHeaderSize() const { return headerSize; }
		std::uint64_t getMajorRuntimeVersion() const { return majorRuntimeVersion; }
		std::uint64_t getMinorRuntimeVersion() const { return minorRuntimeVersion; }
		std::uint64_t getMetadataDirectoryAddress() const { return metadataDirectoryAddress; }
		std::uint64_t getMetadataDirectorySize() const { return metadataDirectorySize; }
		std::uint64_t getFlags() const { return flags; }
		std::uint64_t getEntryPointToken() const { return entryPointToken; }
		std::uint64_t getResourcesAddress() const { return resourcesAddress; }
		std::uint64_t getResourcesSize() const { return resourcesSize; }
		std::uint64_t getStrongNameSignatureAddress() const { return strongNameSignatureAddress; }
		std::uint64_t getStrongNameSignatureSize() const { return strongNameSignatureSize; }
		std::uint64_t getCodeManagerTableAddress() const { return codeManagerTableAddress; }
		std::uint64_t getCodeManagerTableSize() const { return codeManagerTableSize; }
		std::uint64_t getVTableFixupsDirectoryAddress() const { return vTableFixupsDirectoryAddress; }
		std::uint64_t getVTableFixupsDirectorySize() const { return vTableFixupsDirectorySize; }
		std::uint64_t getExportAddressTableAddress() const { return exportAddressTableAddress; }
		std::uint64_t getExportAddressTableSize() const { return exportAddressTableSize; }
		std::uint64_t getPrecompileHeaderAddress() const { return precompileHeaderAddress; }
		std::uint64_t getPrecompileHeaderSize() const { return precompileHeaderSize; }
		/// @}

		/// @name Setters
		/// @{
		void setHeaderSize(std::uint64_t clrHeaderSize) { headerSize = clrHeaderSize; }
		void setMajorRuntimeVersion(std::uint64_t clrMajorRuntimeVersion) { majorRuntimeVersion = clrMajorRuntimeVersion; }
		void setMinorRuntimeVersion(std::uint64_t clrMinorRuntimeVersion) { minorRuntimeVersion = clrMinorRuntimeVersion; }
		void setMetadataDirectoryAddress(std::uint64_t clrMetadataDirectoryAddress) { metadataDirectoryAddress = clrMetadataDirectoryAddress; }
		void setMetadataDirectorySize(std::uint64_t clrMetadataDirectorySize) { metadataDirectorySize = clrMetadataDirectorySize; }
		void setFlags(std::uint64_t clrFlags) { flags = clrFlags; }
		void setEntryPointToken(std::uint64_t clrEntryPointToken) { entryPointToken = clrEntryPointToken; }
		void setResourcesAddress(std::uint64_t clrResourcesAddress) { resourcesAddress = clrResourcesAddress; }
		void setResourcesSize(std::uint64_t clrResourcesSize) { resourcesSize = clrResourcesSize; }
		void setStrongNameSignatureAddress(std::uint64_t clrStrongNameSignatureAddress) { strongNameSignatureAddress = clrStrongNameSignatureAddress; }
		void setStrongNameSignatureSize(std::uint64_t clrStrongNameSignatureSize) { strongNameSignatureSize = clrStrongNameSignatureSize; }
		void setCodeManagerTableAddress(std::uint64_t clrCodeManagerTableAddress) { codeManagerTableAddress = clrCodeManagerTableAddress; }
		void setCodeManagerTableSize(std::uint64_t clrCodeManagerTableSize) { codeManagerTableSize = clrCodeManagerTableSize; }
		void setVTableFixupsDirectoryAddress(std::uint64_t clrVTableFixupsDirectoryAddress) { vTableFixupsDirectoryAddress = clrVTableFixupsDirectoryAddress; }
		void setVTableFixupsDirectorySize(std::uint64_t clrVTableFixupsDirectorySize) { vTableFixupsDirectorySize = clrVTableFixupsDirectorySize; }
		void setExportAddressTableAddress(std::uint64_t clrExportAddressTableAddress) { exportAddressTableAddress = clrExportAddressTableAddress; }
		void setExportAddressTableSize(std::uint64_t clrExportAddressTableSize) { exportAddressTableSize = clrExportAddressTableSize; }
		void setPrecompileHeaderAddress(std::uint64_t clrPrecompileHeaderAddress) { precompileHeaderAddress = clrPrecompileHeaderAddress; }
		void setPrecompileHeaderSize(std::uint64_t clrPrecompileHeaderSize) { precompileHeaderSize = clrPrecompileHeaderSize; }
		/// @}
	};
} // namespace cchips

#endif
