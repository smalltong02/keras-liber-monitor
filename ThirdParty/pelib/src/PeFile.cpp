/*
* PeLib.cpp - Part of the PeLib library.
*
* Copyright (c) 2004 - 2005 Sebastian Porst (webmaster@the-interweb.com)
* All rights reserved.
*
* This software is licensed under the zlib/libpng License.
* For more details see http://www.opensource.org/licenses/zlib-license.php
* or the license information file (license.htm) in the root directory
* of PeLib.
*/

#include "PeFile.h"

namespace PeLib
{
	const double PeFile::log_2 = 1.44269504088896340736; //instead of using M_LOG2E

	PeFile::~PeFile()
	{
	}

	PeFile32::PeFile32() : PeFileT<32>()
	{
	}

	PeFile32::PeFile32(const std::string& strFlename) : PeFileT<32>(strFlename)
	{
	}

	PeFile32::PeFile32(std::istream& stream, _loading_type type) : PeFileT<32>(stream, type)
	{
	}

	PeFile64::PeFile64() : PeFileT<64>()
	{
	}

	PeFile64::PeFile64(const std::string& strFlename) : PeFileT<64>(strFlename)
	{
	}

	PeFile64::PeFile64(std::istream& stream, _loading_type type) : PeFileT<64>(stream, type)
	{
	}

	/**
	* @return A reference to the file's MZ header.
	**/

	const MzHeader& PeFile::mzHeader() const
	{
		return m_mzh;
	}

	/**
	* @return A reference to the file's MZ header.
	**/

	MzHeader& PeFile::mzHeader()
	{
		return m_mzh;
	}

	const RichHeader& PeFile::richHeader() const
	{
		return m_richheader;
	}

	RichHeader& PeFile::richHeader()
	{
		return m_richheader;
	}

	const CoffSymbolTable& PeFile::coffSymTab() const
	{
		return m_coffsymtab;
	}

	CoffSymbolTable& PeFile::coffSymTab()
	{
		return m_coffsymtab;
	}

	const SecurityDirectory& PeFile::securityDir() const
	{
		return m_secdir;
	}

	SecurityDirectory& PeFile::securityDir()
	{
		return m_secdir;
	}
}
