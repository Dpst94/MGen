// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "../stdafx.h"
#include "CsvDb.h"
#include "GLib.h"

#ifdef _DEBUG
#define new DEBUG_NEW 
#endif

CCsvDb::CCsvDb() {
}

CCsvDb::~CCsvDb() {
}

CString CCsvDb::Open(CString pth) {
	path = pth;
	if (!CGLib::fileExists(path)) {
		CString est;
		est.Format("Cannot find file %s", path);
		return est;
	}
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

CString CCsvDb::InsertMultiple(vector<map<CString, CString>> &rows) {
	CString st;
	vector <CString> ast;
	ast.resize(header.size());
	ofs.open(path, ios_base::app);
	for (int r = 0; r < rows.size(); ++r) {
		for (auto const& it : rows[r]) {
			if (header.find(it.first) == header.end())
				return "Cannot find field '" + it.first + "' in file " + path;
			ast[header[it.first]] = it.second;
		}
		for (int i = 0; i < ast.size(); ++i) {
			if (!st.IsEmpty()) st += separator;
			st += ast[i];
		}
		ofs << st << "\n";
	}
	ofs.close();
	return "";
}

CString CCsvDb::LoadHeader(ifstream &ifs) {
	CString st;
	vector<CString> ast;
	char pch[2550];
	// Get sep
	if (!ifs.good()) {
		return "Header line not found in file " + path;
	}
	// Load separator
	ifs.getline(pch, 2550);
	st = pch;
	if (st.Left(4) == "sep=") {
		sep_st = st;
		separator = st[4];
		// Get header
		if (!ifs.good()) {
			return "Header line after sep line not found in file " + path;
		}
		ifs.getline(pch, 2550);
	}
	else {
		sep_st = "";
	}
	header_st = pch;
	// Parse header
	CGLib::Tokenize(header_st, ast, separator);
	header.clear();
	for (int i = 0; i < ast.size(); ++i) {
		header[ast[i]] = i;
	}
	return "";
}

CString CCsvDb::Select() {
	CString st;
	vector<CString> ast;
	result.clear();
	int pos, i;
	char pch[2550];
	// Load logs
	ifs.open(path);
	CString est = LoadHeader(ifs);
	if (est != "") {
		ifs.close();
		return est;
	}
	while (ifs.good()) {
		ifs.getline(pch, 2550);
		st = pch;
		CGLib::Tokenize(st, ast, separator);
		// Finish on empty line
		if (st == "") break;
		// Check column count
		if (ast.size() != header.size())
			return "Wrong column count in file " + path + " at line " + st;
		if (filter.size()) {
			int found = 1;
			for (auto const& it : filter) {
				if (ast[header[it.first]] != it.second) {
					found = 0;
					break;
				}
			}
			if (!found) continue;
		}
		// Save
		result.resize(result.size() + 1);
		for (auto const& it : header) {
			result[result.size() - 1][it.first] = ast[it.second];
		}
	}
	ifs.close();
	return "";
}

CString CCsvDb::Delete() {
	CString st;
	vector<CString> ast, tsa;
	result.clear();
	int pos, i;
	char pch[2550];
	// Load logs
	ifs.open(path);
	CString est = LoadHeader(ifs);
	if (est != "") {
		ifs.close();
		return est;
	}
	while (ifs.good()) {
		ifs.getline(pch, 2550);
		st = pch;
		CGLib::Tokenize(st, ast, separator);
		// Finish on empty line
		if (st == "") break;
		// Check column count
		if (ast.size() != header.size())
			return "Wrong column count in file " + path + " at line " + st;
		if (filter.size()) {
			int found = 1;
			for (auto const& it : filter) {
				if (ast[header[it.first]] != it.second) {
					found = 0;
					break;
				}
			}
			if (found) continue;
		}
		// Save
		tsa.push_back(st);
	}
	ifs.close();
	// Write
	DeleteFile(path);
	ofs.open(path);
	ofs << sep_st << "\n";
	ofs << header_st << "\n";
	for (int i = 0; i < tsa.size(); ++i) {
		ofs << tsa[i] << "\n";
	}
	ofs.close();
	return "";
}

