/*
* PeFile.h - Part of the PeLib library.
*
* Copyright (c) 2004 - 2005 Sebastian Porst (webmaster@the-interweb.com)
* All rights reserved.
*
* This software is licensed under the zlib/libpng License.
* For more details see http://www.opensource.org/licenses/zlib-license.php
* or the license information file (license.htm) in the root directory
* of PeLib.
*/

#ifndef PEFILE_H
#define PEFILE_H

#include "PeLibInc.h"
#include "MzHeader.h"
#include "PeHeader.h"
#include "ImportDirectory.h"
#include "ExportDirectory.h"
#include "BoundImportDirectory.h"
#include "ResourceDirectory.h"
#include "RelocationsDirectory.h"
#include "ComHeaderDirectory.h"
#include "IatDirectory.h"
#include "DebugDirectory.h"
#include "TlsDirectory.h"
#include "RichHeader.h"
#include "CoffSymbolTable.h"
#include "DelayImportDirectory.h"
#include "SecurityDirectory.h"
#include <sstream>

namespace PeLib
{
	class PeFile32;
	class PeFile64;

	/**
	* Visitor base class for PeFiles.
	**/
	class PeFileVisitor
	{
		public:
		  virtual void callback(PeFile32 &file){(void) file; /* avoid warning about unused parameter */}
		  virtual void callback(PeFile64 &file){(void) file; /* avoid warning about unused parameter */}
		  virtual ~PeFileVisitor(){}
	};

	/**
	* Traits class that's used to decide of what type the PeHeader in a PeFile is.
	**/
	template<int>
	struct PeFile_Traits;

	template<>
	struct PeFile_Traits<32>
	{
		typedef PeHeader32 PeHeader32_64;
	};

	template<>
	struct PeFile_Traits<64>
	{
		typedef PeHeader64 PeHeader32_64;
	};

	/**
	* This class represents the common structures of PE and PE+ files.
	**/
	class PeFile
	{
		protected:
		  std::string m_filename; ///< Name of the current file.
		  MzHeader m_mzh; ///< MZ header of the current file.
		  RichHeader m_richheader; ///< Rich header of the current file.
		  CoffSymbolTable m_coffsymtab; ///< Symbol table of the current file.
		  SecurityDirectory m_secdir; ///< Security directory of the current file.
		public:
#define MAX_CHARS_AMOUNT 256
		  virtual ~PeFile();

		  /// Returns the name of the current file.
		  virtual std::string getFileName() const = 0; // EXPORT
		  /// Changes the name of the current file.
		  virtual void setFileName(std::string strFilename) = 0; // EXPORT

		  virtual void visit(PeFileVisitor &v) = 0;

		  /// Reads the MZ header of the current file from disc.
		  virtual int readMzHeader() = 0; // EXPORT
		  /// Reads the export directory of the current file from disc.
		  virtual int readExportDirectory() = 0; // EXPORT
		  /// Reads the PE header of the current file from disc.
		  virtual int readPeHeader()  = 0; // EXPORT
		  /// Reads the import directory of the current file from disc.
		  virtual int readImportDirectory() = 0; // EXPORT
		  /// Reads the bound import directory of the current file from disc.
		  virtual int readBoundImportDirectory() = 0; // EXPORT
		  /// Reads the resource directory of the current file from disc.
		  virtual int readResourceDirectory() = 0; // EXPORT
		  /// Reads the relocations directory of the current file from disc.
		  virtual int readRelocationsDirectory() = 0; // EXPORT
		  /// Reads the COM+ descriptor directory of the current file from disc.
		  virtual int readComHeaderDirectory() = 0; // EXPORT
		  /// Reads the IAT directory of the current file from disc.
		  virtual int readIatDirectory() = 0; // EXPORT
		  /// Reads the Debug directory of the current file.
		  virtual int readDebugDirectory() = 0; // EXPORT
		  /// Reads the TLS directory of the current file.
		  virtual int readTlsDirectory() = 0; // EXPORT
		  /// Reads rich header of the current file.
		  virtual int readRichHeader(std::size_t offset, std::size_t size, bool ignoreInvalidKey = false)  = 0; // EXPORT
		  /// Reads the COFF symbol table of the current file.
		  virtual int readCoffSymbolTable() = 0; // EXPORT
		  /// Reads delay import directory of the current file.
		  virtual int readDelayImportDirectory() = 0; // EXPORT
		  /// Reads security directory of the current file.
		  virtual int readSecurityDirectory() = 0; // EXPORT
		  /// Returns a loader error, if there was any
		  virtual LoaderError loaderError() const = 0;
		  virtual bool isLoadingFile() const = 0;
		  virtual bool isLoadingMemory() const = 0;
		  virtual unsigned int getBits() const = 0;

		  virtual std::size_t getLoadedFileLength() const = 0;
		  virtual const unsigned char* getLoadedBytesData() const = 0;
		  virtual void bytesToString(const unsigned char* data, std::size_t dataSize, std::string& result, std::size_t offset = 0, std::size_t size = 0) const = 0;
		  virtual void bytesToHexString(const unsigned char* data, std::size_t dataSize, std::string& result, std::size_t offset = 0, std::size_t size = 0, bool uppercase = true) const = 0;
		  virtual bool getBytes(std::vector<unsigned char>& result, unsigned long long offset, unsigned long long numberOfBytes) const = 0;
		  virtual bool getEpBytes(std::vector<unsigned char>& result, unsigned long long numberOfBytes) const = 0;
		  virtual bool getHexBytes(std::string& result, unsigned long long offset, unsigned long long numberOfBytes) const = 0;
		  virtual bool getHexEpBytes(std::string& result, unsigned long long numberOfBytes) const = 0;
		  virtual bool getStringBytes(std::string& result, unsigned long long offset, unsigned long long numberOfBytes) const = 0;
		  virtual bool getStringEpBytes(std::string& result, unsigned long long numberOfBytes) const = 0;
		  virtual double caculateEntropy(uint32_t offset, uint32_t length) const = 0;

		  /// Accessor function for the MZ header.
		  const MzHeader& mzHeader() const;
		  /// Accessor function for the MZ header.
		  MzHeader& mzHeader(); // EXPORT

		  /// Accessor function for the Rich header.
		  const RichHeader& richHeader() const;
		  /// Accessor function for the Rich header.
		  RichHeader& richHeader(); // EXPORT

		  /// Accessor function for the COFF symbol table.
		  const CoffSymbolTable& coffSymTab() const;
		  /// Accessor function for the COFF symbol table.
		  CoffSymbolTable& coffSymTab(); // EXPORT

		  /// Accessor function for the security directory.
		  const SecurityDirectory& securityDir() const;
		  /// Accessor function for the security directory.
		  SecurityDirectory& securityDir();

		  static const double log_2;
	};

	/**
	* This class implements the common structures of PE and PE+ files.
	**/
	template<int bits>
	class PeFileT : public PeFile
	{
		typedef typename PeFile_Traits<bits>::PeHeader32_64 PeHeader32_64;

    public:
        using _loading_type = enum {
            loading_file = 0,
            loading_memory = 1,
        };

		private:
	      std::ifstream m_ifStream;
		  std::istringstream m_isStream;
	      std::istream& m_iStream;

          _loading_type m_loading_type;
		  PeHeader32_64 m_peh; ///< PE header of the current file.
		  ExportDirectoryT<bits> m_expdir; ///< Export directory of the current file.
		  ImportDirectory<bits> m_impdir; ///< Import directory of the current file.
		  BoundImportDirectoryT<bits> m_boundimpdir; ///< BoundImportDirectory of the current file.
		  ResourceDirectoryT<bits> m_resdir; ///< ResourceDirectory of the current file.
		  RelocationsDirectoryT<bits> m_relocs; ///< Relocations directory of the current file.
		  ComHeaderDirectoryT<bits> m_comdesc; ///< COM+ descriptor directory of the current file.
		  IatDirectoryT<bits> m_iat; ///< Import address table of the current file.
		  DebugDirectoryT<bits> m_debugdir; ///< Debug directory of the current file.
		  DelayImportDirectory<bits> m_delayimpdir; ///< Delay import directory of the current file.
		  TlsDirectory<bits> m_tlsdir; ///< TLS directory of the current file.
		  std::vector<unsigned char>* m_loadedBytes; ///< reference to serialized content of input file.
		  std::vector<unsigned char> m_bytes; ///< content of file as bytes.

		public:
		  /// Default constructor which exists only for the sake of allowing to construct files without filenames.
		  PeFileT();

		  virtual ~PeFileT() {}

		  /// Initializes a PeFile with a filename
		  explicit PeFileT(const std::string& strFilename);
		  PeFileT(std::istream& stream, _loading_type type);
		  bool isLoadingFile() const { return (m_loading_type == loading_file); }
		  bool isLoadingMemory() const { return (m_loading_type == loading_memory); }
		  /// Returns the name of the current file.
		  std::string getFileName() const;
		  /// Changes the name of the current file.
		  void setFileName(std::string strFilename);

		  /// Reads the MZ header of the current file from disc.
		  int readMzHeader() ;
		  /// Reads the export directory of the current file from disc.
		  int readExportDirectory() ;
		  /// Reads the PE header of the current file from disc.
		  int readPeHeader() ;
		  /// Reads the import directory of the current file from disc.
		  int readImportDirectory() ;
		  /// Reads the bound import directory of the current file from disc.
		  int readBoundImportDirectory() ;
		  /// Reads the resource directory of the current file from disc.
		  int readResourceDirectory() ;
		  /// Reads the relocations directory of the current file from disc.
		  int readRelocationsDirectory() ;
		  /// Reads the COM+ descriptor directory of the current file from disc.
		  int readComHeaderDirectory() ;
		  /// Reads the IAT directory of the current file from disc.
		  int readIatDirectory() ;
		  /// Reads the Debug directory of the current file.
		  int readDebugDirectory() ;
		  /// Reads the TLS directory of the current file.
		  int readTlsDirectory() ;
		  /// Reads rich header of the current file.
		  int readRichHeader(std::size_t offset, std::size_t size, bool ignoreInvalidKey = false) ;
		  /// Reads the COFF symbol table of the current file.
		  int readCoffSymbolTable() ;
		  /// Reads delay import directory of the current file.
		  int readDelayImportDirectory() ;
		  /// Reads the security directory of the current file.
		  int readSecurityDirectory() ;
		  /// Returns a loader error, if there was any
		  LoaderError loaderError() const;

		  unsigned int getBits() const
		  {
			  return bits;
		  }

		  std::size_t getLoadedFileLength() const;
		  const unsigned char* getLoadedBytesData() const;
		  void bytesToString(const unsigned char* data, std::size_t dataSize, std::string& result, std::size_t offset, std::size_t size) const;
		  void bytesToHexString(const unsigned char* data, std::size_t dataSize, std::string& result, std::size_t offset, std::size_t size, bool uppercase) const;
		  bool getBytes(std::vector<unsigned char>& result, unsigned long long offset, unsigned long long numberOfBytes) const;
		  bool getEpBytes(std::vector<unsigned char>& result, unsigned long long numberOfBytes) const;
		  bool getHexBytes(std::string& result, unsigned long long offset, unsigned long long numberOfBytes) const;
		  bool getHexEpBytes(std::string& result, unsigned long long numberOfBytes) const;
		  bool getStringBytes(std::string& result, unsigned long long offset, unsigned long long numberOfBytes) const;
		  bool getStringEpBytes(std::string& result, unsigned long long numberOfBytes) const;
		  double caculateEntropy(uint32_t offset, uint32_t length) const;

		  /// Accessor function for the PE header.
		  const PeHeader32_64& peHeader() const;
		  /// Accessor function for the PE header.
		  PeHeader32_64& peHeader();

		  /// Accessor function for the export directory.
		  const ExportDirectoryT<bits>& expDir() const;
		  /// Accessor function for the export directory.
		  ExportDirectoryT<bits>& expDir(); // EXPORT

		  /// Accessor function for the import directory.
		  const ImportDirectory<bits>& impDir() const;
		  /// Accessor function for the import directory.
		  ImportDirectory<bits>& impDir();

		  /// Accessor function for the bound import directory.
		  const BoundImportDirectoryT<bits>& boundImpDir() const;
		  /// Accessor function for the bound import directory.
		  BoundImportDirectoryT<bits>& boundImpDir(); // EXPORT

		  /// Accessor function for the resource directory.
		  const ResourceDirectoryT<bits>& resDir() const;
		  /// Accessor function for the resource directory.
		  ResourceDirectoryT<bits>& resDir(); // EXPORT

		  /// Accessor function for the relocations directory.
		  const RelocationsDirectoryT<bits>& relocDir() const;
		  /// Accessor function for the relocations directory.
		  RelocationsDirectoryT<bits>& relocDir(); // EXPORT

		  /// Accessor function for the COM+ descriptor directory.
		  const ComHeaderDirectoryT<bits>& comDir() const;
		  /// Accessor function for the COM+ descriptor directory.
		  ComHeaderDirectoryT<bits>& comDir(); // EXPORT

		  /// Accessor function for the IAT directory.
		  const IatDirectoryT<bits>& iatDir() const;
		  /// Accessor function for the IAT directory.
		  IatDirectoryT<bits>& iatDir(); // EXPORT

		  /// Accessor function for the debug directory.
		  const DebugDirectoryT<bits>& debugDir() const;
		  /// Accessor function for the debug directory.
		  DebugDirectoryT<bits>& debugDir(); // EXPORT

		  /// Accessor function for the delay import directory.
		  const DelayImportDirectory<bits>& delayImports() const;
		  /// Accessor function for the delay import directory.
		  DelayImportDirectory<bits>& delayImports(); // EXPORT

		  /// Accessor function for the TLS directory.
		  const TlsDirectory<bits>& tlsDir() const;
		  /// Accessor function for the TLS directory.
		  TlsDirectory<bits>& tlsDir();
	};

	/**
	* This class is the main class for handling PE files.
	**/
	class PeFile32 : public PeFileT<32>
	{
		public:
		  /// Default constructor which exists only for the sake of allowing to construct files without filenames.
		  PeFile32();

		  /// Initializes a PeFile with a filename
		  explicit PeFile32(const std::string& strFlename);
		  PeFile32(std::istream& stream, _loading_type type);
		  virtual void visit(PeFileVisitor &v) { v.callback( *this ); }
	};

	/**
	* This class is the main class for handling PE+ files.
	**/
	class PeFile64 : public PeFileT<64>
	{
		public:
		  /// Default constructor which exists only for the sake of allowing to construct files without filenames.
		  PeFile64();

		  /// Initializes a PeFile with a filename
		  explicit PeFile64(const std::string& strFlename);
		  PeFile64(std::istream& stream, _loading_type type);
		  virtual void visit(PeFileVisitor &v) { v.callback( *this ); }
	};

	//typedef PeFileT<32> PeFile32;
	//typedef PeFileT<64> PeFile64;

	/**
	* @param strFilename Name of the current file.
	**/
	template<int bits>
	PeFileT<bits>::PeFileT(const std::string& strFilename) :
            m_loading_type(loading_file),
			m_loadedBytes(&m_bytes),
			m_iStream(m_ifStream)
	{
		m_filename = strFilename;
		m_ifStream.open(m_filename, std::ios::in | std::ios::binary | std::ios::ate);
		if (!m_ifStream.is_open())
			return;
		if (!m_iStream.good())
			return;
		int file_length = (int)m_iStream.tellg();
		if (!file_length)
			return;
		m_iStream.seekg(0, std::ios::beg);
		m_bytes.resize(file_length);
		m_iStream.read((char*)&m_bytes[0], file_length);
		int readed = (int)m_iStream.tellg();
		if (readed != file_length) {
			m_bytes.clear();
		}
		return;
	}

	/**
	* @param stream Input stream.
	**/
	template<int bits>
	PeFileT<bits>::PeFileT(std::istream& stream, _loading_type type) :
			m_iStream(m_isStream),
			m_loadedBytes(&m_bytes),
            m_loading_type(type)
	{
		if (!stream.good())
			return;
		stream.seekg(0, std::ios::end);
		int file_length = (int)stream.tellg();
		if (!file_length)
			return;
		stream.seekg(0, std::ios::beg);
		m_bytes.resize(file_length);
		stream.read((char*)&m_bytes[0], file_length);
		int readed = (int)stream.tellg();
		if (readed != file_length) {
			m_bytes.clear();
		}
		else {
			m_isStream.str(std::string(m_bytes.begin(), m_bytes.end()));
		}
		return;
 	}

	template<int bits>
	PeFileT<bits>::PeFileT() :
            m_loading_type(loading_file),
			m_iStream(m_isStream)
	{
	}

	template<int bits>
	int PeFileT<bits>::readPeHeader()
	{
        if (m_loading_type == loading_memory)
            peHeader().SetExpansionMode(true);
		return peHeader().read(m_iStream, mzHeader().getAddressOfPeHeader(), mzHeader());
	}

	/**
	* @return A reference to the file's PE header.
	**/
	template<int bits>
	const typename PeFile_Traits<bits>::PeHeader32_64& PeFileT<bits>::peHeader() const
	{
		return m_peh;
	}

	/**
	* @return A reference to the file's PE header.
	**/
	template<int bits>
	typename PeFile_Traits<bits>::PeHeader32_64& PeFileT<bits>::peHeader()
	{
		return m_peh;
	}

	/**
	* @return A reference to the file's import directory.
	**/
	template<int bits>
	const ImportDirectory<bits>& PeFileT<bits>::impDir() const
	{
		return m_impdir;
	}

	/**
	* @return A reference to the file's import directory.
	**/
	template<int bits>
	ImportDirectory<bits>& PeFileT<bits>::impDir()
	{
		return m_impdir;
	}

	template<int bits>
	const TlsDirectory<bits>& PeFileT<bits>::tlsDir() const
	{
		return m_tlsdir;
	}

	template<int bits>
	TlsDirectory<bits>& PeFileT<bits>::tlsDir()
	{
		return m_tlsdir;
	}

	/**
	* @return A reference to the file's delay import directory.
	**/
	template<int bits>
	const DelayImportDirectory<bits>& PeFileT<bits>::delayImports() const
	{
		return m_delayimpdir;
	}

	/**
	* @return A reference to the file's delay import directory.
	**/
	template<int bits>
	DelayImportDirectory<bits>& PeFileT<bits>::delayImports()
	{
		return m_delayimpdir;
	}

	/**
	* @return A reference to the file's export directory.
	**/
	template <int bits>
	const ExportDirectoryT<bits>& PeFileT<bits>::expDir() const
	{
		return m_expdir;
	}

	/**
	* @return A reference to the file's export directory.
	**/
	template <int bits>
	ExportDirectoryT<bits>& PeFileT<bits>::expDir()
	{
		return m_expdir;
	}

	/**
	* @return A reference to the file's bound import directory.
	**/
	template <int bits>
	const BoundImportDirectoryT<bits>& PeFileT<bits>::boundImpDir() const
	{
		return m_boundimpdir;
	}

	/**
	* @return A reference to the file's bound import directory.
	**/
	template <int bits>
	BoundImportDirectoryT<bits>& PeFileT<bits>::boundImpDir()
	{
		return m_boundimpdir;
	}

	/**
	* @return A reference to the file's resource directory.
	**/
	template <int bits>
	const ResourceDirectoryT<bits>& PeFileT<bits>::resDir() const
	{
		return m_resdir;
	}

	/**
	* @return A reference to the file's resource directory.
	**/
	template <int bits>
	ResourceDirectoryT<bits>& PeFileT<bits>::resDir()
	{
		return m_resdir;
	}

	/**
	* @return A reference to the file's relocations directory.
	**/
	template <int bits>
	const RelocationsDirectoryT<bits>& PeFileT<bits>::relocDir() const
	{
		return m_relocs;
	}

	/**
	* @return A reference to the file's relocations directory.
	**/
	template <int bits>
	RelocationsDirectoryT<bits>& PeFileT<bits>::relocDir()
	{
		return m_relocs;
	}

	/**
	* @return A reference to the file's COM+ descriptor directory.
	**/
	template <int bits>
	const ComHeaderDirectoryT<bits>& PeFileT<bits>::comDir() const
	{
		return m_comdesc;
	}

	/**
	* @return A reference to the file's COM+ descriptor directory.
	**/
	template <int bits>
	ComHeaderDirectoryT<bits>& PeFileT<bits>::comDir()
	{
		return m_comdesc;
	}

	template <int bits>
	const IatDirectoryT<bits>& PeFileT<bits>::iatDir() const
	{
		return m_iat;
	}

	template <int bits>
	IatDirectoryT<bits>& PeFileT<bits>::iatDir()
	{
		return m_iat;
	}

	template <int bits>
	const DebugDirectoryT<bits>& PeFileT<bits>::debugDir() const
	{
		return m_debugdir;
	}

	template <int bits>
	DebugDirectoryT<bits>& PeFileT<bits>::debugDir()
	{
		return m_debugdir;
	}

	/**
	* @return Filename of the current file.
	**/
	template<int bits>
	std::string PeFileT<bits>::getFileName() const
	{
		return m_filename;
	}

	/**
	* @param strFilename New filename.
	**/
	template<int bits>
	void PeFileT<bits>::setFileName(std::string strFilename)
	{
		m_filename = strFilename;
		if (m_ifStream.is_open())
		{
			m_ifStream.close();
		}
		m_ifStream.open(m_filename, std::ifstream::binary);
	}

	template<int bits>
	int PeFileT<bits>::readMzHeader()
	{
		return mzHeader().read(m_iStream);
	}

	template<int bits>
	int PeFileT<bits>::readRichHeader(
			std::size_t offset,
			std::size_t size,
			bool ignoreInvalidKey)
	{
		return richHeader().read(m_iStream, offset, size, ignoreInvalidKey);
	}

	template<int bits>
	int PeFileT<bits>::readCoffSymbolTable()
	{
		if (peHeader().getPointerToSymbolTable()
				&& peHeader().getNumberOfSymbols())
		{
			return coffSymTab().read(
					m_iStream,
					static_cast<unsigned int>(peHeader().getPointerToSymbolTable()),
					peHeader().getNumberOfSymbols() * PELIB_IMAGE_SIZEOF_COFF_SYMBOL);
		}
		return ERROR_COFF_SYMBOL_TABLE_DOES_NOT_EXIST;
	}

	template<int bits>
	int PeFileT<bits>::readExportDirectory()
	{
		if (peHeader().calcNumberOfRvaAndSizes() >= 1
			&& peHeader().getIddExportRva())
		{
			return expDir().read(m_iStream, peHeader());
		}
		return ERROR_DIRECTORY_DOES_NOT_EXIST;
	}

	template<int bits>
	int PeFileT<bits>::readImportDirectory()
	{
		if (peHeader().calcNumberOfRvaAndSizes() >= 2
			&& peHeader().getIddImportRva())
		{
			return impDir().read(m_iStream, peHeader());
		}
		return ERROR_DIRECTORY_DOES_NOT_EXIST;
	}

	template<int bits>
	int PeFileT<bits>::readResourceDirectory()
	{
		if (peHeader().calcNumberOfRvaAndSizes() >= 3
			&& peHeader().getIddResourceRva())
		{
			return resDir().read(m_iStream, peHeader());
		}
		return ERROR_DIRECTORY_DOES_NOT_EXIST;
	}

	template<int bits>
	int PeFileT<bits>::readSecurityDirectory()
	{
		if (peHeader().calcNumberOfRvaAndSizes() >= 5
			&& peHeader().getIddSecurityRva()
			&& peHeader().getIddSecuritySize())
		{
			return securityDir().read(
					m_iStream,
					peHeader().getIddSecurityRva(),
					peHeader().getIddSecuritySize());
		}
		return ERROR_DIRECTORY_DOES_NOT_EXIST;
	}

	template<int bits>
	int PeFileT<bits>::readRelocationsDirectory()
	{
		if (peHeader().calcNumberOfRvaAndSizes() >= 6
			&& peHeader().getIddBaseRelocRva() && peHeader().getIddBaseRelocSize())
		{
			return relocDir().read(m_iStream, peHeader());
		}
		return ERROR_DIRECTORY_DOES_NOT_EXIST;
	}

	template<int bits>
	int PeFileT<bits>::readDebugDirectory()
	{
		if (peHeader().calcNumberOfRvaAndSizes() >= 7
			&& peHeader().getIddDebugRva() && peHeader().getIddDebugSize())
		{
			return debugDir().read(m_iStream, peHeader());
		}
		return ERROR_DIRECTORY_DOES_NOT_EXIST;
	}

	template<int bits>
	int PeFileT<bits>::readTlsDirectory()
	{
		if (peHeader().calcNumberOfRvaAndSizes() >= 10
			&& peHeader().getIddTlsRva() && peHeader().getIddTlsSize())
		{
			return tlsDir().read(m_iStream, peHeader());
		}
		return ERROR_DIRECTORY_DOES_NOT_EXIST;
	}

	template<int bits>
	int PeFileT<bits>::readBoundImportDirectory()
	{
		if (peHeader().calcNumberOfRvaAndSizes() >= 12
			&& peHeader().getIddBoundImportRva() && peHeader().getIddBoundImportSize())
		{
			return boundImpDir().read(m_iStream, peHeader());
		}
		return ERROR_DIRECTORY_DOES_NOT_EXIST;
	}

	template<int bits>
	int PeFileT<bits>::readIatDirectory()
	{
		if (peHeader().calcNumberOfRvaAndSizes() >= 13
			&& peHeader().getIddIatRva() && peHeader().getIddIatSize())
		{
			return iatDir().read(m_iStream, peHeader());
		}
		return ERROR_DIRECTORY_DOES_NOT_EXIST;
	}

	template<int bits>
	int PeFileT<bits>::readDelayImportDirectory()
	{
		// Note: Delay imports can have arbitrary size and Windows loader will still load them
		if (peHeader().calcNumberOfRvaAndSizes() >= 14 && peHeader().getIddDelayImportRva() /* && peHeader().getIddDelayImportSize() */)
		{
			return delayImports().read(m_iStream, peHeader());
		}
		return ERROR_DIRECTORY_DOES_NOT_EXIST;
	}

	template<int bits>
	int PeFileT<bits>::readComHeaderDirectory()
	{
		if (peHeader().calcNumberOfRvaAndSizes() >= 15
			&& peHeader().getIddComHeaderRva() && peHeader().getIddComHeaderSize())
		{
			return comDir().read(m_iStream, peHeader());
		}
		return ERROR_DIRECTORY_DOES_NOT_EXIST;
	}

	template<int bits>
	std::size_t PeFileT<bits>::getLoadedFileLength() const
	{
		if (m_loadedBytes)
			return m_loadedBytes->size();
		return 0;
	}

	template<int bits>
	const unsigned char* PeFileT<bits>::getLoadedBytesData() const
	{
		if (m_loadedBytes)
			return m_loadedBytes->data();
		return nullptr;
	}

	template<int bits>
	bool PeFileT<bits>::getBytes(std::vector<unsigned char>& result, unsigned long long offset, unsigned long long numberOfBytes) const
	{
		if (offset >= getLoadedFileLength())
		{
			return false;
		}
		numberOfBytes = offset + numberOfBytes > getLoadedFileLength() ? getLoadedFileLength() - offset : numberOfBytes;
		result.clear();
		result.reserve(numberOfBytes);
		std::copy(m_loadedBytes->begin() + offset, m_loadedBytes->begin() + offset + numberOfBytes, std::back_inserter(result));
		return true;
	}

	template<int bits>
	bool PeFileT<bits>::getEpBytes(std::vector<unsigned char>& result, unsigned long long numberOfBytes) const
	{
		return false;
	}

	template<int bits> 
	void PeFileT<bits>::bytesToString(const unsigned char* data, std::size_t dataSize, std::string& result, std::size_t offset, std::size_t size) const
	{
		if (!data)
		{
			dataSize = 0;
		}

		if (offset >= dataSize)
		{
			size = 0;
		}
		else
		{
			size = (size == 0 || offset + size > dataSize) ? dataSize - offset : size;
		}

		result.clear();
		result.reserve(size);
		result = std::string(reinterpret_cast<const char*>(data + offset), size);
	}

	template<int bits> 
	void PeFileT<bits>::bytesToHexString(const unsigned char* data, std::size_t dataSize, std::string& result, std::size_t offset, std::size_t size, bool uppercase) const
	{
		if (!data)
		{
			dataSize = 0;
		}

		if (offset >= dataSize)
		{
			size = 0;
		}
		else
		{
			size = (size == 0 || offset + size > dataSize) ? dataSize - offset : size;
		}

		// Sample: 4A2A008CF1AEE9BA49D8D1DAA22D8E868365ACE633823D464478239F27ED4F18
		// Tool: redec-fileinfo.exe, Debug, x64, data = image, dataSize = 0xE1BC00
		// Optimized: This code now takes 0.106 seconds to convert (measured in VS 2015 IDE)
		// (down from about 40 seconds)
		const char* intToHex = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";
		std::size_t hexIndex = 0;

		// Reserve the necessary space for the hexa string
		result.resize(size * 2);

		// Convert to hexa byte-by-byte. No reallocations
		for (std::size_t i = 0; i < size; ++i, hexIndex += 2)
		{
			std::uint8_t oneByte = data[offset + i];

			result[hexIndex + 0] = intToHex[(oneByte >> 0x04) & 0x0F];
			result[hexIndex + 1] = intToHex[(oneByte >> 0x00) & 0x0F];
		}
	}

	template<int bits>
	bool PeFileT<bits>::getHexBytes(std::string& result, unsigned long long offset, unsigned long long numberOfBytes) const
	{
		bytesToHexString(getLoadedBytesData(), getLoadedFileLength(), result, offset, (size_t)numberOfBytes, true);
		return offset < getLoadedFileLength();
	}

	template<int bits>
	bool PeFileT<bits>::getHexEpBytes(std::string& result, unsigned long long numberOfBytes) const
	{
		bytesToHexString(getLoadedBytesData(), getLoadedFileLength(), result, peHeader().getAddressOfEntryPoint(), (size_t)numberOfBytes, true);
		return true;
	}

	template<int bits>
	bool PeFileT<bits>::getStringBytes(std::string& result, unsigned long long offset, unsigned long long numberOfBytes) const
	{
		bytesToString(getLoadedBytesData(), getLoadedFileLength(), result, offset, (size_t)numberOfBytes);
		return offset < getLoadedFileLength();
	}

	template<int bits>
	bool PeFileT<bits>::getStringEpBytes(std::string& result, unsigned long long numberOfBytes) const
	{
		bytesToString(getLoadedBytesData(), getLoadedFileLength(), result, peHeader().getAddressOfEntryPoint(), (size_t)numberOfBytes);
		return true;
	}

	template<int bits>
	double PeFileT<bits>::caculateEntropy(uint32_t offset, uint32_t length) const
	{
		double entropy = 0.;
		size_t file_size = getLoadedFileLength();
		if (!file_size || file_size < (size_t)(offset + length))
			return entropy;
		file_size = (size_t)length;
		const unsigned char* file_data = getLoadedBytesData();
		if (!file_data)
			return entropy;

		uint32_t byte_count[MAX_CHARS_AMOUNT] = { 0 }; //Byte count for each of 255 bytes
		for (size_t i = offset; i != file_size; i++)
			++byte_count[file_data[i]];

		for (uint32_t i = 0; i < MAX_CHARS_AMOUNT; ++i)
		{
			double temp = static_cast<double>(byte_count[i]) / file_size;
			if (temp > 0.)
				entropy += std::abs(temp * (std::log(temp) * log_2));
		}

		return entropy;
	}

	// Returns an error code indicating loader problem. We check every part of the PE file
	// for possible loader problem. If anything wrong was found, we report it
	template<int bits>
	LoaderError PeFileT<bits>::loaderError() const
	{
		LoaderError ldrError;

		// Was there a problem in the DOS header?
		ldrError = mzHeader().loaderError();
		if (ldrError != LDR_ERROR_NONE)
			return ldrError;

		// Was there a problem in the NT headers?
		ldrError = peHeader().loaderError();
		if (ldrError != LDR_ERROR_NONE)
			return ldrError;

		// Check the loader error
		ldrError = coffSymTab().loaderError();
		if (ldrError != LDR_ERROR_NONE)
			return ldrError;

		// Check errors in import directory
		ldrError = impDir().loaderError();
		if (ldrError != LDR_ERROR_NONE)
			return ldrError;

		// Check errors in resource directory
		ldrError = resDir().loaderError();
		if (ldrError != LDR_ERROR_NONE)
			return ldrError;

		// Nothing wrond found
		return LDR_ERROR_NONE;
	}
}

#endif
