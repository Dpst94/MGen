#pragma once
#include "CP2Ly.h"

class CGenCP2 :
	public CP2Ly
{
public:
	CGenCP2();
	~CGenCP2();
	void Generate() override;
	void LoadConfigLine(CString * sN, CString * sV, int idata, float fdata);
	void SaveLy(CString dir, CString fname) override;
};

