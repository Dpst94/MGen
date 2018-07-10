// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "../stdafx.h"
#include "GenCA3.h"

#ifdef _DEBUG
#define new DEBUG_NEW 
#endif

CGenCA3::CGenCA3() {
}

CGenCA3::~CGenCA3() {
}

void CGenCA3::LoadConfigLine(CString* sN, CString* sV, int idata, float fdata) {
	CGenCP2::LoadConfigLine(sN, sV, idata, fdata);
}

void CGenCA3::Generate() {
}

void CGenCA3::SaveLy(CString dir, CString fname) {
}
