#pragma once
#include "..\GLibrary\GTemplate.h"

// Maximum species
#define MAX_SPECIES 5
// Maximum voice count
#define MAX_VC 9
// Maximum voice pair (0 = highest voice, 1 = lowest voice, 2 = other voices)
// Voice pairs (0 = lowest + highest, 1 = lowest + non-highest, 2 = non-lowest + non-highest)
#define MAX_VP 2

// Rule string types
#define rsName 0 // Rule name
#define rsSubName 1 // Subrule name
#define rsComment 2 // Rule comment
#define rsSubComment 3 // Subrule comment

// Voice pair types
#define vpExt 0 // Extreme voices (bass and highest)
#define vpBas 1 // Bass + non-highest voice
#define vpNbs 2 // Non-bass + any other voice (including highest)

// This information is specific to rule
struct RuleInfo {
	CString RuleClass;
	CString RuleGroup;
	int viz;
	int viz_v2;
	int viz_int;
	CString viz_text;
	int false_positives_global;
	int false_positives_ignore;
	int sas_emulator_max_delay;
	int sas_emulator_move_ignore;
	int sas_emulator_unstable;
	vector<int> sas_emulator_replace;
	vector<int> flag_replace;
	// Aggregated info
	CString RuleName;
	CString SubRuleName;
	CString RuleComment;
	CString SubRuleComment;
	vector<vector<int>> RuleParam;
};

// This information is specific to rule with particular sp/vc/vg
struct RuleInfo2 {
	CString RuleName;
	CString SubRuleName;
	CString RuleComment;
	CString SubRuleComment;
	vector<vector<int>> RuleParam;
};

class CP2D :
	public CGTemplate
{
public:
	CP2D();
	~CP2D();

protected:
	void LoadConfigLine(CString * sN, CString * sV, int idata, float fdata);
	void LoadRules(CString fname);
	void ResizeRuleVariantVector(vector<vector<vector<vector<int>>>>& ve);
	void ResizeRuleVariantVector(vector<vector<vector<int>>>& ve);
	void ResizeRuleVariantVector(vector<vector<vector<RuleInfo2>>> &ve);
	void ResizeRuleVariantVectors2();
	inline void SaveRuleVariant(int sp, int vc, int vp, int rid, int flag, int sev, CString rule, CString subrule, CString rule_com, CString subrule_com);
	void CheckRuleList();
	int Interval2Chromatic(int iv);
	inline void ParseRule(int sp, int vc, int vp, int rid, int type);
	inline int GetRuleParam(int sp, int vc, int vp, int rid, int type, int id);
	void ParseRules();

	inline void SetRuleParam(vector<vector<vector<int>>>& par, int rid, int type, int id);

	void SetRuleParams();

	int max_rule = 0;
	int av_cnt;
	int c_len = 0;
	int ep2;
	vector<int> minl;
	vector<int> maxl;
	vector<int> fli_size;

	int cp_tempo = 100;
	int step0 = 0;

	// Rules
	vector<RuleInfo> ruleinfo; // [rid]
	vector<vector<vector<vector<RuleInfo2>>>> ruleinfo2; // [sp][vc][vg][rid]
	vector<vector<vector<vector<int>>>> accept; // [sp][vc][vg][rid]
	vector<vector<vector<vector<int>>>> severity; // [rid][sp][vc][vg]

	// Rule parameters [sp][vc][vg]
	vector<vector<vector<int>>> pco_apart; // Minimum allowed distance between pco in quarters
	vector<vector<vector<int>>> sus_last_measures; // Last measures in which sus is allowed in species 2 and 3

	// Main vectors
	vector<int> vid; // [v] Voice id
	vector<vector<int>> c; // [v][s] Diatonic
	vector<vector<int>> cc; // [v][s] Chromatic
	vector<vector<int>> pc; // [v][s] Pitch class (diatonic)
	vector<vector<int>> pcc; // [v][s] Pitch class (chromatic)
	vector<vector<int>> leap; // [v][s] Leaps
	vector<vector<int>> smooth; // [v][s] Smooth movements
	vector<vector<int>> slur; // [v][s] Slurs
	vector<vector<int>> retr; // [v][s] Equals 1 if note should be retriggered
	vector<vector<int>> fli; // [ls] Forward links to start of each non-slurred note
	vector<vector<int>> fli2; // [ls] Forward links to end of each non-slurred note
	vector<vector<int>> llen; // [ls] Length of each linked note in steps
	vector<vector<int>> rlen; // [ls] Real length of each linked note (in croches)
	vector<vector<int>> bli; // [s] Back links from each step to fli2

	// Check data ready
	vector<int> data_ready; // If data is ready to be used
	vector<int> warn_data_ready; // How many warnings of data ready fired
	vector<int> data_ready_persist; // If data is ready to be used (not cleared by ClearReady)
	vector<int> warn_data_ready_persist; // How many warnings of data ready fired

};

