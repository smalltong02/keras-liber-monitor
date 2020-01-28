#pragma once
#include <stdio.h>
#include <windows.h>
#include <string>
#include <vector>

class CComsConfigObject
{
public:
	CComsConfigObject() { ; }
	CComsConfigObject(std::wstring name) { m_name = name; }
	~CComsConfigObject() { ; }

	bool Initialize(void* initData, int len);
private:
	std::wstring m_name;
};