#ifndef FILEFORMAT_TYPES_DOTNET_HEADERS_METADATA_HEADER_H
#define FILEFORMAT_TYPES_DOTNET_HEADERS_METADATA_HEADER_H

#include <cstdint>
#include <string>

namespace cchips {

	const std::uint64_t MetadataHeaderSignature = 0x424A5342;

	/**
		* Metadata header
		*/
	class MetadataHeader
	{
	private:
		std::uint64_t address;
		std::uint64_t majorVersion;
		std::uint64_t minorVersion;
		std::string version;
		std::uint64_t flags;
	public:
		/// @name Getters
		/// @{
		std::uint64_t getAddress() const { return address; }
		std::uint64_t getMajorVersion() const { return majorVersion; }
		std::uint64_t getMinorVersion() const { return minorVersion; }
		const std::string& getVersion() const { return version; }
		std::uint64_t getFlags() const { return flags; }
		/// @}

		/// @name Setters
		/// @{
		void setAddress(std::uint64_t metadataHeaderAddress) { address = metadataHeaderAddress; }
		void setMajorVersion(std::uint64_t metadataMajorVersion) { majorVersion = metadataMajorVersion; }
		void setMinorVersion(std::uint64_t metadataMinorVersion) { minorVersion = metadataMinorVersion; }
		void setVersion(const std::string& metadataVersion) { version = metadataVersion; }
		void setFlags(std::uint64_t metadataFlags) { flags = metadataFlags; }
		/// @}
	};
} // namespace cchips

#endif
