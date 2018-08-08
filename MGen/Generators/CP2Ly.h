#pragma once
#include "CP2R.h"

struct LY_IntermediateCP {
	vector<int> shs; // [shape_type] If current step starts new shape
	vector<int> shsl; // [shape_type] Link to shape start position if current position is shape finish
	vector<int> shf; // [shape_type] If current step finishes new shape
	vector<int> shflag; // [shape_type] Flag index in nflags
	vector<int> shfp; // [shape_type] Flag position
	vector<int> shse; // [shape_type] Highest severity of starting shape
	vector<CString> sht; // [shape_type] Starting shape text
	vector<int> nflags; // [] Current flags
	vector<int> fhide; // [] If flag shape should not be displayed (if sent to separate staff)
	vector<int> fsev; // [] Severity for each flag
	vector<int> fsl; // [] Current flag step links
	vector<int> fs_src; // [] Source flag positions
	vector<int> fsl_src; // [] Source flag links
	vector<int> fv; // [] Current flag source voice
	vector<int> fvl; // [] Current flag voice links
	vector<int> nfn; // [] Note flag number
	vector<int> nfs; // [] Note flag shape
	vector<CString> nfc; // [] Note flag comment
};

class CP2Ly :
	public CP2R
{
protected:
	CP2Ly();
	~CP2Ly();
	void InitFSep();
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
	inline void AddNLink(int f);
	inline void AddNLinkForeign(int f);
	void AddNLinkSep(int f);
	void ParseNLinks();
	void ParseNLinksSep();
	void SetLyShape(int s1, int s2, int f, int fl, int sev, int vtype);
	void ClearLyShape(int s1, int s2, int vtype);
	void InitLyI();
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
	void ExportLyI();

	CString ly_com_st; // Comments
	CString ly_ly_st; // Lyrics
	vector<LY_IntermediateCP> lyi; // Current voice
	vector<LY_IntermediateCP> slyi; // Separate staff
	int ly_flags = 0; // Number of flags sent
	int ly_vflags = 0; // Number of flags in current voice
	int ly_notenames = 0; // Number of note names in current melody

	// Unique flags
	set<int> ly_ufl; // Unique flag checker
};

