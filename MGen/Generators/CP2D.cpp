// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "../stdafx.h"
#include "CP2D.h"
#include "../GLibrary/CsvDb.h"

#ifdef _DEBUG
#define new DEBUG_NEW 
#endif

CP2D::CP2D() {
	ResizeRuleVariantVectorNegative(accept);
	ResizeRuleVariantVector(severity);
	// Set rule colors
	sev_color.resize(MAX_SEVERITY);
	for (int i = 0; i < MAX_SEVERITY; ++i) {
		sev_color[i] = MakeColor(0, 255.0 / MAX_SEVERITY * i, 255 - 255.0 / MAX_SEVERITY * i, 0);
	}
	// Harmony notation
	HarmName.resize(7);
	HarmName_m.resize(7);
	HarmName_ma.resize(7);
	// Test
#ifdef _DEBUG
	//TestCC_C2();
#endif
}

CP2D::~CP2D() {
}

void CP2D::LoadVocalRanges(CString fname) {
	CString est;
	// Check file exists
	if (!fileExists(fname)) {
		est.Format("LoadVocalRanges cannot find file: %s", fname);
		WriteLog(5, est);
		error = 1;
		return;
	}
	CCsvDb cdb;
	est = cdb.Open(fname);
	if (est != "") WriteLog(5, est);
	est = cdb.Select();
	if (est != "") WriteLog(5, est);
	for (int i = 0; i < cdb.result.size(); ++i) {
		int vr_id = atoi(cdb.result[i]["id"]);
		vocra_info.resize(max(vocra_info.size(), vr_id + 1));
		vocra_info[vr_id].name = cdb.result[i]["name"];
		vocra_info[vr_id].clef = cdb.result[i]["clef"];
		vocra_info[vr_id].max_cc = atoi(cdb.result[i]["max_cc"]);
		vocra_info[vr_id].min_cc = atoi(cdb.result[i]["min_cc"]);
		vocra_info[vr_id].high_cc = atoi(cdb.result[i]["high_cc"]);
		vocra_info[vr_id].low_cc = atoi(cdb.result[i]["low_cc"]);
	}
}

void CP2D::LoadSpecies(CString st) {
	vsp.resize(st.GetLength());
	for (int i = 0; i < st.GetLength(); ++i) {
		vsp[i] = atoi(st.Mid(i, 1));
	}
}

void CP2D::LoadConfigLine(CString* sN, CString* sV, int idata, float fdata) {
	CheckVar(sN, sV, "lclimax_notes", &lclimax_notes, 0, 1000);
	CheckVar(sN, sV, "lclimax_mea", &lclimax_mea, 0, 1000);
	CheckVar(sN, sV, "show_ignored_flags", &show_ignored_flags, 0, 1);
	CheckVar(sN, sV, "show_allowed_flags", &show_allowed_flags, 0, 1);
	CheckVar(sN, sV, "show_min_severity", &show_min_severity, 0, 100);
	CheckVar(sN, sV, "show_severity", &show_severity, 0, 1);
	CheckVar(sN, sV, "fill_steps_mul", &fill_steps_mul);
	CheckVar(sN, sV, "ly_rule_verbose", &ly_rule_verbose, 0, 2);
	CheckVar(sN, sV, "harm_notation", &harm_notation, 0, 5);
	CheckVar(sN, sV, "show_harmony_bass", &show_harmony_bass, 0, 2);
	CheckVar(sN, sV, "first_steps_tonic", &first_steps_tonic, 1);
	CheckVar(sN, sV, "show_hatch", &show_hatch, 0, 2);
	// Load species
	if (*sN == "species") {
		++parameter_found;
		conf_species = *sV;
		LoadSpecies(*sV);
	}
	// Load rules
	if (*sN == "rules_file") {
		++parameter_found;
		LoadRules("configs\\" + *sV);
		ParseRules();
		SetRuleParams();
	}
	// Load harmonic notation
	if (*sN == "harm_notation") {
		LoadHarmNotation();
	}
	// Load HSP
	if (*sN == "hsp_file") {
		++parameter_found;
		LoadHSP("configs\\harm\\" + *sV);
	}
	// Load vocal ranges
	if (*sN == "vocal_ranges_file") {
		++parameter_found;
		LoadVocalRanges("configs\\" + *sV);
	}
}

// Load harmonic sequence penalties
void CP2D::LoadHSP(CString fname) {
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

// Load rules
void CP2D::LoadRules(CString fname) {
	//SET_READY_PERSIST(DP_Rules);
	long long time_start = CGLib::time();
	CString st, est, rule, subrule;
	vector<CString> ast, ast2;
	vector<vector<vector<vector<int>>>> rid_unique; // [sp][vc][vp][rid]
	vector<int> rdetailed; // [rid]
	ResizeRuleVariantVector(rid_unique);
	int i = 0;
	int sev = 0;
	CString spec, voices;
	int acpt = 0;
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
			if (ast.size() != 26) {
				est.Format("Wrong column count at line %d in rules file %s: '%s'", i, fname, st);
				WriteLog(5, est);
				error = 1;
				return;
			}
			rid = atoi(ast[1]);
			spec = ast[2];
			voices = ast[3];
			sev = atoi(ast[4]);
			rule = ast[7];
			subrule = ast[8];
			acpt = atoi(ast[9]);
			// Find rule id
			if (rid > max_rule) {
				max_rule = rid;
			}
			//est.Format("Found rule %s - %d", rule, rid);
			//WriteLog(0, est);
			if (rid >= ruleinfo.size()) ruleinfo.resize(rid + 1);
			if (rid >= ruleinfo2.size()) ruleinfo2.resize(rid + 1);
			if (rid >= rdetailed.size()) rdetailed.resize(rid + 1);
			ruleinfo[rid].RuleClass = ast[5];
			ruleinfo[rid].RuleGroup = ast[6];
			// If testing, enable all disabled rules so that expected violations can be confirmed
			//if (m_testing && acpt == -1) acpt = 1;
			ruleinfo[rid].viz = atoi(ast[12]);
			ruleinfo[rid].viz_int = atoi(ast[13]);
			ruleinfo[rid].viz_harm = atoi(ast[14]);
			ruleinfo[rid].viz_sep = atoi(ast[15]);
			ruleinfo[rid].viz_text = ast[16];
			ruleinfo[rid].false_positives_global = atoi(ast[19]);
			ruleinfo[rid].false_positives_ignore = atoi(ast[20]);
			ruleinfo[rid].sas_emulator_max_delay = atoi(ast[21]);
			ruleinfo[rid].sas_emulator_move_ignore = atoi(ast[22]);
			if (!ast[23].IsEmpty()) {
				Tokenize(ast[23], ast2, ",");
				for (int x = 0; x < ast2.size(); ++x) {
					fl = atoi(ast2[x]);
					if (fl >= ruleinfo.size()) ruleinfo.resize(fl + 1);
					if (fl) ruleinfo[fl].sas_emulator_replace.push_back(rid);
				}
			}
			ruleinfo[rid].sas_emulator_unstable = atoi(ast[24]);
			if (!ast[25].IsEmpty()) {
				Tokenize(ast[25], ast2, ",");
				for (int x = 0; x < ast2.size(); ++x) {
					fl = atoi(ast2[x]);
					if (fl >= ruleinfo.size()) ruleinfo.resize(fl + 1);
					if (fl) ruleinfo[fl].flag_replace.push_back(rid);
				}
			}
			if (viz_space[ruleinfo[rid].viz] && ruleinfo[rid].viz_text.IsEmpty()) ruleinfo[rid].viz_text = " ";
			// Find what to replace
			vector<int> nsp; // species
			vector<int> nvc; // voice count
			vector<int> nvp; // voice pair
			nsp.resize(MAX_SPECIES + 1);
			nvc.resize(MAX_VC + 1);
			nvp.resize(MAX_VP + 1);
			for (int x = 0; x < spec.GetLength(); ++x) {
				sp = atoi(spec.Mid(x, 1));
				if (sp > MAX_SPECIES) {
					est.Format("Species id (%d) is greater than MAX_SPECIES (%d) for rule id %d. Consider increasing MAX_SPECIES",
						sp, MAX_SPECIES, rid);
					WriteLog(5, est);
					error = 1;
					return;
				}
				nsp[sp] = 1;
			}
			if (voices[0] == '<') {
				v = atoi(voices.Mid(1, 1));
				if (v < 1) {
					est.Format("Voice count (%d) is below 1 for rule id %d. Consider increasing MAX_SPECIES",
						v, rid);
					WriteLog(5, est);
					error = 1;
					return;
				}
				if (v > MAX_VC) {
					est.Format("Voice count (%d) is above MAX_VC (%d) for rule id %d. Consider increasing MAX_SPECIES",
						v, MAX_VC, rid);
					WriteLog(5, est);
					error = 1;
					return;
				}
				for (int i = 1; i < v; ++i) nvc[i] = 1;
			}
			else if (voices[0] == '>') {
				v = atoi(voices.Mid(1, 1));
				if (v >= MAX_VC) {
					est.Format("Voice count (%d) points above MAX_VC (%d) for rule id %d. Consider increasing MAX_SPECIES",
						v, MAX_VC, rid);
					WriteLog(5, est);
					error = 1;
					return;
				}
				for (int i = v + 1; i <= MAX_VC; ++i) nvc[i] = 1;
			}
			else if (isdigit(voices[0])) {
				nvc[atoi(voices.Mid(0, 1))] = 1;
			}
			if (voices.Find("E") != -1) nvp[vpExt] = 1;
			if (voices.Find("B") != -1) nvp[vpBas] = 1;
			if (voices.Find("N") != -1) nvp[vpNbs] = 1;
			// Set if empty
			int found = 0;
			for (int i = 0; i <= MAX_SPECIES; ++i) if (nsp[i]) found = 1;
			if (!found) for (int i = 0; i <= MAX_SPECIES; ++i) nsp[i] = 1;
			found = 0;
			for (int i = 1; i <= MAX_VC; ++i) if (nvc[i]) found = 1;
			if (!found) for (int i = 1; i <= MAX_VC; ++i) nvc[i] = 1;
			found = 0;
			for (int i = 0; i <= MAX_VP; ++i) if (nvp[i]) found = 1;
			if (!found) for (int i = 0; i <= MAX_VP; ++i) nvp[i] = 1;
			// Detect if rule is detailed
			if (spec == "" && voices == "") {
				if (rdetailed[rid] == 1) {
					est.Format("Rule %d tries to combine detailed and non-detailed approaches",
						rid);
					WriteLog(5, est);
					error = 1;
					return;
				}
				if (rdetailed[rid] == -1) {
					est.Format("Rule %d is duplicated",
						rid);
					WriteLog(5, est);
					error = 1;
					return;
				}
				rdetailed[rid] = -1;
			}
			else {
				if (rdetailed[rid] == -1) {
					est.Format("Rule %d tries to combine detailed and non-detailed approaches",
						rid);
					WriteLog(5, est);
					error = 1;
					return;
				}
				rdetailed[rid] = 1;
				if (!ruleinfo[rid].RuleName.IsEmpty() && !ruleinfo2[rid].size()) {
					if (ruleinfo[rid].RuleName != rule || ruleinfo[rid].SubRuleName != subrule ||
						ruleinfo[rid].RuleComment != ast[10] || ruleinfo[rid].SubRuleComment != ast[11]) {
						// Copy info
						ResizeRuleVariantVector(ruleinfo2[rid]);
						for (sp = 0; sp <= MAX_SPECIES; ++sp) {
							for (vc = 1; vc <= MAX_VC; ++vc) {
								for (vp = 0; vp <= MAX_VP; ++vp) {
									SaveRuleVariant(sp, vc, vp, rid, ruleinfo[rid].RuleName, 
										ruleinfo[rid].SubRuleName, ruleinfo[rid].RuleComment, ruleinfo[rid].SubRuleComment);
								}
							}
						}
					}
				}
			}
			if (!ruleinfo2[rid].size()) {
				for (sp = 0; sp <= MAX_SPECIES; ++sp) {
					for (vc = 1; vc <= MAX_VC; ++vc) {
						for (vp = 0; vp <= MAX_VP; ++vp) {
							// Resize
							if (rid >= RULE_ALLOC && accept[sp][vc][vp].size() <= rid) accept[sp][vc][vp].resize(rid + 1, -1);
							if (rid >= RULE_ALLOC && severity[sp][vc][vp].size() <= rid) severity[sp][vc][vp].resize(rid + 1);
							if (!nsp[sp] || !nvc[vc] || !nvp[vp]) {
							}
							else {
								accept[sp][vc][vp][rid] = acpt;
								severity[sp][vc][vp][rid] = sev;
							}
						}
					}
				}
			}
			else {
				for (sp = 0; sp <= MAX_SPECIES; ++sp) {
					for (vc = 1; vc <= MAX_VC; ++vc) {
						for (vp = 0; vp <= MAX_VP; ++vp) {
							// Resize
							if (rid >= RULE_ALLOC && accept[sp][vc][vp].size() <= rid) accept[sp][vc][vp].resize(rid + 1, -1);
							if (rid >= RULE_ALLOC && severity[sp][vc][vp].size() <= rid) severity[sp][vc][vp].resize(rid + 1);
							if (!nsp[sp] || !nvc[vc] || !nvp[vp]) {
								if (ruleinfo2[rid][sp][vc][vp].RuleName.IsEmpty()) {
									SaveRuleVariant(sp, vc, vp, rid, rule, subrule, ast[10], ast[11]);
									//accept[sp][vc][vp][rid] = -1;
									//severity[sp][vc][vp][rid] = sev;
								}
							}
							else {
								if (rid >= RULE_ALLOC && rid_unique[sp][vc][vp].size() <= rid) rid_unique[sp][vc][vp].resize(rid + 1);
								if (rid_unique[sp][vc][vp][rid]) {
									est.Format("Duplicate rule %d species %d, vc %d, vp %d: '%s (%s)' overwrites '%s (%s)' with species filter %s, voices filter %s",
										rid, sp, vc, vp, rule, subrule, ruleinfo2[rid][sp][vc][vp].RuleName,
										ruleinfo2[rid][sp][vc][vp].SubRuleName, spec, voices);
									WriteLog(5, est);
								}
								else rid_unique[sp][vc][vp][rid] = 1;
								SaveRuleVariant(sp, vc, vp, rid, rule, subrule, ast[10], ast[11]);
								accept[sp][vc][vp][rid] = acpt;
								severity[sp][vc][vp][rid] = sev;
							}
						}
					}
				}
			}
			// Set rule aggregated info
			ruleinfo[rid].RuleName = rule;
			ruleinfo[rid].SubRuleName = subrule;
			ruleinfo[rid].RuleComment = ast[10];
			ruleinfo[rid].SubRuleComment = ast[11];
			// Replace scripts in viz text
			ruleinfo[rid].viz_text.Replace("!rn!", ruleinfo[rid].RuleName);
			ruleinfo[rid].viz_text.Replace("!srn!", ruleinfo[rid].SubRuleName);
			ruleinfo[rid].viz_text.Replace("!src!", ruleinfo[rid].SubRuleComment);
			ruleinfo[rid].viz_text.Replace("!rc!", ruleinfo[rid].RuleComment);
			ruleinfo[rid].viz_text.Replace("!rc!", ruleinfo[rid].RuleClass);
			ruleinfo[rid].viz_text.Replace("!rg!", ruleinfo[rid].RuleGroup);
		}
	}
	fs.close();
	long long time_stop = CGLib::time();
	est.Format("LoadRules loaded %d lines in %lld ms from %s", i, time_stop - time_start, fname);
	WriteLog(0, est);
	// Resize
	max_rule = ruleinfo.size() - 1;
	ResizeRuleVariantVectors2();
	// Check that all rules in lists exist
	CheckRuleList();
}

void CP2D::ResizeRuleVariantVector(vector<vector<vector<vector<int>>>> &ve) {
	ve.resize(MAX_SPECIES + 1);
	for (int sp = 0; sp <= MAX_SPECIES; ++sp) {
		ve[sp].resize(MAX_VC + 1);
		for (int vc = 1; vc <= MAX_VC; ++vc) {
			ve[sp][vc].resize(MAX_VP + 1);
			for (int vp = 0; vp <= MAX_VP; ++vp) {
				ve[sp][vc][vp].resize(RULE_ALLOC);
			}
		}
	}
}

void CP2D::ResizeRuleVariantVectorNegative(vector<vector<vector<vector<int>>>> &ve) {
	ve.resize(MAX_SPECIES + 1);
	for (int sp = 0; sp <= MAX_SPECIES; ++sp) {
		ve[sp].resize(MAX_VC + 1);
		for (int vc = 1; vc <= MAX_VC; ++vc) {
			ve[sp][vc].resize(MAX_VP + 1);
			for (int vp = 0; vp <= MAX_VP; ++vp) {
				ve[sp][vc][vp].resize(RULE_ALLOC, -1);
			}
		}
	}
}

void CP2D::ResizeRuleVariantVector(vector<vector<vector<int>>> &ve) {
	ve.resize(MAX_SPECIES + 1);
	for (int sp = 0; sp <= MAX_SPECIES; ++sp) {
		ve[sp].resize(MAX_VC + 1);
		for (int vc = 1; vc <= MAX_VC; ++vc) {
			ve[sp][vc].resize(MAX_VP + 1);
		}
	}
}

void CP2D::ResizeRuleVariantVector(vector<vector<vector<float>>> &ve) {
	ve.resize(MAX_SPECIES + 1);
	for (int sp = 0; sp <= MAX_SPECIES; ++sp) {
		ve[sp].resize(MAX_VC + 1);
		for (int vc = 1; vc <= MAX_VC; ++vc) {
			ve[sp][vc].resize(MAX_VP + 1);
		}
	}
}

void CP2D::ResizeRuleVariantVector(vector<vector<vector<RuleInfo2>>> &ve) {
	ve.resize(MAX_SPECIES + 1);
	for (int sp = 0; sp <= MAX_SPECIES; ++sp) {
		ve[sp].resize(MAX_VC + 1);
		for (int vc = 1; vc <= MAX_VC; ++vc) {
			ve[sp][vc].resize(MAX_VP + 1);
		}
	}
}

void CP2D::ResizeRuleVariantVectors2() {
	for (int sp = 0; sp <= MAX_SPECIES; ++sp) {
		for (int vc = 1; vc <= MAX_VC; ++vc) {
			for (int vp = 0; vp <= MAX_VP; ++vp) {
				if (accept[sp][vc][vp].size() <= max_rule) accept[sp][vc][vp].resize(max_rule + 1, -1);
				if (severity[sp][vc][vp].size() <= max_rule) severity[sp][vc][vp].resize(max_rule + 1);
			}
		}
	}
}

void CP2D::SaveRuleVariant(int sp, int vc, int vp, int rid, CString rule, CString subrule, CString rule_com, CString subrule_com) {
	// Set values
	ruleinfo2[rid][sp][vc][vp].RuleName = rule;
	ruleinfo2[rid][sp][vc][vp].SubRuleName = subrule;
	ruleinfo2[rid][sp][vc][vp].RuleComment = rule_com;
	ruleinfo2[rid][sp][vc][vp].SubRuleComment = subrule_com;
}

void CP2D::CheckRuleList() {
	CString est;
	for (int rid = 0; rid <= max_rule; ++rid) {
		if (ruleinfo[rid].sas_emulator_replace.size() && ruleinfo[rid].RuleGroup.IsEmpty()) {
			est.Format("Detected undefined rule %d in sas_emulator_replace list",
				rid);
			WriteLog(5, est);
		}
		if (ruleinfo[rid].flag_replace.size() && ruleinfo[rid].RuleGroup.IsEmpty()) {
			est.Format("Detected undefined rule %d in flag_replace list",
				rid);
			WriteLog(5, est);
		}
	}
}

// Return chromatic length of an interval (e.g. return 4 from 3rd)
int CP2D::Interval2Chromatic(int iv) {
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

CString CP2D::GetRuleName(int rid, int sp, int vc, int vp) {
	if (ruleinfo2[rid].size()) {
		return ruleinfo2[rid][sp][vc][vp].RuleName;
	}
	else {
		return ruleinfo[rid].RuleName;
	}
}

CString CP2D::GetSubRuleName(int rid, int sp, int vc, int vp) {
	if (ruleinfo2[rid].size()) {
		return ruleinfo2[rid][sp][vc][vp].SubRuleName;
	}
	else {
		return ruleinfo[rid].SubRuleName;
	}
}

CString CP2D::GetRuleComment(int rid, int sp, int vc, int vp) {
	if (ruleinfo2[rid].size()) {
		return ruleinfo2[rid][sp][vc][vp].RuleComment;
	}
	else {
		return ruleinfo[rid].RuleComment;
	}
}

CString CP2D::GetSubRuleComment(int rid, int sp, int vc, int vp) {
	if (ruleinfo2[rid].size()) {
		return ruleinfo2[rid][sp][vc][vp].SubRuleComment;
	}
	else {
		return ruleinfo[rid].SubRuleComment;
	}
}

// Load rules
void CP2D::ParseRule(int rid, int type) {
	CString st;
	if (type == rsName) st = ruleinfo[rid].RuleName;
	if (type == rsSubName) st = ruleinfo[rid].SubRuleName;
	if (type == rsComment) st = ruleinfo[rid].RuleComment;
	if (type == rsSubComment) st = ruleinfo[rid].SubRuleComment;
	vector<int> v;
	GetVint(st, v);
	if (v.size()) {
		// Create types
		if (!ruleinfo[rid].RuleParam.size()) ruleinfo[rid].RuleParam.resize(4);
		// Set params for type
		ruleinfo[rid].RuleParam[type] = v;
	}
}

void CP2D::ParseRule2(int sp, int vc, int vp, int rid, int type) {
	CString st;
	if (type == rsName) st = ruleinfo2[rid][sp][vc][vp].RuleName;
	if (type == rsSubName) st = ruleinfo2[rid][sp][vc][vp].SubRuleName;
	if (type == rsComment) st = ruleinfo2[rid][sp][vc][vp].RuleComment;
	if (type == rsSubComment) st = ruleinfo2[rid][sp][vc][vp].SubRuleComment;
	vector<int> v;
	GetVint(st, v);
	if (v.size()) {
		// Create types
		if (!ruleinfo2[rid][sp][vc][vp].RuleParam.size()) ruleinfo2[rid][sp][vc][vp].RuleParam.resize(4);
		// Set params for type
		ruleinfo2[rid][sp][vc][vp].RuleParam[type] = v;
	}
}

int CP2D::GetRuleParam(int sp, int vc, int vp, int rid, int type, int id) {
	// If rule is not detailed
	if (!ruleinfo2[rid].size()) {
		// Rule is not detailed and parameter was not parsed
		if (!ruleinfo[rid].RuleParam.size() || id >= ruleinfo[rid].RuleParam[type].size()) {
			CString est, rs;
			CString st;
			if (type == rsName) st = ruleinfo[rid].RuleName;
			else if (type == rsSubName) st = ruleinfo[rid].SubRuleName;
			else if (type == rsComment) st = ruleinfo[rid].RuleComment;
			else if (type == rsSubComment) st = ruleinfo[rid].SubRuleComment;
			if (type == rsName) rs = "rule name";
			else if (type == rsSubName) rs = "subrule name";
			else if (type == rsComment) rs = "rule comment";
			else if (type == rsSubComment) rs = "subrule comment";
			est.Format("Error parsing integer #%d from %s %d: '%s' (species %d)", id + 1, rs, rid, st, sp);
			WriteLog(5, est);
			error = 1;
			return 0;
		}
		return ruleinfo[rid].RuleParam[type][id];
	}
	// If rule is detailed
	else {
		// Rule is detailed, but parameter was not parsed
		if (!ruleinfo2[rid][sp][vc][vp].RuleParam.size() || id >= ruleinfo2[rid][sp][vc][vp].RuleParam[type].size()) {
			CString est, rs;
			CString st;
			if (type == rsName) st = ruleinfo2[rid][sp][vc][vp].RuleName;
			else if (type == rsSubName) st = ruleinfo2[rid][sp][vc][vp].SubRuleName;
			else if (type == rsComment) st = ruleinfo2[rid][sp][vc][vp].RuleComment;
			else if (type == rsSubComment) st = ruleinfo2[rid][sp][vc][vp].SubRuleComment;
			if (type == rsName) rs = "rule name";
			else if (type == rsSubName) rs = "subrule name";
			else if (type == rsComment) rs = "rule comment";
			else if (type == rsSubComment) rs = "subrule comment";
			est.Format("Error parsing detailed integer #%d from %s %d: '%s' (species %d)", id + 1, rs, rid, st, sp);
			WriteLog(5, est);
			error = 1;
			return 0;
		}
		return ruleinfo2[rid][sp][vc][vp].RuleParam[type][id];
	}
}

float CP2D::GetRuleParamF(int sp, int vc, int vp, int rid, int type, int id) {
	int i1 = GetRuleParam(sp, vc, vp, rid, type, id);
	int i2 = GetRuleParam(sp, vc, vp, rid, type, id + 1);
	return  i1 + i2 * 1.0 / pow(10, NumDigits(i2));
}

// Parse rules
void CP2D::ParseRules() {
	long long time_start = CGLib::time();
	for (int rid = 0; rid <= max_rule; ++rid) {
		// If rule is not detailed
		if (!ruleinfo2[rid].size()) {
			for (int rs = 0; rs < 4; ++rs) {
				ParseRule(rid, rs);
			}
		}
		// If rule is detailed
		else {
			for (int sp = 0; sp <= MAX_SPECIES; ++sp) {
				for (int vc = 1; vc <= MAX_VC; ++vc) {
					for (int vp = 0; vp <= MAX_VP; ++vp) {
						for (int rs = 0; rs < 4; ++rs) {
							ParseRule2(sp, vc, vp, rid, rs);
						}
					}
				}
			}
		}
	}
	long long time_stop = CGLib::time();
	CString st;
	st.Format("Parsed rules in %lld ms", time_stop - time_start);
	WriteLog(0, st);
}

void CP2D::SetRuleParam(vector<vector<vector<int>>> &par, int rid, int type, int id) {
	ResizeRuleVariantVector(par);
	for (int sp = 0; sp <= MAX_SPECIES; ++sp) {
		for (int vc = 1; vc <= MAX_VC; ++vc) {
			for (int vp = 0; vp <= MAX_VP; ++vp) {
				par[sp][vc][vp] = GetRuleParam(sp, vc, vp, rid, type, id);
			}
		}
	}
}

void CP2D::SetRuleParam(vector<vector<vector<float>>> &par, int rid, int type, int id) {
	ResizeRuleVariantVector(par);
	for (int sp = 0; sp <= MAX_SPECIES; ++sp) {
		for (int vc = 1; vc <= MAX_VC; ++vc) {
			for (int vp = 0; vp <= MAX_VP; ++vp) {
				par[sp][vc][vp] = GetRuleParamF(sp, vc, vp, rid, type, id);
			}
		}
	}
}

void CP2D::SetRuleParams() {
	long long time_start = CGLib::time();
	SetRuleParam(pco_apart, 248, rsName, 1);
	SetRuleParam(sus_last_measures, 139, rsSubName, 0);
	SetRuleParam(cse_leaps_r, 502, rsSubName, 0);
	SetRuleParam(lclimax_mea5, 325, rsComment, 0);
	SetRuleParam(gis_trail_max, 200, rsSubName, 0);
	SetRuleParam(fis_gis_max, 199, rsSubName, 0);
	SetRuleParam(fis_g_max, 349, rsSubName, 0);
	SetRuleParam(fis_g_max2, 350, rsSubName, 0);
	SetRuleParam(dev_late2, 191, rsSubName, 0);
	SetRuleParam(dev_late3, 192, rsSubName, 0);
	SetRuleParam(dev2_maxlen, 386, rsSubComment, 0);
	SetRuleParam(c4p_last_meas, 144, rsName, 1);
	SetRuleParam(c4p_last_notes, 144, rsName, 2);
	SetRuleParam(fill_pre3_notes, 100, rsComment, 0);
	SetRuleParam(fill_pre4_int, 144, rsComment, 0);
	SetRuleParam(fill_pre4_notes, 144, rsComment, 1);
	SetRuleParam(pre_last_leaps, 204, rsName, 0);
	SetRuleParam(max_leap_steps, 493, rsName, 0);
	SetRuleParam(max_leaps, 493, rsSubName, 0);
	SetRuleParam(max_leaped, 494, rsSubName, 0);
	SetRuleParam(max_leap_steps2, 497, rsName, 0);
	SetRuleParam(max_leaps_r, 495, rsSubName, 0);
	SetRuleParam(max_leaped_r, 496, rsSubName, 0);
	SetRuleParam(max_leaps2, 497, rsSubName, 0);
	SetRuleParam(max_leaped2, 498, rsSubName, 0);
	SetRuleParam(max_leaps2_r, 499, rsSubName, 0);
	SetRuleParam(max_leaped2_r, 500, rsSubName, 0);
	SetRuleParam(thirds_ignored, 501, rsComment, 0);
	SetRuleParam(early_culm, 78, rsSubName, 0);
	SetRuleParam(early_culm2, 79, rsSubName, 0);
	SetRuleParam(early_culm3, 193, rsSubName, 0);
	SetRuleParam(late_culm, 21, rsSubName, 0);
	SetRuleParam(miss_slurs_window, 188, rsName, 0);
	SetRuleParam(slurs_window, 93, rsName, 0);
	//SetRuleParam(max_note_len, 336, rsSubName, 1);
	SetRuleParam(stag_notes, 10, rsSubName, 0);
	SetRuleParam(stag_note_steps, 10, rsSubName, 1);
	SetRuleParam(stag_notes2, 39, rsSubName, 0);
	SetRuleParam(stag_note_steps2, 39, rsSubName, 1);
	SetRuleParam(notes_arange, 15, rsSubName, 0);
	SetRuleParam(notes_arange2, 16, rsSubName, 0);
	SetRuleParam(min_arange, 15, rsSubName, 1);
	SetRuleParam(min_arange2, 16, rsSubName, 1);
	SetRuleParam(repeat_letters_t, 17, rsSubName, 0);
	SetRuleParam(repeat_letters_d, 428, rsSubName, 0);
	SetRuleParam(repeat_letters_s, 429, rsSubName, 0);
	SetRuleParam(miss_letters_t, 20, rsSubName, 0);
	SetRuleParam(miss_letters_d, 430, rsSubName, 0);
	SetRuleParam(miss_letters_s, 431, rsSubName, 0);
	SetRuleParam(tonic_max_cp, 310, rsSubName, 0);
	SetRuleParam(tonic_window_cp, 310, rsSubName, 1);
	SetRuleParam(tonic_wei_inv, 310, rsSubComment, 0);
	SetRuleParam(cross_max_len, 518, rsSubName, 0);
	SetRuleParam(cross_max_len2, 519, rsSubName, 0);
	notes_lrange.resize(4);
	for (int rt = 0; rt < 4; ++rt) {
		SetRuleParam(notes_lrange[rt], 434 + rt, rsSubName, 0);
	}
	tonic_max.resize(2);
	tonic_window.resize(2);
	for (int tt = 0; tt < 2; ++tt) {
		SetRuleParam(tonic_max[tt], 70 + tt, rsSubName, 0);
		SetRuleParam(tonic_window[tt], 70 + tt, rsSubName, 1);
	}
	SetRuleParam(tritone_res_quart, 2, rsSubComment, 0);
	SetRuleParam(max_smooth, 302, rsSubName, 0);
	SetRuleParam(max_smooth_direct, 303, rsSubName, 0);
	SetRuleParam(cse_leaps, 501, rsSubName, 0);
	SetRuleParam(cse_leaps_r, 502, rsSubName, 0);
	SetRuleParam(notes_picount, 344, rsSubName, 0);
	SetRuleParam(min_picount, 344, rsSubName, 1);
	SetRuleParam(notes_picount2, 345, rsSubName, 0);
	SetRuleParam(min_picount2, 345, rsSubName, 1);
	SetRuleParam(notes_picount3, 346, rsSubName, 0);
	SetRuleParam(min_picount3, 346, rsSubName, 1);
	SetRuleParam(mea_per_sus, 341, rsSubName, 0);
	SetRuleParam(vocra_disbal_yel, 524, rsSubName, 0);
	SetRuleParam(vocra_disbal_red, 526, rsSubName, 0);
	// Log
	long long time_stop = CGLib::time();
	CString st;
	st.Format("Set rule parameters in %lld ms", time_stop - time_start);
	WriteLog(0, st);
}

// Fill pause from start step to (start+length) step inclusive
void CP2D::FillPause(int start, int length, int v) {
	if (start + length >= t_allocated) ResizeVectors(max(start + length + 1, t_allocated * 2));
	for (int x = start; x <= start + length; ++x) {
		pause[x][v] = 1;
		note[x][v] = 0;
		len[x][v] = 1;
		coff[x][v] = 0;
		vel[x][v] = 0;
		if (tonic.size()) {
			tonic[x][v] = 0;
			minor[x][v] = 0;
			comment[x][v].clear();
			comment2[x][v].Empty();
		}
		midifile_out_mul[x] = midifile_out_mul0 * midifile_out_mul2;
	}
}

void CP2D::GetFlag(int f) {
	fl = flag[v][s][f];
	v2 = fvl[v][s][f];
	GetSpVcVp();
}

void CP2D::GetSpVcVp() {
	sp = vsp[v];
	vc = vca[s];
	GetVp();
	if (v2 == v) {
		if (v == hva[s]) vp = vpExt;
		else if (v == lva[s]) vp = vpBas;
		else vp = vpNbs;
	}
	else {
		if (v == lva[s] && v2 == hva[s]) vp = vpExt;
		else if (v == lva[s] && v2 != hva[s]) vp = vpBas;
		else vp = vpNbs;
	}
}

void CP2D::GetVp() {
	if (v2 == v) {
		if (v == hva[s]) vp = vpExt;
		else if (v == lva[s]) vp = vpBas;
		else vp = vpNbs;
	}
	else {
		if (v == lva[s] && v2 == hva[s]) vp = vpExt;
		else if (v == lva[s] && v2 != hva[s]) vp = vpBas;
		else vp = vpNbs;
	}
}

void CP2D::LoadHarmNotation() {
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
		if (pos == 0) {
			pos = st.Find(" - ");
			st = st.Mid(pos + 3);
			st.Trim();
			harm_notation_name.resize(cur_nid + 2);
			harm_notation_name[cur_nid + 1] = st;
			continue;
		}
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

CString CP2D::GetHarmName(int pitch, int alter) {
	if (mode == 9) {
		if (alter == 1) return HarmName_ma[pitch];
		else return HarmName_m[pitch];
	}
	else return HarmName[pitch];
}

CString CP2D::GetPrintKey(int bn, int mode, int mminor) {
	CString st, key, key_visual;
	// Key
	if (mode == 9) {
		key = LyMinorKey[bn];
	}
	else {
		key = LyMajorKey[bn];
	}
	key_visual = key[0];
	key_visual.MakeUpper();
	if (key[1] == 'f') key_visual += "b";
	if (key[1] == 's') key_visual += "#";
	st = key_visual;
	st += " " + mode_name[mode];
	if (mminor > -1) {
		if (mminor) st += " (melodic)";
		else st += " (natural)";
	}
	return st;
}

void CP2D::TestCC_C2() {
	CString fname;
	CString st;
	ofstream outfile;

	fname = "log\\testcc_c.csv";
	DeleteFile(fname);
	outfile.open(fname, ios_base::app);
	outfile << "Mode;Bn;Cno;Dno;Cno2;Match;dno_Gradual;cno_Gradual;\n";
	// Test chromatic notes
	for (mode = 0; mode < 12; ++mode) {
		for (bn = 0; bn < 12; ++bn) {
			BuildPitchConvert();
			vector<int> dno;
			dno.resize(128);
			vector<int> cno2;
			cno2.resize(128);
			for (int cno = 0; cno < 128; ++cno) {
				dno[cno] = cc_c[cno];
				cno2[cno] = c_cc[dno[cno]];
				int test_cno_match = 0;
				if (cno == cno2[cno]) test_cno_match = 1;
				int test_dno_gradual = 0;
				if (!cno || (dno[cno] - dno[cno - 1] < 2 && dno[cno] - dno[cno - 1] >= 0)) test_dno_gradual = 1;
				int test_cno_gradual = 0;
				if (!cno || (cno2[cno] - cno2[cno - 1] < 3 && dno[cno] - dno[cno - 1] >= 0)) test_cno_gradual = 1;
				st.Format("%d;%d;%d;%d;%d;%d;%d;%d\n",
					mode, bn, cno, dno[cno], cno2[cno], test_cno_match, test_dno_gradual, test_cno_gradual);
				outfile << st;
			}
		}
	}
	outfile.close();

	fname = "log\\testc_cc.csv";
	DeleteFile(fname);
	outfile.open(fname, ios_base::app);
	outfile << "Mname;Mode;Bn;Dno;Dno % 7;Cno;Dno2;Match;dno_Gradual;cno_Gradual;\n";
	// Test diatonic notes
	for (mode = 0; mode < 12; ++mode) {
		for (bn = 0; bn < 12; ++bn) {
			BuildPitchConvert();
			vector<int> cno;
			cno.resize(90);
			vector<int> dno2;
			dno2.resize(90);
			int first = 1;
			for (int dno = 0; dno < 90; ++dno) {
				cno[dno] = c_cc[dno];
				if (cno[dno] < 0 || cno[dno] > 127) continue;
				dno2[dno] = cc_c[cno[dno]];
				int test_dno_match = 0;
				if (dno == dno2[dno]) test_dno_match = 1;
				int test_dno_gradual = 0;
				if (first || (dno2[dno] - dno2[dno - 1] == 1)) test_dno_gradual = 1;
				int test_cno_gradual = 0;
				if (first || (cno[dno] - cno[dno - 1] < 3 && cno[dno] - cno[dno - 1] > 0)) test_cno_gradual = 1;
				st.Format("%s;%d;%d;%d;%d;%d;%d;%d;%d;%d\n",
					GetPrintKey(bn, mode), mode, bn, dno, dno % 7,
					cno[dno], dno2[dno], test_dno_match, test_dno_gradual, test_cno_gradual);
				outfile << st;
				first = 0;
			}
		}
	}
	outfile.close();
}

void CP2D::BuildPitchConvert() {
	//bn = 5;
	//maj_bn = 1;
	//mode = 4;
	//bn = 10;
	//maj_bn = 11;
	//mode = 11;
	cc_c.clear();
	c_cc.clear();
	cc_c.resize(128);
	c_cc.resize(90);
	for (int oct = -2; oct < 13; ++oct) {
		for (int dn = 0; dn < 7; ++dn) {
			int dng = oct * 7 + dn - chrom_to_dia[mode] + 14;
			int cng = oct * 12 + dia_to_chrom[dn] + maj_bn;
			if (cng >= 0 && cng < 128) cc_c[cng] = dng;
			if (dng >= 0 && dng < 90) c_cc[dng] = cng;
		}
	}
	if (!cc_c[0]) cc_c[0] = cc_c[1] - 1;
	for (int cn = 1; cn < 128; ++cn) {
		if (!cc_c[cn]) cc_c[cn] = cc_c[cn - 1];
	}
}

void CP2D::LogVector(CString print_st, int print_int, int x1, int x2, vector<int> &c, CString fname) {
	CString st, st2;
	st2.Format("%s %d: ", print_st, print_int);
	for (int i = x1; i < x2; ++i) {
		st.Format("%d ", c[i]);
		st2 += st;
	}
	CGLib::AppendLineToFile(fname, st2 + " \n");
}

