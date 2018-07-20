// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "../stdafx.h"
#include "CP2Ly.h"

#ifdef _DEBUG
#define new DEBUG_NEW 
#endif

CP2Ly::CP2Ly() {
}

CP2Ly::~CP2Ly() {
}

CString CP2Ly::GetLyNoteCP() {
	int no2, oct, alter;
	GetRealNote(cc[v][s], bn, mode == 9, no2, oct, alter);
	return LyNoteSharp[no2] + GetLyAlter(alter) + LyOctave[oct];
}

void CP2Ly::SaveLyCP() {
	vector<CString> sv;
	CString clef, key, key_visual;
	int pos, pos2, le, le2, pause_accum, pause_pos, pause_i;
	ly_com_st.Empty();
	ly_ly_st.Empty();
	// Voice melody min pitch
	vector<int> vm_min;
	// Voice melody max pitch
	vector<int> vm_max;
	for (v = 0; v < av_cnt; ++v) {
		GetMelodyInterval(0, c_len);
	}
	CheckLyCP();
	// When debugging expected confirmations, do not show segments without flags
	if (ly_debugexpect && !ly_flags) return;
	// Key
	if (mode == 9) {
		key = LyMinorKey[bn];
	}
	else if (mode == 0) {
		key = LyMajorKey[bn];
	}
	key_visual = key[0];
	key_visual.MakeUpper();
	if (key[1] == 'f') key_visual += "\\flat ";
	if (key[1] == 's') key_visual = "\"" + key_visual + "#\"";
	// First info
	CString st, st3;
	ly_ly_st += "\\markup \\wordwrap \\bold {\n  ";
	ly_ly_st += "    \\vspace #3\n";
	ly_ly_st += " Key: " + key_visual;
	if (mode == 9) ly_ly_st += " minor";
	else if (mode == 0) ly_ly_st += " major";
	ly_ly_st += "\n}\n";
	// Save notes
	ly_ly_st += "<<\n";
	for (v = av_cnt - 1; v >= 0; --v) {
		//InitLyI();
		// Select best clef
		clef = DetectLyClef(nmin[v], nmax[v]);
		st.Format("\\new Staff = \"staff%d\" {\n", v);
		ly_ly_st += st;
		st.Format("  \\set Staff.instrumentName = \"Part %d\"\n", av_cnt - v);
		ly_ly_st += st;
		ly_ly_st += "  \\clef \"" + clef + "\" \\key " + key;
		if (mode == 9) ly_ly_st += " \\minor\n";
		else if (mode == 0) ly_ly_st += " \\major\n";
		read_file_sv("configs\\ly\\staff.ly", sv);
		for (int i = 0; i < sv.size(); ++i) {
			ly_ly_st += sv[i] + "\n";
		}
		ly_ly_st += "  { ";
		for (ls = 0; ls < fli_size[v]; ++ls) {
			s = fli[v][ls];
			if (cc[v][s]) {
				SendLyEvent(GetLyNoteCP());
			}
			else {
				SendLyEvent("r");
			}
		}
		ly_ly_st += "\n  }\n";
		ly_ly_st += "}\n";
		//SendLyMistakes();
		//SendLyNoteNames();
		//SendLyHarm();
		//SendLyIntervals();
	}
	ly_ly_st += ">>\n";
	if (st3 != "") ly_ly_st += "\\markup { " + st3 + " }\n";
	ly_ly_st += ly_com_st;
	if (ly_com_st == "") {
		if (!ly_debugexpect) {
			ly_ly_st += "\\markup \\bold \\with-color #(rgb-color 0 0.8 0) { \\char ##x2705 Excellent }\n ";
		}
	}
	if (ly_st.size() <= cp_id) ly_st.resize(cp_id + 1);
	ly_st[cp_id] = ly_ly_st;
}

// Send note or pause
void CP2Ly::SendLyEvent(CString ev) {
	// Length array
	vector<int> la;
	SplitLyNote(s, llen[v][ls], la);
	SplitLyNoteMeasure(s, llen[v][ls], la);
	for (int lc = 0; lc < la.size(); ++lc) {
		//ly_s = i;
		//ly_s2 = ly_s - ly_step1;
		//SaveLyComments(i, v, pos);
		//SendLyViz(fs, pos, ev, le, i, v, 1);
		ly_ly_st += ev + GetLyLen(la[lc]);
		if (lc < la.size() - 1 && ev != "r") ly_ly_st += "~";
		ly_ly_st += "\n";
		//SendLyViz(fs, pos, ev, le, i, v, 9);
		//SendLyViz(fs, pos, ev, le, i, v, 10);
		//SendLyViz(fs, pos, ev, le, i, v, 11);
		//SendLyViz(fs, pos, ev, le, i, v, 12);
	}
}

void CP2Ly::CheckLyCP() {
	ly_flags = 0;
	for (int s = 0; s < c_len; ++s) {
		for (int v = 0; v < av_cnt; ++v) {
			ly_flags += flag[v][s].size();
		}
	}
}

