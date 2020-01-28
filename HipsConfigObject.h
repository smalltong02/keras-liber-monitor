#pragma once
#include <stdio.h>
#include <windows.h>
#include <string>
#include <vector>
#include <map>
#include "SigsObject.h"
#include "FlagsObject.h"
#include "ComsObject.h"

#define error_log()
#define except_throw()

#define FL_NAME "Name"
#define FL_VERSION "Version"
#define FL_CREATEDATE "CreateDate"
#define FL_DESCRIPTION "Description"
#define FL_SIGS "Sigs"
#define FL_COMS "Coms"
#define FL_FLAGS "Flags"

class CSigsConfigObject;

class CHipsConfigObject
{
public:
	typedef struct _HipsInfo {
		std::wstring Name;
		std::wstring Version;
		std::wstring CreateDate;
		std::wstring Description;
	}HipsInfo, *PHipsInfo;

	CHipsConfigObject() { m_bValid = false; }
	~CHipsConfigObject() { Clear(); }

	void Clear();
	bool Initialize(std::string& json_str);
	bool GetInfo(HipsInfo& info);
	
	int GetApiCount();
	std::vector<CSigsConfigObject*>& GetSigsObjects() { return m_SigsObject; }
	std::vector<CComsConfigObject*>& GetComsObjects() { return m_ComsObject; }
	std::vector<CFlagsConfigObject*>& GetFlagsObjects() { return m_FlagsObject; }
	bool GetTypesInfo(std::wstring type_name, CFlagsConfigObject::TypesInfo& types_info);
	int GetTypesSize(std::wstring type_name);

	static const int InvalidOrdinal = -1;
private:
	bool m_bValid;
	HipsInfo m_Info;
	std::vector<CSigsConfigObject*> m_SigsObject;
	std::vector<CComsConfigObject*> m_ComsObject;
	std::vector<CFlagsConfigObject*> m_FlagsObject;
};