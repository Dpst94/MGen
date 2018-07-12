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

	int av_cnt;
	int c_len = 0;
	int ep2;
	vector<int> minl;
	vector<int> maxl;
	vector<int> fli_size;

	int cp_tempo = 100;
	int step0 = 0;

	// Main vectors
	vector<int> vid; // [v] Voice id
	vector<vector<int>> c; // [v][s] Diatonic
	vector<vector<int>> cc; // [v][s] Chromatic
	vector<vector<int>> pc; // [v][s] Pitch class (diatonic)
	vector<vector<int>> pcc; // [v][s] Pitch class (chromatic)
	vector<vector<int>> leap; // [v][s] Leaps
	vector<vector<int>> smooth; // [v][s] Smooth movements
	vector<vector<int>> slur; // [v][s] Slurs
	vector<vector<int>> retr; // [v][s] Equals 1 if note should be retriggered
	vector<vector<int>> fli; // [ls] Forward links to start of each non-slurred note
	vector<vector<int>> fli2; // [ls] Forward links to end of each non-slurred note
	vector<vector<int>> llen; // [ls] Length of each linked note in steps
	vector<vector<int>> rlen; // [ls] Real length of each linked note (in croches)
	vector<vector<int>> bli; // [s] Back links from each step to fli2

	// Check data ready
	vector<int> data_ready; // If data is ready to be used
	vector<int> warn_data_ready; // How many warnings of data ready fired
	vector<int> data_ready_persist; // If data is ready to be used (not cleared by ClearReady)
	vector<int> warn_data_ready_persist; // How many warnings of data ready fired

};

