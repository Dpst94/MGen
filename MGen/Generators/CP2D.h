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

// Rule string types
#define rsName 0 // Rule name
#define rsSubName 1 // Subrule name
#define rsComment 2 // Rule comment
#define rsSubComment 3 // Subrule comment

// Vocal ranges
#define vrUndef 0 // Undefined
#define vrBas 1 // Bass
#define vrTen 2 // Tenor
#define vrAlt 3 // Alto
#define vrSop 4 // Soprano

// Voice pair types
#define vpExt 0 // Extreme voices (bass and highest)
#define vpBas 1 // Bass + non-highest voice
#define vpNbs 2 // Non-bass + any other voice (including highest)

// Task
#define tGen 0
#define tEval 1
#define tCor 2

const CString degree_name[] = {
	"I", // 0
	"II", // 1
	"III", // 2
	"IV", // 3
	"V", // 4
	"VI", // 5
	"VII" // 6
};

// Miminal note length for each species
const int sp_nlen[] = {
	8, // 0
	8, // 1
	4, // 2
	2, // 3
	4, // 4
	1 // 5
};

// Vocal range information
struct LyLogEntry {
	CString st;
	int level;
	int pos;
};

// Vocal range information
struct VocalRangeInfo {
	CString name;
	CString clef;
	int min_cc; // Absolute minimum
	int max_cc; // Absolute maximum
	int high_cc; // Above this cc is high range
	int low_cc; // Below this cc is low range
};

// This information is specific to rule
struct RuleInfo {
	CString RuleClass;
	CString RuleGroup;
	int viz;
	int viz_harm;
	int viz_sep;
	int viz_int;
	int viz_notecolor;
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

// Flags information
struct FlagInfo {
	int s;
	int id;
	int fsl;
	int voice;
	int fvl;
};

class CP2D :
	public CGTemplate
{
public:
	CP2D();
	~CP2D();

protected:
	void LoadVocalRanges(CString fname);
	void LoadSpecies(CString st);
	void LoadConfigLine(CString * sN, CString * sV, int idata, float fdata);
	void LoadHSP(CString fname);
	void LoadRules(CString fname);
	void ResizeRuleVariantVector(vector<vector<vector<vector<int>>>>& ve);
	void ResizeRuleVariantVectorNegative(vector<vector<vector<vector<int>>>>& ve);
	void ResizeRuleVariantVector(vector<vector<vector<int>>>& ve);
	void ResizeRuleVariantVector(vector<vector<vector<float>>>& ve);
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
	inline float GetRuleParamF(int sp, int vc, int vp, int rid, int type, int id);
	void ParseRules();

	inline void SetRuleParam(vector<vector<vector<int>>>& par, int rid, int type, int id);
	inline void SetRuleParamI2C(vector<vector<vector<int>>>& par, int rid, int type, int id);
	inline void SetRuleParam(vector<vector<vector<float>>>& par, int rid, int type, int id);
	void SetRuleParams();
	void FillPause(int start, int length, int v);
	void GetFlag(int f);
	inline void GetSpVcVp();
	inline void GetVp();
	void LoadHarmNotation();
	void LoadIntNames();
	CString GetHarmName(int pitch, int alter);
	static CString GetPrintKey(int lbn, int lmode, int mminor = -1);
	void TestCC_C2();
	void BuildPitchConvert();
	void LogVector(CString print_st, int print_int, int x1, int x2, vector<int>& c, CString fname);
	void WriteLogLy(int i, CString st, int pos);

	vector<LyLogEntry> ly_log;

	CString xml_text; // Imported text
	CString xml_lyrics; // Imported lyrics

	int max_rule = 0;
	int av_cnt = 0;
	int c_len = 0;
	int ep2 = 0;
	vector<int> minl; // [v]
	vector<int> maxl; // [v]
	vector<int> fli_size; // [v]
	vector<int> vsp; // [v] Species for each voice
	vector<CString> vname; // [v] 
	vector<CString> vname2; // [v] Names are unique
	vector<int> nmin; // [v]
	vector<int> nmax; // [v]
	vector<int> nmind; // [v] diatonic
	vector<int> nmaxd; // [v] diatonic
	vector<int> fin; // [v] first note position in steps (after starting pause)
	vector<int> fil; // [v] First note link (after starting pause)
	int npm; // Steps per measure
	int btype; // Beat type for current cp
	vector<int> nlen; // Minimal note length for species 2 and 4

	// Indexes
	int s, s0, s1, s2, s3, s4, s5, s_1, s_2;
	int v, v2, v3, v4, vi;
	int ls, ls2, ls3, ls4, ls5, ls6;
	int ms;
	int hs;
	int sp, sp2, vc, vp;
	int fl; // Current flag id

	// PcoApart
	int pco5_last = 0; // Last interval start
	int pco8_last = 0;
	int pco5_last2 = 0; // Last interval end
	int pco8_last2 = 0;

	// Accumulated flags
	int var;
	vector<FlagInfo> flagab; // Saved flags of best variant
	vector<FlagInfo> flaga; // Flags of current variant
	FlagInfo temp_flaginfo;
	int hpenalty; // Harmonic penalty
	int hstart; // (s) Start of current harmony
	int hend; // (s) End of current harmony

	// Local
	int hrepeat_fired = 0; // Harmonic repeat in step
	int hmiss_fired = 0; // Harmonic miss in step

	// Key
	int fifths = 0; // Number of alterations near key
	int bn = 9; // Base tonic note (C - 0, Am - 9)
	int maj_bn = 0; // Base note if it was major (C - 0, Am - 0)
	int mminor = 1; // If current cp is in melodic minor
	int mode = 9; // 0 - major, 2 - dorian, 9 - aeolian

	int cp_tempo = 100;
	int step0 = 0;

	// Rules
	vector<RuleInfo> ruleinfo; // [rid]
	vector<vector<vector<vector<RuleInfo2>>>> ruleinfo2; // [sp][vc][vp][rid]
	vector<vector<vector<vector<int>>>> accept; // [sp][vc][vp][rid]
	vector<vector<vector<vector<int>>>> severity; // [sp][vc][vp][rid]
	//vector<int>* vaccept;
	vector <vector <int>> hsp; // [pc][pc] Harmonic sequence penalty

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
	CString conf_species; // Species string

	// Rule parameters [sp][vc][vp]
	vector<vector<vector<int>>> pco_apart; // Minimum allowed distance between pco in quarters
	vector<vector<vector<int>>> sus_last_measures; // Last measures in which sus is allowed in species 2 and 3
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
	vector<vector<vector<int>>> thirds_ignored; // Number of thirds ignored for consecutive leaps rule
	vector<vector<vector<int>>> late_culm; // Early culmination step
	vector<vector<vector<int>>> early_culm; // Early culmination step
	vector<vector<vector<int>>> early_culm2; // Early culmination step (second rule)
	vector<vector<vector<int>>> early_culm3; // Early culmination step percent
	vector<vector<vector<vector<int>>>> tonic_window; // [] Number of notes that are searched for number of tonic notes
	vector<vector<vector<vector<int>>>> tonic_max; // [] Maximum number of tonic notes that can be contained in tonic window
	//vector<vector<vector<int>>> max_note_len; // Maximum note real length in croches by species
	vector<vector<vector<vector<int>>>> notes_lrange; // [range][species] Maximum number of consecutive notes having low range
	vector<vector<vector<int>>> tritone_res_quart; // Search X quarters for tritone resolution
	vector<vector<vector<int>>> max_smooth_direct; // Maximum linear movement in one direction allowed (in steps)
	vector<vector<vector<int>>> max_smooth; // Maximum linear movement allowed (in steps)
	vector<vector<vector<int>>> cse_leaps; // Maximum allowed consecutive leaps for Consecutive leaps
	vector<vector<vector<int>>> cse_leaps_r; // Maximum allowed consecutive leaps for Consecutive leaps+
	vector<vector<vector<int>>> miss_slurs_window; // Number of steps to check for missing slur limit
	vector<vector<vector<int>>> slurs_window; // Number of steps to check for slur limit
	vector<vector<vector<int>>> stag_notes; // Maximum allowed stag_notes (same notes) during stag_note_steps
	vector<vector<vector<int>>> stag_note_steps;
	vector<vector<vector<int>>> stag_notes2; // Maximum allowed stag_notes2 (same notes) during stag_note_steps2
	vector<vector<vector<int>>> stag_note_steps2;
	vector<vector<vector<int>>> notes_arange; // Maximum number of consecutive notes having low average range
	vector<vector<vector<int>>> min_arange; // Minimum allowed local range of notes_arange consecutive notes
	vector<vector<vector<int>>> notes_arange2; // Maximum number of consecutive notes having low average range
	vector<vector<vector<int>>> min_arange2; // Minimum allowed local range of notes_arange consecutive notes
	vector<vector<vector<int>>> notes_picount; // Maximum number of consecutive notes having low pitch count
	vector<vector<vector<int>>> min_picount; // Minimum allowed pitch count of notes_picount consecutive notes
	vector<vector<vector<int>>> notes_picount2; // Maximum number of consecutive notes having low pitch count
	vector<vector<vector<int>>> min_picount2; // Minimum allowed pitch count of notes_picount2 consecutive notes
	vector<vector<vector<int>>> notes_picount3; // Maximum number of consecutive notes having low pitch count
	vector<vector<vector<int>>> min_picount3; // Minimum allowed pitch count of notes_picount3 consecutive notes
	vector<vector<vector<int>>> mea_per_sus; // Maximum measures per suspension
	vector<vector<vector<int>>> repeat_letters_t; // Maximum repeated letters in a row of harmonies
	vector<vector<vector<int>>> repeat_letters_d; // Maximum repeated letters in a row of harmonies
	vector<vector<vector<int>>> repeat_letters_s; // Maximum repeated letters in a row of harmonies
	vector<vector<vector<int>>> miss_letters_t; // Maximum steps with missed letters in a row of harmonies
	vector<vector<vector<int>>> miss_letters_d; // Maximum steps with missed letters in a row of harmonies
	vector<vector<vector<int>>> miss_letters_s; // Maximum steps with missed letters in a row of harmonies
	vector<vector<vector<int>>> tonic_window_cp; // Number of harmonies that are searched for number of tonic chords
	vector<vector<vector<int>>> tonic_max_cp; // Maximum number of tonic chords that can be contained in tonic window
	vector<vector<vector<int>>> tonic_wei_inv; // Percent of weight for inverted tonic chord
	vector<vector<vector<int>>> vocra_disbal_yel; // Minimum disbalance length to flag (yellow)
	vector<vector<vector<int>>> vocra_disbal_red; // Minimum disbalance length to flag (red)
	vector<vector<vector<int>>> sus_insert_max_leap; // Maximum leap to sus resolution insertion
	vector<vector<vector<int>>> sus_insert_max_leap2; // Maximum leap to sus resolution insertion
	vector<vector<vector<float>>> cross_max_len; // Maximum length of voice crossing in measures
	vector<vector<vector<float>>> cross_max_len2; // Maximum length of voice crossing in measures (red)
	int c4p_last_steps; // Last steps that can have leap c4p compensated (converted from measures)
	int c4p_last_notes2; // Last notes that can have leap c4p compensated (corrected with regard to measures)
	int lclimax_notes; // Number of adjacent notes to calculate local climax
	int lclimax_mea; // Number of adjacent measures to calculate local climax

	// Harmony notation
	vector <CString> HarmName;
	vector <CString> HarmName_m;
	vector <CString> HarmName_ma;
	vector <CString> harm_notation_name;

	// Interval names
	vector<vector<CString>> IntName; // [lower_note][higher_note] Interval name

	// Voice vectors
	vector<int> vid; // [v] Voice id for each voice
	vector<int> vocra; // [v] Vocal range for each voice
	vector<int> vocra_detected; // [v] 0 - not detected, 1 - detected by instrument name; 2 - detected by notes range
	vector<int> vocra_used; // [v] how many times vocal range is used
	int vocra_penalty;
	vector<vector<int>> vocra_p; // [v] if vocal range is possible, it is in vector
	vector<VocalRangeInfo> vocra_info; // [v] Information loaded for each vocal range
	vector<int> vca; // [s] Voice count for each step
	vector<int> hva; // [s] Highest voice for this step
	vector<int> lva; // [s] Lowest voice for this step

	// Rhythm
	vector<vector<int>> rh_id;
	vector<vector<int>> rh_pid;

	// Intervals
	int ivl, ivl2; // Diatonic interval between voices
	int ivlc, ivlc2; // Diatonic interval class between voices
	int civl, civl2; // Chromatic interval between voices
	int civlc, civlc2; // Chromatic interval class between voices

	// Main vectors
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
	vector<vector<int>> lclimax; // [v][s] Local highest note (chromatic)
	vector<vector<int>> lclimax2; // [v][s] Local highest note (chromatic)
	vector<vector<int>> beat; // [v][ls] Beat type for each fli2: (0 10 4 11) (1 12 5 13) (2 14 6 15) (3 16 7 17)
	vector<int> nstat; // [c]
	vector<int> nstat2; // [c]
	vector<int> nstat3; // [c]
	vector<vector<int>> dtp; // [v][ls] Distance to closest pause or exercise ending in notes
	vector<vector<int>> dtp_s; // [v][ls] Distance to closest pause in notes
	vector<float> macc; // [s] CC moving average
	vector<float> macc2; // [s] CC moving average smoothed
	int macc_range = 0; // Steps outside window used to build macc
	int macc2_range = 0; // Steps outside window used to build macc2
	vector<float> decc; // [s] CC deviation
	vector<float> decc2; // [s] CC deviation smoothed
	vector<float> maw; // [] Moving average weight
	vector<vector<int>> src_alter; // [v][s] Alteration for each note in source file

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
	int min_sus; // Minimal length of intrabar sus in croches
	vector<vector<int>> sus; // [v][ls] Note suspension flag (when above zero, links to first cantus-changing step)
	vector<vector<int>> ssus; // [v][ls] Points to sus position or note start if there is no sus
	vector<vector<int>> isus; // [v][ls] 0 - note cannot be intrabar sus. -1 - note can be intrabar sus. -2 - note is intrabar sus
	vector<vector<int>> resol; // [v][s] 0 - this note is not resolved; >0 points to step of resolution note
	vector<vector<int>> msh; // [v][s] Melody shape types for fli
	vector<vector<int>> mshb; // [v][s] Best msh
	vector<vector<int>> nih; // [v][s] Is current note in harmony?
	vector<vector<int>> nihb; // [v][s] Best nih
	vector<vector<int>> islt; // [v][s] Is current note a leading tone?
	vector<vector<int>> pat; // [v][ls] Pattern (cambiata, dnt...) for fli
	vector<vector<int>> pat_state; // [v][ls] Pattern (cambiata, dnt...) for fli state: 0 - not applied, 1 - fixed, 2,3 - variants
	vector<int> chn; // [pc] Diatonic pitch classes in chord
	vector<int> cchn; // [pcc] Chromatic pitch classes in chord
	vector<vector<int>> cchn2; // [harm][pcc] Chromatic pitch classes in chord for multiple chords in measure
	vector<vector<int>> cpos; // [harm][pc] Possible chords for multiple chords in measure
	vector<vector<int>> cchnv; // [harm][pcc] Chromatic pitch classes in variant chord
	vector<int> shp; // [s] Harmony position for each step
	vector<int> hli; // [hs] Forward links to first notes of each harmony
	vector<int> ha64; // [hs] Audible 6/4 chord, while hbc will show root position or sixth chord
	vector<int> hli2; // [hs] Forward links to last notes of each harmony
	vector<int> hbcc; // [hs] Bass note of each harmony (chromatic)
	vector<int> hbc; // [hs] Bass note of each harmony (diatonic)
	vector<int> bhli; // [s] Back links to first notes of each harmony
	vector <vector<int>> cct; // [hs][3] Chord tones for each harmony
	vector <int> chm; // [hs] Current harmonic meaning (diatonic)
	vector <int> chm_alter; // [hs] Type of harmonic meaning
	int hv; // Current harmony variant being analysed
	int hv_alt; // Current harmony alteration variant being analysed

	// Flags
	vector<vector<vector<int>>> flag; // [v][s][] Note flags
	vector<vector<vector<int>>> fsl; // [v][s][] Note flags links to steps
	vector<vector<vector<int>>> fvl; // [v][s][] Note flags links to voices
	vector<vector<vector<int>>> fsep; // [v][s][] If this flag should be sent to separate staff
	vector<DWORD>  sev_color; // Severity colors
	int fpenalty; // Additional flags penalty
	int skip_flags;
	int task; // What task to accomplish using the method

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
	int ly_show_xml_text = 0; // show imported text from xml file
	int ly_show_xml_lyrics = 0; // show imported lyrics from xml file

	// Graphs
	vector <vector <float>> tweight; // [ls] Tonic weight for each note
	vector <vector <int>> g_leaps; // [s] Number or leaps in window
	vector <vector <int>> g_leaped; // [s] Number or leaped notes in window

	int cp_id = 0;

	// Pitch convert
	vector <int> cc_c;
	vector <int> c_cc; // Convert to 

	// Scan
	long long cycle = 0; // Cycle number of full scan
};

