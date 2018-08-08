#pragma once
#include "CF1R.h"

struct LY_Intermediate {
	vector<int> shs; // [shape_type] If current step starts new shape
	vector<int> shsl; // [shape_type] Link to shape start position if current position is shape finish
	vector<int> shf; // [shape_type] If current step finishes new shape
	vector<int> shflag; // [shape_type] Flag index in nflags
	vector<int> shfp; // [shape_type] Flag position
	vector<int> shse; // [shape_type] Highest severity of starting shape
	vector<CString> sht; // [shape_type] Starting shape text
	vector<int> nflags; // [] Current flags
	vector<int> fsev; // [] Severity for each flag
	vector<int> nfl; // [] Current flags links
	vector<int> nfn; // [] Note flag number
	vector<int> nff; // [] Note flag foreign
	vector<int> nfs; // [] Note flag shape
	vector<CString> nfc; // [] Note flag comment
};

class CF1Ly :
	public CF1R
{
public:
	CF1Ly();
	~CF1Ly();
	void SaveLy(CString dir, CString fname);

	void SplitLyNoteMeasure(int pos, int le, vector<int>& la);

	void SplitLyNote(int pos, int le, vector<int>& la);

	void SplitLyNote5(int pos, vector<int>& la);

	CString GetRealIntName(int s, int v1, int v2);

protected:
	void GetLyRange(int step1, int step2, vector<int>& vm_min, vector<int>& vm_max);
	void GetLyVcnt(int step1, int step2, vector<int>& vm_max);
	void SendLyViz(ofstream & fs, int pos, CString & ev, int le, int i, int v, int phase);
	void SendLyEvent(ofstream & fs, int pos, CString ev, int le, int i, int v);
	void AddNLink(int i, int i2, int v, int fl, int ln, int foreign);
	void ParseNLinks(int i, int i2, int v, int foreign);
	void SaveLyComments(int i, int v, int pos);
	void SetLyShape(int s1, int s2, int f, int fl, int sev, int vtype);
	void ClearLyShape(int s1, int s2, int vtype);
	void ExportLyI();
	void AddLyITest(int step1, int step2, int fl, int shape);
	void InitLyITest();
	void InitLyI();
	void SaveLySegment(ofstream & fs, int mel, int step1, int step2);
	CString SendLySkips(int count);
	void SendLyMistakes();
	void SendLyHarm();
	void SendLyIntervals();
	void SendLyNoteNames();

	// Lilypond other
	float ly_mul = 1; // midifile_mul
	int ly_nnum = 0; // Note number
	int ly_step1 = 0;
	int ly_step2 = 0;
	int ly_vlow = 0; // Show harmony under this voice
	int ly_vhigh = 0; // Show mistakes above this voice
	int ly_vm_cnt = 0; // Number of voices in segment to display
	int ly_has_lining = 0; // If there is lining in melody
	CString ly_com_st; // Comments
	CString ly_ly_st; // Lyrics
	vector<LY_Intermediate> lyi;
	int ly_vcnt = 0; // Ly voice count
	int ly_v = 0; // Current ly voice
	int ly_v2 = 0; // Second voice for counterpoint analysis
	int ly_s = 0; // Current ly step
	int ly_s2 = 0; // Current ly step inside melody
	int ly_mel = -1; // Currentn ly melody id
	int ly_flags = 0; // Number of flags in current melody
	int ly_notenames = 0; // Number of note names in current melody

};

