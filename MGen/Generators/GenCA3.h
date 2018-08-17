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

private:
	void InitAnalysis();
	void GetAnalysisVectors();
	void LoadConfigLine(CString * sN, CString * sV, int idata, float fdata);
	int XML_to_CP();
	int CheckXML();
	int GetCP();
	int GetCPSpecies();
	void SaveLy(CString dir, CString fname) override;
	void GetCPKey();
	int GetVocalRanges();
	void ScanVocalRanges();
	inline void EvalVocalRanges();
	void GetPossibleVocalRanges();
	int AnalyseCP();
	int FailSpeciesCombination();

	XFIn xfi;

	vector<vector<vector<int>>> cp; // [cp_id][v][s]
	vector<vector<vector<int>>> cp_alter; // [cp_id][v][s]
	vector<vector<vector<int>>> cp_retr; // [cp_id][v][s]
	vector<vector<int>> cp_mea; // [cp_id][s]
	vector<vector<int>> cp_vid; // [cp_id][s] voice id
	vector<int> cp_fi; // [cp_id] fifths
	vector<int> cp_btype; // [cp_id] beat type
	vector<int> cp_error; // [cp_id] error on load
	vector<CString> cp_text; // [cp_id]

};

