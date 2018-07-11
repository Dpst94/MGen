#pragma once
#include "..\GLibrary\GTemplate.h"

class CP2D :
	public CGTemplate
{
public:
	CP2D();
	~CP2D();

protected:
	void LoadConfigLine(CString * sN, CString * sV, int idata, float fdata);

};

