// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#pragma once
#include "Ly.h"

class CGTemplate :
	public CLy
{
public:
	CGTemplate();
	~CGTemplate();

	virtual void Generate() = 0;
	virtual void SaveLy(CString dir, CString fname) = 0;
};
