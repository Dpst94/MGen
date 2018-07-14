// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "../stdafx.h"
#include "CP2D.h"

#ifdef _DEBUG
#define new DEBUG_NEW 
#endif

CP2D::CP2D() {
	ResizeRuleVariantVector(accept);
	ResizeRuleVariantVector(severity);
}

CP2D::~CP2D() {
}

void CP2D::LoadConfigLine(CString* sN, CString* sV, int idata, float fdata) {
	// Load rules
	if (*sN == "rules_file") {
		++parameter_found;
		LoadRules("configs\\" + *sV);
		ParseRules();
		SetRuleParams();
	}
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
			if (ast.size() != 25) {
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
			flag = atoi(ast[9]);
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
			//if (m_testing && flag == -1) flag = 1;
			ruleinfo[rid].viz = atoi(ast[12]);
			ruleinfo[rid].viz_int = atoi(ast[13]);
			ruleinfo[rid].viz_v2 = atoi(ast[14]);
			ruleinfo[rid].viz_text = ast[15];
			ruleinfo[rid].false_positives_global = atoi(ast[18]);
			ruleinfo[rid].false_positives_ignore = atoi(ast[19]);
			ruleinfo[rid].sas_emulator_max_delay = atoi(ast[20]);
			ruleinfo[rid].sas_emulator_move_ignore = atoi(ast[21]);
			if (!ast[22].IsEmpty()) {
				Tokenize(ast[22], ast2, ",");
				for (int x = 0; x < ast2.size(); ++x) {
					int fl = atoi(ast2[x]);
					if (fl >= ruleinfo.size()) ruleinfo.resize(fl + 1);
					if (fl) ruleinfo[fl].sas_emulator_replace.push_back(rid);
				}
			}
			ruleinfo[rid].sas_emulator_unstable = atoi(ast[23]);
			if (!ast[24].IsEmpty()) {
				Tokenize(ast[24], ast2, ",");
				for (int x = 0; x < ast2.size(); ++x) {
					int fl = atoi(ast2[x]);
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
				int sp = atoi(spec.Mid(x, 1));
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
				int v = atoi(voices.Mid(1, 1));
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
				int v = atoi(voices.Mid(1, 1));
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
			else if (voices.Find("B") != -1) nvp[vpBas] = 1;
			else if (voices.Find("N") != -1) nvp[vpNbs] = 1;
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
						for (int sp = 0; sp <= MAX_SPECIES; ++sp) {
							for (int vc = 1; vc <= MAX_VC; ++vc) {
								for (int vp = 0; vp <= MAX_VP; ++vp) {
									SaveRuleVariant(sp, vc, vp, rid, ruleinfo[rid].RuleName, 
										ruleinfo[rid].SubRuleName, ruleinfo[rid].RuleComment, ruleinfo[rid].SubRuleComment);
								}
							}
						}
					}
				}
			}
			if (!ruleinfo2[rid].size()) {
				for (int sp = 0; sp <= MAX_SPECIES; ++sp) {
					for (int vc = 1; vc <= MAX_VC; ++vc) {
						for (int vp = 0; vp <= MAX_VP; ++vp) {
							// Resize
							if (accept[sp][vc][vp].size() <= rid) accept[sp][vc][vp].resize(rid + 1);
							if (severity[sp][vc][vp].size() <= rid) severity[sp][vc][vp].resize(rid + 1);
							accept[sp][vc][vp][rid] = flag;
							severity[sp][vc][vp][rid] = sev;
						}
					}
				}
			}
			else {
				ResizeRuleVariantVector(ruleinfo2[rid]);
				for (int sp = 0; sp <= MAX_SPECIES; ++sp) {
					for (int vc = 1; vc <= MAX_VC; ++vc) {
						for (int vp = 0; vp <= MAX_VP; ++vp) {
							// Resize
							if (accept[sp][vc][vp].size() <= rid) accept[sp][vc][vp].resize(rid + 1);
							if (severity[sp][vc][vp].size() <= rid) severity[sp][vc][vp].resize(rid + 1);
							if (rid_unique[sp][vc][vp].size() <= rid) rid_unique[sp][vc][vp].resize(rid + 1);
							int cur_accept = flag;
							if (!nsp[sp] || !nvc[vc] || !nvp[vp]) {
								if (ruleinfo2[rid][sp][vc][vp].RuleName.IsEmpty()) {
									cur_accept = 0;
									SaveRuleVariant(sp, vc, vp, rid, rule, subrule, ast[10], ast[11]);
								}
							}
							else {
								if (rid_unique[sp][vc][vp][rid]) {
									est.Format("Duplicate rule %d species %d, vc %d, vp %d: '%s (%s)' overwrites '%s (%s)' with species filter %s, voices filter %s",
										rid, sp, vc, vp, rule, subrule, ruleinfo2[rid][sp][vc][vp].RuleName,
										ruleinfo2[rid][sp][vc][vp].SubRuleName, spec, voices);
									WriteLog(5, est);
								}
								else rid_unique[sp][vc][vp][rid] = 1;
								SaveRuleVariant(sp, vc, vp, rid, rule, subrule, ast[10], ast[11]);
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
				if (accept[sp][vc][vp].size() <= max_rule) accept[sp][vc][vp].resize(max_rule + 1);
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

void CP2D::SetRuleParams() {
	long long time_start = CGLib::time();
	SetRuleParam(pco_apart, 248, rsName, 1);
	SetRuleParam(sus_last_measures, 139, rsSubName, 0);
	SetRuleParam(cse_leaps_r, 502, rsSubName, 0);
	// Log
	long long time_stop = CGLib::time();
	CString st;
	st.Format("Set rule parameters in %lld ms", time_stop - time_start);
	WriteLog(0, st);
}
