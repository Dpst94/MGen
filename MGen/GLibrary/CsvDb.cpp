// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "../stdafx.h"
#include "CsvDb.h"

#ifdef _DEBUG
#define new DEBUG_NEW 
#endif

CCsvDb::CCsvDb() {
}

CCsvDb::~CCsvDb() {
}

CString CCsvDb::Open(CString pth) {
	path = pth;
	/*
	if (!CGLib::fileExists(path)) {
		CString est;
		est.Format("Cannot find file %s", path);
		return est;
	}
	*/
	ifs.open(path);
	CString est = LoadHeader(ifs);
	if (est != "") {
		ifs.close();
		return est;
	}
	ifs.close();
	return "";
}

CString CCsvDb::Create(CString pth, vector<CString> &hdr) {
	path = pth;
	// Copy header
	header.clear();
	int i = 0;
	CString st;
	for (auto const& it : hdr) {
		header[it] = i;
		if (!st.IsEmpty()) st += separator;
		st += it;
		++i;
	}
	DeleteFile(path);
	ofs.open(path);
	ofs << "sep=" << separator << "\n";
	ofs << st << "\n";
	ofs.close();
	return "";
}

CString CCsvDb::Insert(map<CString, CString> &row) {
	CString st;
	vector <CString> ast;
	ast.resize(header.size());
	for (auto const& it : row) {
		if (header.find(it.first) == header.end())
			return "Cannot find field '" + it.first + "' in file " + path;
		ast[header[it.first]] = it.second;
	}
	for (int i = 0; i < ast.size(); ++i) {
		if (!st.IsEmpty()) st += separator;
		st += ast[i];
	}
	ofs.open(path, ios_base::app);
	ofs << st << "\n";
	ofs.close();
	return "";
}

CString CCsvDb::LoadHeader(ifstream &ifs) {
	CString st;
	CString header_st;
	vector<CString> ast;
	char pch[2550];
	// Get sep
	if (!ifs.good()) {
		return "Two header lines not found in file " + path;
	}
	ifs.getline(pch, 2550);
	sep_st = pch;
	// Get header
	if (!ifs.good()) {
		return "Two header lines not found in file " + path;
	}
	ifs.getline(pch, 2550);
	header_st = pch;
	// Parse header
	//CGLib::Tokenize(header_st, ast, CSV_SEPARATOR);
	header.clear();
	for (int i = 0; i < ast.size(); ++i) {
		header[ast[i]] = i;
	}
}

CString CCsvDb::Select() {
	CString st;
	int pos, i;
	char pch[2550];
	// Load logs
	ifs.open(path);
	CString est = LoadHeader(ifs);
	if (est != "") {
		ifs.close();
		return est;
	}
	pos = 0;
	i = 0;
	while (ifs.good()) {
		ifs.getline(pch, 2550);
		st = pch;
	}
	ifs.close();
	return "";
}

