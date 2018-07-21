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
	vector<int> fsev; // [] Severity for each flag
	vector<int> nfl; // [] Current flags links
	vector<int> nfn; // [] Note flag number
	vector<int> only_shape; // [] If note flag number should not be shown
	vector<int> nfs; // [] Note flag shape
	vector<CString> nfc; // [] Note flag comment
};

class CP2Ly :
	public CP2R
{
protected:
	CP2Ly();
	~CP2Ly();
	void SaveLyCP();
	vector<CString> ly_st; // Resulting text for each counterpoint

private:
	CString SendLySkips(int count);
	void SendLyMistakes();
	CString GetLyNoteVisualCP(CString sz);
	void SaveLyComments();
	CString GetLyNoteCP();
	void AddNLink(int f);
	void ParseNLinks();
	void SetLyShape(int s1, int s2, int f, int fl, int sev, int vtype);
	void ClearLyShape(int s1, int s2, int vtype);
	void InitLyI();
	void SendLyEvent(CString ev);
	void CheckLyCP();
	void ExportLyI();

	CString ly_com_st; // Comments
	CString ly_ly_st; // Lyrics
	vector<LY_IntermediateCP> lyi;
	int ly_flags = 0; // Number of flags sent
	int ly_vflags = 0; // Number of flags in current voice
	int ly_notenames = 0; // Number of note names in current melody

  // Parameters
	int ly_msh = 1; // 0 - do not show msh, 1 - show msh
	int ly_rule_verbose = 0; // How verbose rule display format is

};

