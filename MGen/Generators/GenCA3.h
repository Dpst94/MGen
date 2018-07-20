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

protected:
	void InitAnalysis();
	void LoadConfigLine(CString * sN, CString * sV, int idata, float fdata);
	int XML_to_CP();
	int CheckXML();
	int GetCP();
	void SaveLy(CString dir, CString fname) override;

	XFIn xfi;

	vector<vector<vector<int>>> cp; // [cp_id][v][s]
	vector<vector<vector<int>>> cp_retr; // [cp_id][v][s]
	vector<vector<int>> cp_mea; // [cp_id][s]
	vector<vector<int>> cp_vid; // [cp_id][s] voice id
	vector<int> cp_fi; // [cp_id] fifths
	vector<int> cp_error; // [cp_id] error on load
	vector<CString> cp_text; // [cp_id]

};

