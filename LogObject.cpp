#include "stdafx.h"
#include "LogObject.h"

namespace cchips {

int CLogObject::m_reference_count = 0;
std::unique_ptr<CLogObject> g_log_object(CLogObject::GetInstance());

bool CLogObject::Initialize()
{
	return true;
}

} // namespace cchips