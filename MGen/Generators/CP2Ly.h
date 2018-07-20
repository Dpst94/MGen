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
	vector<int> nff; // [] Note flag foreign
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
	CString GetLyNoteCP();
	void SendLyEvent(CString ev);
	void CheckLyCP();

	CString ly_com_st; // Comments
	CString ly_ly_st; // Lyrics
	vector<LY_IntermediateCP> lyi;
	int ly_flags = 0; // Number of flags in current melody

};

