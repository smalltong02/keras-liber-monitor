#include "stdafx.h"
#include "HipsConfigObject.h"
#include "utils.h"
#include "rapidjson\document.h"

void CHipsConfigObject::Clear()
{
	m_bValid = false;
	m_Info.Name.clear();
	m_Info.Version.clear();
	m_Info.CreateDate.clear();
	m_Info.Description.clear();

	for (auto& sigs_object : m_SigsObject)
	{
		delete sigs_object;
	}
	for (auto& coms_object : m_ComsObject)
	{
		delete coms_object;
	}
	for (auto& flags_object : m_FlagsObject)
	{
		delete flags_object;
	}

	m_SigsObject.clear();
	m_ComsObject.clear();
	m_FlagsObject.clear();
}

bool CHipsConfigObject::Initialize(std::string& json_str)
{
	if (m_bValid)
	{
		Clear();
	}

	if (json_str.length() == 0) return false;

	HMODULE ModuleHandle = NULL;
	std::vector<BYTE> ResBuffer;
	ModuleHandle = GetModuleHandle(L"hipshook.dll");

	rapidjson::Document document;
	document.Parse(json_str.c_str());
	if (!document.IsObject() || document.IsNull())
	{
		//config data is incorrect.
		error_log("CHipsConfigObject::Initialize failed!");
		return false;
	}

	// initialize info header
	if (!document[FL_NAME].IsNull() && document[FL_NAME].IsString())
	{
		m_Info.Name = A2W(document[FL_NAME].GetString());
	}
	if (!document[FL_VERSION].IsNull() && document[FL_VERSION].IsString())
	{
		m_Info.Version = A2W(document[FL_VERSION].GetString());
	}
	if (!document[FL_CREATEDATE].IsNull() && document[FL_CREATEDATE].IsString())
	{
		m_Info.CreateDate = A2W(document[FL_CREATEDATE].GetString());
	}
	if (!document[FL_DESCRIPTION].IsNull() && document[FL_DESCRIPTION].IsString())
	{
		m_Info.Description = A2W(document[FL_DESCRIPTION].GetString());
	}

	// initialize flags array.
	if (document.HasMember(FL_FLAGS) && document[FL_FLAGS].IsArray())
	{
		for (auto& flags_array : document[FL_FLAGS].GetArray())
		{
			DWORD flags_ord = flags_array.GetInt();

			bool result = ExtractResource(ModuleHandle,
				L"JSONRES",
				MAKEINTRESOURCE(flags_ord),
				ResBuffer);
			if (result && ResBuffer.size() > 0)
			{
				wchar_t flags_name[50];
				wsprintf(flags_name, L"JSONRES_%d", flags_ord);
				CFlagsConfigObject* flags_object = new CFlagsConfigObject(flags_name);
				if (flags_object && flags_object->Initialize(std::string((char*)&ResBuffer[0], ResBuffer.size())))
				{
					m_FlagsObject.push_back(flags_object);
				}
			}
			else
			{
				error_log("CHipsConfigObject::Initialize extract(flags) %ws failed!", flags_name.c_str());
			}
		}
	}

	// initialize sigs array.
	if (document.HasMember(FL_SIGS) && document[FL_SIGS].IsArray())
	{
		for (auto& sigs_array : document[FL_SIGS].GetArray())
		{
			DWORD sigs_ord = sigs_array.GetInt();

			bool result = ExtractResource(ModuleHandle,
				L"JSONRES",
				MAKEINTRESOURCE(sigs_ord),
				ResBuffer);
			if (result && ResBuffer.size() > 0)
			{
				wchar_t sigs_name[50];
				wsprintf(sigs_name, L"JSONRES_%d", sigs_ord);
				CSigsConfigObject* sigs_object = new CSigsConfigObject(sigs_name, this);
				if (sigs_object && sigs_object->Initialize(std::string((char*)&ResBuffer[0], ResBuffer.size())))
				{
					m_SigsObject.push_back(sigs_object);
				}
			}
			else
			{
				error_log("CHipsConfigObject::Initialize extract(sigs) %ws failed!", flags_name.c_str());
			}
		}
	}
	
	// initialize coms array.
	if (document.HasMember(FL_COMS) && document[FL_COMS].IsArray())
	{
		for (auto& coms_array : document[FL_COMS].GetArray())
		{
			DWORD coms_ord = coms_array.GetInt();

			bool result = ExtractResource(ModuleHandle,
				L"JSONRES",
				MAKEINTRESOURCE(coms_ord),
				ResBuffer);
			if (result && ResBuffer.size() > 0)
			{
				wchar_t coms_name[50];
				wsprintf(coms_name, L"JSONRES_%d", coms_ord);
				CComsConfigObject* coms_object = new CComsConfigObject(coms_name);
				if (coms_object && coms_object->Initialize(&ResBuffer[0], ResBuffer.size()))
				{
					m_ComsObject.push_back(coms_object);
				}
			}
			else
			{
				error_log("CHipsConfigObject::Initialize extract(coms) %ws failed!", flags_name.c_str());
			}
		}
	}

	m_bValid = true;
	return true;
}

bool CHipsConfigObject::GetInfo(HipsInfo& info)
{
	if (!m_bValid) return false;
	info = m_Info;
	return true;
}

int CHipsConfigObject::GetApiCount()
{
	int count = 0;
	for (auto& sigs_obj : GetSigsObjects())
	{
		count += sigs_obj->GetApiObjects().size();
	}
	return count;
}

bool CHipsConfigObject::GetTypesInfo(std::wstring type_name, CFlagsConfigObject::TypesInfo& types_info)
{
	int bret = false;
	for (auto& flags_obj : GetFlagsObjects())
	{
		if ((bret = flags_obj->GetTypesInfo(type_name, types_info)))
			break;
	}
	return bret;
}

int CHipsConfigObject::GetTypesSize(std::wstring type_name)
{
	int type_size = InvalidOrdinal;
	for (auto& flags_obj : GetFlagsObjects())
	{
		if ((type_size = flags_obj->GetTypesSize(type_name)) != InvalidOrdinal)
			break;
	}
	return type_size;
}



