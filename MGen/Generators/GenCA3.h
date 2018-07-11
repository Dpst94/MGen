#pragma once
#include "GenCP2.h"
#include "../GLibrary/XFIn.h"

class CGenCA3 :
	public CGenCP2
{
public:
	CGenCA3();
	~CGenCA3();
	void Generate() override;
	void LoadConfigLine(CString * sN, CString * sV, int idata, float fdata);
	void SaveLy(CString dir, CString fname) override;

	XFIn xfi;
};

