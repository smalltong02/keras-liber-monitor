#include "VersionInfoBuilder.h"

namespace cchips {

    struct VS_VERSIONINFO { 
	WORD  wLength; 
	WORD  wValueLength; 
	WORD  wType; 
	WCHAR szKey[1]; 
	WORD  Padding1[1]; 
	VS_FIXEDFILEINFO Value; 
	WORD  Padding2[1]; 
	WORD  Children[1]; 
};

struct String { 
	WORD   wLength; 
	WORD   wValueLength; 
	WORD   wType; 
	WCHAR  szKey[1]; 
	WORD   Padding[1]; 
	WORD   Value[1]; 
}; 

struct StringTable { 
	WORD   wLength; 
	WORD   wValueLength; 
	WORD   wType; 
	WCHAR  szKey[1]; 
	WORD   Padding[1]; 
	String Children[1]; 
};

struct StringFileInfo { 
	WORD        wLength; 
	WORD        wValueLength; 
	WORD        wType; 
	WCHAR       szKey[1]; 
	WORD        Padding[1]; 
	StringTable Children[1]; 
};

struct Var { 
	WORD  wLength; 
	WORD  wValueLength; 
	WORD  wType; 
	WCHAR szKey[1]; 
	WORD  Padding[1]; 
	DWORD Value[1]; 
}; 

struct VarFileInfo { 
	WORD  wLength; 
	WORD  wValueLength; 
	WORD  wType; 
	WCHAR szKey[1]; 
	WORD  Padding[1]; 
	Var   Children[1]; 
}; 

//-------------------------------------------------------------------
// CFileVersionInfo
//-------------------------------------------------------------------

CFileVersionInfo::CFileVersionInfo()
{
	Reset();
}


CFileVersionInfo::~CFileVersionInfo()
{}


BOOL CFileVersionInfo::GetTranslationId(LPVOID lpData, UINT unBlockSize, WORD wLangId, DWORD &dwId, BOOL bPrimaryEnough/*= FALSE*/)
{
	LPWORD lpwData;
	for (lpwData = (LPWORD)lpData; (LPBYTE)lpwData < ((LPBYTE)lpData)+unBlockSize; lpwData+=2)
	{
		if (*lpwData == wLangId)
		{
			dwId = *((DWORD*)lpwData);
			return TRUE;
		}
	}

	if (!bPrimaryEnough)
		return FALSE;

	for (lpwData = (LPWORD)lpData; (LPBYTE)lpwData < ((LPBYTE)lpData)+unBlockSize; lpwData+=2)
	{
		if (((*lpwData)&0x00FF) == (wLangId&0x00FF))
		{
			dwId = *((DWORD*)lpwData);
			return TRUE;
		}
	}

	return FALSE;
}


BOOL CFileVersionInfo::Create(HMODULE hModule /*= NULL*/)
{
	TCHAR	strPath[_MAX_PATH] = { 0 };

	GetModuleFileName(hModule, strPath, _MAX_PATH);
	return Create(strPath);
}

void CFileVersionInfo::SetFileInfo(PCWSTR pKey, PCWSTR pValue)
{
	char szTemp[256] = { 0 };
	if(wcscmp(pKey, L"CompanyName") == 0)
	{
#ifdef UNICODE
		m_strCompanyName = pValue;
#else
		wcstombs(szTemp, pValue, 256);
		m_strCompanyName = szTemp;
#endif
	}
	else if(wcscmp(pKey, L"FileDescription") == 0)
	{
#ifdef UNICODE
		m_strCompanyName = pValue;
#else
		wcstombs(szTemp, pValue, 256);
		m_strFileDescription = szTemp;
#endif
	}
	else if(wcscmp(pKey, L"FileVersion") == 0)
	{
#ifdef UNICODE
		m_strCompanyName = pValue;
#else
		wcstombs(szTemp, pValue, 256);
		m_strFileVersion = szTemp;
#endif
	}
	else if(wcscmp(pKey, L"InternalName") == 0)
	{
#ifdef UNICODE
		m_strCompanyName = pValue;
#else
		wcstombs(szTemp, pValue, 256);
		m_strInternalName = szTemp;
#endif
	}
	else if(wcscmp(pKey, L"LegalCopyright") == 0)
	{
#ifdef UNICODE
		m_strCompanyName = pValue;
#else
		wcstombs(szTemp, pValue, 256);
		m_strLegalCopyright = szTemp;
#endif
	}
	else if(wcscmp(pKey, L"OriginalFilename") == 0)
	{
#ifdef UNICODE
		m_strCompanyName = pValue;
#else
		wcstombs(szTemp, pValue, 256);
		m_strOriginalFileName = szTemp;
#endif
	}
	else if(wcscmp(pKey, L"ProductName") == 0)
	{
#ifdef UNICODE
		m_strCompanyName = pValue;
#else
		wcstombs(szTemp, pValue, 256);
		m_strProductName = szTemp;
#endif
	}
	else if(wcscmp(pKey, L"ProductVersion") == 0)
	{
#ifdef UNICODE
		m_strCompanyName = pValue;
#else
		wcstombs(szTemp, pValue, 256);
		m_strProductVersion = szTemp;
#endif
	}
	else if(wcscmp(pKey, L"LegalTrademarks") == 0)
	{
#ifdef UNICODE
		m_strCompanyName = pValue;
#else
		wcstombs(szTemp, pValue, 256);
		m_strLegalTrademarks = szTemp;
#endif
	}
	else if(wcscmp(pKey, L"Comments") == 0)
	{
#ifdef UNICODE
		m_strCompanyName = pValue;
#else
		wcstombs(szTemp, pValue, 256);
		m_strComments = szTemp;
#endif
	}
	else if(wcscmp(pKey, L"PrivateBuild") == 0)
	{
#ifdef UNICODE
		m_strCompanyName = pValue;
#else
		wcstombs(szTemp, pValue, 256);
		m_strPrivateBuild = szTemp;
#endif
	}
	else if(wcscmp(pKey, L"SpecialBuild") == 0)
	{
#ifdef UNICODE
		m_strCompanyName = pValue;
#else
		wcstombs(szTemp, pValue, 256);
		m_strSpecialBuild = szTemp;
#endif
	}
}

BOOL CFileVersionInfo::RawGetFileInfo(LPVOID pVer, UINT unBlockSize)
{
	// Interpret the VS_VERSIONINFO header pseudo-struct
	VS_VERSIONINFO* pVS = (VS_VERSIONINFO*)pVer;

#define roundoffs(a,b,r)	(((byte*)(b) - (byte*)(a) + ((r)-1)) & ~((r)-1))
#define roundpos(b, a, r)	(((byte*)(a))+roundoffs(a,b,r))

	//	byte* nEndRaw   = roundpos((((byte*)pVer) + size), pVer, 4);
	//	byte* nEndNamed = roundpos((((byte*) pVS) + pVS->wLength), pVS, 4);
	//	ASSERT(nEndRaw == nEndNamed); // size reported from GetFileVersionInfoSize is much padded for some reason...

	if(wcscmp(pVS->szKey, L"VS_VERSION_INFO"))
		return FALSE;
	byte* pVt = (byte*) &pVS->szKey[wcslen(pVS->szKey)+1];
	VS_FIXEDFILEINFO* pValue = (VS_FIXEDFILEINFO*) roundpos(pVt, pVS, 4);
	// Iterate over the 'Children' elements of VS_VERSIONINFO (either StringFileInfo or VarFileInfo)
	StringFileInfo* pSFI = (StringFileInfo*) roundpos(((byte*)pValue) + pVS->wValueLength, pValue, 4);
	for ( ; ((byte*) pSFI) < (((byte*) pVS) + pVS->wLength); pSFI = (StringFileInfo*)roundpos((((byte*) pSFI) + pSFI->wLength), pSFI, 4)) { // StringFileInfo / VarFileInfo
		if (!wcscmp(pSFI->szKey, L"StringFileInfo")) {
			// The current child is a StringFileInfo element
			if(1 != pSFI->wType)
				return FALSE;
			if(pSFI->wValueLength)
				return FALSE;
			// Iterate through the StringTable elements of StringFileInfo
			StringTable* pST = (StringTable*) roundpos(&pSFI->szKey[wcslen(pSFI->szKey)+1], pSFI, 4);
			for ( ; ((byte*) pST) < (((byte*) pSFI) + pSFI->wLength); pST = (StringTable*)roundpos((((byte*) pST) + pST->wLength), pST, 4)) {
				if(pST->wValueLength)
					continue;
				// Iterate through the String elements of StringTable
				String* pS = (String*) roundpos(&pST->szKey[wcslen(pST->szKey)+1], pST, 4);
				for ( ; ((byte*) pS) < (((byte*) pST) + pST->wLength); pS = (String*) roundpos((((byte*) pS) + pS->wLength), pS, 4)) {
					wchar_t* psVal = (wchar_t*) roundpos(&pS->szKey[wcslen(pS->szKey)+1], pS, 4);
					//printf("  %-18S: %.*S\n", pS->szKey, pS->wValueLength, psVal); // print <sKey> : <sValue>
					SetFileInfo(pS->szKey, psVal);
				}
			}
		}
		//else {
		//	// The current child is a VarFileInfo element
		//	ASSERT(1 == pSFI->wType); // ?? it just seems to be this way...
		//	VarFileInfo* pVFI = (VarFileInfo*) pSFI;
		//	ASSERT(!wcscmp(pVFI->szKey, L"VarFileInfo"));
		//	ASSERT(!pVFI->wValueLength);
		//	// Iterate through the Var elements of VarFileInfo (there should be only one, but just in case...)
		//	Var* pV = (Var*) roundpos(&pVFI->szKey[wcslen(pVFI->szKey)+1], pVFI, 4);
		//	for ( ; ((byte*) pV) < (((byte*) pVFI) + pVFI->wLength); pV = (Var*)roundpos((((byte*) pV) + pV->wLength), pV, 4)) {
		//		printf(" %S: ", pV->szKey);
		//		// Iterate through the array of pairs of 16-bit language ID values that make up the standard 'Translation' VarFileInfo element.
		//		WORD* pwV = (WORD*) roundpos(&pV->szKey[wcslen(pV->szKey)+1], pV, 4);
		//		for (WORD* wpos = pwV ; ((byte*) wpos) < (((byte*) pwV) + pV->wValueLength); wpos+=2) {
		//			printf("%04x%04x ", (int)*wpos++, (int)(*(wpos+1)));
		//		}
		//		printf("\n");
		//	}
		//}
	}
//	ASSERT((byte*) pSFI == roundpos((((byte*) pVS) + pVS->wLength), pVS, 4));
//	return pValue->dwFileVersionMS; // !!! return major version number
	return TRUE;
}

BOOL CFileVersionInfo::Create(LPCTSTR lpszFileName)
{
	Reset();

	DWORD	dwHandle;
	DWORD	dwFileVersionInfoSize = GetFileVersionInfoSize((LPTSTR)lpszFileName, &dwHandle);
	if (!dwFileVersionInfoSize)
		return FALSE;

	LPVOID	lpData = (LPVOID)new BYTE[dwFileVersionInfoSize];
	if (!lpData)
		return FALSE;

	try
	{
		if (!GetFileVersionInfo((LPTSTR)lpszFileName, dwHandle, dwFileVersionInfoSize, lpData))
			throw FALSE;

		// catch default information
		LPVOID	lpInfo;
		UINT		unInfoLen;
		if (VerQueryValue(lpData, "\\", &lpInfo, &unInfoLen))
		{
//			ASSERT(unInfoLen == sizeof(m_FileInfo));
			if (unInfoLen == sizeof(m_FileInfo))
				memcpy(&m_FileInfo, lpInfo, unInfoLen);
		}

		// find best matching language and codepage
		VerQueryValue(lpData, "\\VarFileInfo\\Translation", &lpInfo, &unInfoLen);
		
		DWORD	dwLangCode = 0;
		if (!GetTranslationId(lpInfo, unInfoLen, GetUserDefaultLangID(), dwLangCode, FALSE))
		{
			if (!GetTranslationId(lpInfo, unInfoLen, GetUserDefaultLangID(), dwLangCode, TRUE))
			{
				if (!GetTranslationId(lpInfo, unInfoLen, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), dwLangCode, TRUE))
				{
					if (!GetTranslationId(lpInfo, unInfoLen, MAKELANGID(LANG_ENGLISH, SUBLANG_NEUTRAL), dwLangCode, TRUE))
						// use the first one we can get
						dwLangCode = *((DWORD*)lpInfo);
				}
			}
		}
		

		//std::wstring strSubBlock;
		//strSubBlock.Format(_T("\\StringFileInfo\\%04X%04X\\"), dwLangCode&0x0000FFFF, (dwLangCode&0xFFFF0000)>>16);
		std::stringstream ss;
		ss << "\\StringFileInfo\\" << std::setfill('0') << std::setw(4) << std::hex << (dwLangCode & 0x0000FFFF) << std::setfill('0') << std::setw(4) << std::hex << ((dwLangCode & 0xFFFF0000) >> 16) << "\\";
		std::string strSubBlock = ss.str();
		
		// catch string table
		if (VerQueryValue(lpData, (LPTSTR)(LPCTSTR)(strSubBlock + "CompanyName").c_str(), &lpInfo, &unInfoLen))
			m_strCompanyName = std::string((LPCTSTR)lpInfo);
		if (VerQueryValue(lpData, (LPTSTR)(LPCTSTR)(strSubBlock + "FileDescription").c_str(), &lpInfo, &unInfoLen))
			m_strFileDescription = std::string((LPCTSTR)lpInfo);
		if (VerQueryValue(lpData, (LPTSTR)(LPCTSTR)(strSubBlock + "FileVersion").c_str(), &lpInfo, &unInfoLen))
			m_strFileVersion = std::string((LPCTSTR)lpInfo);
		if (VerQueryValue(lpData, (LPTSTR)(LPCTSTR)(strSubBlock + "InternalName").c_str(), &lpInfo, &unInfoLen))
			m_strInternalName = std::string((LPCTSTR)lpInfo);
		if (VerQueryValue(lpData, (LPTSTR)(LPCTSTR)(strSubBlock + "LegalCopyright").c_str(), &lpInfo, &unInfoLen))
			m_strLegalCopyright = std::string((LPCTSTR)lpInfo);
		if (VerQueryValue(lpData, (LPTSTR)(LPCTSTR)(strSubBlock + "OriginalFileName").c_str(), &lpInfo, &unInfoLen))
			m_strOriginalFileName = std::string((LPCTSTR)lpInfo);
		if (VerQueryValue(lpData, (LPTSTR)(LPCTSTR)(strSubBlock + "ProductName").c_str(), &lpInfo, &unInfoLen))
			m_strProductName = std::string((LPCTSTR)lpInfo);
		if (VerQueryValue(lpData, (LPTSTR)(LPCTSTR)(strSubBlock + "ProductVersion").c_str(), &lpInfo, &unInfoLen))
			m_strProductVersion = std::string((LPCTSTR)lpInfo);
		if (VerQueryValue(lpData, (LPTSTR)(LPCTSTR)(strSubBlock + "Comments").c_str(), &lpInfo, &unInfoLen))
			m_strComments = std::string((LPCTSTR)lpInfo);
		if (VerQueryValue(lpData, (LPTSTR)(LPCTSTR)(strSubBlock + "LegalTrademarks").c_str(), &lpInfo, &unInfoLen))
			m_strLegalTrademarks = std::string((LPCTSTR)lpInfo);
		if (VerQueryValue(lpData, (LPTSTR)(LPCTSTR)(strSubBlock + "PrivateBuild").c_str(), &lpInfo, &unInfoLen))
			m_strPrivateBuild = std::string((LPCTSTR)lpInfo);
		if (VerQueryValue(lpData, (LPTSTR)(LPCTSTR)(strSubBlock + "SpecialBuild").c_str(), &lpInfo, &unInfoLen))
			m_strSpecialBuild = std::string((LPCTSTR)lpInfo);

		if(m_strFileVersion.empty())
			RawGetFileInfo(lpData, dwFileVersionInfoSize);

		delete[] lpData;
	}
	catch (BOOL)
	{
		delete[] lpData;
		return FALSE;
	}

	return TRUE;
}


WORD CFileVersionInfo::GetFileVersion(int nIndex) const
{
	if (nIndex == 0)
		return (WORD)(m_FileInfo.dwFileVersionLS & 0x0000FFFF);
	else if (nIndex == 1)
		return (WORD)((m_FileInfo.dwFileVersionLS & 0xFFFF0000) >> 16);
	else if (nIndex == 2)
		return (WORD)(m_FileInfo.dwFileVersionMS & 0x0000FFFF);
	else if (nIndex == 3)
		return (WORD)((m_FileInfo.dwFileVersionMS & 0xFFFF0000) >> 16);
	else
		return 0;
}


WORD CFileVersionInfo::GetProductVersion(int nIndex) const
{
	if (nIndex == 0)
		return (WORD)(m_FileInfo.dwProductVersionLS & 0x0000FFFF);
	else if (nIndex == 1)
		return (WORD)((m_FileInfo.dwProductVersionLS & 0xFFFF0000) >> 16);
	else if (nIndex == 2)
		return (WORD)(m_FileInfo.dwProductVersionMS & 0x0000FFFF);
	else if (nIndex == 3)
		return (WORD)((m_FileInfo.dwProductVersionMS & 0xFFFF0000) >> 16);
	else
		return 0;
}


DWORD CFileVersionInfo::GetFileFlagsMask() const
{
	return m_FileInfo.dwFileFlagsMask;
}


DWORD CFileVersionInfo::GetFileFlags() const
{
	return m_FileInfo.dwFileFlags;
}


DWORD CFileVersionInfo::GetFileOs() const
{
	return m_FileInfo.dwFileOS;
}


DWORD CFileVersionInfo::GetFileType() const
{
	return m_FileInfo.dwFileType;
}

std::string CFileVersionInfo::GetFileTypeStr() const
{
	switch (m_FileInfo.dwFileType) {
	case VFT_APP:
		return "Application";
	case VFT_DLL:
		return "Dll";
	case VFT_DRV:
		return "Driver";
	case VFT_FONT:
		return "Font";
	case VFT_STATIC_LIB:
		return "Static library";
	case VFT_VXD:
		return "Virtual device";
	}
	return "Unknown";
}


DWORD CFileVersionInfo::GetFileSubtype() const
{
	return m_FileInfo.dwFileSubtype;
}


/*
CTime CFileVersionInfo::GetFileDate() const
{
	FILETIME	ft;
	ft.dwLowDateTime = m_FileInfo.dwFileDateLS;
	ft.dwHighDateTime = m_FileInfo.dwFileDateMS;
	return CTime(ft);
}
*/


std::string CFileVersionInfo::GetCompanyName() const
{
	return m_strCompanyName;
}


std::string CFileVersionInfo::GetFileDescription() const
{
	return m_strFileDescription;
}


std::string CFileVersionInfo::GetFileVersion() const
{
	return m_strFileVersion;
}


std::string CFileVersionInfo::GetInternalName() const
{
	return m_strInternalName;
}


std::string CFileVersionInfo::GetLegalCopyright() const
{
	return m_strLegalCopyright;
}


std::string CFileVersionInfo::GetOriginalFileName() const
{
	return m_strOriginalFileName;
}


std::string CFileVersionInfo::GetProductName() const
{
	return m_strProductName;
}


std::string CFileVersionInfo::GetProductVersion() const
{
	return m_strProductVersion;
}


std::string CFileVersionInfo::GetComments() const
{
	return m_strComments;
}


std::string CFileVersionInfo::GetLegalTrademarks() const
{
	return m_strLegalTrademarks;
}


std::string CFileVersionInfo::GetPrivateBuild() const
{
	return m_strPrivateBuild;
}


std::string CFileVersionInfo::GetSpecialBuild() const
{
	return m_strSpecialBuild;
}


void CFileVersionInfo::Reset()
{
	ZeroMemory(&m_FileInfo, sizeof(m_FileInfo));
	m_strCompanyName.clear();
	m_strFileDescription.clear();
	m_strFileVersion.clear();
	m_strInternalName.clear();
	m_strLegalCopyright.clear();
	m_strOriginalFileName.clear();
	m_strProductName.clear();
	m_strProductVersion.clear();
	m_strComments.clear();
	m_strLegalTrademarks.clear();
	m_strPrivateBuild.clear();
	m_strSpecialBuild.clear();
}
} // namespace cchips