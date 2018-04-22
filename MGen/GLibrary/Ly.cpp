// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "../stdafx.h"
#include "Ly.h"

#ifdef _DEBUG
#define new DEBUG_NEW 
#endif

CLy::CLy() {
	//TestKeyMatrix();
}

CLy::~CLy() {
}

// One-time function
void CLy::TestKeyMatrix() {
	CString st, st2;
	vector<int> v;
	for (int k = 0; k < 12; ++k) {
		v.clear();
		v.resize(12, -1);
		int d0 = CC_C(key_base[k], 0, 0);
		st2.Empty();
		for (int n = 0; n < 7; ++n) {
			int d = (d0 + n) % 7;
			int c0 = (C_CC(d, 0, 0) + 12) % 12;
			int c = (C_CC(n, k, 0) + 12) % 12;
			v[c] = c0;
			st.Format("%d, ", d);
			st2 += st;
		}
		//AppendLineToFile("log\\key_build.log", "  { " + st2 + "}, \n");
		st2.Empty();
		for (int i = 0; i < 12; ++i) {
			st.Format("%d, ", v[i]);
			st2 += st;
		}
		//AppendLineToFile("log\\key_build.log", "  { " + st2 + "}, \n");
	}
	for (int k = 0; k < 12; ++k) {
		v.clear();
		v.resize(12, -1);
		int d0 = CC_C(key_base_m[k], 0, 0);
		st2.Empty();
		for (int n = 0; n < 7; ++n) {
			int d = (d0 + n) % 7;
			int c0 = (C_CC(d, 0, 0) + 12) % 12;
			int c = (C_CC(n, k, 1) + 12) % 12;
			v[c] = c0;
			st.Format("%d, ", d);
			st2 += st;
		}
		//AppendLineToFile("log\\key_build.log", "  { " + st2 + "}, \n");
		st2.Empty();
		for (int i = 0; i < 12; ++i) {
			st.Format("%d, ", v[i]);
			st2 += st;
		}
		AppendLineToFile("log\\key_build.log", "  { " + st2 + "}, \n");
	}
}

void CLy::LoadLyShapes(CString fname) {
	// Check file exists
	if (!fileExists(fname)) {
		CString est;
		est.Format("LoadLyShapes cannot find file: %s", fname);
		WriteLog(5, est);
		return;
	}
	ifstream fs;
	int i = 0;
	vector <CString> ast;
	CString st, est;
	fs.open(fname);
	char pch[2550];
	// Init
	shsc.resize(SHAPE_PHASE_CNT);
	for (int i = 0; i < shsc.size(); ++i) shsc[i].resize(2);
	//shsc[10][ssStart][vBracket] = "1";
	//shsc[10][ssStart][vOttava] = "2";
	//shsc[10][ssStart][vInterval] = "3";
	//shsc[10][ssStart][vOttava] = "4";
	// Load header
	fs.getline(pch, 2550);
	while (fs.good()) {
		i++;
		// Get line
		fs.getline(pch, 2550);
		st = pch;
		st.Trim();
		if (st.Find(";") != -1) {
			Tokenize(st, ast, ";");
			if (ast.size() != 4) {
				est.Format("Wrong column count at line %d in shapes file %s: '%s'", i, fname, st);
				WriteLog(5, est);
				error = 1;
				return;
			}
			ast[0].Trim();
			ast[1].Trim();
			ast[2].Trim();
			ast[3].Trim();
			int phase = atoi(ast[0]);
			if (phase >= SHAPE_PHASE_CNT) {
				est.Format("Wrong phase number at line %d in shapes file %s: '%s'", i, fname, st);
				WriteLog(5, est);
				error = 1;
				return;
			}
			int task = -1;
			if (ast[1] == "Start") task = ssStart;
			if (ast[1] == "Finish") task = ssFinish;
			if (task == -1) {
				est.Format("Wrong task at line %d in shapes file %s: '%s'", i, fname, st);
				WriteLog(5, est);
				error = 1;
				return;
			}
			int shape = -1;
			ast[2].MakeLower();
			if (ast[2] == "default") shape = vDefault;
			if (ast[2] == "harm") shape = vHarm;
			if (ast[2] == "interval") shape = vInterval;
			if (ast[2] == "vbracket") shape = vVBracket;
			if (ast[2] == "volta") shape = vVolta;
			if (ast[2] == "slur") shape = vSlur;
			if (ast[2] == "pslur") shape = vPSlur;
			if (ast[2] == "glis") shape = vGlis;
			if (ast[2] == "bracket") shape = vBracket;
			if (ast[2] == "trill") shape = vTrill;
			if (ast[2] == "ts") shape = vTS;
			if (ast[2] == "ottava") shape = vOttava;
			if (ast[2] == "pedal") shape = vPedal;
			if (ast[2] == "notename") shape = vNoteName;
			if (shape == -1) {
				est.Format("Wrong shape at line %d in shapes file %s: '%s'", i, fname, st);
				WriteLog(5, est);
				error = 1;
				return;
			}
			if (ast[3][0] == '"') ast[3].Delete(0);
			if (ast[3][ast[3].GetLength() - 1] == '"') ast[3].Delete(ast[3].GetLength() - 1);
			ast[3].Replace("\"\"", "\"");
			// Save
			if (!(shsc[phase][task][shape].IsEmpty())) {
				est.Format("Duplicate phase/task/shape at line %d in shapes file %s: '%s'", i, fname, st);
				WriteLog(5, est);
			}
			shsc[phase][task][shape] = ast[3];
		}
	}
	return;
}

void CLy::GetLyRange(int step1, int step2, vector<int> &vm_min, vector<int> &vm_max) {
	vm_min.clear();
	vm_max.clear();
	vm_min.resize(v_cnt, 128);
	vm_max.resize(v_cnt, 0);
	for (int s = step1; s < step2; ++s) {
		for (int v = v_cnt - 1; v >= 0; --v) {
			if (!pause[s][v]) {
				if (vm_min[v] > note[s][v]) vm_min[v] = note[s][v];
				if (vm_max[v] < note[s][v]) vm_max[v] = note[s][v];
			}
		}
	}
}

void CLy::GetLyVcnt(int step1, int step2, vector<int> &vm_max) {
	ly_vm_cnt = 0;
	for (int v = v_cnt - 1; v >= 0; --v) {
		if (vm_max[v]) ++ly_vm_cnt;
	}
}

CString CLy::GetLyAlter(int alter) {
	if (alter == -1) return "f";
	else if (alter == -2) return "ff";
	else if (alter == 1) return "s";
	else if (alter == 2) return "ss";
	return "";
}

CString CLy::GetLyAlterVisual(int alter, CString sz) {
	if (alter > 2) alter -= 12;
	if (alter < -2) alter += 12; 
	if (alter == -1) return sz + "\\flat ";
	else if (alter == -2) return sz + "\\doubleflat ";
	else if (alter == 1) return "\"#\"";
	else if (alter == 2) return "\\doublesharp ";
	return "";
}

CString CLy::GetLyNote(int i, int v) {
	int no2, oct, alter;
	GetRealNote(note[i][v], tonic[i][v], minor[i][v], no2, oct, alter);
	return LyNoteSharp[no2] + GetLyAlter(alter) + LyOctave[oct];
}

CString CLy::GetLyNoteVisual(int i, int v, CString sz) {
	int no2, oct, alter;
	GetRealNote(note[i][v], tonic[i][v], minor[i][v], no2, oct, alter);
	return NoteName[no2] + GetLyAlterVisual(alter, sz);
}

CString CLy::GetLyLen(int length) {
	if (length == 1) return "8";
	else if (length == 2) return "4";
	else if (length == 3) return "4.";
	else if (length == 4) return "2";
	else if (length == 5) return "2"; // This is wrong length
	else if (length == 6) return "2.";
	else if (length == 7) return "2..";
	else if (length == 8) return "1";
	//else if (length == 16) return "\\breve";
	//else if (length == 32) return "\\longa";
	else return "";
}

// Split note of length 5
void CLy::SplitLyNote5(int pos, vector<int> &la) {
	if (pos % 4 == 0) {
		la.push_back(4);
		la.push_back(1);
	}
	else if (pos % 4 == 1) {
		la.push_back(3);
		la.push_back(2);
	}
	else if (pos % 4 == 2) {
		la.push_back(2);
		la.push_back(3);
	}
	else if (pos % 4 == 3) {
		la.push_back(1);
		la.push_back(4);
	}
}

// Split note at first measure border
void CLy::SplitLyNoteMeasure(int pos, int le, vector<int> &la) {
	int left = 8 - (pos % 8);
	if (la[0] > left) {
		// Convert first part to second
		la[0] = la[0] - left;
		// Add first part
		vpush_front(la, left, 1);
	}
}

// Create la array of common lengths if note is too long for single note
void CLy::SplitLyNote(int pos, int le, vector<int> &la) {
	la.clear();
	if (le < 5 || le == 6 || le == 7 || le == 8) {
		la.push_back(le);
		return;
	}
	if (le == 5) {
		SplitLyNote5(pos, la);
	}
	// All other lengths starting from 9
	else {
		// Get first length
		if (pos % 8 == 3) {
			la.push_back(1);
			la.push_back(4);
			le -= 5;
			pos += 5;
		}
		else {
			la.push_back(8 - pos % 8);
			le -= 8 - pos % 8;
			pos += 8 - pos % 8;
		}
		// Create middle lengths
		while (le > 8) {
			la.push_back(8);
			le -= 8;
			pos += 8;
		}
		// Get last length
		if (le == 5) {
			la.push_back(4);
			la.push_back(1);
		}
		else {
			if (le) la.push_back(le);
		}
	}
}

void CLy::SendLyViz(ofstream &fs, int pos, CString &ev, int le, int i, int v, int phase) {
	int shape, sev;
	if (!lyi.size()) return;
	for (int task = ssFinish; task >= ssStart; --task) {
		for (auto it : shsc[phase][task]) {
			shape = it.first;
			if (task == ssFinish) {
				if (!lyi[ly_s2].shf[shape]) continue;
				sev = lyi[ly_s2 + lyi[ly_s2].shsl[shape]].shse[shape];
			}
			if (task == ssStart) {
				if (!lyi[ly_s2].shs[shape]) continue;
				sev = lyi[ly_s2].shse[shape];
			}
			CString script = it.second;
			CString text2;
			if (lyi[ly_s2].sht[shape].IsEmpty()) {
				text2 = "#f\n ";
			}
			else {
				text2 = "\\markup{ \\raise #0.6 \\teeny \"" + lyi[ly_s2].sht[shape] + "\" }\n ";
			}
			script.Replace("$n$", "\n");
			script.Replace("$COLOR$", GetLyColor(flag_color[sev]));
			script.Replace("$TEXT$", lyi[ly_s2].sht[shape]);
			script.Replace("$TEXT2$", text2);
			fs << script << "\n";
		}
	}
}

// Send note or pause
void CLy::SendLyEvent(ofstream &fs, int pos, CString ev, int le, int i, int v) {
	// Length array
	vector<int> la;
	SplitLyNote(pos, le, la);
	SplitLyNoteMeasure(pos, le, la);
	for (int lc = 0; lc < la.size(); ++lc) {
		ly_s = i;
		ly_s2 = ly_s - ly_step1;
		SaveLyComments(i, v, pos);
		SendLyViz(fs, pos, ev, le, i, v, 1);
		if (show_lining && ev != "r") {
			if (la[lc] == 8) {
				if (lining[i][v] == HatchStyleNarrowHorizontal) fs << " \\speakOff \\override NoteHead.style = #'xcircle ";
				else if (lining[i][v] == HatchStyleLargeConfetti) fs << " \\speakOff \\override NoteHead.style = #'petrucci ";
				else fs << " \\speakOff \\revert NoteHead.style ";
			}
			else {
				if (lining[i][v] == HatchStyleNarrowHorizontal) fs << " \\revert NoteHead.style \\speakOn ";
				else if (lining[i][v] == HatchStyleLargeConfetti) fs << " \\speakOff \\override NoteHead.style = #'petrucci ";
				else fs << " \\speakOff \\revert NoteHead.style ";
			}
			if (lining[i][v] == HatchStyleLightUpwardDiagonal) fs << " \\circle ";
		}
		fs << ev + GetLyLen(la[lc]);
		if (lc < la.size() - 1 && ev != "r") fs << "~";
		fs << "\n";
		if (midifile_export_marks && !mark[i][v].IsEmpty()) {
			CString st = mark[i][v];
			st.Replace("\n", "");
			if (st == "PD" || st == "CA" || st == "DN") {
				if (ly_msh) {
					if (GetGreen(mark_color[i][v]) == GetRed(mark_color[i][v])) {
						fs << " \\staccato ";
					}
					else {
						fs << "  \\staccatissimo ";
					}
				}
			}
		}
		if (i > -1) {
			i += la[lc] / midifile_out_mul[i];
			pos += la[lc];
		}
		SendLyViz(fs, pos, ev, le, i, v, 9);
		SendLyViz(fs, pos, ev, le, i, v, 10);
		SendLyViz(fs, pos, ev, le, i, v, 11);
		SendLyViz(fs, pos, ev, le, i, v, 12);
	}
}

CString CLy::GetLyColor(DWORD col) {
	float coef = 1.5;
	if (col == color_noflag) return "0 0 0";
	CString st;
	if (GetGreen(col) == GetRed(col) && GetRed(col) == GetBlue(col)) coef = 1;
	st.Format("%.3f %.3f %.3f",
		GetRed(col) / 255.0, GetGreen(col) / coef / 255.0, GetBlue(col) / 255.0);
	return st;
}

CString CLy::GetLyMarkColor(DWORD col) {
	if (col == color_noflag) return "1 1 1";
	CString st;
	if (GetGreen(col) == GetRed(col) && GetRed(col) == GetBlue(col)) return "1 1 1";
	st.Format("%.3f %.3f %.3f",
		Lighten(GetRed(col), 2) / 255.0,
		Lighten(GetGreen(col) * 1.5, 2) / 255.0,
		Lighten(GetBlue(col), 2) / 255.0);
	return st;
}

CString CLy::GetLyMarkColor2(DWORD col) {
	if (col == color_noflag) return "1 1 1";
	CString st;
	if (GetGreen(col) == GetRed(col) && GetRed(col) == GetBlue(col)) return "1 1 1";
	st.Format("%.3f %.3f %.3f",
		Lighten(GetRed(col), 2) / 255.0,
		Lighten(GetGreen(col), 2) / 255.0,
		Lighten(GetBlue(col), 2) / 255.0);
	return st;
}

void CLy::SendLyNoteColor(ofstream &fs, DWORD col) {
	fs << "\n    \\override NoteHead.color = #(rgb-color " << GetLyColor(col) << ") ";
	fs << "\n    \\override Stem.color = #(rgb-color " << GetLyColor(col) << ") ";
}

CString CLy::GetRealIntName(int s, int v1, int v2) {
	// Exact interval
	int in = abs(note[s][v2] - note[s][v1]);
	if (in > 14) {
		in = in % 12;
		if (in < 3) in += 12;
	}
	// Interval between base notes
	int no, oct, alter;
	int no2, oct2, alter2;
	GetRealNote(note[s][v1], tonic[s][v1], minor[s][v1], no, oct, alter);
	GetRealNote(note[s][v2], tonic[s][v2], minor[s][v2], no2, oct2, alter2);
	int fno = no + oct * 12;
	int fno2 = no2 + oct2 * 12;
	int bin = abs(fno - fno2);
	if (bin > 14) {
		bin = bin % 12;
		if (bin < 3) bin += 12;
	}
	// Diatonic interval
	int din = CC_C(abs(note[s][v1] - note[s][v2]), 0, 0) - 7;
	// Base diatonic interval
	int bdin = CC_C(abs(fno - fno2), 0, 0) - 7;
	int bdin2 = bdin;
	if (bdin2 > 8) {
		bdin2 = bdin2 % 7;
		if (bdin2 < 3) bdin2 += 7;
	}
	// Build string
	// Diatonic did not change or triton / triton base
	if (din == bdin || in == 6 || bin == 6) {
		if (in == 0) return "1";
		else if (in == 1) return "m2";
		else if (in == 2) return "M2";
		else if (in == 3) return "m3";
		else if (in == 4) return "M3";
		else if (in == 5) return "4";
		else if (in == 6) return "tri";
		else if (in == 7) return "5";
		else if (in == 8) return "m6";
		else if (in == 9) return "M6";
		else if (in == 10) return "m7";
		else if (in == 11) return "M7";
		else if (in == 12) return "8";
		else if (in == 13) return "m9";
		else if (in == 14) return "M9";
	}
	// Diatonic changed
	CString st;
	st.Format("%d", bdin2 + 1);
	if (din < bdin) st = "dim" + st;
	else st = "aug" + st;
	return st;
}

CString CLy::GetIntName(int iv) {
	if (iv == 0) return "1";
	else if (iv == 1) return "m2";
	else if (iv == 2) return "M2";
	else if (iv == 3) return "m3";
	else if (iv == 4) return "M3";
	else if (iv == 5) return "4";
	else if (iv == 6) return "tri";
	else if (iv == 7) return "5";
	else if (iv == 8) return "m6";
	else if (iv == 9) return "M6";
	else if (iv == 10) return "m7";
	else if (iv == 11) return "M7";
	else return "8";
}

void CLy::AddNLink(int i, int i2, int v, CString st, int fl, int ln, int foreign) {
	lyi[i2 - ly_step1].nflags.push_back(fl / 10);
	lyi[i2 - ly_step1].fsev.push_back(fsev[i][v][fl]);
	cspecies = fl % 10;
	if (foreign) {
		lyi[i2 - ly_step1].nfl.push_back(i + ln - coff[i + ln][ly_v] - i2);
	}
	else {
		lyi[i2 - ly_step1].nfl.push_back(ln);
	}
	lyi[i2 - ly_step1].nfn.push_back(ly_flags + 1);
	lyi[i2 - ly_step1].nff.push_back(foreign);
	lyi[i2 - ly_step1].nfs.push_back(0);
	//lyi[i2 - ly_step1].nfc.push_back(st);
	lyi[i2 - ly_step1].nfc.push_back("");
	if (!foreign) ++ly_flags;
}

void CLy::ParseNLinks(int i, int i2, int v, int foreign) {
	CString com;
	int x = 0;
	for (auto const& it : nlink[i][v]) {
		if (foreign && !rule_viz_v2[it.first / 10]) continue;
		AddNLink(i, i2, v, comment[i][v][x], it.first, it.second, foreign);
		++x;
	}
}

void CLy::SaveLyComments(int i, int v, int pos) {
	CString st, com, note_st;
	int pos1, pos2, found;
	if (!lyi.size()) return;
	if (lyi[ly_s2].nflags.size()) {
		note_st = "\\markup \\wordwrap \\bold {\n  ";
		// Show voice number if more than 1 voice
		if (ly_vm_cnt > 1) {
			st.Format("PART %d, ", ly_vcnt - v);
			note_st += st;
		}
		st.Format("NOTE %d at %d:%d - %s",
			ly_nnum, pos / 8 + 1, pos % 8 + 1, GetLyNoteVisual(i, v, "\\raise #0.3 \\magnify #0.7 "));
		if (coff[i][v])
			st += " (slur)";
		note_st += st + "\n}\n";
		found = 0;
		for (int c = 0; c < lyi[ly_s2].nflags.size(); ++c) {
			// Do not process foreign flags
			if (lyi[ly_s2].nff[c]) break;
			int fl = lyi[ly_s2].nflags[c];
			int sev = lyi[ly_s2].fsev[c];
			if (!accept[fl]) st = "- ";
			else if (accept[fl] == -1) st = "$ ";
			else st = "+ ";
			CString rule_name = RuleName[cspecies][fl];
			if (ly_debugexpect) {
				rule_name.Format("[%d/%d] ", fl, sstep[ly_s] + 1);
				rule_name += RuleName[cspecies][fl];
			}
			else {
				if (!ly_rule_colon) {
					if (rule_name.Find(":") != -1) {
						rule_name = rule_name.Left(rule_name.Find(":"));
					}
				}
			}
			com = st + rule_name;
			if (ly_subrules) com += " (" + SubRuleName[cspecies][fl] + ")";
			if (ly_comments && !RuleComment[cspecies][fl].IsEmpty()) com += ". " + RuleComment[cspecies][fl];
			if (ly_comments && !SubRuleComment[cspecies][fl].IsEmpty()) com += " (" + SubRuleComment[cspecies][fl] + ")";
			com += " " + lyi[ly_s2].nfc[c];
			// Send note number with first comment
			if (!found) {
				found = 1;
				ly_com_st += note_st;
			}
			ly_com_st += "\\markup \\wordwrap \\with-color #(rgb-color " +
				GetLyColor(flag_color[sev]) + ") {\n  ";
			com.Replace("\"", "\\\"");
			com.Replace(" ", "\" \"");
			st.Format("\\teeny \\raise #0.2 \\circle %d \"", lyi[ly_s2].nfn[c]);
			ly_com_st += st;
			ly_com_st += com + "\"\n";
			ly_com_st += "\n}\n";
		}
	}
}

CString CLy::DetectLyClef(int vmin, int vmax) {
	vector<int> clef_penalty;
	int min_penalty = INT_MAX;
	int best_clef = 4;
	clef_penalty.resize(MAX_CLEF);
	// Calculate penalty
	for (int c = 0; c < MAX_CLEF; ++c) {
		clef_penalty[c] = max(abs(vmax - LyClefCenter[c]), abs(vmin - LyClefCenter[c]));
	}
	// Get best clef
	for (int c = 0; c < MAX_CLEF; ++c) {
		if (clef_penalty[c] < min_penalty) {
			min_penalty = clef_penalty[c];
			best_clef = c;
		}
	}
	return LyClef[best_clef];
}

void CLy::SetLyShape(int s1, int s2, int f, int fl, int sev, int vtype) {
	if (lyi[s1].shse[vtype] <= sev) {
		// Start
		lyi[s1].shs[vtype] = 1;
		// Finish
		lyi[s2].shf[vtype] = 1;
		// Link to start
		lyi[s2].shsl[vtype] = s1 - s2;
		lyi[s1].shse[vtype] = sev;
		if (vtype == vInterval || vtype == vNoteName || vtype == vHarm) {
			if (lyi[s2].shse[vtype] <= sev) {
				lyi[s2].shse[vtype] = sev;
			}
			if (vtype == vNoteName) ++ly_notenames;
		}
		lyi[s1].sht[vtype] = rule_viz_t[fl];
		// Save flag shape (step depends if link is forward or backward)
		lyi[ly_s2].nfs[f] = vtype;
		lyi[s1].shflag[vtype] = f;
		lyi[s1].shfp[vtype] = ly_s2;
	}
}

void CLy::ClearLyShape(int s1, int s2, int vtype) {
	lyi[s1].shs[vtype] = 0;
	lyi[s2].shf[vtype] = 0;
	// Calculate maximum severity
	lyi[s1].shse[vtype] = -1;
	// Remove link
	lyi[lyi[s1].shfp[vtype]].nfs[lyi[s1].shflag[vtype]] = 0;
	lyi[s1].shflag[vtype] = -1;
	lyi[s1].shfp[vtype] = -1;
}

void CLy::ExportLyI() {
	ofstream fs;
	fs.open(as_dir + "\\lyi-" + as_fname + ".csv", ios_base::app);
	fs << "Step[" << ly_mel << "];";
	for (int x = 0; x < MAX_VIZ; ++x) {
		fs << "shs[" << x << "];";
		fs << "shf[" << x << "];";
		fs << "shsl[" << x << "];";
		fs << "shse[" << x << "];";
		fs << "sht[" << x << "];";
	}
	fs << "\n";
	for (ly_s = ly_step1; ly_s < ly_step2; ++ly_s) {
		ly_s2 = ly_s - ly_step1;
		fs << ly_s2 << ";";
		for (int x = 0; x < MAX_VIZ; ++x) {
			fs << lyi[ly_s2].shs[x] << ";";
			fs << lyi[ly_s2].shf[x] << ";";
			fs << lyi[ly_s2].shsl[x] << ";";
			fs << lyi[ly_s2].shse[x] << ";";
			fs << lyi[ly_s2].sht[x] << ";";
		}
		fs << "\n";
	}
	fs << "\n";
	fs.close();
}

void CLy::AddLyITest(int step1, int step2, int fl, int shape) {
	ly_s2 = step1;
	lyi[step1].nflags.push_back(fl);
	lyi[step1].nfl.push_back(step2 - step1);
	lyi[step1].nfn.push_back(ly_flags + 1);
	lyi[step1].nff.push_back(0);
	lyi[step1].nfs.push_back(0);
	lyi[step1].nfc.push_back("");
	lyi[step1].nfc[lyi[step1].nfc.size() - 1].Format("Type %d", shape);
	SetLyShape(step1, step2, lyi[step1].nfs.size() - 1, fl, randbw(0, 100), shape);
	++ly_flags;
}

void CLy::InitLyITest() {
	if (m_config != "test-ly-overlap") return;
	if (ly_v != ly_vhigh) return;
	int step0, step1, step2, step3, step4, step5, step6;
	int fl = 31;
	step0 = 4;
	ly_flags = 0;
	for (int shape = 0; shape < MAX_VIZ; ++shape) {
		if (viz_type[shape] == vtPoint) {
			step1 = step0;
			step2 = step0;
			step3 = step0;
			step4 = step0;
		}
		else if (viz_type[shape] == vtVBracket) {
			step1 = step0 - 1;
			step2 = step0;
			step3 = step0 + 2;
			step4 = step0 + 3;
		}
		else if (viz_type[shape] == vtGroup || viz_type[shape] == vtVolta) {
			step1 = step0 - 1;
			step2 = step0;
			step3 = step0 + 1;
			step4 = step0 + 2;
		}
		else {
			step1 = step0 - 1;
			step2 = step0;
			step3 = step0;
			step4 = step0 + 1;
		}
		step5 = lyi.size() - 2;
		step6 = lyi.size() - 1;
		AddLyITest(step1, step2, fl, shape);
		AddLyITest(step3, step4, fl, shape);
		AddLyITest(step5, step6, fl, shape);
	}
}

void CLy::InitLyI() {
	if (ly_mel == -1) return;
	ly_v2 = ly_v;
	if (ly_vm_cnt > 1) ly_v2 = (ly_v / 2) * 2 + !(ly_v % 2);
	ly_flags = 0;
	ly_notenames = 0;
	if (m_algo_id == 111) {
		ly_vhigh = ly_v;
		ly_vlow = ly_v;
	}
	else {
		ly_vhigh = max(ly_v, ly_v2);
		ly_vlow = min(ly_v, ly_v2);
	}
	lyi.clear();
	lyi.resize(ly_step2 - ly_step1 + 1);
	for (int i = 0; i < lyi.size(); ++i) {
		// Init vectors
		lyi[i].shs.resize(MAX_VIZ);
		lyi[i].shsl.resize(MAX_VIZ);
		lyi[i].shf.resize(MAX_VIZ);
		lyi[i].shse.resize(MAX_VIZ, -1);
		lyi[i].shflag.resize(MAX_VIZ, -1);
		lyi[i].shfp.resize(MAX_VIZ, -1);
		lyi[i].sht.resize(MAX_VIZ);
	}
	if (m_config == "test-ly-overlap") return;
	for (ly_s = ly_step1; ly_s < ly_step2; ++ly_s) {
		ly_s2 = ly_s - ly_step1;
		// Find current note position
		int cur_note_step = ly_s;
		if (ly_s2 > 0) {
			for (int x = ly_s; x >= ly_step1; --x) {
				if (note[x][ly_v] != note[ly_s][ly_v]) {
					cur_note_step = x + 1;
					break;
				}
				if (x == ly_step1) cur_note_step = x;
			}
		}
		// Parse flags
		ParseNLinks(ly_s, ly_s, ly_v, 0);
		if (!lyi[ly_s2].nflags.size() && v_cnt > 1) {
			if (ly_v2 < v_cnt) {
				ParseNLinks(ly_s, cur_note_step, ly_v2, 1);
			}
		}
		else {
			// Detect flags that are not at note start and not at cantus note start
			for (int f = 0; f < lyi[ly_s2].nflags.size(); ++f) {
				int link = lyi[ly_s2].nfl[f];
				if ((!coff[ly_s][ly_v] || !coff[ly_s][ly_v2]) &&
					(!coff[ly_s + link][ly_v] || !coff[ly_s + link][ly_v2])) continue;
				int fl = lyi[ly_s2].nflags[f];
				CString est;
				est.Format("Detected flag at hidden position %d/%d: [%d] %s %s (%s)",
					ly_s, ly_s + link, fl, accept[fl] ? "+" : "-",
					RuleName[cspecies][fl], SubRuleName[cspecies][fl]);
				WriteLog(5, est);
			}
		}
		SelectSpeciesRules();
	}
	for (ly_s = ly_step1; ly_s < ly_step2; ++ly_s) {
		ly_s2 = ly_s - ly_step1;
		// Find next note position
		int next_note_step = ly_s + noff[ly_s][ly_v];
		// Find previous note position
		int prev_note_step = max(ly_step1, ly_s - poff[ly_s][ly_v]);
		// Parse flags
		for (int f = 0; f < lyi[ly_s2].nflags.size(); ++f) {
			int fl = lyi[ly_s2].nflags[f];
			int link = lyi[ly_s2].nfl[f];
			int vtype = rule_viz[fl];
			int sev = lyi[ly_s2].fsev[f];
			int skip_shape = 0;
			int prev_link_note = max(ly_step1, ly_s + link - poff[ly_s + link][ly_v]);
			// Find link note position
			int link_note_step = ly_s + link;
			if (ly_s2 > 0) {
				for (int x = ly_s + link; x >= ly_step1; --x) {
					if (note[x][ly_v] != note[ly_s + link][ly_v]) {
						link_note_step = x + 1;
						break;
					}
					if (x == ly_step1) link_note_step = x;
				}
			}
			// Get flag start/stop
			int s1 = min(ly_s2, ly_s2 + link);
			int s2 = max(ly_s2, ly_s2 + link);
			if (lyi[ly_s2].nff[f]) {
				s1 = min(ly_s2, link_note_step - ly_step1);
				s2 = max(ly_s2, link_note_step - ly_step1);
			}
			// If shape cannot highlight single note, but flag does not contain link, then link to next note
			if (!viz_singlenote[vtype] && s1 == s2) s2 = next_note_step - ly_step1;
			// Set interval
			if (rule_viz_int[fl] == 1) {
				SetLyShape(s1, s2, f, fl, sev, vInterval);
			}
			if (rule_viz_int[fl] == 2) {
				SetLyShape(s1, s1, f, fl, sev, vInterval);
			}
			if (rule_viz_int[fl] == 3) {
				SetLyShape(s2, s2, f, fl, sev, vInterval);
			}
			if (!viz_can_overlap[vtype]) {
				// Check that flag overlaps
				int overlap1 = -1;
				int overlap2 = -1;
				int overlap_limit = s1;
				// For groups check for collision between borders
				if (viz_type[vtype] == vtGroup || viz_type[vtype] == vtVolta)
					overlap_limit = s1 - 1;
				// For vbrackets check for collision between notes
				if (viz_type[vtype] == vtVBracket)
					overlap_limit = min(prev_note_step, prev_link_note) - ly_step1 - 1;
				for (int x = ly_step2 - ly_step1 - 1; x > overlap_limit; --x) {
					if (lyi[x].shf[vtype]) {
						overlap2 = x;
						overlap1 = x + lyi[x].shsl[vtype];
						// Choose highest severity
						if (sev > lyi[overlap1].shse[vtype]) {
							ClearLyShape(overlap1, overlap2, vtype);
						}
						else {
							// Skip shape
							skip_shape = 1;
							break;
						}
					}
				}
				if (skip_shape) continue;
			}
			SetLyShape(s1, s2, f, fl, sev, vtype);
		}
	}
#if defined(_DEBUG)
	ExportLyI();
#endif
}

void CLy::SaveLySegment(ofstream &fs, int mel, int step1, int step2) {
	vector<CString> sv;
	CString clef, key, key_visual;
	int pos, pos2, le, le2, pause_accum, pause_pos, pause_i;
	ly_com_st.Empty();
	ly_ly_st.Empty();
	// Voice melody min pitch
	vector<int> vm_min;
	// Voice melody max pitch
	vector<int> vm_max;
	// Calculate stats
	ly_step1 = step1;
	ly_step2 = step2;
	GetLyRange(step1, step2, vm_min, vm_max);
	GetLyVcnt(step1, step2, vm_max);
	ly_mul = midifile_out_mul[step1];
	//if (ly_vm_cnt == 1 && (m_algo_id == 121 || m_algo_id == 112)) mul = 8;
	// Key
	if (minor[step1][0]) {
		key = LyMinorKey[tonic[step1][0]];
	}
	else {
		key = LyMajorKey[tonic[step1][0]];
	}
	key_visual = key[0];
	key_visual.MakeUpper();
	if (key[1] == 'f') key_visual += "\\flat ";
	if (key[1] == 's') key_visual = "\"" + key_visual + "#\"";
	// First info
	CString st, st3;
	if (mel == -1) st = "Whole piece";
	else {
		st = mel_info[mel];
		st3 = mel_info3[mel];
	}
	st.Replace("\n", ", ");
	st.Replace("#", "\"#\"");
	st.Replace("\\", "/");
	st.Replace("=>", " \\char ##x27F9 ");
	st.Replace("->", " \\char ##x27F6 ");
	fs << "\\markup \\wordwrap \\bold {\n  ";
	fs << "    \\vspace #3\n";
	fs << st << ", Key: " << key_visual << (minor[step1][0] ? " minor" : " major") << "\n}\n";
	// Save notes
	fs << "<<\n";
	ly_vcnt = 0;
	for (int v = v_cnt - 1; v >= 0; --v) {
		// Do not show voice if no notes inside
		if (!vm_max[v]) continue;
		if (!ly_vcnt) ly_vcnt = v + 1;
		ly_v = v;
		InitLyI();
		InitLyITest();
		// Select best clef
		clef = DetectLyClef(vm_min[v], vm_max[v]);
		st.Format("\\new Staff = \"staff%d\" {\n", ly_v);
		fs << st;
		st.Format("  \\set Staff.instrumentName = \"Part %d\"\n", ly_vcnt - v);
		fs << st;
		fs << "  \\clef \"" << clef << "\" \\key " << key;
		fs << " \\" << (minor[step1][0] ? "minor" : "major") << "\n";
		read_file_sv("configs\\ly\\staff.ly", sv);
		write_file_sv(fs, sv);
		fs << "  { ";
		ly_nnum = 0;
		pause_accum = 0;
		pause_pos = -1;
		for (int i = step1; i < step2; i++) {
			pos = ly_mul * (i - step1);
			le = ly_mul * len[i][v];
			if (pause[i][v]) {
				pause_accum += le;
				if (pause_pos == -1) {
					pause_pos = pos;
					pause_i = i;
				}
			}
			else {
				++ly_nnum;
				SendLyEvent(fs, pos, GetLyNote(i, v), le, i, v);
			}
			if (pause_accum && (i == step2 - 1 || !pause[i + 1][v])) {
				SendLyEvent(fs, pause_pos, "r", pause_accum, pause_i, v);
				pause_accum = 0;
				pause_pos = -1;
			}
			if (noff[i][v] == 0) break;
			i += noff[i][v] - 1;
		}
		// Finish with pause
		if ((pos + le) % 8) {
			SendLyEvent(fs, pos, "r", 8 - (pos + le) % 8, 0, 0);
		}
		fs << "\n  }\n";
		fs << "}\n";
		SendLyMistakes();
		SendLyNoteNames();
		SendLyHarm();
		SendLyIntervals();
	}
	fs << ly_ly_st;
	fs << ">>\n";
	if (st3 != "") fs << "\\markup { " << st3 << " }\n";
	fs << ly_com_st;
	if (ly_com_st == "") {
		if (!ly_debugexpect) {
			fs << "\\markup \\bold \\with-color #(rgb-color 0 0.8 0) { \\char ##x2705 Excellent }\n ";
		}
	}
	if (ly_pagebreak) fs << "\\pageBreak\n";
	// Second info
	//st2.Replace("\n", "\n}\n\\markup \\wordwrap \\italic {\n  ");
	//st2.Replace("#", "\"#\"");
	//fs << "\\markup \\wordwrap \\italic {\n  \\vspace #2\n  " << st2 << "\n}\n";
}

CString CLy::SendLySkips(int count) {
	CString lst;
	for (int x = 0; x < count; ++x) {
		lst += " \\skip 8 ";
	}
	return lst;
}

void CLy::SendLyMistakes() {
	CString st;
	if (!ly_flags) return;
	st.Format("  \\new Lyrics \\with { alignAboveContext = \"staff%d\" } {\n", ly_vhigh);
	ly_ly_st += st;
	ly_ly_st += "    \\lyricmode {\n";
	ly_ly_st += "      \\set stanza = #\" Flags:\"\n";
	for (ly_s = ly_step1; ly_s < ly_step2; ++ly_s) {
		ly_s2 = ly_s - ly_step1;
		if (!lyi[ly_s2].nflags.size()) {
			ly_ly_st += SendLySkips(ly_mul);
			continue;
		}
		ly_ly_st += "      \\markup{ \\teeny \\override #`(direction . ,UP) { \\dir-column {\n";
		int max_mist = lyi[ly_s2].nflags.size() - 1;
		// Do not show too many mistakes
		if (max_mist > 5) {
			max_mist = 4;
			ly_ly_st += "...\n";
		}
		for (int f = max_mist; f >= 0; --f) {
			int fl = lyi[ly_s2].nflags[f];
			int sev = lyi[ly_s2].fsev[f];
			st.Format("        \\with-color #(rgb-color " +
				GetLyColor(flag_color[sev]) + ") %s \\circle %d\n",
				lyi[ly_s2].nfs[f] ? "\\underline" : "", lyi[ly_s2].nfn[f]);
			// \override #'(offset . 5) \override #'(thickness . 2) 
			ly_ly_st += st;
		}
		ly_ly_st += "      } } }8\n";
		ly_ly_st += SendLySkips(ly_mul - 1);
	}
	ly_ly_st += "    }\n";
	ly_ly_st += "  }\n";
}

void CLy::SendLyHarm() {
	CString st, lst;
	int hcount = 0;
	//if (!ly_flags) return;
	st.Format("  \\new Lyrics \\with { alignBelowContext = \"staff%d\" } {\n", ly_vlow);
	lst += st;
	lst += "    \\lyricmode {\n";
	lst += "      \\set stanza = #\" Harmony:\"\n";
	for (ly_s = ly_step1; ly_s < ly_step2; ++ly_s) {
		ly_s2 = ly_s - ly_step1;
		CString st = mark[ly_s][ly_v2];
		st.Replace("\n", "");
		if (!st.IsEmpty() && st != "PD" && st != "CA" && st != "DN") {
			++hcount;
			lst += "  \\markup{ ";
			int found = 0;
			// Replace dominant symbol
			st.Replace("#", " \"#\" ");
			st.Replace("b", " \\raise #0.3 \\magnify #0.5 \\flat ");
			if (ly_dominant_letter) {
				if (st[0] == 'D') {
					st = "\\concat { \\char ##x00D0 " + st.Right(st.GetLength() - 1) + " } ";
				}
				else if (st[0] == 'd') {
					st = "\\concat { \\char ##x0111 " + st.Right(st.GetLength() - 1) + " } ";
				}
				else st = "\\concat { " + st + " } ";
			}
			else st = "\\concat { " + st + " } ";
			st.Replace("6", " \\raise #0.7 6");
			//if (found) st = ", " + st;
			found = 1;
			lst += "\\teeny ";
			if (lyi[ly_s2].shs[vHarm] || lyi[ly_s2].shf[vHarm]) {
				DWORD col = flag_color[lyi[ly_s2].shse[vHarm]];
				if (col && col != color_noflag) {
					lst += " \\on-color #(rgb-color " + GetLyMarkColor2(col) + ") ";
				}
			}
			lst += "\\pad-markup #0.4 " + st + " ";
			lst += "}8\n";
			lst += SendLySkips(ly_mul - 1);
		}
		else {
			lst += SendLySkips(ly_mul);
		}
	}
	lst += "    }\n";
	lst += "  }\n";
	if (hcount) ly_ly_st += lst;
}

void CLy::SendLyIntervals() {
	CString st;
	if (!ly_flags) return;
	if (ly_vlow == ly_vhigh) return;
	st.Format("  \\new Lyrics \\with { alignBelowContext = \"staff%d\" } {\n", ly_vlow);
	ly_ly_st += st;
	ly_ly_st += "    \\lyricmode {\n";
	ly_ly_st += "      \\set stanza = #\" Interval:\"\n";
	for (ly_s = ly_step1; ly_s < ly_step2; ++ly_s) {
		ly_s2 = ly_s - ly_step1;
		if (!lyi[ly_s2].shs[vInterval] && !lyi[ly_s2].shf[vInterval]) {
			ly_ly_st += SendLySkips(ly_mul);
			continue;
		}
		//int in = note[ly_s][ly_vhigh] - note[ly_s][ly_vlow];
		//int in2 = in % 12;
		//in = in ? (in2 ? in2 : 12) : 0;
		//CString st = GetIntName(in);
		CString st = GetRealIntName(ly_s, ly_vhigh, ly_vlow);
		ly_ly_st += "\\markup{ ";
		ly_ly_st += "\\teeny ";
		if (lyi[ly_s2].shse[vInterval] > -1) {
			DWORD col = flag_color[lyi[ly_s2].shse[vInterval]];
			if (col && col != color_noflag)
				ly_ly_st += " \\on-color #(rgb-color " + GetLyMarkColor2(col) + ") ";
		}
		ly_ly_st += " \\pad-markup #0.4 \\concat { " + st + " ";
		ly_ly_st += "} }\n";
		ly_ly_st += SendLySkips(ly_mul - 1);
	}
	ly_ly_st += "    }\n";
	ly_ly_st += "  }\n";
}

void CLy::SendLyNoteNames() {
	CString st;
	if (!ly_notenames) return;
	st.Format("  \\new Lyrics \\with { alignBelowContext = \"staff%d\" } {\n", ly_v);
	ly_ly_st += st;
	ly_ly_st += "    \\lyricmode {\n";
	ly_ly_st += "      \\set stanza = #\" Note:\"\n";
	for (ly_s = ly_step1; ly_s < ly_step2; ++ly_s) {
		ly_s2 = ly_s - ly_step1;
		if (!lyi[ly_s2].shs[vNoteName] && !lyi[ly_s2].shf[vNoteName]) {
			ly_ly_st += SendLySkips(ly_mul);
			continue;
		}
		CString st = GetLyNoteVisual(ly_s, ly_v, "\\raise #0.3 \\magnify #0.5 ");
		ly_ly_st += "\\markup{ ";
		ly_ly_st += "\\teeny ";
		if (lyi[ly_s2].shse[vNoteName] > -1) {
			DWORD col = flag_color[lyi[ly_s2].shse[vNoteName]];
			if (col && col != color_noflag)
				ly_ly_st += " \\on-color #(rgb-color " + GetLyMarkColor2(col) + ") ";
		}
		ly_ly_st += " \\pad-markup #0.4 \\concat { " + st + " } ";
		ly_ly_st += "}\n";
		ly_ly_st += SendLySkips(ly_mul - 1);
	}
	ly_ly_st += "    }\n";
	ly_ly_st += "  }\n";
}

void CLy::SaveLy(CString dir, CString fname) {
	LoadLyShapes("configs\\ly\\shapes.csv");
	vector<CString> sv;
	CString title;
	// Remove server config prefix
	CString my_config;
	my_config = m_config;
	if (my_config.Left(3) == "sv_") {
		my_config = my_config.Mid(3);
	}
	DeleteFile(dir + "\\lyi-" + fname + ".csv");
	title = m_algo_name + ": " + my_config + " (" +
		CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M") + ")";
	ly_fs.open(dir + "\\" + fname + ".ly");
	read_file_sv("configs\\ly\\header.ly", sv);
	for (int i = 0; i < sv.size(); ++i) {
		sv[i].Replace("$SUBTITLE$", title);
		if (ly_debugexpect) {
			sv[i].Replace("$TITLE$", "Debug expected confirmations");
		}
		else sv[i].Replace("$TITLE$", "");
		sv[i].Replace("$DEDICATION$", "");
		ly_fs << sv[i] << "\n";
	}

	if (!mel_info.size()) {
		CString st;
		st = "Whole piece";
		ly_mel = -1;
		SaveLySegment(ly_fs, -1, 0, t_generated);
	}
	else {
		int first_step = 0;
		int last_step = 0;
		int found, s;
		for (int m = 0; m < mel_info.size(); ++m) {
			found = 0;
			for (s = last_step; s < t_generated; ++s) {
				if (!found && mel_id[s][0] == m && mel_id[s][0] > -1) {
					first_step = s;
					last_step = s;
					found = 1;
				}
				if (found && mel_id[s][0] != m) {
					last_step = s;
					break;
				}
			}
			if (s >= t_generated - 1 && mel_id[t_generated - 1][0] > -1 &&
				found && first_step == last_step)	last_step = t_generated - 1;
			ly_mel = m;
			if (found) SaveLySegment(ly_fs, m, first_step, last_step);
			//if (m < mel_info.size() - 1) fs << "\\pageBreak\n";
		}
	}
	ly_fs << "\\header {tagline = \"This file was created by MGen ";
	ly_fs << APP_VERSION << " at " << CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S") << "\"}\n";
	read_file_sv("configs\\ly\\footer.ly", sv);
	write_file_sv(ly_fs, sv);
	ly_fs.close();
	ly_saved = 1;
}

// Select rules
void CLy::SelectSpeciesRules() {
	if (cspecies == cspecies0) return;
	cspecies0 = cspecies;
	// Load rules
	for (int i = 0; i < max_flags; ++i) {
		accept[i] = accepts[cspecies][i];
		severity[i] = severities[cspecies][i];
	}
	// Check that at least one rule is accepted
	for (int i = 0; i < max_flags; ++i) {
		if (accept[i]) break;
		if (i == max_flags - 1) {
			WriteLog(5, "Warning: all rules are rejected (0) in configuration file");
			error = 1;
		}
	}
	// Calculate second level flags count
	flags_need2 = 0;
	for (int i = 0; i < max_flags; ++i) {
		if (accept[i] == 2) ++flags_need2;
	}
}

