#pragma once

class CCsvDb
{
public:
	CCsvDb();
	~CCsvDb();

	CString Open(CString pth);
	CString Create(CString pth, vector<CString>& hdr);
	CString Insert(map<CString, CString>& row);
	CString InsertMultiple(vector<map<CString, CString>>& rows);
	CString LoadHeader(ifstream & fs);

	CString Select();

	CString Delete();

	CString path;
	CString sep_st;
	CString header_st;
	map<CString, int> header;
	map<CString, CString> filter;
	vector<map<CString, CString>> result;

	//Parameters
	CString separator = ";";

private:
	ifstream ifs;
	ofstream ofs;

};

