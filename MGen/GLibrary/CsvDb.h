#pragma once

class CCsvDb
{
public:
	CCsvDb();
	~CCsvDb();

	CString Open(CString pth);
	CString Create(CString pth, vector<CString>& hdr);
	CString Insert(map<CString, CString>& row);
	CString LoadHeader(ifstream & fs);

	CString Select();

	CString path;
	CString sep_st;
	map<CString, int> header;
	map<CString, CString> filter;

	//Parameters
	CString separator = "\t";

private:
	ifstream ifs;
	ofstream ofs;

};

