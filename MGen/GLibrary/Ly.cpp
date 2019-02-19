// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "../stdafx.h"
#include "Ly.h"
#include "CsvDb.h"

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
	CCsvDb cdb;
	CString est = cdb.Open(fname);
	if (est != "") WriteLog(5, fname + ": " + est);
	est = cdb.Select();
	if (est != "") WriteLog(5, fname + ": " + est);
	// Init shapeinfo
	shinfo.resize(MAX_VIZ);
	for (int i = 0; i < cdb.result.size(); ++i) {
		int sh = atoi(cdb.result[i]["Shape id"]);
		shinfo[sh].name = cdb.result[i]["Name"];
		shinfo[sh].comment = cdb.result[i]["Comment"];
		shinfo[sh].type = atoi(cdb.result[i]["Type"]);
		shinfo[sh].can_overlap = atoi(cdb.result[i]["Can overlap"]);
		shinfo[sh].can_text = atoi(cdb.result[i]["Can show text"]);
		shinfo[sh].can_singlenote = atoi(cdb.result[i]["Can single note"]);
		shinfo[sh].can_anyposition = atoi(cdb.result[i]["Can any position"]);
		shinfo[sh].can_separate = atoi(cdb.result[i]["Can separate"]);
		shinfo[sh].empty_space = atoi(cdb.result[i]["Empty space"]);
	}
}

void CLy::LoadLyShapeScripts(CString fname) {
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
	for (int sh = 0; sh < MAX_VIZ; ++sh) {
		shinfo[sh].script.resize(2);
		shinfo[sh].script[0].resize(SHAPE_PHASE_CNT);
		shinfo[sh].script[1].resize(SHAPE_PHASE_CNT);
	}
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
				est.Format("Wrong column count at line %d in shape_scripts file %s: '%s'", i, fname, st);
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
				est.Format("Wrong phase number at line %d in shape_scripts file %s: '%s'", i, fname, st);
				WriteLog(5, est);
				error = 1;
				return;
			}
			int task = -1;
			if (ast[1] == "Start") task = ssStart;
			if (ast[1] == "Finish") task = ssFinish;
			if (task == -1) {
				est.Format("Wrong task at line %d in shape_scripts file %s: '%s'", i, fname, st);
				WriteLog(5, est);
				error = 1;
				return;
			}
			int shape = -1;
			for (int sh = 0; sh < MAX_VIZ; ++sh) {
				if (shinfo[sh].name == ast[2]) shape = sh;
			}
			if (shape == -1) {
				est.Format("Wrong shape at line %d in shape_scripts file %s: '%s'", i, fname, st);
				WriteLog(5, est);
				error = 1;
				return;
			}
			if (ast[3][0] == '"') ast[3].Delete(0);
			if (ast[3][ast[3].GetLength() - 1] == '"') ast[3].Delete(ast[3].GetLength() - 1);
			ast[3].Replace("\"\"", "\"");
			// Save
			if (!shinfo[shape].script[task][phase].IsEmpty()) {
				est.Format("Duplicate phase/task/shape at line %d in shape_scripts file %s: '%s'", i, fname, st);
				WriteLog(5, est);
			}
			shinfo[shape].script[task][phase] = ast[3];
			// Detect TEXT macro
			if (ast[3].Find("$TEXT") != -1) {
				shinfo[shape].has_text = 1;
			}
		}
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
	else if (alter == 2) return "\\raise #0.5 \\doublesharp ";
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
	else if (length == 9) return "1"; // This is wrong length
	else if (length == 10) return "1"; // This is wrong length
	else if (length == 11) return "1"; // This is wrong length
	else if (length == 12) return "1.";
	//else if (length == 16) return "\\breve";
	//else if (length == 32) return "\\longa";
	else return "";
}

CString CLy::GetLyColor(int sev) {
	CString st;
	DWORD green = MakeColor(0, 0, 180, 0);
	DWORD yellow = MakeColor(0, 197, 176, 0);
	DWORD red = MakeColor(0, 255, 0, 0);
	if (sev <= 50) {
		st.Format("%.3f %.3f %.3f",
			(GetRed(yellow) * sev + GetRed(green) * (50 - sev)) / 50.0 / 255.0, 
			(GetGreen(yellow) * sev + GetGreen(green) * (50 - sev)) / 50.0 / 255.0,
			(GetBlue(yellow) * sev + GetBlue(green) * (50 - sev)) / 50.0 / 255.0
		);
	}
	else {
		st.Format("%.3f %.3f %.3f",
			(GetRed(red) * (sev - 50) + GetRed(yellow) * (100 - sev)) / 50.0 / 255.0,
			(GetGreen(red) * (sev - 50) + GetGreen(yellow) * (100 - sev)) / 50.0 / 255.0,
			(GetBlue(red) * (sev - 50) + GetBlue(yellow) * (100 - sev)) / 50.0 / 255.0
		);
	}
	return st;
}

CString CLy::GetLyMarkColor(int sev) {
	CString st;
	DWORD green = MakeColor(0, 130, 255, 130);
	DWORD yellow = MakeColor(0, 255, 236, 71);
	DWORD red = MakeColor(0, 255, 160, 160);
	if (sev <= 50) {
		st.Format("%.3f %.3f %.3f",
			(GetRed(yellow) * sev + GetRed(green) * (50 - sev)) / 50.0 / 255.0,
			(GetGreen(yellow) * sev + GetGreen(green) * (50 - sev)) / 50.0 / 255.0,
			(GetBlue(yellow) * sev + GetBlue(green) * (50 - sev)) / 50.0 / 255.0
		);
	}
	else {
		st.Format("%.3f %.3f %.3f",
			(GetRed(red) * (sev - 50) + GetRed(yellow) * (100 - sev)) / 50.0 / 255.0,
			(GetGreen(red) * (sev - 50) + GetGreen(yellow) * (100 - sev)) / 50.0 / 255.0,
			(GetBlue(red) * (sev - 50) + GetBlue(yellow) * (100 - sev)) / 50.0 / 255.0
		);
	}
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
