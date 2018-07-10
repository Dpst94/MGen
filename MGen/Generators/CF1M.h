#pragma once
#include "CF1D.h"
class CF1M :
	public CF1D
{
public:
	CF1M();
	~CF1M();

protected:
	void LoadCantus(CString path);

	vector< vector <int> > cantus; // Cantus loaded from midi file
	vector< vector <CString> > cantus_incom; // Cantus lyrics loaded from midi file
	vector< vector <int> > cantus_len; // Cantus loaded from midi file
	vector< vector <float> > cantus_tempo; // Cantus loaded from midi file
	vector< vector <vector<int>>> cpoint; // Counterpoint loaded from midi file
	vector< vector <vector<int>>> cp_retrig; // [cantus_id][v][s] Counterpoint loaded from midi file

};

