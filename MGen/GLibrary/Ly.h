// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#pragma once
#include "MFOut.h"

#define SHAPE_PHASE_CNT 15

#define ssStart 0
#define ssFinish 1

#define MAX_CLEF 6
const int LyClefCenter[] = { 26, 38, 50, 71, 83, 95 };
const CString LyClef[] = { "bass_15", "bass_8", "bass", "treble", "treble^8", "treble^15" };

const int key_base[] = {
	0, // 0
	2, // 1
	2, // 2
	4, // 3
	4, // 4
	5, // 5
	7, // 6
	7, // 7
	9, // 8
	9, // 9
	11, // 10
	11 // 11
};

const CString LyMajorKey[] = {
	"c", // 0
	"df", // 1
	"d", // 2
	"ef", // 3
	"e", // 4
	"f", // 5
	"gf", // 6
	"g", // 7
	"af", // 8
	"a", // 9
	"bf", // 10
	"b" // 11
};

const int key_base_m[] = {
	0, // 0
	0, // 1
	2, // 2
	4, // 3
	4, // 4
	5, // 5
	5, // 6
	7, // 7
	7, // 8
	9, // 9
	11, // 10
	11 // 11
};

const CString LyMinorKey[] = {
	"c", // 0
	"cs", // 1
	"d", // 2
	"ef", // 3
	"e", // 4
	"f", // 5
	"fs", // 6
	"g", // 7
	"gs", // 8
	"a", // 9
	"bf", // 10
	"b" // 11
};

const int LyMajorKeySharp[] = {
	1, // 0
	0, // 1
	1, // 2
	0, // 3
	1, // 4
	0, // 5
	0, // 6
	1, // 7
	0, // 8
	1, // 9
	0, // 10
	1 // 11
};

const int LyMinorKeySharp[] = {
	0, // 0
	1, // 1
	0, // 2
	0, // 3
	1, // 4
	0, // 5
	1, // 6
	0, // 7
	1, // 8
	1, // 9
	0, // 10
	1 // 11
};

const CString LyNoteSharp[] = {
	"c", // 0
	"cs", // 1
	"d", // 2
	"ds", // 3
	"e", // 4
	"f", // 5
	"fs", // 6
	"g", // 7
	"gs", // 8
	"a", // 9
	"as", // 10
	"b" // 11
};

const CString LyNoteFlat[] = {
	"c", // 0
	"df", // 1
	"d", // 2
	"ef", // 3
	"e", // 4
	"f", // 5
	"gf", // 6
	"g", // 7
	"af", // 8
	"a", // 9
	"bf", // 10
	"b" // 11
};

const CString LyOctave[] = {
	",,,,", // 0
	",,,", // 1
	",,", // 2
	",", // 3
	"", // 4
	"'", // 5
	"''", // 6
	"'''", // 7
	"''''", // 8
	"'''''", // 9
	"''''''", // 10
	"'''''''" // 11
};

const CString mode_name[] = {
	"major", // 0
	"", // 1
	"dorian", // 2
	"", // 3
	"phrygian", // 4
	"lydian", // 5
	"", // 6
	"mixolydian", // 7
	"", // 8
	"minor", // 9
	"", // 10
	"locrian" // 11
};

const CString mode_name2[] = {
	"ionian", // 0
	"", // 1
	"dorian", // 2
	"", // 3
	"phrygian", // 4
	"lydian", // 5
	"", // 6
	"mixolydian", // 7
	"", // 8
	"aeolian", // 9
	"", // 10
	"locrian" // 11
};

// Rule visualization
#define vDefault 0
#define vHarm 1
#define vInterval 2
#define vVBracket 3
#define vVolta 4
#define vSlur 5
#define vPSlur 6
#define vGlis 7
#define vBracket 8
#define vTrill 9
#define vTS 10 // Text spanner
#define vOttava 11
#define vPedal 12
#define vNoteName 13
#define MAX_VIZ 14

// Visualisation types
#define vtPoint 1 // Can link only to one note
#define vtVBracket 2 // Cannot collide in same interval between notes
#define vtVolta 3 // Can mark single note
#define vtLink 4 // Cannot mark less than two notes
#define vtGroup 5 // Cannot mark less than two notes, borders cannot overlap

const int viz_type[MAX_VIZ] = { 1, 1, 1, 2, 3, 4, 4, 4, 5, 5, 4, 5, 5, 3 };

// For each visualisation, specify if it can overlap
const int viz_can_overlap[MAX_VIZ] = { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 };

// For each visualisation, specify if empty string should be replaced with space
const int viz_space[MAX_VIZ] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0 };

// For each visualisation, specify if single note can be marked
const int viz_singlenote[MAX_VIZ] = { 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1 };

class CLy :
	public MFOut
{
public:
	CLy();
	~CLy();
	int ly_saved = 0;

protected:
	void TestKeyMatrix();
	void LoadLyShapes(CString fname);
	CString GetLyAlter(int alter);
	CString GetLyAlterVisual(int alter, CString sz);
	CString GetLyNote(int i, int v);
	CString GetLyNoteVisual(int i, int v, CString sz);
	CString GetLyLen(int length);
	void SplitLyNote5(int pos, vector<int>& la);
	void SplitLyNoteMeasure(int pos, int le, vector<int>& la);
	void SplitLyNote(int pos, int le, vector<int>& la);
	CString GetLyColor(int sev);
	CString GetLyMarkColor(int sev);
	CString GetRealIntName(int s, int v1, int v2);
	CString GetIntName(int iv);
	CString DetectLyClef(int vmin, int vmax);

	// Shape scripts
	vector <vector<unordered_map<int, CString>>> shsc;

	// General
	ofstream ly_fs;
};

