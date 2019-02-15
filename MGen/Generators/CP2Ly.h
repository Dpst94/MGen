#pragma once
#include "CP2R.h"

struct LY_Shape {
	int start; // If current step starts new shape
	int start_s; // Link to shape start position if current position is shape finish
	int fin; // If current step finishes new shape
	int fl = -1; // Flag index in nflags
	int fs = -1; // Flag step
	int sev = -1; // Highest severity of starting shape
	int fv = -1; // Flag voice (differs from shape voice when shape is in separate staff)
	CString txt; // Starting shape text
};

struct LY_Flag {
	int fid; // Flag id
	int shide = 0; // If flag shape should not be displayed (if sent to separate staff)
	int fhide = 0; // If flag number should not be displayed (if already displayed in shape)
	int fsev; // Severity for each flag
	int sl; // Current flag step link
	int v; // Current flag source voice
	int vl; // Current flag voice link
	int s_base; // Step in base flag voice, where it is located in lyv
	int f_base; // Flag index of base flag in lyv
	int s_src; // Source flag position
	int sl_src; // Source flag link
	int dfgn = 0; // Displayed flag global number
	int sh = 0; // Note flag shape
	int sep = 0; // If this flag is sent to separate staff
	bool operator<(const LY_Flag &rhs) const { return fsev < rhs.fsev; }
};

struct LY_Step {
	int dfgn_count = 0;
};

struct LY_Voice {
	int flags = 0;
	int shapes = 0;
	int note_names = 0;
	int intervals = 0;
	vector<vector<LY_Flag>> f; // [s][] LY Flag
	vector<vector<LY_Shape>> s; // [s][shape_type] LY Shape
	vector<LY_Step> st; // [s] LY Step
	vector<vector<pair<int, int>>> fss; // Flags severity sequence for mistakes list
	vector<vector<pair<int, int>>> fss2; // Flags severity sequence for displaying flag numbers
};

class CP2Ly :
	public CP2R
{
protected:
	CP2Ly();
	~CP2Ly();
	void InitLy();
	inline int GetFCount();
	void DistributeFlags();
	void ParseLy();
	void HideFlags();
	void SortFlagsBySev();
	void SortFlagsBySev2();
	void SaveLyCP();
	void SendLyIntervals();
	void SendLySeparate();
	vector<CString> ly_st; // Resulting text for each counterpoint

private:
	CString SendLySkips(int count);
	CString GetRealNoteNameCP(int no);
	void SendLyHarm();
	void SendLyNoteNames();
	void SendLyMistakes();
	CString GetLyNoteVisualCP(CString sz);
	void SendLyViz(int phase);
	void SaveLyComments();
	CString GetLyNoteCP();
	inline int GetNoteStart(int voice, int step);
	inline void AddNLink(int f);
	void ParseNLinks();
	void SetLyShape(int st1, int st2, int f, int fl, int sev, int vtype, int voice);
	void ClearLyShape(int s1, int s2, int vtype);
	void SendLyEvent(CString ev, int leng);
	void CheckLyCP();
	void SplitLyNoteMeasure(int pos, vector<int>& la);
	void SplitLyNote5(int pos, int i, vector<int>& la);
	void SplitLyNote9(int pos, int i, vector<int>& la);
	void SplitLyNote10(int pos, int i, vector<int>& la);
	void SplitLyNote11(int pos, int i, vector<int>& la);
	void SplitLyNote(int pos, vector<int>& la);
	CString GetRealIntName(int s, int v1, int v2);
	void ParseLyI();
	void ParseLyISep();
	void ExportLy();

	CString ly_com_st; // Comments
	CString ly_ly_st; // Lyrics
	vector<LY_Voice> lyv; // [v] LY Voice
	int ly_flags = 0; // Number of flags sent

	// Unique flags
	set<int> ly_ufl; // Unique flag checker
};

