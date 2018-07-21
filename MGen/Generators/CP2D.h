#pragma once
#include "..\GLibrary\GTemplate.h"

#define MAX_NOTE 127
#define MAX_SEVERITY 101
// Allocate at least this number of rules
#define RULE_ALLOC 500
// Maximum species
#define MAX_SPECIES 5
// Maximum voice count
#define MAX_VC 9
// Maximum voice pair (0 = highest voice, 1 = lowest voice, 2 = other voices)
// Voice pairs (0 = lowest + highest, 1 = lowest + non-highest, 2 = non-lowest + non-highest)
#define MAX_VP 2

#define CC_C(note, bn, mode) (chrom_to_dia[(note + 12 - bn + mode) % 12] + ((note + 12 - bn + mode) / 12) * 7)

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
	CString GetRuleName(int rid, int sp, int vc, int vp);
	CString GetSubRuleName(int rid, int sp, int vc, int vp);
	CString GetRuleComment(int rid, int sp, int vc, int vp);
	CString GetSubRuleComment(int rid, int sp, int vc, int vp);
	inline void ParseRule(int rid, int type);
	inline void ParseRule2(int sp, int vc, int vp, int rid, int type);
	inline int GetRuleParam(int sp, int vc, int vp, int rid, int type, int id);
	void ParseRules();

	inline void SetRuleParam(vector<vector<vector<int>>>& par, int rid, int type, int id);

	void SetRuleParams();

	void FillPause(int start, int length, int v);

	void GetFlag(int f);

	void GetSpVcVp();

	int max_rule = 0;
	int av_cnt = 0;
	int c_len = 0;
	int ep2 = 0;
	vector<int> minl; // [v]
	vector<int> maxl; // [v]
	vector<int> fli_size; // [v]
	vector<int> vsp; // Species for each voice
	vector<CString> vname; // [v] 
	vector<int> nmin; // [v]
	vector<int> nmax; // [v]
	vector<int> nmind; // [v] diatonic
	vector<int> nmaxd; // [v] diatonic
	int npm;
	int s, s0, s1, s2, s_1, s_2;
	int v, v2, vi;
	int ls;
	int sp, vc, vp;
	int fl; // Current flag id

	// Key
	int fifths = 0; // Number of alterations near key
	int bn = 9; // Base tonic note (C - 0, Am - 9)
	int mminor = 1; // If current cp is in melodic minor
	int mode = 9; // 0 - major, 2 - dorian, 9 - aeolian

	int cp_tempo = 100;
	int step0 = 0;

	// Rules
	vector<RuleInfo> ruleinfo; // [rid]
	vector<vector<vector<vector<RuleInfo2>>>> ruleinfo2; // [sp][vc][vp][rid]
	vector<vector<vector<vector<int>>>> accept; // [sp][vc][vp][rid]
	vector<vector<vector<vector<int>>>> severity; // [sp][vc][vp][rid]
	vector<int>* vaccept;

	// Parameters
	int show_allowed_flags = 0; // Show even allowed flags(bold in rules.xlsm)
	int show_ignored_flags = 0; // Show even ignored flags(with strikethrough in rules.xlsm)
	int show_min_severity = 0; // Minimum severity to highlight note
	int show_severity = 0; // =1 to show severity and flag id in square brackets in comments to notes (also when exporting to MIDI file)
	int fill_steps_mul = 2; // Multiply number of notes between leap notes to get steps for filling
	int harm_notation = 1; // 1 - wordwide harmonic notation(I, II...), 2 - Sposobin harmonic notation(T, SII...)
	int show_hatch = 1; // 0 - show no hatch, 1 = show dissonance hatch, 2 = show msh hatch
	int show_harmony_bass = 1; // 0 = do not show bass, 1 = Show harmony bass for higher cantus only, 2 = always show harmony bass
	int first_steps_tonic = 3; // Number of first steps, which must contain tonic note

	// Rule parameters [sp][vc][vp]
	vector<vector<vector<int>>> pco_apart; // Minimum allowed distance between pco in quarters
	vector<vector<vector<int>>> sus_last_measures; // Last measures in which sus is allowed in species 2 and 3
	vector<vector<vector<int>>> cse_leaps_r; // Last measures in which sus is allowed in species 2 and 3
	vector<vector<vector<int>>> lclimax_mea5; // Last measures in which sus is allowed in species 2 and 3
	vector<vector<vector<int>>> gis_trail_max; // Minimum notes between G# and next G note in Am
	vector<vector<vector<int>>> fis_gis_max; // Maximum allowed distance between F# and G#
	vector<vector<vector<int>>> fis_g_max; // Minimum distance from G to F# (+1 to allow)
	vector<vector<vector<int>>> fis_g_max2; // Minimum distance from F# to G (+1 to allow)

	vector<vector<vector<int>>> pre_last_leaps; // Last leaps that can be precompensated
	vector<vector<vector<int>>> dev_late2; // Maximum note count to consider non-late leap compensation deviation to 2nd
	vector<vector<vector<int>>> dev_late3; // Maximum note count to consider non-late leap compensation deviation to 3rd
	vector<vector<vector<int>>> dev2_maxlen; // Maximum >5th 2nd deviation length in number of 1/4
	vector<vector<vector<int>>> fill_pre3_notes; // How many notes to search for fill preparation for compensation to 3rd
	vector<vector<vector<int>>> fill_pre4_notes; // How many notes to search for fill preparation for compensation to Xth in the end
	vector<vector<vector<int>>> fill_pre4_int; // Interval to be compensated in the end
	vector<vector<vector<int>>> c4p_last_meas; // Last measures that can have leap c4p compensated
	vector<vector<vector<int>>> c4p_last_notes; // Last notes that can have leap c4p compensated

	vector<vector<vector<int>>> max_leaps; // Maximum allowed leaps during max_leap_steps
	vector<vector<vector<int>>> max_leaped; // Maximum allowed leaped-over-notes during max_leap_steps
	vector<vector<vector<int>>> max_leap_steps;
	vector<vector<vector<int>>> max_leaps_r; // Maximum allowed leaps during max_leap_steps2
	vector<vector<vector<int>>> max_leaped_r; // Maximum allowed leaped-over-notes during max_leap_steps
	vector<vector<vector<int>>> max_leap_steps2;
	vector<vector<vector<int>>> max_leaps2; // Maximum allowed leaps during max_leap_steps
	vector<vector<vector<int>>> max_leaped2; // Maximum allowed leaped-over-notes during max_leap_steps
	vector<vector<vector<int>>> max_leaps2_r; // Maximum allowed leaps during max_leap_steps
	vector<vector<vector<int>>> max_leaped2_r; // Maximum allowed leaped-over-notes during max_leap_steps
	int c4p_last_steps; // Last steps that can have leap c4p compensated (converted from measures)
	int c4p_last_notes2; // Last notes that can have leap c4p compensated (corrected with regard to measures)
	int lclimax_notes; // Number of adjacent notes to calculate local climax
	int lclimax_mea; // Number of adjacent measures to calculate local climax

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
	vector<int> nstat2; // [c]
	vector<int> nstat3; // [c]
	vector<vector<int>> dtp; // [v] Distance to closest pause in notes
	vector<vector<int>> dtp_s; // [v] Distance to closest pause in notes

	// FailLeap local variables
	int leap_start; // Step where leap starts
	int leap_end; // Step where leap ends
	int leap_mid; // Middle step of leap in case when leap is created by two consecutive 3rds
	int fleap_start; // fli2 position where leap starts
	int fleap_end; // fli2 position where leap ends
	int leap_size; // Diatonic size of leap
	int leap_id; // Id of leap size
	int filled, prefilled; // If leap is filled and prefilled
	int mdc1, mdc2; // Status of melody direction change before and after leap
	vector <int> tc; // [] Tail diatonic notes

	// Harmonic data
	vector<vector<int>> sus; // [v][ls] Note suspension flag (when above zero, links to first cantus-changing step)
	vector<vector<int>> susres; // [v][ls] =1 if sus is resolved correctly
	vector<vector<int>> isus; // [v][ls] Points to sus position or note start if there is no sus
	vector<vector<int>> mshb; // [v][ls] Melody shape types for fli (basic without patterns)
	vector<vector<int>> mshf; // [v][ls] Melody shape types for fli (with fixed patterns)
	vector<vector<int>> msh; // [v][ls] Melody shape types for fli
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
	vector<DWORD>  sev_color; // Severity colors
	int fpenalty; // Additional flags penalty
	int skip_flags;

	// Check data ready
	vector<int> data_ready; // If data is ready to be used
	vector<int> warn_data_ready; // How many warnings of data ready fired
	vector<int> data_ready_persist; // If data is ready to be used (not cleared by ClearReady)
	vector<int> warn_data_ready_persist; // How many warnings of data ready fired

	// Warnings
	int warn_rule_undefined = 0;

	// Ly Parameters
	int ly_msh = 1; // 0 - do not show msh, 1 - show msh
	int ly_rule_verbose = 0; // How verbose rule display format is

	// Graphs
	vector <vector <float>> tweight; // [ls] Tonic weight for each note
	vector <vector <int>> g_leaps; // [s] Number or leaps in window
	vector <vector <int>> g_leaped; // [s] Number or leaped notes in window

	int cp_id = 0;
};

