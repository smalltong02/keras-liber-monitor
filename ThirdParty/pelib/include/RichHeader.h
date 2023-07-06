/**
 * @file RichHeader.h
 * @brief Class for rich header.
 * @copyright (c) 2017 Avast Software, licensed under the MIT license
 */

#ifndef RICHHEADER_H
#define RICHHEADER_H

#include <cstdint>
#include <vector>
#include <map>

namespace PeLib
{
 /**
 * Information about linker
 */
	class LinkerInfo
	{
	private:
		std::uint32_t productId = 0;    ///< Product ID from the RichHeader
		std::uint32_t productBuild = 0; ///< Product Build from the RichHeader
		std::uint32_t count = 0;        ///< number of uses
		std::string productName;        ///< Product codename
		std::string visualStudioName;   ///< Product codename
	public:
		/// @name Getters
		/// @{
		std::uint32_t getProductId() const { return productId; }
		std::uint32_t getProductBuild() const { return productBuild; }
		std::uint32_t getNumberOfUses() const { return count; }
		std::string getProductName() const { return productName; }
		std::string getVisualStudioName() const { return visualStudioName; }
		/// @}

		/// @name Setters
		/// @{
		void setProductId(std::uint32_t richProductId) { productId = richProductId; }
		void setProductBuild(std::uint32_t richProductBuild) { productBuild = richProductBuild; }
		void setNumberOfUses(std::uint32_t richProductCount) { count = richProductCount; }
		void setProductName(const std::string& richProductName) { productName = richProductName; }
		void setVisualStudioName(const std::string& richVisualStudioName) { visualStudioName = richVisualStudioName; }
		/// @}
	};

	/**
	 * This class handless rich header.
	 */
	class RichHeader
	{
		public:
			typedef std::vector<PELIB_IMAGE_RICH_HEADER_RECORD>::const_iterator richHeaderIterator;
		private:
			bool headerIsValid;
			bool validStructure;
			dword key;
			std::size_t noOfIters;
			std::vector<dword> decryptedHeader;
			std::vector<PELIB_IMAGE_RICH_HEADER_RECORD> records;
			std::vector<LinkerInfo> header;  ///< all records in header

			void init();
			void setValidStructure();
			void getUserFriendlyProductName(PELIB_IMAGE_RICH_HEADER_RECORD & record);
			bool analyze(bool ignoreInvalidKey = false);
			void read(InputBuffer& inputbuffer, std::size_t uiSize, bool ignoreInvalidKey);
		public:
			RichHeader();
			~RichHeader();

			int read(
					std::istream& inStream,
					std::size_t uiOffset,
					std::size_t uiSize,
					bool ignoreInvalidKey);
			bool isHeaderValid() const;
			bool isStructureValid() const;
			std::size_t getNumberOfIterations() const;
			dword getKey() const;
			const dword* getDecryptedHeaderItem(std::size_t index) const;
			std::string getDecryptedHeaderItemSignature(std::size_t index) const;
			std::string getDecryptedHeaderItemsSignature(std::initializer_list<std::size_t> indexes) const;
			std::vector<std::uint8_t> getDecryptedHeaderBytes() const;
			richHeaderIterator begin() const;
			richHeaderIterator end() const;
			void addLinkerInfo(LinkerInfo& record);
			const std::vector<LinkerInfo>& getLinkerInfo() const { return header; }
			bool hasRecords() const { return !header.empty(); }
	};
}

#endif
