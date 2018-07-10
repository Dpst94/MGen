// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "../stdafx.h"
#include "CF1D.h"

#ifdef _DEBUG
#define new DEBUG_NEW 
#endif

CF1D::CF1D() {
}

CF1D::~CF1D() {
}

// Load variants of possible harmonic meaning
void CF1D::LoadHarmVar() {
	//SET_READY_PERSIST(DP_hv);
	hv.resize(7);
	if (cantus_high) {
		// Create harmonic meaning variants for higher cantus
		for (int i = 0; i < 7; i++) {
			hv[i].clear();
			hv[i].push_back((i + 5) % 7);
			if (i != 3) hv[i].push_back((i + 3) % 7);
			hv[i].push_back(i);
		}
	}
	else {
		// Create harmonic meaning variants for lower cantus
		for (int i = 0; i < 7; i++) {
			hv[i].clear();
			hv[i].push_back((i + 5) % 7);
			hv[i].push_back(i);
		}
	}
}

// Load harmonic sequence penalties
void CF1D::LoadHSP(CString fname)
{
	//SET_READY_PERSIST(DP_hsp);
	CString st, est;
	vector<CString> ast;
	int i = 0;
	hsp.resize(7);
	ifstream fs;
	// Check file exists
	if (!fileExists(fname)) {
		est.Format("LoadHSP cannot find file: %s", fname);
		WriteLog(5, est);
		error = 1;
		return;
	}
	fs.open(fname);
	char pch[2550];
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
			if (ast.size() != 9) {
				est.Format("Wrong column count at line in hsp file %s: '%s'", fname, st);
				WriteLog(5, est);
				error = 1;
				return;
			}
			if (i > 7) {
				est.Format("Wrong line count at line %d in hsp file %s: '%s'", i, fname, st);
				WriteLog(5, est);
				error = 1;
				return;
			}
			hsp[i - 1].clear();
			for (int x = 0; x < 7; ++x) {
				hsp[i - 1].push_back(atoi(ast[x + 1]));
			}
		}
	}
	fs.close();
	est.Format("LoadHSP loaded %d lines from %s", i, fname);
	WriteLog(0, est);
}

void CF1D::SaveSpecRule(int sp, int rid, int flag, int sev, CString rule, CString subrule, CString rule_com, CString subrule_com) {
	if (!RuleName[sp][rid].IsEmpty()) {
		//CString est;
		//est.Format("Species %d rule '%d: %s (%s)' overwrite detected", sp, rid, rule, subrule);
		//WriteLog(5, est);
	}
	RuleName[sp][rid] = rule;
	SubRuleName[sp][rid] = subrule;
	RuleComment[sp][rid] = rule_com;
	SubRuleComment[sp][rid] = subrule_com;
	accepts[sp][rid] = flag;
	severities[sp][rid] = sev;
	rule_viz_t[rid].Replace("!rn!", RuleName[sp][rid]);
	rule_viz_t[rid].Replace("!srn!", SubRuleName[sp][rid]);
	rule_viz_t[rid].Replace("!src!", SubRuleComment[sp][rid]);
	rule_viz_t[rid].Replace("!rc!", RuleComment[sp][rid]);
}

// Load rules
void CF1D::LoadRules(CString fname) {
	//SET_READY_PERSIST(DP_Rules);
	CString st, est, rule, subrule;
	vector<CString> ast, ast2;
	vector<map<int, int>> rid_unique; // [rid][sp]
	rid_unique.resize(MAX_RULES);
	int i = 0;
	int sev = 0;
	CString spec;
	int flag = 0;
	int rid;
	ifstream fs;
	// Check file exists
	if (!fileExists(fname)) {
		est.Format("LoadRules cannot find file: %s", fname);
		WriteLog(5, est);
		error = 1;
		return;
	}
	fs.open(fname);
	char pch[2550];
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
			if (ast.size() != 24) {
				est.Format("Wrong column count at line %d in rules file %s: '%s'", i, fname, st);
				WriteLog(5, est);
				error = 1;
				return;
			}
			rid = atoi(ast[1]);
			spec = ast[2];
			sev = atoi(ast[3]);
			rule = ast[6];
			subrule = ast[7];
			flag = atoi(ast[8]);
			// Find rule id
			if (rid >= max_flags) {
				max_flags = rid + 1;
				if (max_flags >= MAX_RULES) {
					est.Format("Rule id (%d) is equal or greater than MAX_RULES (%d). Consider increasing MAX_RULES", rid, MAX_RULES);
					WriteLog(5, est);
					error = 1;
					return;
				}
			}
			//est.Format("Found rule %s - %d", rule, rid);
			//WriteLog(0, est);
			if (spec == "") spec = "012345";
			RuleClass[rid] = ast[4];
			RuleGroup[rid] = ast[5];
			// If testing, enable all disabled rules so that expected violations can be confirmed
			//if (m_testing && flag == -1) flag = 1;
			rule_viz[rid] = atoi(ast[11]);
			rule_viz_int[rid] = atoi(ast[12]);
			rule_viz_v2[rid] = atoi(ast[13]);
			rule_viz_t[rid] = ast[14];
			false_positives_global[rid] = atoi(ast[17]);
			false_positives_ignore[rid] = atoi(ast[18]);
			sas_emulator_max_delay[rid] = atoi(ast[19]);
			sas_emulator_move_ignore[rid] = atoi(ast[20]);
			if (!ast[21].IsEmpty()) {
				Tokenize(ast[21], ast2, ",");
				for (int x = 0; x < ast2.size(); ++x) {
					int fl = atoi(ast2[x]);
					if (fl) sas_emulator_replace[fl].push_back(rid);
				}
			}
			sas_emulator_unstable[rid] = atoi(ast[22]);
			if (!ast[23].IsEmpty()) {
				Tokenize(ast[23], ast2, ",");
				for (int x = 0; x < ast2.size(); ++x) {
					int fl = atoi(ast2[x]);
					if (fl) flag_replace[fl].push_back(rid);
				}
			}
			if (viz_space[rule_viz[rid]] && rule_viz_t[rid].IsEmpty()) rule_viz_t[rid] = " ";
			// Disable all species
			if (RuleName[0][rid].IsEmpty())
				for (int sp = 0; sp < MAX_SPECIES; ++sp) {
					SaveSpecRule(sp, rid, -1, sev, rule, subrule, ast[9], ast[10]);
				}
			for (int x = 0; x < spec.GetLength(); ++x) {
				int sp = atoi(spec.Mid(x, 1));
				if (rid_unique[rid][sp]) {
					est.Format("Duplicate rule %d species %d: '%s (%s)' overwrites '%s (%s)' with species filter %s",
						rid, sp, rule, subrule, RuleName[sp][rid], SubRuleName[sp][rid], spec);
					WriteLog(5, est);
				}
				else rid_unique[rid][sp] = 1;
				SaveSpecRule(sp, rid, flag, sev, rule, subrule, ast[9], ast[10]);
			}
			rule_viz_t[rid].Replace("!rc!", RuleClass[rid]);
			rule_viz_t[rid].Replace("!rg!", RuleGroup[rid]);
		}
	}
	fs.close();
	est.Format("LoadRules loaded %d lines from %s", i, fname);
	WriteLog(0, est);
	// Check that all rules in lists exist
	CheckRuleList("flag_replace", flag_replace);
	CheckRuleList("sas_emulator_replace", sas_emulator_replace);
}

void CF1D::CheckRuleList(CString list_name, vector<vector<int>> &v) {
	for (int rs = 0; rs < accepts.size(); ++rs) if (accepts[rs].size()) {
		for (int f = 0; f < v.size(); ++f) {
			for (int f2 = 0; f2 < v[f].size(); ++f2) {
				if (SubRuleName[rs][v[f][f2]].IsEmpty()) {
					CString est;
					est.Format("Detected undefined rule %d in rule list %s for rule %d",
						v[f][f2], list_name, f);
					WriteLog(5, est);
				}
			}
		}
	}
}

// Return chromatic length of an interval (e.g. return 4 from 3rd)
int CF1D::Interval2Chromatic(int iv) {
	if (iv > 0) --iv;
	else ++iv;
	int aiv = iv % 7;
	int res = 0;
	if (aiv == 1) res = 2;
	else if (aiv == 2) res = 4;
	else if (aiv == 3) res = 5;
	else if (aiv == 4) res = 7;
	else if (aiv == 5) res = 9;
	else if (aiv == 6) res = 11;
	else if (aiv == -1) res = 1;
	else if (aiv == -2) res = 3;
	else if (aiv == -3) res = 5;
	else if (aiv == -4) res = 6; // Tritone
	else if (aiv == -5) res = 8;
	else if (aiv == -6) res = 10;
	// Add octaves
	res += 12 * abs(iv / 7);
	return res;
}

// Load rules
void CF1D::ParseRule(int sp, int rid, int type) {
	CString st;
	if (type == rsName) st = RuleName[sp][rid];
	if (type == rsSubName) st = SubRuleName[sp][rid];
	if (type == rsComment) st = RuleComment[sp][rid];
	if (type == rsSubComment) st = SubRuleComment[sp][rid];
	vector<int> v;
	GetVint(st, v);
	if (v.size()) {
		// Create types
		if (!RuleParam[sp][rid].size()) RuleParam[sp][rid].resize(4);
		// Set params for type
		RuleParam[sp][rid][type] = v;
	}
}

int CF1D::GetRuleParam(int sp, int rid, int type, int id) {
	if (!RuleParam[sp][rid].size() || id >= RuleParam[sp][rid][type].size()) {
		CString est, rs;
		CString st;
		if (type == rsName) st = RuleName[sp][rid];
		if (type == rsSubName) st = SubRuleName[sp][rid];
		if (type == rsComment) st = RuleComment[sp][rid];
		if (type == rsSubComment) st = SubRuleComment[sp][rid];
		if (type == rsName) rs = "rule name";
		if (type == rsSubName) rs = "subrule name";
		if (type == rsComment) rs = "rule comment";
		if (type == rsSubComment) rs = "subrule comment";
		est.Format("Error parsing integer #%d from %s %d: '%s' (species %d)", id + 1, rs, rid, st, sp);
		WriteLog(5, est);
		error = 1;
		return 0;
	}
	return RuleParam[sp][rid][type][id];
}

// Parse rules
void CF1D::ParseRules() {
	//SET_READY_PERSIST(DP_RuleParam);
	for (int sp = 0; sp < MAX_SPECIES; ++sp) {
		RuleParam[sp].resize(MAX_RULES);
		for (int rid = 0; rid < MAX_RULES; ++rid) {
			for (int rs = 0; rs < 4; ++rs) {
				ParseRule(sp, rid, rs);
			}
		}
	}
}

// Set parsed parameters of current ruleset
void CF1D::SetRuleParams() {
	//CHECK_READY_PERSIST(DP_RuleParam);
	//SET_READY_PERSIST(DP_RuleSetParam);
	lclimax_notes = GetRuleParam(cspecies, 0, rsComment, 0);
	lclimax_mea = GetRuleParam(cspecies, 0, rsComment, 1);
	lclimax_mea5 = GetRuleParam(cspecies, 325, rsComment, 0);
	mea_per_sus = GetRuleParam(cspecies, 341, rsSubName, 0);
	max_note_len[1] = GetRuleParam(cspecies, 336, rsSubName, 1);
	max_note_len[2] = GetRuleParam(cspecies, 337, rsSubName, 1);
	max_note_len[3] = GetRuleParam(cspecies, 338, rsSubName, 1);
	max_note_len[4] = GetRuleParam(cspecies, 339, rsSubName, 1);
	max_note_len[5] = GetRuleParam(cspecies, 340, rsSubName, 1);
	tritone_res_quart = GetRuleParam(cspecies, 2, rsSubComment, 0);
	sus_last_measures = GetRuleParam(cspecies, 139, rsSubName, 0);
	sus_insert_max_leap = Interval2Chromatic(GetRuleParam(cspecies, 295, rsSubComment, 0));
	cambiata_max_leap3 = Interval2Chromatic(GetRuleParam(cspecies, 264, rsSubComment, 1));
	cambiata_max_leap4 = Interval2Chromatic(GetRuleParam(cspecies, 265, rsSubComment, 1));
	pco_apart = GetRuleParam(cspecies, 248, rsName, 1);
	c4p_last_meas = GetRuleParam(cspecies, 144, rsName, 1);
	fill_pre3_notes = GetRuleParam(cspecies, 100, rsComment, 0);
	fill_pre4_int = GetRuleParam(cspecies, 144, rsComment, 0) - 1;
	fill_pre4_notes = GetRuleParam(cspecies, 144, rsComment, 1);
	c4p_last_notes = GetRuleParam(cspecies, 144, rsName, 2);
	pre_last_leaps = GetRuleParam(cspecies, 204, rsName, 0);
	max_smooth = GetRuleParam(cspecies, 4, rsSubName, 0);
	max_smooth_direct = GetRuleParam(cspecies, 5, rsSubName, 0);
	max_smooth2 = GetRuleParam(cspecies, 302, rsSubName, 0);
	max_smooth_direct2 = GetRuleParam(cspecies, 303, rsSubName, 0);
	stag_notes = GetRuleParam(cspecies, 10, rsSubName, 0);
	stag_note_steps = GetRuleParam(cspecies, 10, rsSubName, 1);
	stag_notes2 = GetRuleParam(cspecies, 39, rsSubName, 0);
	stag_note_steps2 = GetRuleParam(cspecies, 39, rsSubName, 1);
	min_interval = Interval2Chromatic(GetRuleParam(cspecies, 38, rsSubName, 0));
	min_iv_minnotes = GetRuleParam(cspecies, 38, rsSubComment, 0);
	min_iv_minmea = GetRuleParam(cspecies, 38, rsSubComment, 1);
	min_interval = Interval2Chromatic(GetRuleParam(cspecies, 38, rsSubName, 0));
	max_interval_cf = Interval2Chromatic(GetRuleParam(cspecies, 37, rsSubName, 0));
	max_interval_cp = Interval2Chromatic(GetRuleParam(cspecies, 304, rsSubName, 0));
	sum_interval = Interval2Chromatic(GetRuleParam(cspecies, 7, rsSubName, 0));
	max_between = Interval2Chromatic(GetRuleParam(cspecies, 11, rsSubName, 0));
	burst_between = Interval2Chromatic(GetRuleParam(cspecies, 11, rsSubComment, 0));
	burst_steps = GetRuleParam(cspecies, 11, rsSubComment, 1);
	slurs_window = GetRuleParam(cspecies, 93, rsName, 0);
	miss_slurs_window = GetRuleParam(cspecies, 188, rsName, 0);
	contrary_min = GetRuleParam(cspecies, 35, rsSubName, 0);
	contrary_min2 = GetRuleParam(cspecies, 46, rsSubName, 0);

	notes_picount = GetRuleParam(cspecies, 344, rsSubName, 0);
	min_picount = GetRuleParam(cspecies, 344, rsSubName, 1);
	notes_picount2 = GetRuleParam(cspecies, 345, rsSubName, 0);
	min_picount2 = GetRuleParam(cspecies, 345, rsSubName, 1);
	notes_picount3 = GetRuleParam(cspecies, 346, rsSubName, 0);
	min_picount3 = GetRuleParam(cspecies, 346, rsSubName, 1);

	for (int fl = 0; fl < 24; ++fl) {
		notes_lrange[fl % 4][fl / 4] = GetRuleParam(cspecies, 434 + fl, rsSubName, 0);
	}

	max_leap_steps = GetRuleParam(cspecies, 493, rsName, 0);
	max_leaps = GetRuleParam(cspecies, 493, rsSubName, 0);
	max_leaped = GetRuleParam(cspecies, 494, rsSubName, 0);
	max_leap_steps2 = GetRuleParam(cspecies, 497, rsName, 0);
	max_leaps_r = GetRuleParam(cspecies, 495, rsSubName, 0);
	max_leaped_r = GetRuleParam(cspecies, 496, rsSubName, 0);
	max_leaps2 = GetRuleParam(cspecies, 497, rsSubName, 0);
	max_leaped2 = GetRuleParam(cspecies, 498, rsSubName, 0);
	max_leaps2_r = GetRuleParam(cspecies, 499, rsSubName, 0);
	max_leaped2_r = GetRuleParam(cspecies, 500, rsSubName, 0);
	cse_leaps = GetRuleParam(cspecies, 501, rsSubName, 0);
	cse_leaps_r = GetRuleParam(cspecies, 502, rsSubName, 0);
	thirds_ignored = GetRuleParam(cspecies, 501, rsComment, 0);

	notes_arange = GetRuleParam(cspecies, 15, rsSubName, 0);
	min_arange = GetRuleParam(cspecies, 15, rsSubName, 1) / 10.0;
	notes_arange2 = GetRuleParam(cspecies, 16, rsSubName, 0);
	min_arange2 = GetRuleParam(cspecies, 16, rsSubName, 1) / 10.0;

	dev_late2 = GetRuleParam(cspecies, 191, rsSubName, 0);
	dev_late3 = GetRuleParam(cspecies, 192, rsSubName, 0);
	dev2_maxlen = GetRuleParam(cspecies, 386, rsSubComment, 0);
	early_culm = GetRuleParam(cspecies, 78, rsSubName, 0);
	early_culm2 = GetRuleParam(cspecies, 79, rsSubName, 0);
	early_culm3 = GetRuleParam(cspecies, 193, rsSubName, 0);
	late_culm = GetRuleParam(cspecies, 21, rsSubName, 0);
	hsp_leap = Interval2Chromatic(GetRuleParam(cspecies, 194, rsSubName, 0));
	repeat_letters_t = GetRuleParam(cspecies, 17, rsSubName, 0);
	repeat_letters_d = GetRuleParam(cspecies, 428, rsSubName, 0);
	repeat_letters_s = GetRuleParam(cspecies, 429, rsSubName, 0);
	miss_letters_t = GetRuleParam(cspecies, 20, rsSubName, 0);
	miss_letters_d = GetRuleParam(cspecies, 430, rsSubName, 0);
	miss_letters_s = GetRuleParam(cspecies, 431, rsSubName, 0);
	ico_chain = GetRuleParam(cspecies, 89, rsSubName, 0);
	ico_chain2 = GetRuleParam(cspecies, 96, rsSubName, 0);
	gis_trail_max = GetRuleParam(cspecies, 200, rsSubName, 0);
	tonic_max_cp = GetRuleParam(cspecies, 310, rsSubName, 0);
	tonic_window_cp = GetRuleParam(cspecies, 310, rsSubName, 1);
	tonic_wei_inv = GetRuleParam(cspecies, 310, rsSubComment, 0);

	for (int tt = 0; tt < 2; ++tt) {
		tonic_max[tt] = GetRuleParam(cspecies, 70 + tt, rsSubName, 0);
		tonic_window[tt] = GetRuleParam(cspecies, 70 + tt, rsSubName, 1);
	}

	fis_gis_max = GetRuleParam(cspecies, 199, rsSubName, 0);
	fis_g_max = GetRuleParam(cspecies, 349, rsSubName, 0);
	fis_g_max2 = GetRuleParam(cspecies, 350, rsSubName, 0);
	// Check rule parameters
	if (burst_between <= max_between) {
		WriteLog(5, "Warning: maximum burst interval should be greater than maximum interval between voices (check config)");
	}
}

void CF1D::ProcessSpecies() {
	// GenCP1
	if (m_algo_id == 121) {
		if (species == 1) {
			npm = 1;
			fn = 0;
			midifile_out_mul2 = 8;
		}
		if (species == 2) {
			npm = 2;
			fn = 1;
			if (accept[273] && rand() > RAND_MAX / 2) fn = 0;
			midifile_out_mul2 = 4;
		}
		if (species == 3) {
			npm = 4;
			fn = 1;
			if (accept[273] && rand() > RAND_MAX / 2) fn = 0;
			midifile_out_mul2 = 2;
		}
		if (species == 4) {
			npm = 2;
			fn = 1;
			if (accept[273] && rand() > RAND_MAX / 2) fn = 0;
			midifile_out_mul2 = 4;
		}
		if (species == 5) {
			npm = 8;
			if (accept[273]) fn = randbw(0, 2);
			else fn = randbw(1, 2);
			if (fn == 2) fn = 4;
			else if (fn == 1) fn = 2;
			midifile_out_mul2 = 1;
		}
	}
}

void CF1D::CheckConfig() {
	//CHECK_READY_PERSIST(DP_Config);
	//SET_READY_PERSIST(DP_ConfigTest);
	// GenCP1
	if (m_algo_id == 121) {
		ProcessSpecies();
		if (accept_cantus_rechoose && cantus_id2) {
			WriteLog(1, "Warning: accept_cantus_rechoose will not work with cantus_id above zero (check config)");
		}
		if (species == 1 && fn > 0) {
			WriteLog(5, "Warning: Counterpoint species 1 cannot have starting_pause (check config)");
		}
		if (fn >= npm) {
			WriteLog(5, "Warning: Starting_pause is greater or equals to notes_per_measure (check config)");
		}
		if (fn > 1 && npm < 8) {
			WriteLog(1, "Warning: Starting_pause > 1 is not recommended for 1-4 notes per measure (check config)");
		}
		if (fn == 1 && npm == 8) {
			WriteLog(1, "Warning: Starting_pause 1 is not recommended for 8 notes per measure (check config)");
		}
		if (species == 2 && npm != 2) {
			WriteLog(5, "Warning: Counterpoint species 2 should have notes_per_measure = 2 or just comment out notes_per_measure so that it is controlled by species parameter automatically (check config)");
		}
		if (species == 3 && npm != 4) {
			WriteLog(5, "Warning: Counterpoint species 3 should have notes_per_measure = 4 or just comment out notes_per_measure so that it is controlled by species parameter automatically (check config)");
		}
		if (species == 4 && npm != 2) {
			WriteLog(5, "Warning: Counterpoint species 4 should have notes_per_measure = 2 or just comment out notes_per_measure so that it is controlled by species parameter automatically (check config)");
		}
		if (species == 5 && npm != 8) {
			WriteLog(5, "Warning: Counterpoint species 5 should have notes_per_measure = 8 or just comment out notes_per_measure so that it is controlled by species parameter automatically (check config)");
		}
	}
	if (m_algo_id == 111 || m_algo_id == 112) {
		if (m_testing == 1 && cantus_id2 != 1) {
			WriteLog(1, "Warning: cantus_id parameter should be 1 when testing, or you can get partial results. Reset to 1");
			cantus_id2 = 1;
		}
	}
	if (s_len != 1) {
		WriteLog(5, "Warning: s_len should equal 1. Other values are not tested and usually do not have any advantages");
	}
	if (best_rejected) {
		WriteLog(5, "Warning: Best rejected algorithm is not tested and can have bugs. Please set best_rejected to 0");
	}
	if (corrections > 1) {
		WriteLog(5, "Warning: Corrections > 1 algorithm is not tested and can have bugs. Please set corrections to 0 or 1. Also, it is not optimized for fn_source.");
	}
	if (swa_steps < s_len) {
		WriteLog(5, "Warning: Swa_steps cannot be below s_len. Changed to ");
	}
	// Check configuration parameters
	if (accept_reseed == 1 && random_seed == 0) {
		WriteLog(5, "Warning: accept_reseed=1 while random_seed=0. You will get same results after every reseed (check config)");
	}
	if (method == mScan) {
		//WriteLog(1, "Warning: Window-scan method is currently not working correctly (needs debugging). Please choose another method in config file.");
	}
	if (midifile_export_marks && midifile_export_comments) {
		WriteLog(5, "Warning: You are trying to export both marks and comments to MIDI file: midifile_export_marks and midifile_export_comments both set. They can overlap (check config)");
	}
	if (calculate_correlation || calculate_blocking || calculate_stat || calculate_ssf || best_rejected) {
		WriteLog(1, "Algorithm is running in low performance mode. To increase performance, reset calculate_correlation, calculate_blocking, calculate_stat, calculate_ssf, best_rejected (check config)");
	}
	if (log_pmap && !calculate_correlation && !calculate_blocking && !calculate_stat) {
		WriteLog(1, "Log_pmap will not work correctly if all these flags are reset: calculate_correlation, calculate_blocking, calculate_stat. Enable at least one of them.");
	}
	if (shuffle && random_seed) {
		WriteLog(1, "Shuffling after random_seed will not add randomness (check config)");
	}
}

void CF1D::LoadHarmNotation() {
	CString fname = "configs\\harm\\harm-notation.csv";
	if (!CGLib::fileExists(fname)) {
		CString est;
		est.Format("Cannot find file: %s", fname);
		WriteLog(5, est);
		error = 1;
		return;
	}
	vector <CString> sv;
	ifstream fs;
	int cur_nid = -1;
	fs.open(fname);
	CString st;
	char pch[2550];
	int pos = 0;
	// Load header
	//fs.getline(pch, 2550);
	while (fs.good()) {
		fs.getline(pch, 2550);
		st = pch;
		// Skip comments
		pos = st.Find("#");
		if (pos == 0)	continue;
		st.Trim();
		pos = 0;
		if (st.Find(";") != -1) {
			Tokenize(st, sv, ";");
			if (sv.size() != 8) {
				CString est;
				est.Format("Wrong count of columns (%d) in file %s", sv.size(), fname);
				WriteLog(5, est);
				error = 1;
				return;
			}
			for (int i = 0; i < sv.size(); ++i) sv[i].Trim();
			if (sv[0] == "Major") ++cur_nid;
			if (cur_nid == harm_notation) {
				if (sv[0] == "Major")
					for (int i = 1; i < 8; ++i) HarmName[i - 1] = sv[i];
				if (sv[0] == "Minor natural")
					for (int i = 1; i < 8; ++i) HarmName_m[i - 1] = sv[i];
				if (sv[0] == "Minor altered")
					for (int i = 1; i < 8; ++i) HarmName_ma[i - 1] = sv[i];
			}
		}
	}
	fs.close();
	if (HarmName[6].IsEmpty() || HarmName_m[6].IsEmpty() || HarmName_ma[6].IsEmpty()) {
		CString est;
		est.Format("Error loading harmonic notation");
		WriteLog(5, est);
		error = 1;
	}
}

void CF1D::LoadConfigLine(CString* sN, CString* sV, int idata, float fdata) {
	//SET_READY_PERSIST(DP_Config);
	CheckVar(sN, sV, "cantus_id", &cantus_id2, 0);
	CheckVar(sN, sV, "reduce_between", &reduce_between, 0, 100);
	CheckVar(sN, sV, "confirm_mode", &confirm_mode, 0, 2);
	CheckVar(sN, sV, "ly_pagebreak", &ly_pagebreak, 0, 1);
	CheckVar(sN, sV, "ly_rule_verbose", &ly_rule_verbose, 0, 2);
	CheckVar(sN, sV, "ly_msh", &ly_msh, 0, 1);
	CheckVar(sN, sV, "ly_flag_style", &ly_flag_style, 0, 2);
	CheckVar(sN, sV, "harm_notation", &harm_notation, 0, 5);
	CheckVar(sN, sV, "show_harmony_bass", &show_harmony_bass, 0, 2);
	CheckVar(sN, sV, "log_pmap", &log_pmap, 0, 1);
	CheckVar(sN, sV, "show_correct_hatch", &show_correct_hatch, 0, 1);
	CheckVar(sN, sV, "show_hatch", &show_hatch, 0, 2);
	CheckVar(sN, sV, "show_min_severity", &show_min_severity, 0, 100);
	CheckVar(sN, sV, "cor_ack", &cor_ack, 0, 1);
	CheckVar(sN, sV, "show_ignored_flags", &show_ignored_flags, 0, 1);
	CheckVar(sN, sV, "show_allowed_flags", &show_allowed_flags, 0, 1);
	CheckVar(sN, sV, "emulate_sas", &emulate_sas, 0, 1);
	CheckVar(sN, sV, "max_correct_ms", &max_correct_ms, 0);
	CheckVar(sN, sV, "animate", &animate, 0);
	CheckVar(sN, sV, "prohibit_min_severity", &prohibit_min_severity, 0, 101);
	CheckVar(sN, sV, "animate_delay", &animate_delay, 0);
	CheckVar(sN, sV, "tempo_bell", &tempo_bell, 0, 100);
	CheckVar(sN, sV, "cantus_high", &cantus_high, 0, 2);
	CheckVar(sN, sV, "rpenalty_accepted", &rpenalty_accepted, 0);
	CheckVar(sN, sV, "c_len", &c_len, 1);
	CheckVar(sN, sV, "s_len", &s_len, 1);
	CheckVar(sN, sV, "transpose_cantus", &transpose_cantus, 0, 127);
	LoadNote(sN, sV, "first_note", &first_note);
	LoadNote(sN, sV, "last_note", &last_note);
	CheckVar(sN, sV, "fill_steps_mul", &fill_steps_mul);
	CheckVar(sN, sV, "transpose_back", &transpose_back, 0, 1);
	CheckRange(sN, sV, "tempo", &min_tempo, &max_tempo, 1);
	CheckVar(sN, sV, "random_choose", &random_choose, 0, 1);
	CheckVar(sN, sV, "random_key", &random_key, 0, 1);
	CheckVar(sN, sV, "random_seed", &random_seed, 0, 1);
	CheckVar(sN, sV, "random_range", &random_range, 0, 1);
	CheckVar(sN, sV, "accept_reseed", &accept_reseed, 0, 1);
	CheckVar(sN, sV, "shuffle", &shuffle, 0, 1);
	CheckVar(sN, sV, "first_steps_tonic", &first_steps_tonic, 1);
	CheckVar(sN, sV, "show_severity", &show_severity, 0, 1);
	CheckVar(sN, sV, "calculate_correlation", &calculate_correlation, 0, 1);
	CheckVar(sN, sV, "calculate_stat", &calculate_stat, 0, 1);
	CheckVar(sN, sV, "calculate_ssf", &calculate_ssf, 0, 1);
	CheckVar(sN, sV, "best_rejected", &best_rejected, 0);
	CheckVar(sN, sV, "calculate_blocking", &calculate_blocking, 0, 1);
	CheckVar(sN, sV, "late_require", &late_require, 0, 1);
	// Random SWA
	//CheckVar(sN, sV, "fullscan_max", &fullscan_max);
	CheckVar(sN, sV, "approximations", &approximations, 1);
	CheckVar(sN, sV, "swa_steps", &swa_steps, 1);
	CheckVar(sN, sV, "correct_range", &correct_range, 1);
	CheckVar(sN, sV, "correct_inrange", &correct_inrange, 0);
	CheckVar(sN, sV, "optimize_dpenalty", &optimize_dpenalty, 0, 1);

	// Load harmonic notation
	if (*sN == "harm_notation") {
		LoadHarmNotation();
	}
	// Load HSP
	if (*sN == "hsp_file") {
		++parameter_found;
		LoadHSP("configs\\harm\\" + *sV);
	}
	// Load rules
	if (*sN == "rules_file") {
		++parameter_found;
		LoadRules("configs\\rules\\" + *sV);
		ParseRules();
		// Load default parameters
		cspecies = 0;
		SelectSpeciesRules();
		SetRuleParams();
	}
	// Load method
	if (*sN == "method") {
		++parameter_found;
		if (*sV == "window-scan") method = mScan;
		else if (*sV == "swa") method = mSWA;
		else {
			CString est;
			est.Format("Warning: method name unrecognized: %s", *sV);
			WriteLog(5, est);
			error = 1;
		}
	}
	// Load tonic
	if (*sN == "key") {
		++parameter_found;
		if (sV->Right(1) == "m") {
			*sV = sV->Left(sV->GetLength() - 1);
			minor_cur = 1;
		}
		tonic_cur = GetPC(*sV);
	}
}

void CF1D::LogCantus(CString st3, int x, int size, vector<int> &c) {
	CString st, st2;
	st2.Format("%s %d: ", st3, x);
	for (int i = 0; i < size; ++i) {
		st.Format("%d ", c[i]);
		st2 += st;
	}
	CGLib::AppendLineToFile("log/temp.log", st2 + " \n");
}

CString CF1D::vint2st(int size, vector<int> &c) {
	CString st, st2;
	for (int i = 0; i < size; ++i) {
		st.Format("%d ", c[i]);
		st2 += st;
	}
	return st2;
}

// Step2 must be exclusive
void CF1D::FillCantus(vector<int>& c, int step1, int step2, int value)
{
	for (int i = step1; i < step2; ++i) {
		c[i] = value;
	}
}

// Step2 must be exclusive
void CF1D::FillCantus(vector<int>& c, int step1, int step2, vector<int> &value)
{
	for (int i = step1; i < step2; ++i) {
		c[i] = value[i];
	}
}

// Step2 must be exclusive
void CF1D::FillCantus(vector<int>& c, int step1, int step2, vector<vector<int>> &value)
{
	for (int i = step1; i < step2; ++i) {
		c[i] = value[i][0];
	}
}

void CF1D::FillCantusMap(vector<int>& c, vector<int>& smap, int step1, int step2, vector<int>& value)
{
	// Step2 must be exclusive
	for (int i = step1; i < step2; ++i) {
		c[smap[i]] = value[smap[i]];
	}
}

void CF1D::FillCantusMap(vector<int>& c, vector<int>& smap, int step1, int step2, vector<vector<int>>& value)
{
	// Step2 must be exclusive
	for (int i = step1; i < step2; ++i) {
		c[smap[i]] = value[smap[i]][0];
	}
}

void CF1D::FillCantusMap(vector<int>& c, vector<int>& smap, int step1, int step2, int value)
{
	// Step2 must be exclusive
	for (int i = step1; i < step2; ++i) {
		c[smap[i]] = value;
	}
}

void CF1D::WritePerfLog() {
	if (cycle > 100) {
		CString est;
		long long ms = time() - scan_start_time;
		est.Format("Scan speed: %.1f cycles/ms (%lld cycles during %lld ms)", (double)cycle / ms, cycle, ms);
		WriteLog(2, est);
	}
}

void CF1D::LogPerf() {
	CString st, st2;
	long long ms = time() - gen_start_time;
	if (!ms) ms = 1;
	st.Format("%s\\%s.pl %s CY %s/ms, sent %s/s, c_len %d, swa_steps %d, sent %d, ignored %d, ",
		m_algo_folder, m_config, method == mScan ? "SAS" : "SWA",
		HumanFloat(tcycle / (float)ms), HumanFloat(cantus_sent / (float)ms * 1000),
		c_len, swa_steps, cantus_sent, cantus_ignored);
	st2 += st;
	if (m_algo_id == 111 || m_algo_id == 112) {
		st.Format("analy %d/%d, cor %d, cor_full %d, cor_rp0 %d, ",
			cantus_id + 1, cpoint.size() + cantus.size(), cor_sent,
			cor_full, cor_rp0);
		st2 += st;
		if (cor_sent) {
			st.Format("~swa %s, ~rp %s, ~dp %s, ",
				HumanFloat(swa_sum / cor_sent), HumanFloat(rp_sum / cor_sent),
				HumanFloat(dp_sum / cor_sent));
			st2 += st;
		}
	}
	if (m_testing) {
		st.Format("time_res %ds, ", ANALYZE_RESERVE);
		st2 += st;
	}
	if (cantus_id) {
		st.Format("%s #%d, ", midi_file, cantus_id);
		st2 += st;
	}
	st.Format("time %ss, cor_ack%d, fstat%d, fblock%d, fcor%d, ssf%d ",
		HumanFloat(ms / 1000.0), cor_ack, calculate_stat, calculate_blocking,
		calculate_correlation, calculate_ssf);
	st2 += st;
#ifdef CF_DEBUG
	st2 += "DCF ";
#endif

#if defined(_DEBUG)
	st2 += "Debug ";
#else
	st2 += "Release ";
#endif

#ifdef _WIN64
	st2 += "x64 ";
#else
	st2 += "x86 ";
#endif
	WriteLog(2, st2);
	if (m_testing == 1) AppendLineToFile("autotest\\perf.log", st2 + "\n");
}

void CF1D::OutputFlagDelays() {
	for (int f = 0; f < MAX_RULES; ++f) if (flag_delay[f] > sas_emulator_max_delay[f]) {
		//WriteLog(6, flag_delay_st[f]); // 1
	}
}

// Select rules
void CF1D::SelectSpeciesRules() {
	if (cspecies == cspecies0) return;
	cspecies0 = cspecies;
	// Load rules
	for (int i = 0; i < max_flags; ++i) {
		if (severities[cspecies][i] >= prohibit_min_severity) {
			accept[i] = accepts[cspecies][i];
		}
		else {
			accept[i] = 1;
		}
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

