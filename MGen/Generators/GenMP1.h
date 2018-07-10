#pragma once

#include "../GLibrary/GTemplate.h"
class CGenMP1 :
	public CGTemplate
{
public:
	CGenMP1();
	~CGenMP1();
	void Generate() override;
	void LoadConfigLine(CString * sN, CString * sV, int idata, float fdata);
	void SaveLy(CString dir, CString fname) override;

	CString midi_file;
};
