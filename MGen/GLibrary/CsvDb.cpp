// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "CsvDb.h"

#ifdef _DEBUG
#define new DEBUG_NEW 
#endif

CCsvDb::CCsvDb() {
}

CCsvDb::~CCsvDb() {
}

CString CCsvDb::Open(CString pth) {
	return "";
}

CString CCsvDb::Select(map<short, CString> filter) {
	CString st;
	ifstream fs;
	int pos, i;
	char pch[2550];
	// Load logs
	if (!fileExists(path)) {
		CString est;
		est.Format("Cannot find file %s", path);
		return est;
	}
	fs.open(path);
	// Get sep
	if (!fs.good()) {
		return "Two header lines not found in file " + path;
	}
	fs.getline(pch, 2550);
	// Get header
	if (!fs.good()) {
		return "Two header lines not found in file " + path;
	}
	fs.getline(pch, 2550);
	pos = 0;
	i = 0;
	while (fs.good()) {
		fs.getline(pch, 2550);
		st = pch;
		if (!st.IsEmpty()) WriteLog(3, st);
		if (++i > MAX_LOAD_LOG) break;
	}
	fs.close();
	return "";
}
