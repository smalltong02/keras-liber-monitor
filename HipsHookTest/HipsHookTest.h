#pragma once
#include <stdio.h>
#include <windows.h>
#include <string>
#include <vector>
#include "gtest\gtest.h"
#include "resource.h"
#include "HipsCfgObject.h"
#include "HookImplementObject.h"
#include "utils.h"
#include "commutils.h"

class HipsHookTest
{
public:
	HipsHookTest() : m_valid(false), m_hipsCfgObject(nullptr), m_hookImplObject(nullptr) {}
	~HipsHookTest() {}

	bool Initialize() {
		m_hookImplObject = g_impl_object;
		m_hipsCfgObject = InitializeConfig();
		if (!m_hookImplObject) return false;
		if (!m_hipsCfgObject) return false;
		if (!m_hookImplObject->Initialize(m_hipsCfgObject))
			return false;
		if (!m_hookImplObject->HookAllApis())
			return false;
		m_valid = true;
		return m_valid;
	}

	bool EnableAllApis() { return m_hookImplObject->EnableAllApis(); }
	bool DisableAllApis() { return m_hookImplObject->DisableAllApis(); }

private:
	bool m_valid;
	std::shared_ptr<cchips::CHipsCfgObject> m_hipsCfgObject;
	std::shared_ptr<cchips::CHookImplementObject> m_hookImplObject;
};

extern std::unique_ptr<HipsHookTest> g_hook_test_object;




