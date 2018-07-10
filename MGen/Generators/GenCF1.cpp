// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "../stdafx.h"
#include "GenCF1.h"

#ifdef _DEBUG
#define new DEBUG_NEW 
#endif

CGenCF1::CGenCF1() {
	// Init vector parameters
	tonic_max.resize(2);
	tonic_window.resize(2);
	notes_lrange.resize(4, vector<int>(MAX_SPECIES, 19));
	progress.resize(MAX_PROGRESS);
	av_cnt = 1;
	v_cnt = 1;
	ngraph_size = 3;
	RegisterGraph("Tonic rating", 2);
	RegisterGraph("Leaps rating", 1);
	RegisterGraph("Leaped rating", 0.2);
	cpv = 0;
	//midifile_tpq_mul = 8;
	accept.resize(MAX_RULES);
	rule_viz.resize(MAX_RULES);
	rule_viz_v2.resize(MAX_RULES);
	rule_viz_int.resize(MAX_RULES);
	rule_viz_t.resize(MAX_RULES);
	false_positives_ignore.resize(MAX_RULES);
	false_positives_global.resize(MAX_RULES);
	sas_emulator_max_delay.resize(MAX_RULES);
	sas_emulator_move_ignore.resize(MAX_RULES);
	sas_emulator_replace.resize(MAX_RULES);
	flag_replace.resize(MAX_RULES);
	sas_emulator_unstable.resize(MAX_RULES);
	flag_delay.resize(MAX_RULES);
	flag_delay_st.resize(MAX_RULES);
	RuleParam.resize(MAX_SPECIES);
	SubRuleName.resize(MAX_SPECIES, vector<CString>(MAX_RULES));
	RuleName.resize(MAX_SPECIES, vector<CString>(MAX_RULES));
	SubRuleComment.resize(MAX_SPECIES, vector<CString>(MAX_RULES));
	RuleComment.resize(MAX_SPECIES, vector<CString>(MAX_RULES));
	severities.resize(MAX_SPECIES, vector<int>(MAX_RULES));
	accepts.resize(MAX_SPECIES, vector<int>(MAX_RULES));
	RuleClass.resize(MAX_RULES);
	RuleGroup.resize(MAX_RULES);
	ssf.resize(MAX_RULES);
	severity.resize(MAX_RULES);
	flag_color.resize(MAX_SEVERITY);
	max_note_len.resize(6);
	// Start severity
	severity[0] = 0;
	// Data ready
	data_ready.resize(MAX_DATA_READY);
	data_ready_persist.resize(MAX_DATA_READY_PERSIST);
	warn_data_ready.resize(MAX_DATA_READY);
	warn_data_ready_persist.resize(MAX_DATA_READY_PERSIST);
	// Harmony notation
	HarmName.resize(7);
	HarmName_m.resize(7);
	HarmName_ma.resize(7);
}

CGenCF1::~CGenCF1()
{
}

// General init sequence for CF1/CP1/CA1/CA2...
int CGenCF1::InitGen() {
	// Set rule colors
	for (int i = 0; i < MAX_SEVERITY; ++i) {
		flag_color[i] = MakeColor(0, 255.0 / MAX_SEVERITY*i, 255 - 255.0 / MAX_SEVERITY*i, 0);
	}
	// Check that method is selected
	if (method == mUndefined) {
		WriteLog(5, "Error: method not specified in algorithm configuration file");
		error = 2;
		return error;
	}
	// Check harmonic meaning loaded
	LoadHarmVar();
	// Check config
	CheckConfig();
	return error;
}

int CGenCF1::InitCantus()
{
	InitGen();
	return error;
}

void CGenCF1::TestDiatonic()
{
	CString st;
	tonic_cur = 2;
	minor_cur = 1;
	for (int i = 0; i < 32; ++i) {
		int d = CC_C(i, tonic_cur, minor_cur);
		int cc = C_CC(d, tonic_cur, minor_cur);
		st.Format("Test diatonic: %d [to d]-> %d [to c]-> %d", i, d, cc);
		WriteLog(0, st);
	}
}

void CGenCF1::CheckSASEmulatorFlags(vector<int> &cc) {
	int fl, fl2, found, delay, good, error_level;
	CString error_st, est;
	if (ep2 == c_len) for (s = 0; s < ep2; ++s) {
		// Loop through all source flags if last emulator run
		for (int f = 0; f < nflags_full[s].size(); ++f) {
			fl = nflags_full[s][f];
			found = 0;
			for (int f2 = 0; f2 < anflags[cpv][s].size(); ++f2) {
				if (anflags[cpv][s][f2] == fl) {
					found = 1;
					break;
				}
			}
			// Stop processing if this flag is found
			if (found) continue;
			// Not found in same position: can it be replaced?
			if (flag_replace[fl].size()) {
				for (int i = 0; i < flag_replace[fl].size(); ++i) {
					fl2 = flag_replace[fl][i];
					for (int f2 = 0; f2 < anflags[cpv][s].size(); ++f2) if (anflags[cpv][s][f2] == fl2) {
						found = 1;
						break;
					}
				}
			}
			if (found) {
				est.Format("+ Second scan at step %d replaced flag [%d] with: [%d] %s %s (%s) at %d:%d (beat %d:%d) %s",
					ep2, fl2, fl, accept[fl] ? "+" : "-", RuleName[cspecies][fl], SubRuleName[cspecies][fl],
					cantus_id + 1, s + 1, cpos[s] / 8 + 1, cpos[s] % 8 + 1, midi_file);
				WriteLog(7, est);
				if (m_testing == 1) AppendLineToFile("autotest\\sas-emu.log", est + "\n");
				continue;
			}
			est.Format("- Flag does not appear on second full evaluation: [%d] %s %s (%s) at %d:%d (beat %d:%d) %s",
				fl, accept[fl] ? "+" : "-", RuleName[cspecies][fl], SubRuleName[cspecies][fl],
				cantus_id + 1, s + 1, cpos[s] / 8 + 1, cpos[s] % 8 + 1, midi_file);
			WriteLog(5, est);
			if (m_testing == 1) AppendLineToFile("autotest\\sas-emu.log", est + "\n");
		}
	}
	for (s = 0; s < ep2; ++s) {
		// Loop through all current flags
		for (int f = 0; f < anflags[cpv][s].size(); ++f) {
			fl = anflags[cpv][s][f];
			// Do not analyse unstable flags
			if (sas_emulator_unstable[fl]) continue;
			// Check that flag exists in previous SAS run if this is not last step and previous run exists
			found = 0;
			good = 0;
			error_st.Empty();
			error_level = 0;
			if (s < ep2 - 1 && nflags_prev.size() > s) {
				for (int f2 = 0; f2 < nflags_prev[s].size(); ++f2) if (nflags_prev[s][f2] == fl) {
					found = 1;
					break;
				}
			}
			// Stop processing if this flag is not new
			if (found) continue;
			// Find longest delay for each new flag
			delay = ep2 - 1 - s;
			if (delay > flag_delay[fl]) {
				CString est;
				est.Format("SAS emulator at step %d has delay %d steps: [%d] %s %s (%s) at %d:%d (beat %d:%d) %s",
					ep2, delay, fl, accept[fl] ? "+" : "-", RuleName[cspecies][fl], SubRuleName[cspecies][fl],
					cantus_id + 1, s + 1, cpos[s] / 8 + 1, cpos[s] % 8 + 1, midi_file);
				//WriteLog(1, est);
				flag_delay[fl] = delay;
				flag_delay_st[fl] = est;
			}
			// Check that flag exists in full analysis in same position
			for (int f2 = 0; f2 < nflags_full[s].size(); ++f2) if (nflags_full[s][f2] == fl) {
				found = 1;
				break;
			}
			// Stop processing if this flag is found
			if (found) continue;
			// Not found in same position: can it be replaced?
			if (sas_emulator_replace[fl].size()) {
				for (int i = 0; i < sas_emulator_replace[fl].size(); ++i) {
					fl2 = sas_emulator_replace[fl][i];
					for (int f2 = 0; f2 < nflags_full[s].size(); ++f2) if (nflags_full[s][f2] == fl2) {
						found = 1;
						break;
					}
				}
			}
			if (!found && flag_replace[fl].size()) {
				for (int i = 0; i < flag_replace[fl].size(); ++i) {
					fl2 = flag_replace[fl][i];
					for (int f2 = 0; f2 < nflags_full[s].size(); ++f2) if (nflags_full[s][f2] == fl2) {
						found = 1;
						break;
					}
				}
			}
			if (found) {
				est.Format("+ SAS emulator at step %d replaced flag [%d] with: [%d] %s %s (%s) at %d:%d (beat %d:%d) %s",
					ep2, fl2, fl, accept[fl] ? "+" : "-", RuleName[cspecies][fl], SubRuleName[cspecies][fl],
					cantus_id + 1, s + 1, cpos[s] / 8 + 1, cpos[s] % 8 + 1, midi_file);
				WriteLog(7, est);
				if (m_testing == 1) AppendLineToFile("autotest\\sas-emu.log", est + "\n");
				continue;
			}
			error_st.Format("- SAS emulator at step %d assigned wrong flag: [%d] %s %s (%s) at %d:%d (beat %d:%d) %s",
				ep2, fl, accept[fl] ? "+" : "-", RuleName[cspecies][fl], SubRuleName[cspecies][fl],
				cantus_id + 1, s + 1, cpos[s] / 8 + 1, cpos[s] % 8 + 1, midi_file);
			error_level = 5;
			// Not found in same position: does it exist in any position?
			if (flags_full[fl]) {
				if (sas_emulator_move_ignore[fl]) {
					est.Format("+ SAS emulator at step %d assigned moved flag: [%d] %s %s (%s) at %d:%d (beat %d:%d) %s",
						ep2, fl, accept[fl] ? "+" : "-", RuleName[cspecies][fl], SubRuleName[cspecies][fl],
						cantus_id + 1, s + 1, cpos[s] / 8 + 1, cpos[s] % 8 + 1, midi_file);
					WriteLog(7, est);
					if (m_testing == 1) AppendLineToFile("autotest\\sas-emu.log", est + "\n");
					continue;
				}
				else {
					error_st.Format("- SAS emulator at step %d assigned moved flag: [%d] %s %s (%s) at %d:%d (beat %d:%d) %s",
						ep2, fl, accept[fl] ? "+" : "-", RuleName[cspecies][fl], SubRuleName[cspecies][fl],
						cantus_id + 1, s + 1, cpos[s] / 8 + 1, cpos[s] % 8 + 1, midi_file);
					error_level = 5;
				}
			}
			// Not found in any position: can it be replaced in any position?
			if (sas_emulator_replace[fl].size() && sas_emulator_move_ignore[fl]) {
				for (int i = 0; i < sas_emulator_replace[fl].size(); ++i) {
					fl2 = sas_emulator_replace[fl][i];
					if (flags_full[fl2]) {
						found = 1;
						break;
					}
				}
				if (!found && flag_replace[fl].size()) {
					for (int i = 0; i < flag_replace[fl].size(); ++i) {
						fl2 = flag_replace[fl][i];
						if (flags_full[fl2]) {
							found = 1;
							break;
						}
					}
				}
				if (found) {
					CString est;
					est.Format("+ SAS emulator at step %d replaced and moved flag %d with: [%d] %s %s (%s) at %d:%d (beat %d:%d) %s",
						ep2, fl2, fl, accept[fl] ? "+" : "-", RuleName[cspecies][fl], SubRuleName[cspecies][fl],
						cantus_id + 1, s + 1, cpos[s] / 8 + 1, cpos[s] % 8 + 1, midi_file);
					WriteLog(7, est);
					if (m_testing == 1) AppendLineToFile("autotest\\sas-emu.log", est + "\n");
					continue;
				}
			}
			// Show error
			error_st += " [" + vint2st(c_len, cc) + "]";
			error_st += " {" + vint2st(nflags_full[s].size(), nflags_full[s]) + "}";
			WriteLog(error_level, error_st);
			if (m_testing == 1) AppendLineToFile("autotest\\sas-emu.log", error_st + "\n");
		}
	}
}

void CGenCF1::EmulateSAS() {
	if (v_cnt == 1) {
		// Evaluate for CF1
		ScanCantus(tEval, -1, &(m_cc));
	}
	// Save full analysis flags
	nflags_full = anflags[cpv];
	flags_full = flags;
	nflags_prev.clear();
	for (fixed_ep2 = 1; fixed_ep2 <= m_cc.size(); ++fixed_ep2) {
		// Show emulator status
		CString est;
		est.Format("SAS emulator: %d of %zu", fixed_ep2, m_cc.size());
		SetStatusText(7, est);
		// Visible emulation
		if (emulate_sas) {
			step0 = step;
			FillPause(step0, floor((real_len + 1) / 8 + 1) * 8, 0);
			if (v_cnt > 1) FillPause(step0, floor((real_len + 1) / 8 + 1) * 8, 1);
			ScanCantus(tEval, 0, &(m_cc));
		}
		// Hidden emulation
		else {
			ScanCantus(tEval, -1, &(m_cc));
		}
		if (need_exit) break;
		CheckSASEmulatorFlags(m_cc);
		nflags_prev = anflags[cpv];
	}
	if (!need_exit) OutputFlagDelays();
	fixed_ep2 = 0;
	SetStatusText(7, "SAS emulator: finished");
}

// Create random cantus and optimize it using SWA
void CGenCF1::RandomSWA()
{
	// Init animation
	acycle = 0;
	CString st;
	// Unique checker
	VSet<int> vs; 
	// Disable debug flags
	calculate_blocking = 0;
	calculate_correlation = 0;
	calculate_stat = 0;
	// Create single cantus
	cantus.resize(1);
	cantus[0].resize(c_len);
	scantus = &(cantus[0]);
	ScanCantusInit();
	// Set random_seed to initiate random cantus
	random_seed = 1;
	// Set random_range to limit scanning to one of possible fast-scan ranges
	random_range = 1;
	// Prohibit limits recalculation during SWA
	swa_inrange = 1;
	for (int i = 0; i < INT_MAX; ++i) {
		if (need_exit) break;
		// Load first note, because it was overwritten by random generator
		first_note = first_note0;
		last_note = last_note0;
		// Create random cantus
		task = tGen;
		MakeNewCantus(m_c, m_cc);
		EmulateSAS();
		task = tCor;
		min_cc0 = min_cc;
		max_cc0 = max_cc;
		cantus[0] = m_cc;
		// Set scan matrix to scan all
		smatrixc = c_len;
		smatrix.resize(c_len);
		for (int x = 0; x < c_len; ++x) {
			smatrix[x] = 1;
		}
		// Optimize cantus
		rpenalty_cur = MAX_PENALTY;
		correct_start_time = time();
		SWA(0, 0);
		// Show cantus if it is perfect
		if (rpenalty_min <= rpenalty_accepted) {
			if (vs.Insert(m_cc)) {
				int step = t_generated;
				// Add line
				linecolor[t_generated] = MakeColor(255, 0, 0, 0);
				ScanCantus(tEval, 0, &(m_cc));
				if (rpenalty_cur > rpenalty_accepted) {
					st.Format("Error calculating rpenalty %f min %f at step %d", 
						rpenalty_cur, rpenalty_min, t_generated);
					WriteLog(5, st);
					TestRpenalty();
				}
				Adapt(step, t_generated - 1);
				t_sent = t_generated;
				EmulateSAS();
			}
			else {
				++cantus_ignored;
			}
		}
		st.Format("Random SWA: %d", i);
		SetStatusText(3, st);
		st.Format("Sent: %ld (ignored %ld)", cantus_sent, cantus_ignored);
		SetStatusText(0, st);
		//SendCantus(0, 0);
		// Check limit
		if (t_generated >= t_cnt) {
			return;
		}
	}
	ShowStuck();
}

// Do not calculate dpenalty (dp = 0). Calculate dpenalty (dp = 1).
void CGenCF1::SWA(int i, int dp) {
	CString st;
	long long time_start = CGLib::time();
	swa_len = 1;
	// Save source rpenalty
	rpenalty_source = rpenalty_cur;
	long cnum = 0;
	// Save cantus only if its penalty is less or equal to source rpenalty
	rpenalty_min = rpenalty_cur;
	dpenalty_min = 0;
	m_cc = cantus[i];
	swa_full = 0;
	best_flags.clear();
	int a;
	for (a = 0; a < approximations; a++) {
		// Save previous minimum penalty
		int rpenalty_min_old = rpenalty_min;
		int dpenalty_min_old = dpenalty_min;
		// Clear before scan
		clib.clear();
		clib_vs.clear();
		rpenalty.clear();
		dpenalty.clear();
		// Add current cantus if this is not first run
		if (a > 0) {
			clib.push_back(m_cc);
			clib_vs.Insert(m_cc);
			rpenalty.push_back(rpenalty_min_old);
			dpenalty.push_back(dpenalty_min_old);
		}
		// Sliding Windows Approximation
		ScanCantus(tCor, 0, &m_cc);
		CheckClibSize();
		cnum = clib.size();
		if (cnum) {
			// Get all best corrections
			cids.clear();
			for (int x = 0; x < cnum; x++) if (rpenalty[x] <= rpenalty_min && (!dp || dpenalty[x] == dpenalty_min)) {
				cids.push_back(x);
			}
			if (cids.size()) {
				// Get random cid
				int cid = randbw(0, cids.size() - 1);
				// Get random cantus to continue
				m_cc = clib[cids[cid]];
			}
		}
		// Send log
		if (debug_level > 1) {
			CString est;
			est.Format("SWA%d #%d: rp %.0f from %.0f, dp %d, cnum %ld", swa_len, a, rpenalty_min, rpenalty_source, dpenalty_min, cnum);
			WriteLog(3, est);
		}
		if (m_cc.size() > 60 || swa_len > 0) {
			st.Format("SWA%d attempt: %d", swa_len, a);
			SetStatusText(4, st);
		}
		// Animation
		long long time = CGLib::time();
		// Limit correction time
		if (max_correct_ms && time - gen_start_time > max_correct_ms) break;
		if (dp) {
			// Abort SWA if dpenalty and rpenalty not decreasing
			if (rpenalty_min >= rpenalty_min_old && dpenalty_min >= dpenalty_min_old) {
				if (swa_len >= swa_steps || swa_len >= smap.size()) {
					swa_full = 1;
					if (swa_len >= smap.size()) swa_full = 2;
					break;
				}
				else ++swa_len;
			}
		}
		else {
			// Abort SWA if rpenalty zero or not decreasing
			if (!rpenalty_min) break;
			if (rpenalty_min >= rpenalty_min_old) {
				if (swa_len >= swa_steps || swa_len >= smap.size()) {
					// Record SWA stuck flags
					if (best_flags.size()) for (int x = 0; x < max_flags; ++x) {
						if (best_flags[x]) ++ssf[x];
					}
					swa_full = 1;
					break;
				}
				else ++swa_len;
			}
		}
	}
	// Log
	long long time_stop = CGLib::time();
	CString est;
	// For successful rpenalty_cur == 0, show last flag that was fixed. For unsuccessful, show best variant
	CString stuck_st = GetStuck();
	est.Format("Finished SWA%d #%d: rp %.0f from %.0f, dp %d, cnum %ld (in %lld ms): %s", 
		swa_len, a, rpenalty_min, rpenalty_source, dpenalty_min, cnum, time_stop - time_start, stuck_st);
	WriteLog(0, est);
	TestBestRpenalty();
}


void CGenCF1::Generate()
{
	CString test_st = "72 67 69 71 69 65 67 64 62 60";
	test_cc.resize(10);
	StringToVector(&test_st, " ", test_cc);

	first_note0 = first_note;
	last_note0 = last_note;
	if (error) return;
	// Voice
	int v = 0;
	//TestDiatonic();
	// If error, return;
	if (InitCantus()) return;
	CalcCcIncrement();
	// Set uniform length of each cantus note
	cc_len.resize(c_len);
	cc_tempo.resize(c_len);
	real_len = c_len;
	for (int i = 0; i < c_len; ++i) cc_len[i] = 1;
	if (method == mSWA) {
		RandomSWA();
	}
	else {
		ScanCantus(tGen, 0, 0);
	}
	// Random shuffle
	if (shuffle) {
		vector<unsigned short> ci(accepted); // cantus indexes
		vector<unsigned char> note2(t_generated);
		vector<vector<CString>> comment3(t_generated);
		vector<CString> comment4(t_generated);
		vector<DWORD> color2(t_generated);
		for (int i = 0; i < accepted; ++i) ci[i] = i;
		// Shuffled indexes
		long long seed = CGLib::time();
		::shuffle(ci.begin(), ci.end(), default_random_engine(seed));
		// Swap
		int s1, s2;
		for (int i = 0; i < accepted; ++i) {
			for (int x = 0; x < c_len; ++x) {
				s1 = i*(c_len + 1) + x;
				s2 = ci[i]*(c_len + 1) + x;
				note2[s1] = note[s2][v];
				comment3[s1] = comment[s2][v];
				comment4[s1] = comment2[s2][v];
				color2[s1] = color[s2][v];
			}
		}
		// Replace
		for (int i = 0; i < accepted; ++i) {
			for (int x = 0; x < c_len; ++x) {
				s1 = i*(c_len + 1) + x;
				note[s1][v] = note2[s1];
				comment[s1][v] = comment3[s1];
				comment2[s1][v] = comment4[s1];
				color[s1][v] = color2[s1];
			}
		}
		// Adapt
		Adapt(0, t_generated-1);
		// Send
		t_sent = t_generated;
		::PostMessage(m_hWnd, WM_GEN_FINISH, 2, 0);
		CString est;
		est.Format("Shuffle of %lld melodies finished", accepted);
		WriteLog(3, est);
	}
	LogPerf();
}

void CGenCF1::LogPerf() {
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

