#pragma once
#include "../stdafx.h"

class CCsvDb
{
public:
	CCsvDb();
	~CCsvDb();

	CString Open(CString pth);
	CString Create(CString pth, map<CString, CString> hdr);

	CString Select(map<short, CString> filter);

	CString path;

};

