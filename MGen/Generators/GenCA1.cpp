// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "../stdafx.h"
#include "GenCA1.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CGenCA1::CGenCA1()
{
	v_cnt = 2;
	track_id[0] = 2;
	track_id[1] = 1;
}

CGenCA1::~CGenCA1()
{
}

void CGenCA1::LoadConfigLine(CString* sN, CString* sV, int idata, float fdata)
{
	LoadVar(sN, sV, "midi_file", &midi_file);
	CheckVar(sN, sV, "corrections", &corrections, 0);
	CheckVar(sN, sV, "pre_bad", &pre_bad, 0);
	CheckVar(sN, sV, "post_bad", &post_bad, 0);
	CheckVar(sN, sV, "step_penalty", &step_penalty, 0);
	CheckVar(sN, sV, "pitch_penalty", &pitch_penalty, 0);

	CGenCF1::LoadConfigLine(sN, sV, idata, fdata);
}

void CGenCA1::GetCPKey()
{
	CString kst1, kst2;
	int t1, t2;
	// Major
	int c1 = GetCPKey2(t1, kst1, 0);
	// Minor
	int c2 = GetCPKey2(t2, kst2, 1);
	key_eval.Format("major confidence %d [%s], minor confidence %d [%s]", c1, kst1, c2, kst2);
	if (c1 == 0 && c2 == 0) {
		tonic_cur = -1;
		WriteLog(5, "Key cannot be detected: " + key_eval);
		return;
	}
	// Cope with same confidence
	while (c1 == c2) {
		c1 = randbw(0, 100);
		c2 = randbw(0, 100);
	}
	// Select best confidence
	if (c1 > c2) {
		tonic_cur = t1;
		minor_cur = 0;
	}
	else {
		tonic_cur = t2;
		minor_cur = 1;
	}
}

void CGenCA1::GetCPKey3(vector <int> &key_miss, int &min_miss, int &min_key, int minor_cur2, int diatonic_repeat_check)
{
	key_miss.clear();
	key_miss.resize(12);
	// Cycle all keys and count miss
	for (int i = 0; i < 12; i++) {
		key_miss[i] = 0;
		// Cycle all notes
		for (int v = 0; v < av_cnt; v++) {
			for (int x = 0; x < c_len; x++) {
				if (minor_cur2) {
					// Check all possible pitches for minor
					if (!m_diatonic_full[(acc[v][x] - i) % 12]) key_miss[i]++;
				}
				else {
					// Check normal pitches for major
					if (!diatonic[(acc[v][x] - i) % 12]) key_miss[i]++;
				}
				// Check if diatonic repeats
				//if (diatonic_repeat_check) {
					//if (x && CC_C(acc[v][x], i, minor_cur2) == CC_C(acc[v][x - 1], i, minor_cur2) && abs(acc[v][x] - acc[v][x - 1]) == 1) key_miss[i]++;
				//}
			}
		}
	}
	// Find minimum miss
	min_key = 0;
	min_miss = c_len;
	for (int i = 0; i < 12; i++) {
		if (key_miss[i] < min_miss) {
			min_miss = key_miss[i];
			min_key = i;
		}
	}
}

int CGenCA1::GetCPKey2(int &tonic_cur2, CString &ext_st, int minor_cur2)
{
	c_len = acc[0].size();
	vector<int> key_miss;
	int min_key = 0;
	int min_miss = c_len;
	CString cst, kst, st2;
	// Create melody string for log
	for (int x = 0; x < min(c_len, 30); x++) {
		st2.Format("%d", acc[0][x] / 12);
		cst += NoteName[acc[0][x] % 12] + st2 + " ";
	}
	GetCPKey3(key_miss, min_miss, min_key, minor_cur2, 1);
	// If no key selected run again without checking for repeating diatonic steps
	//if (min_miss > 0) {
		//GetCPKey3(key_miss, min_miss, min_key, minor_cur2, 0);
	//}
	// If no key selected
	if (min_miss > 0) {
		ext_st.Format("Cannot detect key due to chromatic alterations");
		tonic_cur2 = -1;
		return 0;
	}
	// Count best keys
	vector <int> keys;
	int key_count = 0;
	for (int i = 0; i < 12; i++) {
		if (key_miss[i] == min_miss) {
			key_count++;
			keys.push_back(i);
			tonic_cur2 = i;
		}
	}
	// Create keys string for log
	for (int x = 0; x < keys.size(); x++) {
		if (!kst.IsEmpty()) kst += " ";
		kst += NoteName[keys[x]];
	}
	// Check if only one key
	if (key_count == 1) {
		if (acc[0][c_len - 1] % 12 == tonic_cur2) {
			ext_st.Format("Single key %s selected as last lower note", NoteName[tonic_cur2]);
			return 500 - key_count;
		}
		if (av_cnt > 1 && acc[1][c_len - 1] % 12 == tonic_cur2) {
			ext_st.Format("Single key %s selected as last higher note", NoteName[tonic_cur2]);
			return 450 - key_count;
		}
		else if (acc[0][0] % 12 == tonic_cur2) {
			ext_st.Format("Single key %s selected as first lower note", NoteName[tonic_cur2]);
			return 400 - key_count;
		}
		else if (av_cnt > 1 && acc[1][0] % 12 == tonic_cur2) {
			ext_st.Format("Single key %s selected as first higher note", NoteName[tonic_cur2]);
			return 350 - key_count;
		}
		else {
			ext_st.Format("Single key %s selected", NoteName[tonic_cur2]);
			return 300 - key_count;
		}
	}
	// If multiple keys and random_key set
	else if (random_key) {
		tonic_cur2 = keys[randbw(0, keys.size() - 1)];
		ext_st.Format("Ambiguous %zu keys (%s) resolved to %s (random)", keys.size(), kst, NoteName[tonic_cur2]);
		return 100 - keys.size();
	}
	// If multiple keys and random_key not set
	else {
		// Find accepted tonic same as last note
		for (int i = 0; i < keys.size(); i++) {
			if (acc[0][c_len - 1] % 12 == keys[i]) {
				tonic_cur2 = keys[i];
				ext_st.Format("Ambiguous %zu keys (%s) resolved to %s as last lower note", keys.size(), kst, NoteName[tonic_cur2]);
				return 500 - keys.size();
			}
		}
		// Find accepted tonic same as last note
		if (av_cnt > 1) for (int i = 0; i < keys.size(); i++) {
			if (acc[1][c_len - 1] % 12 == keys[i]) {
				tonic_cur2 = keys[i];
				ext_st.Format("Ambiguous %zu keys (%s) resolved to %s as last higher note", keys.size(), kst, NoteName[tonic_cur2]);
				return 400 - keys.size();
			}
		}
		// Find accepted tonic same as first note
		for (int i = 0; i < keys.size(); i++) {
			if (acc[0][0] % 12 == keys[i]) {
				tonic_cur2 = keys[i];
				ext_st.Format("Ambiguous %zu keys (%s) resolved to %s as first lower note", keys.size(), kst, NoteName[tonic_cur2]);
				return 300 - keys.size();
			}
		}
		// Find accepted tonic same as first note
		if (av_cnt > 1) for (int i = 0; i < keys.size(); i++) {
			if (acc[1][0] % 12 == keys[i]) {
				tonic_cur2 = keys[i];
				ext_st.Format("Ambiguous %zu keys (%s) resolved to %s as first higher note", keys.size(), kst, NoteName[tonic_cur2]);
				return 300 - keys.size();
			}
		}
		// If nothing found, return random of accepted
		tonic_cur2 = keys[randbw(0, keys.size() - 1)];
		ext_st.Format("Ambiguous %zu keys (%s) resolved to %s (random)", keys.size(), kst, NoteName[tonic_cur2]);
		return 100 - keys.size();
	}
}

void CGenCA1::CreateScanMatrix(int i) {
	int found = 0;
	smatrix.resize(c_len);
	// Search each note
	for (int x = 0; x < c_len; x++) {
		// Search each flag
		for (int f = 0; f < anflags[cpv][x].size(); f++) {
			// Find prohibited flag
			if (accept[anflags[cpv][x][f]] == 0) {
				++found;
				break;
			}
		}
		if (found) break;
	}
	if (found) {
		fill(smatrix.begin(), smatrix.end(), 1);
		smatrixc = c_len;
	}
	else {
		fill(smatrix.begin(), smatrix.end(), 0);
		smatrixc = 0;
		WriteLog(0, "No need to correct - no prohibited rule violations found");
	}
}

void CGenCA1::SendCorrections(int i, long long time_start) {
	CString st, st2;
	// Count penalty
	long cnum = clib.size();
	CheckClibSize();
	// Find minimum penalty
	int ccount = 0;
	// Cycle through all best matches
	st2.Empty();
	for (int m = 0; m < corrections; ++m) {
		// Find minimum penalty
		cids.clear();
		int dpenalty_min2 = MAX_PENALTY;
		for (int x = 0; x < cnum; x++) if (dpenalty[x] < dpenalty_min2) dpenalty_min2 = dpenalty[x];
		// Get all best corrections
		for (int x = 0; x < cnum; x++) if (dpenalty[x] == dpenalty_min2) {
			cids.push_back(x);
		}
		if (!cids.size() || dpenalty_min2 == MAX_PENALTY) break;
		// Shuffle cids
		long long seed = CGLib::time();
		::shuffle(cids.begin(), cids.end(), default_random_engine(seed));
		for (int x = 0; x < cids.size(); x++) {
			ccount++;
			if (ccount > corrections) break;
			// Write log
			st.Format("%.0f/%d/%.0f/%zu/%zu/%d:%d ", rpenalty_min, dpenalty_min, 
				rpenalty_source, cids.size(), clib.size(), swa_full, scan_full);
			st2 += st;
			// Show initial melody again if this is not first iteration
			if (ccount > 1) {
				dpenalty_cur = 0; 
				ScanCantus(tEval, 0, &(cantus[i]));
				step = step0;
			}
			// Get cantus
			m_cc = clib[cids[x]];
			dpenalty_cur = dpenalty[cids[x]]; //-V519
			// Clear penalty
			dpenalty[cids[x]] = MAX_PENALTY;
			// Show result
			ScanCantus(tEval, 1, &(m_cc));
			++cor_sent;
			dp_sum += dpenalty_cur;
			rp_sum += rpenalty_min;
			swa_sum += swa_len;
			if (scan_full && (method != mSWA || swa_full)) ++cor_full;
			if (!rpenalty_min) ++cor_rp0;
			EmulateSAS();
			//LogCantus("scor", cantus_id + 1, m_cc.size(), m_cc);
			// Go back
			step = step0;
			if (step < 0) break;
			// Add lining
			int pos = step;
			for (int z = 0; z < c_len; z++) {
				if (cantus[i][z] != clib[cids[x]][z]) {
					for (int g = 0; g < cc_len[z]; g++) {
						//lining[pos + g][0] = HatchStyleLightUpwardDiagonal;
					}
				}
				pos += cc_len[z];
			}
			// Go forward
			step = t_generated;
			Adapt(step0, step - 1);
			t_generated = step;
			t_sent = t_generated;
		}
	}
	long long time_stop = CGLib::time();
	// Send log
	cor_log.Format("Sent corrections #%d in %lld ms to %d:%d with rp/dp/srp/variants/lib/full: %s", 
		cantus_id + 1, time_stop - time_start, step0/8 + 1, step0%8 + 1, st2);
	WriteLog(3, cor_log);
}

void CGenCA1::ParseExpect() {
	// Obsolete function
	ASSERT(0);
	int fl;
	// Continue if there are lyrics
	vector<CString> ast;
	// Clear expected flags
	enflags.clear();
	enflags2.clear();
	enflags3.clear();
	enflags_count = 0;
	// Detect maximum lyrics
	int max_i = cantus_incom[cantus_id].size();
	if (!max_i) return;
	enflags.resize(max_i);
	enflags2.resize(MAX_RULES);
	enflags3.resize(MAX_RULES);
	for (int f = 0; f < MAX_RULES; ++f) enflags2[f].resize(c_len);
	// Load expected flags
	for (int i = 0; i < max_i; ++i) {
		if (!cantus_incom[cantus_id][i].IsEmpty()) {
			Tokenize(cantus_incom[cantus_id][i], ast, ", ");
			for (int n = 0; n < ast.size(); ++n) {
				fl = atoi(ast[n]);
				if (fl) {
					enflags[i].push_back(fl);
					++enflags2[fl][i];
					++enflags3[fl];
					++enflags_count;
				}
			}
		}
	}
}

void CGenCA1::SelectExpect() {
	CString est;
	int fl;
	est = edb.Open("db/expect.csv");
	if (est != "") WriteLog(5, est);
	edb.filter["File"] = midi_file;
	est = edb.Select();
	if (est != "") WriteLog(5, est);
	if (m_testing != 2) {
		if (cantus_incom.size() && cantus_incom[0].size() && cantus_incom[0][0].Find("expect") != -1) {
			if (!edb.result.size())
				WriteLog(5, "This midi file '" + midi_file + "' requires expected flags, but none found in database");
		}
		else {
			if (edb.result.size())
				WriteLog(5, "This midi file does not require expected flags, but they were found in database");
		}
	}
}

void CGenCA1::LoadExpect() {
	// Clear expected flags
	enflags.clear();
	ef_log.clear();
	ef_name.clear();
	ef_subname.clear();
	enflags2.clear();
	enflags3.clear();
	enflags_count = 0;
	// Detect maximum lyrics
	enflags.resize(c_len);
	ef_log.resize(c_len);
	ef_name.resize(c_len);
	ef_subname.resize(c_len);
	enflags2.resize(MAX_RULES);
	enflags3.resize(MAX_RULES);
	for (int f = 0; f < MAX_RULES; ++f) enflags2[f].resize(c_len);
	// Load expected flags
	for (int i = 0; i < edb.result.size(); ++i) {
		if (atoi(edb.result[i]["Cid"]) != cantus_id) continue;
		int fl = atoi(edb.result[i]["Rid"]);
		s = atoi(edb.result[i]["Step"]);
		if (s >= c_len) {
			CString est;
			est.Format("Melody #%d length in database does not match real length (%d)",
				cantus_id + 1, c_len);
			WriteLog(5, est);
			return;
		}
		if (fl) {
			enflags[s].push_back(fl);
			ef_name[s].push_back(edb.result[i]["Rule"]);
			ef_subname[s].push_back(edb.result[i]["Subrule"]);
			ef_log[s].push_back(edb.result[i]["Log"]);
			++enflags2[fl][s];
			++enflags3[fl];
			++enflags_count;
		}
	}
}

void CGenCA1::ExportExpectToDb() {
	CString est;
	int max_x = enflags.size();
	int fl;
	// Prepare to insert expected flags
	CCsvDb cdb;
	map <CString, CString> row;
	est = cdb.Open("db/expect.csv");
	if (est != "") WriteLog(5, est);
	cdb.filter["File"] = midi_file;
	cdb.filter["Cid"].Format("%d", cantus_id);
	est = cdb.Delete();
	if (est != "") WriteLog(5, est);
	for (int x = 0; x < max_x; ++x) if (enflags[x].size()) {
		for (int e = 0; e < enflags[x].size(); ++e) {
			fl = enflags[x][e];
			row["File"] = midi_file;
			row["Cid"].Format("%d", cantus_id);
			row["Step"].Format("%d", x);
			row["Rid"].Format("%d", fl);
			row["Rule"] = RuleName[cspecies][fl];
			row["Subrule"] = SubRuleName[cspecies][fl];
			row["Species"].Format("%d", cspecies);
			row["High"] = cantus_high ? "High" : "Low";
			row["Group"] = RuleGroup[fl];
			row["Comment"] = RuleComment[cspecies][fl];
			row["Subcomment"] = SubRuleComment[cspecies][fl];
			row["Accept"].Format("%d", accepts[cspecies][fl]);
			row["Severity"].Format("%d", severities[cspecies][fl]);
			row["GFP"].Format("%d", false_positives_global[fl]);
			row["IFP"].Format("%d", false_positives_ignore[fl]);
			row["Log"] = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M ") + "Import from midi lyrics";
			est = cdb.Insert(row);
			if (est != "") WriteLog(5, est);
		}
	}
}

void CGenCA1::CreateExpectDb(CString path, CCsvDb &cdb) {
	CString est;
	vector <CString> hdr;
	hdr.push_back("File");
	hdr.push_back("Cid");
	hdr.push_back("Step");
	hdr.push_back("Rid");
	hdr.push_back("Rule");
	hdr.push_back("Subrule");
	hdr.push_back("Species");
	hdr.push_back("High");
	hdr.push_back("Group");
	hdr.push_back("Comment");
	hdr.push_back("Subcomment");
	hdr.push_back("Accept");
	hdr.push_back("Severity");
	hdr.push_back("GFP");
	hdr.push_back("IFP");
	hdr.push_back("Log");
	est = cdb.Create(path, hdr);
	if (est != "") WriteLog(5, est);
}

void CGenCA1::ConfirmExpect() {
	CString st;
	map <CString, CString> row;
	vector <vector<short>> flag_exported;
	flag_exported.resize(MAX_RULES, vector<short>(c_len));
	int found, fl;
	int max_s = enflags.size();
	if (!enflags_count) return;
	for (s = 0; s < max_s; ++s) if (enflags[s].size()) {
		for (int e = 0; e < enflags[s].size(); ++e) {
			fl = enflags[s][e];
			// Do not confirm rule violation if rule checking is disabled
			//if (accept[fl] == -1) continue;
			found = 0;
			// Check if expected flag fires
			for (int f = 0; f < anflags[cpv][s].size(); ++f) {
				if (anflags[cpv][s][f] == fl) {
					found = 1;
					break;
				}
			}
			if (!found) {
				CString est;
				est.Format("Expected flag not confirmed: [%d] %s %s (%s) at %d:%d (beat %d:%d) %s",
					fl, accept[fl] ? "+" : "-", ef_name[s][e], ef_subname[s][e], 
					cantus_id + 1, s + 1, cpos[s] / 8 + 1, cpos[s] % 8 + 1, midi_file); // RuleName[cspecies][fl] SubRuleName[cspecies][fl]
				WriteLog(5, est);
				// Test log
				if (m_testing == 1) AppendLineToFile("autotest\\expect.log", est + "\n");
				// Send to LY
				if (ly_debugexpect) {
					nlink[cpos[s]][cpv][fl * 10 + cspecies] = 0;
					fsev[cpos[s]][cpv][fl * 10 + cspecies] = 100;
				}
			}
			else if (debug_level > 0) {
				CString est;
				est.Format("Expected flag confirmed: [%d] %s %s (%s) at %d:%d (beat %d:%d) %s",
					fl, accept[fl] ? "+" : "-", RuleName[cspecies][fl], SubRuleName[cspecies][fl], 
					cantus_id + 1, s + 1, cpos[s] / 8 + 1, cpos[s] % 8 + 1, midi_file);
				WriteLog(6, est); 
				// Test log
				if (m_testing == 1) AppendLineToFile("autotest\\expect.log", est + "\n");
				if (!flag_exported[fl][s]) {
					++flag_exported[fl][s];
					row["File"] = midi_file;
					row["Cid"].Format("%d", cantus_id);
					row["Step"].Format("%d", s);
					row["Rid"].Format("%d", fl);
					row["Rule"] = RuleName[cspecies][fl];
					row["Subrule"] = SubRuleName[cspecies][fl];
					row["Species"].Format("%d", cspecies);
					row["High"] = cantus_high ? "High" : "Low";
					row["Group"] = RuleGroup[fl];
					row["Comment"] = RuleComment[cspecies][fl];
					row["Subcomment"] = SubRuleComment[cspecies][fl];
					row["Accept"].Format("%d", accepts[cspecies][fl]);
					row["Severity"].Format("%d", severities[cspecies][fl]);
					row["GFP"].Format("%d", false_positives_global[fl]);
					row["IFP"].Format("%d", false_positives_ignore[fl]);
					row["Log"] = ef_log[s][e];
					est = edb2.Insert(row);
					if (est != "") WriteLog(5, est);
				}
			}
		}
	}
	if (confirm_mode == 1) {
		// Check global false positives
		for (s = 0; s < c_len; ++s) {
			for (int f = 0; f < anflags[cpv][s].size(); ++f) {
				fl = anflags[cpv][s][f];
				if (!enflags2[fl][s] && false_positives_global[fl]) {
					CString est;
					est.Format("Global false positive flag: [%d] %s %s (%s) at %d:%d (beat %d:%d) %s",
						fl, accept[fl] ? "+" : "-", RuleName[cspecies][fl], SubRuleName[cspecies][fl],
						cantus_id + 1, s + 1, cpos[s] / 8 + 1, cpos[s] % 8 + 1, midi_file);
					WriteLog(5, est);
					if (m_testing == 1) AppendLineToFile("autotest\\expect.log", est + "\n");
					if (ly_debugexpect) {
						nlink[cpos[s]][cpv][fl * 10 + cspecies] = anfl[cpv][s][f] - s;
						fsev[cpos[s]][cpv][fl * 10 + cspecies] = 0;
					}
					// Collect global false positives statistics
					//if (m_testing == 1) AppendLineInFile("autotest\\global_false.txt", fl, " 0");
					// Send to corrected CSV database
					if (!flag_exported[fl][s]) {
						++flag_exported[fl][s];
						row["File"] = midi_file;
						row["Cid"].Format("%d", cantus_id);
						row["Step"].Format("%d", s);
						row["Rid"].Format("%d", fl);
						row["Rule"] = RuleName[cspecies][fl];
						row["Subrule"] = SubRuleName[cspecies][fl];
						row["Species"].Format("%d", cspecies);
						row["High"] = cantus_high ? "High" : "Low";
						row["Group"] = RuleGroup[fl];
						row["Comment"] = RuleComment[cspecies][fl];
						row["Subcomment"] = SubRuleComment[cspecies][fl];
						row["Accept"].Format("%d", accepts[cspecies][fl]);
						row["Severity"].Format("%d", severities[cspecies][fl]);
						row["GFP"].Format("%d", false_positives_global[fl]);
						row["IFP"].Format("%d", false_positives_ignore[fl]);
						row["Log"] = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M Global false positive");
						est = edb2.Insert(row);
						if (est != "") WriteLog(5, est);
					}
				}
			}
		}
		// Do not check local false positives if disabled
		for (int fl = 0; fl < MAX_RULES; ++fl) {
			if (!enflags3[fl] || false_positives_ignore[fl]) continue;
			for (s = 0; s < c_len; ++s) {
				for (int f = 0; f < anflags[cpv][s].size(); ++f) if (fl == anflags[cpv][s][f]) {
					if (!enflags2[fl][s]) {
						if (!flag_exported[fl][s]) {
							CString est;
							est.Format("Local false positive flag: [%d] %s %s (%s) at %d:%d (beat %d:%d) %s",
								fl, accept[fl] ? "+" : "-", RuleName[cspecies][fl], SubRuleName[cspecies][fl],
								cantus_id + 1, s + 1, cpos[s] / 8 + 1, cpos[s] % 8 + 1, midi_file);
							WriteLog(5, est);
							// Send to LY
							if (ly_debugexpect) {
								nlink[cpos[s]][cpv][fl * 10 + cspecies] = anfl[cpv][s][f] - s;
								fsev[cpos[s]][cpv][fl * 10 + cspecies] = 0;
							}
							// Test log
							if (m_testing == 1) AppendLineToFile("autotest\\expect.log", est + "\n");
							// Send to corrected CSV database
							++flag_exported[fl][s];
							row["File"] = midi_file;
							row["Cid"].Format("%d", cantus_id);
							row["Step"].Format("%d", s);
							row["Rid"].Format("%d", fl);
							row["Rule"] = RuleName[cspecies][fl];
							row["Subrule"] = SubRuleName[cspecies][fl];
							row["Species"].Format("%d", cspecies);
							row["High"] = cantus_high ? "High" : "Low";
							row["Group"] = RuleGroup[fl];
							row["Comment"] = RuleComment[cspecies][fl];
							row["Subcomment"] = SubRuleComment[cspecies][fl];
							row["Accept"].Format("%d", accepts[cspecies][fl]);
							row["Severity"].Format("%d", severities[cspecies][fl]);
							row["GFP"].Format("%d", false_positives_global[fl]);
							row["IFP"].Format("%d", false_positives_ignore[fl]);
							row["Log"] = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M Local false positive");
							est = edb2.Insert(row);
							if (est != "") WriteLog(5, est);
						}
					}
				}
			}
		}
	}
	if (confirm_mode == 2) {
		for (s = 0; s < c_len; ++s) {
			for (int f = 0; f < anflags[cpv][s].size(); ++f) {
				fl = anflags[cpv][s][f];
				// Do not check ignored or accepted flags
				if (false_positives_ignore[fl] || accept[fl]) continue;
				if (!enflags2[fl][s]) {
					CString est;
					est.Format("False positive mistake: [%d] %s %s (%s) at %d:%d (beat %d:%d) %s",
						fl, accept[fl] ? "+" : "-", RuleName[cspecies][fl], SubRuleName[cspecies][fl], //-V547
						cantus_id + 1, s + 1, cpos[s] / 8 + 1, cpos[s] % 8 + 1, midi_file);
					WriteLog(1, est);
					if (m_testing == 1) AppendLineToFile("autotest\\mistakes.log", est + "\n");
					// Show in note comment
					for (int v = 0; v < av_cnt; ++v) {
						for (int x = 0; x < comment[cpos[s]][v].size(); ++x) {
							st.Format("[%d/", fl);
							if (comment[cpos[s]][v][x].Find(st) != -1) {
								if (comment[cpos[s]][v][x][0] == '-') comment[cpos[s]][v][x].SetAt(0, 'x');
							}
						}
					}
				}
				else {
					// Confirm in note comment
					for (int v = 0; v < av_cnt; ++v) {
						for (int x = 0; x < comment[cpos[s]][v].size(); ++x) {
							st.Format("[%d/", fl);
							if (comment[cpos[s]][v][x].Find(st) != -1) {
								if (comment[cpos[s]][v][x][0] == '-') comment[cpos[s]][v][x].SetAt(0, 'o');
							}
						}
					}
				}
			}
		}
	}
}

void CGenCA1::InitCorAck() {
	if (!cor_ack) return;
	cor_ack_dp.clear();
	cor_ack_rp.clear();
	cor_ack_st.clear();
	cor_log = "";
	method = mSWA;
}

void CGenCA1::SaveCorAck(CString st) {
	if (!cor_ack) return;
	// Do not check if scan was aborted
	if (!scan_full) return;
	// Do not check if scan was aborted
	if (method == mSWA && !swa_full) return;
	cor_ack_dp.push_back(dpenalty_min);
	cor_ack_rp.push_back(rpenalty_min);
	cor_ack_st.push_back(st + " " + cor_log);
}

void CGenCA1::CorAck() {
	CString est;
	if (!cor_ack || cor_ack_dp.size() < 2 || cor_ack_rp.size() < 2 || cor_ack_st.size() < 2) return;
	if (cor_ack_dp[0] == cor_ack_dp[1] && cor_ack_rp[0] == cor_ack_rp[1]) {
		est.Format("+ Correction acknowledged in %s: %s %s", 
			midi_file, cor_ack_st[0], cor_ack_st[1]);
		WriteLog(8, est);
		if (m_testing == 1) AppendLineToFile("autotest\\cor-ack.log", est + "\n");
		return;
	}
	est.Format("- Correction not acknowledged in %s: %s %s", 
		midi_file, cor_ack_st[0], cor_ack_st[1]);
	WriteLog(5, est);
	if (m_testing == 1) AppendLineToFile("autotest\\cor-ack.log", est + "\n");
}

void CGenCA1::LogFlags() {
	if (m_testing != 1) return;
	CString st, fst;
	int fl;
	st.Format("+ Finished analysis of %s #%d",
		midi_file, cantus_id + 1);
	AppendLineToFile("autotest\\analysis.log", st + "\n");
	for (s = 0; s < ep2; ++s) {
		// Loop through all flags 
		for (int f = 0; f < anflags[cpv][s].size(); ++f) {
			fl = anflags[cpv][s][f];
			fst.Empty();
			if (accept[fl] == 0) fst = "-";
			if (accept[fl] == 1) fst = "+";
			if (accept[fl] == -1) fst = "$";
			st.Format("Detected flag: [%d] %s %s (%s) at %d:%d %s",
				fl, fst, RuleName[cspecies][fl], SubRuleName[cspecies][fl],
				cantus_id + 1, s + 1, midi_file);
			AppendLineToFile("autotest\\flags.log", st + "\n");
			//if (fl == 98) WriteLog(1, st);
		}
	}
}

void CGenCA1::Generate() {
	CString test_st = "60 72 71 69 67 71 67 69 71 67 65 64 62 59 60";
	test_cc.resize(15);
	StringToVector(&test_st, " ", test_cc);

	CString st;
	int s_len2 = s_len;
	if (error) return;
	InitCantus();
	if (midi_file == "") {
		WriteLog(5, "Midi file not specified in configuration file");
		error = 7;
	}
	if (error) return;
	SetStatusText(8, "MIDI file: " + fname_from_path(midi_file));
	LoadCantus(midi_file);
	SelectExpect();
	CreateExpectDb(as_dir + "\\edb-" + as_fname + ".csv", edb2);
	if (cantus.size() < 1) return;
	// Saved t_generated
	int t_generated2 = 0; 
	if (cantus_id2) {
		if (cantus_id2 > cantus.size()) {
			CString est;
			est.Format("Warning: cantus_id in configuration file (%d) is greater than number of canti loaded (%zu). Selecting highest cantus.",
				cantus_id2, cantus.size());
			WriteLog(1, est);
			cantus_id2 = cantus.size() - 1;
		}
	}
	for (cantus_id = cantus_id2 - 1; cantus_id < cantus.size(); cantus_id++) {
		// Check limit
		if (t_generated >= t_cnt) {
			WriteLog(3, "Reached t_cnt steps. Generation stopped");
			break;
		}
		st.Format("Analyzing: %d of %zu", cantus_id+1, cantus.size());
		SetStatusText(3, st);
		if (need_exit) break;
		if (step < 0) step = 0;
		step0 = step;
		step00 = step0;
		long long time_start = CGLib::time();
		// Add line
		linecolor[step] = MakeColor(255, 0, 0, 0);
		// Get key
		acc.resize(1);
		acc[0] = cantus[cantus_id];
		GetCPKey();
		if (tonic_cur == -1) continue;
		CalcCcIncrement();
		// Show imported melody
		cc_len = cantus_len[cantus_id];
		cc_tempo = cantus_tempo[cantus_id];
		real_len = accumulate(cantus_len[cantus_id].begin(), cantus_len[cantus_id].end(), 0);
		dpenalty_cur = 0;
		c_len = cantus[cantus_id].size();
		GetSourceRange(cantus[cantus_id]);
		if (confirm_mode) {
			LoadExpect();
		}
		ScanCantus(tEval, 0, &(cantus[cantus_id]));
		LogFlags();
		if (confirm_mode) {
			ConfirmExpect();
		}
		EmulateSAS();
		key_eval.Empty();
		// Check if cantus was shown
		if (t_generated2 == t_generated) continue;
		t_generated2 = t_generated;
		// Fill pauses if no results generated
		FillPause(step0, step-step0, 1);
		// Count additional variables
		CountOff(step0, step - 1);
		CountTime(step0, step - 1);
		UpdateNoteMinMax(step0, step - 1);
		UpdateTempoMinMax(step0, step - 1);
		CreateScanMatrix(cantus_id);
		// If no corrections needed
		if (!corrections || !smatrixc || 
			(m_testing && time() - gen_start_time > (m_test_sec - ANALYZE_RESERVE) * 1000)) {
			// Go forward
			t_generated = step;
			Adapt(step0, step - 1);
			t_sent = t_generated;
			continue;
		}
		step1 = step;
		step = step0;
		// Init animation
		acycle = 0;
		correct_start_time = CGLib::time();
		// Save source rpenalty
		rpenalty_source = rpenalty_cur;
		InitCorAck();
		if (method == mSWA) {
			clib.clear();
			SWA(cantus_id, 1);
			// Check if we have results
			if (clib.size()) {
				SendCorrections(cantus_id, time_start);
				SaveCorAck("SWA");
			}
			else {
				// Go forward
				step = t_generated;
				Adapt(step0, step - 1);
				t_sent = t_generated;
			}
		}
		if (cor_ack) {
			method = mScan;
			FillPause(step, step - step0, 0);
			test_cc = m_cc;
		}
		if (method == mScan) {
			s_len = s_len2;
			clib.clear();
			rpenalty.clear();
			dpenalty.clear();
			rpenalty_min = 0;
			dpenalty_min = MAX_PENALTY;
			// Full scan marked notes
			ScanCantus(tCor, 0, &(cantus[cantus_id]));
			rpenalty_min = 0;
			// Check if we have results
			if (clib.size()) {
				SendCorrections(cantus_id, time_start);
				SaveCorAck("SAS");
			}
			else {
				// Go forward
				step = t_generated;
				Adapt(step0, step - 1);
				t_sent = t_generated;
			}
		}
		CorAck();
		if (need_exit) break;
	}
	st.Format("Analyzed %d of %zu", cantus_id, cantus.size());
	SetStatusText(3, st);
	ShowStuck();
	LogPerf();
}
