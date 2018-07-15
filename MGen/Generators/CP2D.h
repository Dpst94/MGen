#pragma once
#include "..\GLibrary\GTemplate.h"

// Allocate at least this number of rules
#define RULE_ALLOC 500
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
	void LoadSpecies(CString st);
	void LoadConfigLine(CString * sN, CString * sV, int idata, float fdata);
	void LoadRules(CString fname);
	void ResizeRuleVariantVector(vector<vector<vector<vector<int>>>>& ve);
	void ResizeRuleVariantVector(vector<vector<vector<int>>>& ve);
	void ResizeRuleVariantVector(vector<vector<vector<RuleInfo2>>> &ve);
	void ResizeRuleVariantVectors2();
	inline void SaveRuleVariant(int sp, int vc, int vp, int rid, CString rule, CString subrule, CString rule_com, CString subrule_com);
	void CheckRuleList();
	int Interval2Chromatic(int iv);
	inline CString GetRuleName(int rid, int sp, int vc, int vp);
	inline CString GetSubRuleName(int rid, int sp, int vc, int vp);
	inline CString GetRuleComment(int rid, int sp, int vc, int vp);
	inline CString GetSubRuleComment(int rid, int sp, int vc, int vp);
	inline void ParseRule(int rid, int type);
	inline void ParseRule2(int sp, int vc, int vp, int rid, int type);
	inline int GetRuleParam(int sp, int vc, int vp, int rid, int type, int id);
	void ParseRules();

	inline void SetRuleParam(vector<vector<vector<int>>>& par, int rid, int type, int id);

	void SetRuleParams();

	int max_rule = 0;
	int av_cnt = 0;
	int c_len = 0;
	int ep2 = 0;
	vector<int> minl;
	vector<int> maxl;
	vector<int> fli_size;
	vector<int> vsp; // Species for each voice
	int npm;
	int s, s2;
	int v;
	int ls;

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
	vector<vector<vector<int>>> cse_leaps_r; // Last measures in which sus is allowed in species 2 and 3
	vector<vector<vector<int>>> lclimax_mea5; // Last measures in which sus is allowed in species 2 and 3
	int lclimax_notes = 12; // Number of adjacent notes to calculate local climax
	int lclimax_mea = 6; // Number of adjacent measures to calculate local climax

	// Main vectors
	vector<int> vid; // [v] Voice id for each voice
	vector<int> vca; // [s] Voice count for each step
	vector<int> hva; // [s] Highest voice for this step
	vector<int> lva; // [s] Lowest voice for this step
	vector<int> mli; // [s] Links to measure start step
	vector<int> bmli; // [s] Links from step to measure
	vector<vector<int>> c; // [v][s] Diatonic
	vector<vector<int>> cc; // [v][s] Chromatic
	vector<vector<int>> pc; // [v][s] Pitch class (diatonic)
	vector<vector<int>> pcc; // [v][s] Pitch class (chromatic)
	vector<vector<int>> leap; // [v][s] Leaps
	vector<vector<int>> smooth; // [v][s] Smooth movements
	vector<vector<int>> slur; // [v][s] Slurs
	vector<vector<int>> retr; // [v][s] Equals 1 if note should be retriggered
	vector<vector<int>> fli; // [v][ls] Forward links to start of each non-slurred note
	vector<vector<int>> fli2; // [v][ls] Forward links to end of each non-slurred note
	vector<vector<int>> llen; // [v][ls] Length of each linked note in steps
	vector<vector<int>> rlen; // [v][ls] Real length of each linked note (in croches)
	vector<vector<int>> bli; // [v][s] Back links from each step to fli2
	vector<vector<float>> macc; // [v][s] CC moving average
	vector<vector<float>> macc2; // [v][s] CC moving average smoothed
	vector<vector<int>> lclimax; // [v][s] Local highest note (chromatic)
	vector<vector<int>> lclimax2; // [v][s] Local highest note (chromatic)
	vector<vector<int>> beat; // [v][ls] Beat type for each fli2: 0 = downbeat, 1 = beat 3

	// Harmonic data
	vector<vector<int>> sus; // [v][ls] Note suspension flag (when above zero, links to first cantus-changing step)
	vector<vector<int>> susres; // [v][ls] =1 if sus is resolved correctly
	vector<vector<int>> isus; // [v][ls] Points to sus position or note start if there is no sus
	vector<vector<int>> mshb; // [v][ls] Melody shape types for fli (basic without patterns)
	vector<vector<int>> mshf; // [v][ls] Melody shape types for fli (with fixed patterns)
	vector<vector<int>> pat; // [v][ls] Pattern (cambiata, dnt...) for fli
	vector<vector<int>> pat_state; // [v][ls] Pattern (cambiata, dnt...) for fli state: 0 - not applied, 1 - fixed, 2,3 - variants
	vector<int> hli; // [hs] Forward links to first notes of each harmony
	vector<int> ha64; // [hs] Audible 6/4 chord, while hbc will show root position or sixth chord
	vector<int> hli2; // [hs] Forward links to last notes of each harmony
	vector<int> hbcc; // [hs] Bass note of each harmony (chromatic)
	vector<int> hbc; // [hs] Bass note of each harmony (diatonic)
	vector<int> bhli; // [s] Back links to first notes of each harmony

	// Flags
	vector<vector<vector<int>>> flag; // [v][s][] Note flags
	vector<vector<vector<int>>> fsl; // [v][s][] Note flags links to steps
	vector<vector<vector<int>>> fvl; // [v][s][] Note flags links to voices
	int fpenalty; // Additional flags penalty

	// Check data ready
	vector<int> data_ready; // If data is ready to be used
	vector<int> warn_data_ready; // How many warnings of data ready fired
	vector<int> data_ready_persist; // If data is ready to be used (not cleared by ClearReady)
	vector<int> warn_data_ready_persist; // How many warnings of data ready fired

};

