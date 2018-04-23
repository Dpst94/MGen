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
	vector<int> nfn; // Note flag number
	vector<int> nff; // Note flag foreign
	vector<int> nfs; // Note flag shape
	vector<CString> nfc; // Note flag comment
};

class CLy :
	public MFOut
{
public:
	CLy();
	~CLy();

	void TestKeyMatrix();
	void LoadLyShapes(CString fname);
	void GetLyRange(int step1, int step2, vector<int>& vm_min, vector<int>& vm_max);
	void GetLyVcnt(int step1, int step2, vector<int>& vm_max);
	CString GetLyAlter(int alter);
	CString GetLyAlterVisual(int alter, CString sz);
	CString GetLyNote(int i, int v);
	CString GetLyNoteVisual(int i, int v, CString sz);
	CString GetLyLen(int length);
	void SplitLyNote5(int pos, vector<int>& la);
	void SplitLyNoteMeasure(int pos, int le, vector<int>& la);
	void SplitLyNote(int pos, int le, vector<int>& la);
	void SendLyViz(ofstream & fs, int pos, CString & ev, int le, int i, int v, int phase);
	void SendLyEvent(ofstream & fs, int pos, CString ev, int le, int i, int v);
	CString GetLyColor(DWORD col);
	CString GetLyMarkColor(DWORD col);
	CString GetLyMarkColor2(DWORD col);
	void SendLyNoteColor(ofstream & fs, DWORD col);
	CString GetRealIntName(int s, int v1, int v2);
	CString GetIntName(int iv);
	void AddNLink(int i, int i2, int v, int fl, int ln, int foreign);
	void ParseNLinks(int i, int i2, int v, int foreign);
	void SaveLyComments(int i, int v, int pos);
	CString DetectLyClef(int vmin, int vmax);
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
	void SaveLy(CString dir, CString fname);
	void SelectSpeciesRules();

	// Shape scripts
	vector <vector<unordered_map<int, CString>>> shsc;

	// Rules
	vector <int> accept; // Each 1 allows showing canti with specific properties
	vector<int> rule_viz; // [r_id] Rule visualization type
	vector<int> rule_viz_v2; // [r_id] Rule visualization type for second voice
	vector<int> rule_viz_int; // [r_id] Rule visualization with interval text
	vector<CString> rule_viz_t; // [r_id] Rule visualization text
	vector <int> severity; // Get severity by flag id
	vector<DWORD>  flag_color; // Flag colors
	vector<vector <CString>> RuleName; // [sp][rid] Names of all rules
	vector<vector <CString>> SubRuleName; // [sp][rid] Names of all rules
	vector<vector <CString>> RuleComment; // [sp][rid] Comments for flag groups
	vector<vector <CString>> SubRuleComment; // [sp][rid] Comments for flags
	int cspecies = 0; // Counterpoint species (current). For example, in CA2 can be zero when evaluating CF
	int cspecies0 = -1; // Last saved species, for which rules and parameters are loaded
	int flags_need2 = 0; // Number of second level flags set
	vector <vector<int>> accepts; // [sp][rid] Each 1 allows showing canti with specific properties
	vector <vector<int>> severities; // [sp][rid] 
	int max_flags = 0; // Maximum number of rules

	// Lilypond parameters
	int ly_flag_style = 1; // 0 - no flag visualisation, 1 - color note, 2 - x above note
	int ly_dominant_letter = 0; // 0 - use normal D/d letters; 1 - use dashed D/d letters
	int ly_msh = 1; // 0 - do not show msh, 1 - show msh
	int ly_pagebreak = 1; // Page break after each analysis
	int ly_subrules = 1; // Show subrule names in ly
	int ly_comments = 1; // Show rule and subrule comments in ly
	int ly_rule_colon = 1; // Show rule name after colon

	// Lilypond other
	ofstream ly_fs;
	float ly_mul = 1; // midifile_mul
	int ly_nnum = 0; // Note number
	int ly_step1 = 0;
	int ly_step2 = 0;
	int ly_vlow = 0; // Show harmony under this voice
	int ly_vhigh = 0; // Show mistakes above this voice
	int ly_vm_cnt = 0; // Number of voices in segment to display
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
	int ly_saved = 0;
};

