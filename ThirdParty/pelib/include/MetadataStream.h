#ifndef FILEFORMAT_TYPES_DOTNET_HEADERS_METADATA_STREAM_H
#define FILEFORMAT_TYPES_DOTNET_HEADERS_METADATA_STREAM_H

#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <memory>

#include "MetadataTable.h"

namespace cchips {

	enum class StreamType
	{
		Metadata,
		String,
		UserString,
		Blob,
		Guid
	};

	template<typename Map>
	bool mapHasKey(const Map& m, const typename Map::key_type& k) {
		return m.find(k) != m.end();
	}

	class Stream
	{
	private:
		StreamType type;
		std::uint64_t offset;
		std::uint64_t size;
		std::string name;
	protected:
		Stream(StreamType streamType, std::uint64_t streamOffset, std::uint64_t streamSize) : type(streamType), offset(streamOffset), size(streamSize)
		{
			switch (type)
			{
			case StreamType::Metadata:
				name = "#~";
				break;
			case StreamType::String:
				name = "#Strings";
				break;
			case StreamType::UserString:
				name = "#US";
				break;
			case StreamType::Blob:
				name = "#Blob";
				break;
			case StreamType::Guid:
				name = "#GUID";
				break;
			default:
				name.clear();
				break;
			}
		}
	public:
		virtual ~Stream() = default;

		/// @name Getters
		/// @{
		StreamType getType() const { return type; }
		std::uint64_t getOffset() const { return offset; }
		std::uint64_t getSize() const { return size; }
		const std::string& getName() const { return name; }
		/// @}
	};

	class MetadataStream : public Stream
	{
	private:
		using TypeToTableMap = std::map<MetadataTableType, std::unique_ptr<BaseMetadataTable>>;

		std::uint32_t majorVersion;
		std::uint32_t minorVersion;
		std::uint32_t stringStreamIndexSize;
		std::uint32_t guidStreamIndexSize;
		std::uint32_t blobStreamIndexSize;
		TypeToTableMap metadataTables;
	public:
		MetadataStream(std::uint64_t streamOffset, std::uint64_t streamSize) : Stream(StreamType::Metadata, streamOffset, streamSize) {}

		/// @name Getters
		/// @{
		std::uint32_t getMajorVersion() const { return majorVersion; }
		std::uint32_t getMinorVersion() const { return minorVersion; }
		std::uint32_t getStringStreamIndexSize() const { return stringStreamIndexSize; }
		std::uint32_t getGuidStreamIndexSize() const { guidStreamIndexSize; }
		std::uint32_t getBlobStreamIndexSize() const { blobStreamIndexSize; }
		BaseMetadataTable* getMetadataTable(MetadataTableType tableType) {
			auto itr = metadataTables.find(tableType);
			if (itr == metadataTables.end())
				return nullptr;

			return itr->second.get();
		}
		const BaseMetadataTable* getMetadataTable(MetadataTableType tableType) const {
			auto itr = metadataTables.find(tableType);
			if (itr == metadataTables.end())
				return nullptr;

			return itr->second.get();
		}
		/// @}

		/// @name Setters
		/// @{
		void setMajorVersion(std::uint32_t streamMajorVersion) { majorVersion = streamMajorVersion; }
		void setMinorVersion(std::uint32_t streamMinorVersion) { minorVersion = streamMinorVersion; }
		void setStringStreamIndexSize(std::uint32_t indexSize) { stringStreamIndexSize = (indexSize > 2) ? 4 : 2; }
		void setGuidStreamIndexSize(std::uint32_t indexSize) { guidStreamIndexSize = (indexSize > 2) ? 4 : 2; }
		void setBlobStreamIndexSize(std::uint32_t indexSize) { blobStreamIndexSize = (indexSize > 2) ? 4 : 2; }
		/// @}

		/// @name Metadata tables methods
		/// @{
		BaseMetadataTable* addMetadataTable(MetadataTableType tableType, std::uint32_t tableSize) {
			bool success = false;
			TypeToTableMap::iterator itr;
			switch (tableType)
			{
			case MetadataTableType::Module:
				std::tie(itr, success) = metadataTables.emplace(tableType, std::make_unique<MetadataTable<BaseMetadataTable>>(tableType, tableSize));
				break;
			case MetadataTableType::TypeRef:
				std::tie(itr, success) = metadataTables.emplace(tableType, std::make_unique<MetadataTable<BaseMetadataTable>>(tableType, tableSize));
				break;
			case MetadataTableType::TypeDef:
				std::tie(itr, success) = metadataTables.emplace(tableType, std::make_unique<MetadataTable<BaseMetadataTable>>(tableType, tableSize));
				break;
			case MetadataTableType::FieldPtr:
				std::tie(itr, success) = metadataTables.emplace(tableType, std::make_unique<MetadataTable<BaseMetadataTable>>(tableType, tableSize));
				break;
			case MetadataTableType::Field:
				std::tie(itr, success) = metadataTables.emplace(tableType, std::make_unique<MetadataTable<BaseMetadataTable>>(tableType, tableSize));
				break;
			case MetadataTableType::MethodPtr:
				std::tie(itr, success) = metadataTables.emplace(tableType, std::make_unique<MetadataTable<BaseMetadataTable>>(tableType, tableSize));
				break;
			case MetadataTableType::MethodDef:
				std::tie(itr, success) = metadataTables.emplace(tableType, std::make_unique<MetadataTable<BaseMetadataTable>>(tableType, tableSize));
				break;
			case MetadataTableType::ParamPtr:
				std::tie(itr, success) = metadataTables.emplace(tableType, std::make_unique<MetadataTable<BaseMetadataTable>>(tableType, tableSize));
				break;
			case MetadataTableType::Param:
				std::tie(itr, success) = metadataTables.emplace(tableType, std::make_unique<MetadataTable<BaseMetadataTable>>(tableType, tableSize));
				break;
			case MetadataTableType::InterfaceImpl:
				std::tie(itr, success) = metadataTables.emplace(tableType, std::make_unique<MetadataTable<BaseMetadataTable>>(tableType, tableSize));
				break;
			case MetadataTableType::MemberRef:
				std::tie(itr, success) = metadataTables.emplace(tableType, std::make_unique<MetadataTable<BaseMetadataTable>>(tableType, tableSize));
				break;
			case MetadataTableType::Constant:
				std::tie(itr, success) = metadataTables.emplace(tableType, std::make_unique<MetadataTable<BaseMetadataTable>>(tableType, tableSize));
				break;
			case MetadataTableType::CustomAttribute:
				std::tie(itr, success) = metadataTables.emplace(tableType, std::make_unique<MetadataTable<BaseMetadataTable>>(tableType, tableSize));
				break;
			case MetadataTableType::FieldMarshal:
				std::tie(itr, success) = metadataTables.emplace(tableType, std::make_unique<MetadataTable<BaseMetadataTable>>(tableType, tableSize));
				break;
			case MetadataTableType::DeclSecurity:
				std::tie(itr, success) = metadataTables.emplace(tableType, std::make_unique<MetadataTable<BaseMetadataTable>>(tableType, tableSize));
				break;
			case MetadataTableType::ClassLayout:
				std::tie(itr, success) = metadataTables.emplace(tableType, std::make_unique<MetadataTable<BaseMetadataTable>>(tableType, tableSize));
				break;
			case MetadataTableType::FieldLayout:
				std::tie(itr, success) = metadataTables.emplace(tableType, std::make_unique<MetadataTable<BaseMetadataTable>>(tableType, tableSize));
				break;
			case MetadataTableType::StandAloneSig:
				std::tie(itr, success) = metadataTables.emplace(tableType, std::make_unique<MetadataTable<BaseMetadataTable>>(tableType, tableSize));
				break;
			case MetadataTableType::EventMap:
				std::tie(itr, success) = metadataTables.emplace(tableType, std::make_unique<MetadataTable<BaseMetadataTable>>(tableType, tableSize));
				break;
			case MetadataTableType::Event:
				std::tie(itr, success) = metadataTables.emplace(tableType, std::make_unique<MetadataTable<BaseMetadataTable>>(tableType, tableSize));
				break;
			case MetadataTableType::PropertyMap:
				std::tie(itr, success) = metadataTables.emplace(tableType, std::make_unique<MetadataTable<BaseMetadataTable>>(tableType, tableSize));
				break;
			case MetadataTableType::PropertyPtr:
				std::tie(itr, success) = metadataTables.emplace(tableType, std::make_unique<MetadataTable<BaseMetadataTable>>(tableType, tableSize));
				break;
			case MetadataTableType::Property:
				std::tie(itr, success) = metadataTables.emplace(tableType, std::make_unique<MetadataTable<BaseMetadataTable>>(tableType, tableSize));
				break;
			case MetadataTableType::MethodSemantics:
				std::tie(itr, success) = metadataTables.emplace(tableType, std::make_unique<MetadataTable<BaseMetadataTable>>(tableType, tableSize));
				break;
			case MetadataTableType::MethodImpl:
				std::tie(itr, success) = metadataTables.emplace(tableType, std::make_unique<MetadataTable<BaseMetadataTable>>(tableType, tableSize));
				break;
			case MetadataTableType::ModuleRef:
				std::tie(itr, success) = metadataTables.emplace(tableType, std::make_unique<MetadataTable<BaseMetadataTable>>(tableType, tableSize));
				break;
			case MetadataTableType::TypeSpec:
				std::tie(itr, success) = metadataTables.emplace(tableType, std::make_unique<MetadataTable<BaseMetadataTable>>(tableType, tableSize));
				break;
			case MetadataTableType::ImplMap:
				std::tie(itr, success) = metadataTables.emplace(tableType, std::make_unique<MetadataTable<BaseMetadataTable>>(tableType, tableSize));
				break;
			case MetadataTableType::FieldRVA:
				std::tie(itr, success) = metadataTables.emplace(tableType, std::make_unique<MetadataTable<BaseMetadataTable>>(tableType, tableSize));
				break;
			case MetadataTableType::ENCLog:
				std::tie(itr, success) = metadataTables.emplace(tableType, std::make_unique<MetadataTable<BaseMetadataTable>>(tableType, tableSize));
				break;
			case MetadataTableType::ENCMap:
				std::tie(itr, success) = metadataTables.emplace(tableType, std::make_unique<MetadataTable<BaseMetadataTable>>(tableType, tableSize));
				break;
			case MetadataTableType::Assembly:
				std::tie(itr, success) = metadataTables.emplace(tableType, std::make_unique<MetadataTable<BaseMetadataTable>>(tableType, tableSize));
				break;
			case MetadataTableType::AssemblyProcessor:
				std::tie(itr, success) = metadataTables.emplace(tableType, std::make_unique<MetadataTable<BaseMetadataTable>>(tableType, tableSize));
				break;
			case MetadataTableType::AssemblyOS:
				std::tie(itr, success) = metadataTables.emplace(tableType, std::make_unique<MetadataTable<BaseMetadataTable>>(tableType, tableSize));
				break;
			case MetadataTableType::AssemblyRef:
				std::tie(itr, success) = metadataTables.emplace(tableType, std::make_unique<MetadataTable<BaseMetadataTable>>(tableType, tableSize));
				break;
			case MetadataTableType::AssemblyRefProcessor:
				std::tie(itr, success) = metadataTables.emplace(tableType, std::make_unique<MetadataTable<BaseMetadataTable>>(tableType, tableSize));
				break;
			case MetadataTableType::AssemblyRefOS:
				std::tie(itr, success) = metadataTables.emplace(tableType, std::make_unique<MetadataTable<BaseMetadataTable>>(tableType, tableSize));
				break;
			case MetadataTableType::File:
				std::tie(itr, success) = metadataTables.emplace(tableType, std::make_unique<MetadataTable<BaseMetadataTable>>(tableType, tableSize));
				break;
			case MetadataTableType::ExportedType:
				std::tie(itr, success) = metadataTables.emplace(tableType, std::make_unique<MetadataTable<BaseMetadataTable>>(tableType, tableSize));
				break;
			case MetadataTableType::ManifestResource:
				std::tie(itr, success) = metadataTables.emplace(tableType, std::make_unique<MetadataTable<BaseMetadataTable>>(tableType, tableSize));
				break;
			case MetadataTableType::NestedClass:
				std::tie(itr, success) = metadataTables.emplace(tableType, std::make_unique<MetadataTable<BaseMetadataTable>>(tableType, tableSize));
				break;
			case MetadataTableType::GenericParam:
				std::tie(itr, success) = metadataTables.emplace(tableType, std::make_unique<MetadataTable<BaseMetadataTable>>(tableType, tableSize));
				break;
			case MetadataTableType::MethodSpec:
				std::tie(itr, success) = metadataTables.emplace(tableType, std::make_unique<MetadataTable<BaseMetadataTable>>(tableType, tableSize));
				break;
			case MetadataTableType::GenericParamContstraint:
				std::tie(itr, success) = metadataTables.emplace(tableType, std::make_unique<MetadataTable<BaseMetadataTable>>(tableType, tableSize));
				break;
			default:
				return nullptr;
			}

			return success ? itr->second.get() : nullptr;
		}
		/// @}

		/// @name Detection
		/// @{
		bool hasTable(MetadataTableType metadataTableType) const {
			return mapHasKey(metadataTables, metadataTableType);
		}
		/// @}
	};
} // namespace cchips

#endif
