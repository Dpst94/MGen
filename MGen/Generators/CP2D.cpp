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
	ResizeRuleVariantVector(ruleinfo2);
}

CP2D::~CP2D() {
}

void CP2D::LoadConfigLine(CString* sN, CString* sV, int idata, float fdata) {
	// Load rules
	if (*sN == "rules_file") {
		++parameter_found;
		LoadRules("configs\\" + *sV);
		ParseRules();
	}
}

// Load rules
void CP2D::LoadRules(CString fname) {
	//SET_READY_PERSIST(DP_Rules);
	long long time_start = CGLib::time();
	CString st, est, rule, subrule;
	vector<CString> ast, ast2;
	vector<vector<vector<vector<int>>>> rid_unique; // [sp][vc][vp][rid]
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
			if (voices.Find("E") != -1) nvp[0] = 1;
			else if (voices.Find("B") != -1) nvp[1] = 1;
			else if (voices.Find("M") != -1) nvp[2] = 1;
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
			// Apply all if one is empty
			for (int sp = 0; sp <= MAX_SPECIES; ++sp) {
				for (int vc = 1; vc <= MAX_VC; ++vc) {
					for (int vp = 0; vp <= MAX_VP; ++vp) {
						// Resize
						if (accept[sp][vc][vp].size() <= rid) accept[sp][vc][vp].resize(rid + 1);
						if (severity[sp][vc][vp].size() <= rid) severity[sp][vc][vp].resize(rid + 1);
						if (ruleinfo2[sp][vc][vp].size() <= rid) ruleinfo2[sp][vc][vp].resize(rid + 1);
						if (rid_unique[sp][vc][vp].size() <= rid) rid_unique[sp][vc][vp].resize(rid + 1);
						int cur_accept = flag;
						if (!nsp[sp] || !nvc[vc] || !nvp[vp]) {
							if (ruleinfo2[sp][vc][vp][rid].RuleName.IsEmpty()) {
								cur_accept = 0;
								SaveRuleVariant(sp, vc, vp, rid, cur_accept, sev, rule, subrule, ast[10], ast[11]);
							}
						}
						else {
							if (rid_unique[sp][vc][vp][rid]) {
								est.Format("Duplicate rule %d species %d, vc %d, vp %d: '%s (%s)' overwrites '%s (%s)' with species filter %s, voices filter %s",
									rid, sp, vc, vp, rule, subrule, ruleinfo2[sp][vc][vp][rid].RuleName,
									ruleinfo2[sp][vc][vp][rid].SubRuleName, spec, voices);
								WriteLog(5, est);
							}
							else rid_unique[sp][vc][vp][rid] = 1;
							SaveRuleVariant(sp, vc, vp, rid, cur_accept, sev, rule, subrule, ast[10], ast[11]);
						}
					}
				}
			}
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

void CP2D::ResizeRuleVariantVector(vector<vector<vector<map<int, int>>>> &ve) {
	ve.resize(MAX_SPECIES + 1);
	for (int sp = 0; sp <= MAX_SPECIES; ++sp) {
		ve[sp].resize(MAX_VC + 1);
		for (int vc = 1; vc <= MAX_VC; ++vc) {
			ve[sp][vc].resize(MAX_VP + 1);
		}
	}
}

void CP2D::ResizeRuleVariantVector(vector<vector<vector<vector<RuleInfo2>>>> &ve) {
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
				if (ruleinfo2[sp][vc][vp].size() <= max_rule) ruleinfo2[sp][vc][vp].resize(max_rule + 1);
			}
		}
	}
}

void CP2D::SaveRuleVariant(int sp, int vc, int vp, int rid, int flag, int sev, CString rule, CString subrule, CString rule_com, CString subrule_com) {
	// Set values
	ruleinfo2[sp][vc][vp][rid].RuleName = rule;
	ruleinfo2[sp][vc][vp][rid].SubRuleName = subrule;
	ruleinfo2[sp][vc][vp][rid].RuleComment = rule_com;
	ruleinfo2[sp][vc][vp][rid].SubRuleComment = subrule_com;
	accept[sp][vc][vp][rid] = flag;
	severity[sp][vc][vp][rid] = sev;
	// Replace viz text
	ruleinfo[rid].viz_text.Replace("!rn!", ruleinfo2[sp][vc][vp][rid].RuleName);
	ruleinfo[rid].viz_text.Replace("!srn!", ruleinfo2[sp][vc][vp][rid].SubRuleName);
	ruleinfo[rid].viz_text.Replace("!src!", ruleinfo2[sp][vc][vp][rid].SubRuleComment);
	ruleinfo[rid].viz_text.Replace("!rc!", ruleinfo2[sp][vc][vp][rid].RuleComment);
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

// Load rules
void CP2D::ParseRule(int sp, int vc, int vp, int rid, int type) {
	CString st;
	if (type == rsName) st = ruleinfo2[sp][vc][vp][rid].RuleName;
	if (type == rsSubName) st = ruleinfo2[sp][vc][vp][rid].SubRuleName;
	if (type == rsComment) st = ruleinfo2[sp][vc][vp][rid].RuleComment;
	if (type == rsSubComment) st = ruleinfo2[sp][vc][vp][rid].SubRuleComment;
	vector<int> v;
	GetVint(st, v);
	if (v.size()) {
		// Create types
		if (!ruleinfo2[sp][vc][vp][rid].RuleParam.size()) ruleinfo2[sp][vc][vp][rid].RuleParam.resize(4);
		// Set params for type
		ruleinfo2[sp][vc][vp][rid].RuleParam[type] = v;
	}
}

int CP2D::GetRuleParam(int sp, int vc, int vp, int rid, int type, int id) {
	if (!ruleinfo2[sp][vc][vp][rid].RuleParam.size() || id >= ruleinfo2[sp][vc][vp][rid].RuleParam[type].size()) {
		CString est, rs;
		CString st;
		if (type == rsName) st = ruleinfo2[sp][vc][vp][rid].RuleName;
		if (type == rsSubName) st = ruleinfo2[sp][vc][vp][rid].SubRuleName;
		if (type == rsComment) st = ruleinfo2[sp][vc][vp][rid].RuleComment;
		if (type == rsSubComment) st = ruleinfo2[sp][vc][vp][rid].SubRuleComment;
		if (type == rsName) rs = "rule name";
		if (type == rsSubName) rs = "subrule name";
		if (type == rsComment) rs = "rule comment";
		if (type == rsSubComment) rs = "subrule comment";
		est.Format("Error parsing integer #%d from %s %d: '%s' (species %d)", id + 1, rs, rid, st, sp);
		WriteLog(5, est);
		error = 1;
		return 0;
	}
	return ruleinfo2[sp][vc][vp][rid].RuleParam[type][id];
}

// Parse rules
void CP2D::ParseRules() {
	long long time_start = CGLib::time();
	for (int sp = 0; sp <= MAX_SPECIES; ++sp) {
		for (int vc = 1; vc <= MAX_VC; ++vc) {
			for (int vp = 0; vp <= MAX_VP; ++vp) {
				for (int rid = 0; rid <= max_rule; ++rid) {
					for (int rs = 0; rs < 4; ++rs) {
						ParseRule(sp, vc, vp, rid, rs);
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
