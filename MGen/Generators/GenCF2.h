#pragma once
#include "../GLibrary/GTemplate.h"

class CGenCF2 :
	public CGTemplate
{
public:
	CGenCF2();
	~CGenCF2();
	void Generate() override;
	void LoadConfigLine(CString * sN, CString * sV, int idata, float fdata);
};

